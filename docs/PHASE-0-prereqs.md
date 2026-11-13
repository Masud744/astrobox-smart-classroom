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

### 0C — Backend Deploy (Railway)

- [ ] [railway.app](https://railway.app) → New project → GitHub repo connect
- [ ] Environment variables set করো:
  ```
  SUPABASE_URL = https://oxxfnnecwvslmkxqhfgb.supabase.co
  SUPABASE_ANON_KEY = (তোমার actual key — backend/.env থেকে)
  ```
- [ ] Deploy হলে public URL note করো
- [ ] Test: `curl https://<your-url>/` → `{"status":"ok"}` দেখাবে
- [ ] `dashboard/.env`-এ যোগ করো: `VITE_API_URL=https://<your-url>`
- [ ] Dashboard rebuild: `npm run build` (dashboard/ folder-এ)
- [ ] Git commit + push

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
