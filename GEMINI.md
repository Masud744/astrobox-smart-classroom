# Project Rules — FastRobox Smart Classroom

This file is automatically loaded by Antigravity when working in this project.
Follow all rules below at all times without being asked.

---

## Auto-Push Rule (Most Important)

After EVERY file change, addition, or deletion — immediately run:
```bash
git add -A
git commit -m "<verb> <what>"
git push origin main
```
Never leave changes uncommitted. Never ask the user if they want to push. Just do it.

---

## Git Rules

- Every commit message must follow: `<verb> <what> [<where>]`
  - Examples: `add DHT11 read with retry`, `fix flame sensor pin polarity`, `post sensor data to /api/sensor`
  - Never use: `fix`, `update`, `changes`, `final`, `final2`
- After every level pass, immediately run:
  ```bash
  git add .
  git commit -m "level-NN: <what was done>"
  git tag level-NN
  git push origin main --tags
  ```
- Never commit `.env`, secrets, tokens, or API keys — ever.
- Always maintain full commit history. Never squash or force-push.
- Update `README.md` in every commit from Level 2 onwards.

---

## Code Style Rules

- Write code like a human developer, not like an AI assistant.
- Comments explain **why**, never **what**. If code is obvious, no comment needed.
- Variable names: short, meaningful. No `tempVariable`, `myData`, `handleEverything`.
- Functions do one thing. Name matches what it does exactly.
- No dead code, no commented-out blocks, no TODOs left in final submission.
- Group related variables in structs.
- No hardcoded WiFi credentials, Room ID, class start time, or any judge-supplied values.

---

## Security Rules

- Never put API keys, tokens, passwords, or Supabase keys in any code file or git commit.
- Always use `.env` for secrets on the backend.
- Always use SmartProv dynamic fields or NVS for ESP32 credentials.
- `.env` is in `.gitignore` — never override this.
- `.env.example` must exist with placeholder values only.

---

## Evidence Rules (Competition Day)

After every judging call pass:
1. Git commit + tag + push immediately
2. Fill write-up within 30 minutes (use `.skills/writeup-template.md`)
3. Export video (720p+, max 10 min, max 3 clips)
4. Rename files: `<TeamID>_L<NN>_video.mp4` and `<TeamID>_L<NN>_writeup.pdf`
5. Upload both files to submission folder within 30 minutes

---

## Firmware Rules (ESP32)

- Never hardcode WiFi SSID, password, Room ID, or class start time — instant fail.
- All runtime config must come from SmartProv captive portal or cloud settings endpoint.
- Always keep local HTTP status page working (offline fallback).
- Always keep all previous level features working when adding new ones.
- Test every change on hardware before calling judges.

---

## File Naming

- Firmware: `firmware/main/`
- Backend: `backend/`
- Dashboard: `dashboard/`
- Skills: `.skills/`
- Plan: `plan.md`
- Rules: `GEMINI.md` (this file)
- Rulebook: `rulebook.pdf`

---

## When Writing Any Code for This Project

1. Check `.skills/human-code-style.md` before writing
2. Check `.skills/secret-safety.md` before any credential handling
3. Check `.skills/evidence-checklist.md` before any judging call
4. Check `.skills/judging-call-strategy.md` for call timing decisions
5. Use `.skills/writeup-template.md` for every write-up

