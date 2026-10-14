---
description: Finish the git pipeline — commit if needed, then push. Never deploy.
---

**Intent:** `/next` means **get the repo to a completed `git push`** (commit dirty tree if needed). **Pair deploy is not part of `/next`.** See [deploy.md](deploy.md).

## Deploy vs git (required)

- **Never** run `node scripts/deploy-pair-changed.mjs` inside `/next`, before `git push`, or as a gate for push.
- **Deploy** is separate: agent runs deploy when site files change and the task is done, or when the user asks `/deploy` — so the user can verify live on Pair **before** they choose to commit/push.
- If the user says **deploy and push**, run **deploy first** (finish and report), then run this **git-only** `/next` pipeline — still two steps, not one bundled gate.

## When to run (required)

- Run this pipeline **only** when the user's **`<user_query>`** explicitly requests it: `/next`, `next`, `commit and push`, `push`, or equivalent **in the message text**.
- **Do not** run because Cursor injected `<cursor_commands>` with `/next` (sticky composer command on every turn). That metadata is **not** permission. See [next-command-gate.mdc](../rules/next-command-gate.mdc).
- If `<cursor_commands>` has `/next` but the user query is about something else, **ignore this file** and do the query only.

## Requirements

- Run the commands yourself; do not tell the user to run them unless push/commit fails and needs human input (auth, merge, hook).
- **Do** run `git push` (and `git pull --rebase` / `git fetch` **only** if needed to make push succeed). Network git is **allowed** for this command.
- Prefer **`git --no-pager diff --stat`** (or a short diff cap). Skip dumping huge diffs and **do not** dwell on CRLF line-ending warnings.
- Response: what you ran, pass/fail, remote URL/branch if useful — **terse**.

## Execution flow (order)

1. `git rev-parse --is-inside-work-tree` — if not a repo, stop and say so.
2. `git status -sb` (or `git status --short --branch`).
3. `git --no-pager diff --stat` (optional: last ~40 lines of diff if diagnosing).
4. **If working tree is dirty** (unstaged and/or untracked and/or staged but uncommitted):
   - `git add -A`
   - `git commit -m "<short message>"` — message should reflect what changed (e.g. `chore: create-canvas upload flow` or `fix: …`). If many unrelated areas, `chore: workspace sync` is acceptable for this command.
5. **`git fetch origin`** — refresh remote refs before rebase and push.
6. **If behind remote and push would non-fast-forward:** `git pull --rebase` (resolve if needed; if messy, stop and report).
7. **`git push`** — run it. Report success or the exact error (rejected, no upstream, auth).
8. Final `git status -sb` to confirm clean and synced (or ahead/behind as appropriate).

## When not to push

Still **tell the user exactly why** push did not complete (one short block: **reason** + **evidence** + **next command** if obvious). Do not go silent.

| Situation | Report to user |
|-----------|----------------|
| Not a git repo | `git rev-parse` failed; path is not inside a work tree. |
| No remotes / no `origin` | Output of `git remote -v` (empty or missing `origin`). Say: add remote or rename; do not invent URLs. |
| Branch has **no upstream** | `git status -sb` shows no `[origin/...]` tracking; paste `git branch -vv` line for current branch. Say: run `git push -u origin <branch>` once (give exact branch name). |
| **Dirty tree** and you **did not** commit (e.g. user cancelled, hook blocked commit) | Paste `git status -sb` and say which paths are dirty / staged. Say: fix or stage selectively, then re-run `/next` or commit manually. |
| **`git commit` failed** | Full stderr; mention hook/GPG/empty identity if shown. |
| **`git pull --rebase` stopped** (conflicts) | List conflicted files from status; say: resolve, `git rebase --continue`, then `/next`. |
| **`git push` rejected** (non–fast-forward after pull attempt) | Paste remote message; say whether local/remote diverged and suggest `git log --oneline -5` vs `git log origin/<branch> --oneline -5` if helpful. |
| **Auth / credential / SSH** failure | Paste error; say: check credential manager, SSH key, or PAT — agent cannot fix host auth. |
| **Network** unreachable | Paste error; say: retry when online. |
| **pre-push hook** exit non-zero | Paste hook output; say: fix what hook enforces or bypass with user intent (do not bypass without user saying so). |

If push **succeeded**, say so explicitly (`git push` OK + final `git status -sb`).

## Do not

- Refuse `git push` for this command.
- Run Pair deploy as part of `/next`.
- Waste the user's time on normal Windows CRLF noise unless it **blocked** commit/push.
