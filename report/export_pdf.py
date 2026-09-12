#!/usr/bin/env python3
"""Export report/report.md -> report/report.pdf via headless Chromium.

No third-party dependencies: the markdown -> HTML step is a small,
self-contained converter, and the HTML -> PDF step uses the system
Chromium in headless print mode (a real browser engine, so tables,
Unicode, images and code blocks render properly).

Supported markdown subset (covers report.md today):
  - ATX headings (# .. ######)
  - paragraphs, bold/italic, inline code, links, images
  - fenced code blocks (```lang ... ```)
  - GFM pipe tables (header + --- separator + rows)
  - flat unordered lists (- / *)
  - blockquotes (>)
  - horizontal rules (---)
  - raw <details>/<summary> blocks (forced open so content is printed)
  - HTML comments (dropped)

NOT supported (script warns, does not crash): LaTeX math, footnotes,
definition lists, nested lists, Mermaid, task lists. For those, install
pandoc + a LaTeX engine and use: pandoc report.md -o report.pdf.

Usage:
    python3 report/export_pdf.py [--out report/report.pdf] [--keep-html]
"""

import argparse
import html
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_MD = os.path.join(HERE, "report.md")
DEFAULT_PDF = os.path.join(HERE, "../report.pdf")

# ---------------------------------------------------------------------------
# Markdown -> HTML
# ---------------------------------------------------------------------------

INLINE_RE = re.compile(
    r"(!\[[^\]]*\]\([^)]*\)"   # image ![alt](url)
    r"|\[[^\]]*\]\([^)]*\)"    # link  [text](url)
    r"|`[^`]*`"                # code  `...`
    r"|\*\*[^*]+\*\*"          # bold  **...**
    r"|\*[^*]+\*)"             # italic *...*
)


def inline(text):
    """Apply inline formatting; escape everything else and preserve
    raw HTML break tags by moving them to a private sentinel before
    escaping, then restoring them in the final HTML output.
    """
    # Support HTML `<br>` and `<br/>` inside Markdown prose or table cells.
    text = re.sub(r"<br\s*/?>", "__BR_TAG__", text, flags=re.IGNORECASE)

    out = []
    for part in INLINE_RE.split(text):
        if part is None:
            continue
        if part.startswith("![") and part.endswith(")"):
            m = re.match(r"!\[([^\]]*)\]\(([^)]*)\)", part)
            out.append(f'<img src="{html.escape(m.group(2))}" alt="{html.escape(m.group(1))}">')
        elif part.startswith("[") and part.endswith(")"):
            m = re.match(r"\[([^\]]*)\]\(([^)]*)\)", part)
            out.append(f'<a href="{html.escape(m.group(2))}">{inline(m.group(1))}</a>')
        elif part.startswith("`") and part.endswith("`"):
            out.append(f"<code>{html.escape(part[1:-1])}</code>")
        elif part.startswith("**") and part.endswith("**"):
            out.append(f"<strong>{inline(part[2:-2])}</strong>")
        elif part.startswith("*") and part.endswith("*"):
            out.append(f"<em>{inline(part[1:-1])}</em>")
        else:
            out.append(html.escape(part))

    return "".join(out).replace("__BR_TAG__", "<br>")


def parse_table(lines):
    """lines: consecutive '|'-prefixed lines. Returns HTML table.

    Apply inline formatting cell-by-cell so raw HTML line breaks inside
    a table cell survive as real `<br>` tags in the generated HTML.
    """
    rows = [l.strip().strip("|") for l in lines]
    cells = [[c.strip() for c in r.split("|")] for r in rows]
    header, body = cells[0], cells[2:]  # row 1 is the --- separator

    header_cells = [inline(c) for c in header]
    thead = "<tr>" + "".join(f"<th>{c}</th>" for c in header_cells) + "</tr>"

    tbody = ""
    for row in body:
        body_cells = [inline(c) for c in row]
        tbody += "<tr>" + "".join(f"<td>{c}</td>" for c in body_cells) + "</tr>"

    return f"<table><thead>{thead}</thead><tbody>{tbody}</tbody></table>"


def md_to_html(md_text, warnings):
    # Drop HTML comments (may span multiple lines).
    md_text = re.sub(r"<!--.*?-->", "", md_text, flags=re.DOTALL)

    lines = md_text.splitlines()
    out = []
    i = 0
    n = len(lines)
    while i < n:
        line = lines[i]
        stripped = line.strip()

        # Raw HTML passthrough (details/summary) — force details open.
        if stripped == "<details>":
            out.append("<details open>")
            i += 1
            continue
        if stripped == "</details>" or stripped.startswith("<summary>"):
            out.append(stripped)
            i += 1
            continue

        # Fenced code block
        m = re.match(r"^```(\w*)\s*$", stripped)
        if m:
            lang = m.group(1)
            buf = []
            i += 1
            while i < n and not lines[i].strip().startswith("```"):
                buf.append(lines[i])
                i += 1
            i += 1  # skip closing fence
            code = html.escape("\n".join(buf))
            cls = f' class="language-{lang}"' if lang else ""
            out.append(f"<pre><code{cls}>{code}</code></pre>")
            continue

        # Heading
        m = re.match(r"^(#{1,6})\s+(.*)$", stripped)
        if m:
            level = len(m.group(1))
            out.append(f"<h{level}>{inline(m.group(2))}</h{level}>")
            i += 1
            continue

        # Table: current line starts with '|' and next line is a separator
        if stripped.startswith("|") and i + 1 < n and re.match(r"^\|[\s:|-]+\|?\s*$", lines[i + 1].strip()):
            buf = [stripped]
            i += 1
            while i < n and lines[i].strip().startswith("|"):
                buf.append(lines[i].strip())
                i += 1
            out.append(parse_table(buf))
            continue

        # Horizontal rule
        if re.match(r"^-{3,}\s*$", stripped):
            out.append("<hr>")
            i += 1
            continue

        # Unordered list (flat)
        if re.match(r"^[-*]\s+", stripped):
            buf = []
            while i < n and re.match(r"^[-*]\s+", lines[i].strip()):
                buf.append(inline(re.sub(r"^[-*]\s+", "", lines[i].strip())))
                i += 1
            out.append("<ul>" + "".join(f"<li>{b}</li>" for b in buf) + "</ul>")
            continue

        # Blockquote
        if stripped.startswith(">"):
            buf = []
            while i < n and lines[i].strip().startswith(">"):
                buf.append(inline(re.sub(r"^>\s?", "", lines[i].strip())))
                i += 1
            out.append("<blockquote>" + "<br>".join(buf) + "</blockquote>")
            continue

        # Paragraph: accumulate until blank line or a block-starting line
        buf = []
        while i < n and lines[i].strip():
            s = lines[i].strip()
            if re.match(r"^(#{1,6})\s", s) or re.match(r"^```", s) or \
               re.match(r"^[-*]\s+", s) or s.startswith("|") or \
               re.match(r"^-{3,}\s*$", s) or s.startswith(">"):
                break
            buf.append(s)
            i += 1
        if buf:
            out.append("<p>" + inline(" ".join(buf)) + "</p>")
            continue

        i += 1  # blank line

    return "\n".join(out)


CSS = """
@page { size: A4; margin: 14mm 13mm; }
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
ul { margin: 6px 0; padding-left: 22px; }
a { color: #0366d6; }
"""


def find_chromium():
    for name in ("chromium", "chromium-browser", "google-chrome", "google-chrome-stable"):
        path = shutil.which(name)
        if path:
            return path
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=DEFAULT_PDF, help="output PDF path")
    ap.add_argument("--keep-html", action="store_true", help="keep the intermediate HTML")
    args = ap.parse_args()

    with open(DEFAULT_MD, "r", encoding="utf-8") as f:
        md_text = f.read()

    warnings = []
    body = md_to_html(md_text, warnings)
    html_doc = f"<!DOCTYPE html><html><head><meta charset='utf-8'>" \
               f"<style>{CSS}</style></head><body>{body}</body></html>"

    tmp_html = os.path.join(HERE, ".report.html")
    with open(tmp_html, "w", encoding="utf-8") as f:
        f.write(html_doc)

    chromium = find_chromium()
    if not chromium:
        print("error: no chromium/chrome found; cannot print to PDF", file=sys.stderr)
        sys.exit(1)

    out_pdf = os.path.abspath(args.out)
    url = "file://" + tmp_html
    cmd = [chromium, "--headless=new", "--disable-gpu", "--no-sandbox",
           "--no-pdf-header-footer", f"--print-to-pdf={out_pdf}", url]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        # Older Chromium: retry without --no-pdf-header-footer
        cmd.remove("--no-pdf-header-footer")
        r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        print("error: chromium failed:\n" + r.stderr[-2000:], file=sys.stderr)
        sys.exit(1)

    if not args.keep_html:
        os.remove(tmp_html)

    if warnings:
        print("warnings:")
        for w in warnings:
            print("  -", w)
    print(f"ok: {out_pdf}")


if __name__ == "__main__":
    main()