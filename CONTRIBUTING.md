# Contributing — DSA Group Project

How we work together. Read this once, then read `README.md` (project, deliverables, timeline).

## 1. Setup

```bash
git clone https://github.com/TraiNguyenVan/dsa-group-project.git
cd dsa-group-project
git checkout -b feature/<your-branch>
```

- **Never commit on `main`.** Every change lives on a branch.
- Branch naming — pick the prefix that matches the work:
  - `feature/<lang>-<thing>` — new functionality (e.g. `feature/cpp-quicksort`)
  - `experiment/<thing>` — throwaway experiments, benchmarks, or spikes (e.g. `experiment/pivot-strategy-benchmark`)
  - `fix/<thing>` — bug fixes (e.g. `fix/interpolation-edge-case`)
  - `docs/<thing>` — documentation-only changes (e.g. `docs/part-a-position`)
  - `chore/<thing>` — tooling, CI, config, no behavior change (e.g. `chore/makefile-clean`)
  - `refactor/<thing>` — code restructuring with no behavior change (e.g. `refactor/main-readability`)

## 2. Commit style

- Small commits, one logical change each.
- Message: short imperative — `add merge sort demo`, `fix off-by-one in quicksort`.
- No build artifacts, binaries, or raw datasets in git. Datasets come from the generator in `data/` (`.gitignore` handles the rest).

## 3. Issues & PRs

- Every task starts as an issue. No issue = no work.
- Claim it: comment "I'll take this" — one person per issue at a time.
- Work on your branch, push, open a PR that says `Fixes #N`.
- A PR is done when: 1 approving review **and** it compiles/runs.
- Reviewer: actually run the code before approving. Never approve blind.
- Author merges after approval, then deletes the branch.

## 4. Definition of done (this project)

- [x] Compiles and runs (C++ demo: `make && ./demo`)
- [x] Runs on the shared dataset (n ≥ 100 000)
- [x] Measurements recorded **with conditions** (CPU/RAM, OS, compiler + flags, dataset) — a measurement without its conditions is not a measurement
- [x] Relevant docs updated (`report/report.pdf`)
- [x] Sources cited — textbook chapter/page or URL + access date; adapted code cited in a comment at the point of use
- [x] AI use declared

## 5. Communication

- **GitHub issues/PRs** Write questions in English in issues so everyone sees the answer.

## 6. Rules (enforced, from README)

1. Cite everything.
2. AI is a tool, not an author — declare where it was used; be able to explain every line on request.
3. No shared code between groups.
4. Real measurements only — fabricated timings = zero for parts C and D.
5. Everyone contributes — free riders marked individually in the contribution statement.

## 7. Roles

- Flexible right now, things change too fast..