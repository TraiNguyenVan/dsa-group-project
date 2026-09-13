#!/usr/bin/env node
/**
 * Export report/report.md -> report.pdf.
 *
 * Pipeline: math-first Markdown -> HTML (marked GFM + KaTeX + highlight.js),
 * then headless Chromium prints the HTML to PDF. A real browser engine
 * renders tables, Unicode, images and code properly.
 *
 * Math handling (the part that used to break):
 *   1. Fenced code blocks + inline `code` are extracted to placeholders
 *      FIRST, so `$` inside code is never mistaken for math.
 *   2. `$$display$$` and `$inline$` spans are extracted to placeholders
 *      BEFORE marked runs, so emphasis/table/escape parsing cannot mangle
 *      TeX contents (`_`, `*`, `|`, `<`, `\\`).
 *   3. marked parses the placeholder text, then placeholders are restored
 *      as KaTeX HTML (throwOnError:false) and highlighted <pre>/<code>.
 *   Only pure-currency shapes (`$100`, `$3.50`) are left as literal text;
 *   everything else non-empty renders as math.
 *
 * Assets:
 *   - Relative image `src` (e.g. `../benchmark/plot.png`) is rewritten to
 *     absolute `file://` URLs resolved from report.md, so images survive
 *     the move to the temp dir Chromium actually prints.
 *   - KaTeX `fonts/` are copied next to the temp HTML so the inlined
 *     katex.min.css `url(fonts/...)` references resolve.
 *
 * Usage:
 *   node report/export_pdf.mjs [--out report.pdf] [--keep-html]
 *   make pdf
 */

import { execFileSync } from "node:child_process";
import { cpSync, existsSync, mkdirSync } from "node:fs";
import { mkdtempSync, readFileSync, rmSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";
import { createRequire } from "node:module";

import { Marked } from "marked";
import { markedHighlight } from "marked-highlight";
import hljs from "highlight.js";
import katex from "katex";

const require = createRequire(import.meta.url);
const HERE = dirname(fileURLToPath(import.meta.url));
const DEFAULT_MD = join(HERE, "report.md");
const DEFAULT_PDF = join(HERE, "..", "report.pdf");

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function escapeHtml(s) {
    return s
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;");
}

function highlightBlock(code, lang) {
    const escaped = escapeHtml(code);
    if (lang && hljs.getLanguage(lang)) {
        try {
            const value = hljs.highlight(code, { language: lang }).value;
            return `<pre><code class="hljs language-${escapeHtml(lang)}">${value}</code></pre>`;
        } catch {
            /* fall through to plain */
        }
    }
    // No language / unknown language (incl. `text` output blocks): no
    // auto-guessing — measurement output must stay plain, not mis-colored.
    const cls = lang ? ` class="language-${escapeHtml(lang)}"` : "";
    return `<pre><code${cls}>${escaped}</code></pre>`;
}

// ---------------------------------------------------------------------------
// Markdown -> HTML (math-first: code out, math out, marked, restore)
// ---------------------------------------------------------------------------

const ESCAPED_DOLLAR = "@@KA-ESCAPED-DOLLAR@@";

function mdToHtml(mdText, warnings) {
    // 0. Protect escaped dollars so `\$` never opens math.
    let text = mdText.replace(/\\\$/g, ESCAPED_DOLLAR);

    // 1. Fenced code blocks -> placeholders (highlight now, restore later as
    //    block HTML so marked never sees the contents).
    const codeBlocks = [];
    text = text.replace(/```(\w*)[ \t]*\n([\s\S]*?)\n```[ \t]*/g, (_whole, lang, code) => {
        const id = codeBlocks.length;
        codeBlocks.push(highlightBlock(code.replace(/\n$/, ""), (lang || "").trim()));
        return `\n\n@@KACODEBLOCK${id}@@\n\n`;
    });
    // Unclosed fence: warn, treat rest as code instead of swallowing math.
    const fenceLeft = (text.match(/^```/gm) || []).length;
    if (fenceLeft > 0) {
        warnings.push(`unclosed fenced code block (${fenceLeft} fence marker(s) left)`);
    }

    // 2. Inline `code` spans -> placeholders.
    const codeSpans = [];
    text = text.replace(/`([^`\n]+?)`/g, (_whole, code) => {
        const id = codeSpans.length;
        codeSpans.push(`<code>${escapeHtml(code)}</code>`);
        return `@@KACODEINLINE${id}@@`;
    });

    // 3. Display math $$...$$ (may span lines) -> placeholders.
    const maths = [];
    text = text.replace(/\$\$([\s\S]+?)\$\$/g, (_whole, src) => {
        const tex = src.trim();
        if (!tex) return _whole;
        const id = maths.length;
        maths.push({ tex, display: true });
        return `@@KAMATH${id}@@`;
    });

    // 4. Inline math $...$ (single line). Skip placeholders we introduced
    //    (they contain no `$`, so the regex cannot match them anyway).
    text = text.replace(/\$([^$\n]+?)\$/g, (whole, src) => {
        const tex = src.trim();
        if (!tex) return whole;
        // Keep pure currency as literal text: $100, $3.50, $1,000.25.
        if (/^\d[\d,]*(\.\d+)?$/.test(tex)) return whole;
        // `$ ...` / `... $` with boundary spaces is prose, not math.
        if (/^\s|\s$/.test(src)) return whole;
        const id = maths.length;
        maths.push({ tex, display: false });
        return `@@KAMATH${id}@@`;
    });

    // 5. Parse the placeholder-laden markdown.
    const raw = marked.parse(text, { async: false });

    // 6. Restore: code blocks (unwrap the <p> marked puts around a lone
    //    placeholder), inline code, escaped dollars, then math -> KaTeX.
    let html = raw;
    html = html.replace(/<p>\s*@@KACODEBLOCK(\d+)@@\s*<\/p>/g, (_w, id) => codeBlocks[Number(id)] ?? _w);
    html = html.replace(/@@KACODEBLOCK(\d+)@@/g, (_w, id) => codeBlocks[Number(id)] ?? _w);
    html = html.replace(/@@KACODEINLINE(\d+)@@/g, (_w, id) => codeSpans[Number(id)] ?? _w);
    html = html.split(ESCAPED_DOLLAR).join("$");
    html = html.replace(/<p>\s*@@KAMATH(\d+)@@\s*<\/p>/g, (whole, id) => {
        const m = maths[Number(id)];
        if (!m || !m.display) return whole; // inline math keeps its <p>
        try {
            return katex.renderToString(m.tex, { displayMode: true, throwOnError: false, strict: false });
        } catch {
            warnings.push(`KaTeX display failed: ${m.tex.slice(0, 80)}`);
            return `<pre>${escapeHtml(m.tex)}</pre>`;
        }
    });
    html = html.replace(/@@KAMATH(\d+)@@/g, (_w, id) => {
        const m = maths[Number(id)];
        if (!m) return _w;
        try {
            return katex.renderToString(m.tex, {
                displayMode: m.display,
                throwOnError: false,
                strict: false,
            });
        } catch {
            warnings.push(`KaTeX inline failed: ${m.tex.slice(0, 80)}`);
            return escapeHtml(m.tex);
        }
    });

    // 7. Post-checks: leftover math-looking spans the regex deliberately
    //    skipped (multiline inline, boundary spaces) — surface, don't crash.
    const leftover = html.match(/\$[^$\n<>]{1,60}\$/g);
    if (leftover) {
        const sample = [...new Set(leftover)].slice(0, 5).join(", ");
        warnings.push(`possible unrendered math left as literal: ${sample}`);
    }
    return html;
}

const marked = new Marked(
    { gfm: true, breaks: false },
    markedHighlight({
        langPrefix: "hljs language-",
        highlight(code, lang) {
            if (lang && hljs.getLanguage(lang)) {
                try {
                    return hljs.highlight(code, { language: lang }).value;
                } catch {
                    /* fall through */
                }
            }
            return escapeHtml(code);
        },
    })
);

// Custom renderer: force <details> open (incl. `<details ...>`) so content prints.
marked.use({
    renderer: {
        html({ text }) {
            return text.replace(/<details(?![^>]*\bopen\b)[^>]*>/g, "<details open>");
        },
    },
});

// Rewrite relative <img src> to absolute file:// URLs rooted at the .md dir,
// so images load after the HTML is staged in a temp dir.
function fixImageSrcs(bodyHtml, mdDir, warnings) {
    return bodyHtml.replace(/<img([^>]*?)src="([^"]+)"([^>]*?)>/g, (whole, pre, src, post) => {
        if (/^(https?:|data:|file:|#|mailto:)/i.test(src)) return whole;
        const clean = src.split("#")[0].split("?")[0];
        const hash = src.slice(clean.length);
        const abs = resolve(mdDir, decodeURIComponent(clean));
        if (!existsSync(abs)) {
            warnings.push(`missing image: ${src}`);
            return whole;
        }
        return `<img${pre}src="${pathToFileURL(abs).href}${hash}"${post}>`;
    });
}

// ---------------------------------------------------------------------------
// CSS (print-oriented)
// ---------------------------------------------------------------------------

const CSS = `
@page { size: A4; margin: 14mm 13mm; }
html { -webkit-print-color-adjust: exact; print-color-adjust: exact; }
body { font-family: 'DejaVu Sans', 'Noto Sans', sans-serif; font-size: 9pt;
       line-height: 1.45; color: #1a1a1a; }
h1 { font-size: 17pt; border-bottom: 2px solid #333; padding-bottom: 4px;
     margin: 8px 0 10px; page-break-after: avoid; }
h2 { font-size: 12pt; border-bottom: 1px solid #ccc; padding-bottom: 2px;
     margin-top: 16px; margin-bottom: 6px; page-break-after: avoid; }
h3 { font-size: 10pt; margin-top: 10px; margin-bottom: 4px;
     page-break-after: avoid; }
p, li { orphans: 3; widows: 3; }
table { border-collapse: collapse; width: 100%; font-size: 7.5pt; margin: 6px 0;
        table-layout: fixed; }
thead { display: table-header-group; }
tr { page-break-inside: avoid; }
th, td { border: 1px solid #bbb; padding: 3px 4px; text-align: left;
         vertical-align: top; overflow-wrap: anywhere; }
th { background: #f0f0f0; }
pre { background: #f6f8fa; border: 1px solid #ddd; border-radius: 4px;
      padding: 8px; font-size: 7.8pt; line-height: 1.4;
      white-space: pre-wrap; word-break: break-word; }
pre code { background: none; padding: 0; }
code { font-family: 'DejaVu Sans Mono', 'Noto Sans Mono', monospace;
       font-size: 0.92em; background: #f0f0f0;
       padding: 0 2px; border-radius: 3px; }
.hljs { background: transparent; }
img { max-width: 100%; height: auto; display: block; margin: 8px auto;
      page-break-inside: avoid; }
blockquote { border-left: 3px solid #ccc; margin: 8px 0; padding-left: 12px;
             color: #555; page-break-inside: avoid; }
details { margin: 8px 0; }
summary { font-weight: bold; cursor: pointer; }
hr { border: none; border-top: 1px solid #ccc; margin: 18px 0; }
ul, ol { margin: 6px 0; padding-left: 22px; }
li { margin: 2px 0; }
a { color: #0366d6; overflow-wrap: anywhere; }
/* KaTeX */
.katex { font-size: 1.02em; }
.katex-display { margin: 8px 0; overflow-x: auto; page-break-inside: avoid; }
`;

// ---------------------------------------------------------------------------
// Chromium -> PDF
// ---------------------------------------------------------------------------

function findChromium() {
    for (const name of ["chromium", "chromium-browser", "google-chrome", "google-chrome-stable"]) {
        try {
            const path = execFileSync("which", [name], { encoding: "utf8" }).trim();
            if (path) return path;
        } catch {
            /* try next */
        }
    }
    return null;
}

function main() {
    const args = process.argv.slice(2);
    let outPdf = DEFAULT_PDF;
    let keepHtml = false;
    for (let i = 0; i < args.length; i++) {
        if (args[i] === "--out") outPdf = args[++i];
        else if (args[i] === "--keep-html") keepHtml = true;
        else if (args[i] === "--help" || args[i] === "-h") {
            console.log("Usage: node export_pdf.mjs [--out report.pdf] [--keep-html]");
            process.exit(0);
        }
    }
    if (!outPdf) {
        console.error("error: --out requires a path");
        process.exit(1);
    }

    const warnings = [];
    const mdText = readFileSync(DEFAULT_MD, "utf8");
    let body = mdToHtml(mdText, warnings);
    body = fixImageSrcs(body, HERE, warnings);

    const katexCssPath = require.resolve("katex/dist/katex.min.css");
    const hljsCssPath = require.resolve("highlight.js/styles/github.css");
    const katexCssText = readFileSync(katexCssPath, "utf8");
    const hljsCssText = readFileSync(hljsCssPath, "utf8");

    const htmlDoc = `<!DOCTYPE html>
<html><head><meta charset="utf-8">
<title>Phonebook Lookup — Group Report</title>
<style>${katexCssText}</style>
<style>${hljsCssText}</style>
<style>${CSS}</style>
</head><body>${body}</body></html>`;

    const tmpDir = mkdtempSync(join(tmpdir(), "report-pdf-"));
    const tmpHtml = join(tmpDir, "report.html");
    writeFileSync(tmpHtml, htmlDoc, "utf8");

    // KaTeX fonts: the inlined CSS references url(fonts/...) relative to
    // this document, so copy the dist fonts next to the temp HTML.
    try {
        const katexDist = dirname(katexCssPath);
        const srcFonts = join(katexDist, "fonts");
        if (existsSync(srcFonts)) {
            mkdirSync(join(tmpDir, "fonts"), { recursive: true });
            cpSync(srcFonts, join(tmpDir, "fonts"), { recursive: true });
        } else {
            warnings.push("katex fonts dir not found; math glyphs may fall back");
        }
    } catch (e) {
        warnings.push(`could not stage katex fonts: ${e.message}`);
    }

    const chromium = findChromium();
    if (!chromium) {
        console.error("error: no chromium/chrome found; cannot print to PDF");
        process.exit(1);
    }

    const outAbs = resolve(outPdf);
    const url = pathToFileURL(tmpHtml).href;
    const base = [chromium, "--headless=new", "--disable-gpu", "--no-sandbox",
        "--hide-scrollbars", "--allow-file-access-from-files",
        "--run-all-compositor-stages-before-draw", "--virtual-time-budget=8000"];
    const variants = [
        [...base, "--no-pdf-header-footer", `--print-to-pdf=${outAbs}`, url],
        [...base, `--print-to-pdf=${outAbs}`, url],
    ];
    let ok = false;
    let lastErr = "";
    for (const cmd of variants) {
        try {
            execFileSync(cmd[0], cmd.slice(1), { stdio: "inherit" });
            ok = true;
            break;
        } catch (e) {
            lastErr = e.stderr || e.message;
        }
    }
    if (!ok) {
        console.error("error: chromium failed:\n" + lastErr);
        process.exit(1);
    }

    if (keepHtml) {
        console.log(`html: ${tmpHtml}`);
    } else {
        rmSync(tmpDir, { recursive: true, force: true });
    }
    if (warnings.length > 0) {
        console.log("warnings:");
        for (const w of warnings) console.log(`  - ${w}`);
    }
    console.log(`ok: ${outAbs}`);
}

main();
