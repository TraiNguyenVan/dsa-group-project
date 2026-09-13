#!/usr/bin/env node
/**
 * Export report/report.md -> report.pdf.
 *
 * Pipeline: marked (GFM markdown -> HTML) + KaTeX (math) + highlight.js
 * (code highlighting), then headless Chromium prints the HTML to PDF.
 * A real browser engine renders tables, Unicode, images and code properly.
 *
 * Usage:
 *   node report/export_pdf.mjs [--out report.pdf] [--keep-html]
 *   make pdf
 */

import { execFileSync } from "node:child_process";
import { existsSync } from "node:fs";
import { mkdtempSync, readFileSync, rmSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";
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
// Markdown -> HTML (marked + KaTeX + highlight.js)
// ---------------------------------------------------------------------------

// KaTeX renderer: inline $...$ and display $$...$$.
// Guard against stray dollar signs (currency, ranges) by requiring the
// content to contain at least one backslash command or a known math token.
const MATH_RE = /\$\$([\s\S]+?)\$\$|\$([^$\n]+?)\$/g;

function renderMath(text) {
    return text.replace(MATH_RE, (whole, display, inline) => {
        const src = (display ?? inline).trim();
        if (!/\\[a-zA-Z]+|_|\^/.test(src)) return whole; // not math, keep as-is
        try {
            return katex.renderToString(src, {
                displayMode: Boolean(display),
                throwOnError: false,
                strict: false,
            });
        } catch {
            return whole;
        }
    });
}

const marked = new Marked(
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
            return hljs.highlightAuto(code).value;
        },
    })
);

// Custom renderer: force <details> open so content prints.
const renderer = {
    html({ text }) {
        return text.replace(/<details>/g, "<details open>");
    },
};
marked.use({ renderer });

function mdToHtml(mdText) {
    const raw = marked.parse(mdText, { async: false });
    return renderMath(raw);
}

// ---------------------------------------------------------------------------
// CSS (print-oriented)
// ---------------------------------------------------------------------------

const CSS = `
@page { size: A4; margin: 14mm 13mm; }
html { -webkit-print-color-adjust: exact; print-color-adjust: exact; }
body { font-family: 'DejaVu Sans', sans-serif; font-size: 9pt;
       line-height: 1.4; color: #1a1a1a; }
h1 { font-size: 17pt; border-bottom: 2px solid #333; padding-bottom: 4px; margin: 8px 0 10px; }
h2 { font-size: 12pt; border-bottom: 1px solid #ccc; padding-bottom: 2px;
     margin-top: 16px; margin-bottom: 6px; }
h3 { font-size: 10pt; margin-top: 10px; margin-bottom: 4px; }
table { border-collapse: collapse; width: 100%; font-size: 7.5pt; margin: 6px 0;
        table-layout: fixed; }
th, td { border: 1px solid #bbb; padding: 3px 4px; text-align: left;
         vertical-align: top; overflow-wrap: anywhere; }
th { background: #f0f0f0; }
pre { background: #f6f8fa; border: 1px solid #ddd; border-radius: 4px;
      padding: 8px; font-size: 8.5pt; white-space: pre-wrap;
      word-wrap: break-word; }
code { font-family: 'DejaVu Sans Mono', monospace; background: #f0f0f0;
       padding: 0 2px; border-radius: 3px; }
pre code { background: none; padding: 0; }
img { max-width: 100%; }
blockquote { border-left: 3px solid #ccc; margin: 8px 0; padding-left: 12px;
             color: #555; }
details { margin: 8px 0; }
summary { font-weight: bold; cursor: pointer; }
hr { border: none; border-top: 1px solid #ccc; margin: 18px 0; }
ul, ol { margin: 6px 0; padding-left: 22px; }
li { margin: 2px 0; }
a { color: #0366d6; }
/* KaTeX */
.katex { font-size: 1.05em; }
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

    const mdText = readFileSync(DEFAULT_MD, "utf8");
    const body = mdToHtml(mdText);

    const katexCss = require.resolve("katex/dist/katex.min.css");
    const hljsCss = require.resolve("highlight.js/styles/github.css");
    const katexCssText = readFileSync(katexCss, "utf8");
    const hljsCssText = readFileSync(hljsCss, "utf8");

    const htmlDoc = `<!DOCTYPE html>
<html><head><meta charset="utf-8">
<style>${katexCssText}</style>
<style>${hljsCssText}</style>
<style>${CSS}</style>
</head><body>${body}</body></html>`;

    const tmpDir = mkdtempSync(join(tmpdir(), "report-pdf-"));
    const tmpHtml = join(tmpDir, "report.html");
    writeFileSync(tmpHtml, htmlDoc, "utf8");

    const chromium = findChromium();
    if (!chromium) {
        console.error("error: no chromium/chrome found; cannot print to PDF");
        process.exit(1);
    }

    const outAbs = resolve(outPdf);
    const url = "file://" + tmpHtml;
    const cmd = [chromium, "--headless=new", "--disable-gpu", "--no-sandbox",
        "--no-pdf-header-footer", `--print-to-pdf=${outAbs}`, url];
    try {
        execFileSync(cmd[0], cmd.slice(1), { stdio: "inherit" });
    } catch {
        // Older Chromium: retry without --no-pdf-header-footer
        const retry = cmd.filter((a) => a !== "--no-pdf-header-footer");
        try {
            execFileSync(retry[0], retry.slice(1), { stdio: "inherit" });
        } catch (e) {
            console.error("error: chromium failed:\n" + (e.stderr || e.message));
            process.exit(1);
        }
    }

    if (!keepHtml) rmSync(tmpDir, { recursive: true, force: true });
    console.log(`ok: ${outAbs}`);
}

main();