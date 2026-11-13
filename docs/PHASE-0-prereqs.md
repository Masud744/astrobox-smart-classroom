# Phase 0 — Pre-requisites (Critical Bugs + Deploy)
**Hardware ছাড়াই করা যাবে | আজকেই শেষ করো**

> [!CAUTION]
> এই phase শেষ না করে competition-এ গেলে সব Level fail। এই bugs active থাকলে Door toggle কাজ করবে না, RFID alert 500 error দেবে, backend ESP32 reach করতে পারবে না।

---

## ✅ Task List

### 0A — Critical Bug Fixes

- [ ] **Bug 1:** `dashboard/src/lib/supabase.js` → hardcoded key দুটো `import.meta.env.VITE_*` দিয়ে replace করো
- [ ] **Bug 2:** `dashboard/.env` ফাইল বানাও (actual keys দিয়ে, git-এ যাবে না)
- [ ] **Bug 3:** `dashboard/.env.example` বানাও (placeholder দিয়ে, git-এ যাবে)
- [ ] **Bug 4:** `dashboard/src/App.jsx` line ~158 → `trigger: 'dashboard'` করো
- [ ] **Bug 5:** Supabase SQL Editor-এ নিচের SQL চালাও:

```sql
ALTER TABLE alerts DROP CONSTRAINT IF EXISTS alerts_alert_type_check;
ALTER TABLE alerts ADD CONSTRAINT alerts_alert_type_check
  CHECK (alert_type IN (
    'flame','panic','temp_high','hum_high','offline','unknown_face','unknown_card'
  ));
```

- [ ] **git commit** করো: `fix: move supabase keys to env, fix door trigger, add unknown_card alert type`
- [ ] **git push** করো: `git push origin main`

### 0B — New Supabase Tables (Level 2 দরকার)

- [ ] `backend/supabase/migration_v2.sql` Supabase SQL Editor-এ চালাও:
  - `telemetry` table (device_id + seq UNIQUE)
  - `events` table

### 0C — Backend + Frontend Deploy (Render)

- [ ] [render.com](https://render.com) → Sign in with GitHub → **New** → **Web Service**
- [ ] Connect repo, settings:
  - **Root Directory**: `backend`
  - **Build Command**: `pip install -r requirements.txt`
  - **Start Command**: `uvicorn main:app --host 0.0.0.0 --port $PORT`
- [ ] Environment variables on Render dashboard:
  ```
  SUPABASE_URL = https://oxxfnnecwvslmkxqhfgb.supabase.co
  SUPABASE_ANON_KEY = (backend/.env থেকে নাও)
  ```
- [ ] Deploy হলে backend URL note করো (e.g. `https://fastrobox-backend.onrender.com`)
- [ ] Test: `curl https://fastrobox-backend.onrender.com/` → `{"status":"ok"}` দেখাবে
- [ ] **New** → **Static Site** (dashboard):
  - **Root Directory**: `dashboard`
  - **Build Command**: `npm install && npm run build`
  - **Publish Directory**: `dist`
  - Env vars: `VITE_SUPABASE_URL`, `VITE_SUPABASE_ANON_KEY`, `VITE_API_URL=<backend-url>`
- [ ] Dashboard URL note করো
- [ ] `dashboard/.env`-এ update করো: `VITE_API_URL=https://fastrobox-backend.onrender.com`
- [ ] Rebuild dashboard locally + git commit + push
- [ ] Full deploy guide: `docs/RENDER-DEPLOY.md`

### 0D — Dashboard Camera URL Fix

- [ ] `App.jsx`-এ camera stream URL dynamic করো (settings থেকে নাও, hardcoded IP সরাও)

---

## ✅ Phase 0 Verification

```
[ ] supabase.js-এ কোনো hardcoded key নেই (grep করে confirm করো)
[ ] dashboard build করলে error নেই
[ ] Door toggle → Network tab-এ 200 OK দেখাচ্ছে (Browser DevTools)
[ ] Unknown card scan → 500 error আর আসছে না
[ ] backend URL publicly accessible (curl দিয়ে test করো)
[ ] git log → কোনো secret নেই কোনো commit-এ
[ ] git status → clean (0 uncommitted changes)
```

---

## 📌 Notes

- `dashboard/.env` → `.gitignore`-এ `.env` already আছে, সরাসরি এই নামে save করলেই হবে
- Supabase anon key technically public-safe, কিন্তু কোডে রাখলে judge DQ করবে
- Railway free tier-এ sleep mode আছে — competition-এ আগে একবার ping করো

