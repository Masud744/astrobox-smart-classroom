# Render Deployment Guide — FastRobox T07
**render.com-এ backend + frontend deploy করার step-by-step**

---

## Strategy: কী কোথায় deploy হবে

```
Backend (FastAPI)  → Render Web Service   (Python)
Frontend (React)   → Render Static Site   (Node build)
```

> [!TIP]
> Backend already dashboard static files serve করতে পারে (`dashboard/dist/`).
> আলাদা Static Site না বানালেও চলবে — backend-এ `/dashboard` route-এ dashboard পাবে।
> কিন্তু আলাদা করলে faster CDN পাবে। দুটো option-ই নিচে আছে।

---

## Option A — Backend + Frontend একসাথে (Simple, Recommended)

Backend serve করবে API + dashboard static files একই URL থেকে।

### Step 1: `render.yaml` (repo root-এ রাখো)

```yaml
# render.yaml — Render.com deployment config
services:
  - type: web
    name: fastrobox-backend
    runtime: python
    rootDir: backend
    buildCommand: pip install -r requirements.txt
    startCommand: uvicorn main:app --host 0.0.0.0 --port $PORT
    envVars:
      - key: SUPABASE_URL
        sync: false   # Render dashboard-এ manually set করবে
      - key: SUPABASE_ANON_KEY
        sync: false
      - key: PYTHON_VERSION
        value: 3.11.0
    healthCheckPath: /
```

### Step 2: Render-এ করো

1. [render.com](https://render.com) → Sign in with GitHub
2. **New** → **Web Service**
3. Connect repo: `astrobox-smart-classroom`
4. Settings:
   - **Root Directory**: `backend`
   - **Runtime**: Python 3
   - **Build Command**: `pip install -r requirements.txt`
   - **Start Command**: `uvicorn main:app --host 0.0.0.0 --port $PORT`
5. **Environment Variables** → Add:
   - `SUPABASE_URL` = `https://oxxfnnecwvslmkxqhfgb.supabase.co`
   - `SUPABASE_ANON_KEY` = (backend/.env থেকে)
6. **Create Web Service** → Deploy শুরু হবে
7. URL পাবে: `https://fastrobox-backend.onrender.com`

### Step 3: Dashboard-কে backend URL দাও

```
dashboard/.env-এ update করো:
VITE_API_URL=https://fastrobox-backend.onrender.com

তারপর build:
npm run build   (dashboard/ folder-এ)

git add -A
git commit -m "chore: update VITE_API_URL for Render deploy"
git push origin main
```

---

## Option B — আলাদা Static Site (Faster CDN)

### Step 1: Static Site বানাও

1. **New** → **Static Site**
2. **Root Directory**: `dashboard`
3. **Build Command**: `npm install && npm run build`
4. **Publish Directory**: `dist`
5. **Environment Variables**:
   - `VITE_SUPABASE_URL` = `https://oxxfnnecwvslmkxqhfgb.supabase.co`
   - `VITE_SUPABASE_ANON_KEY` = (actual key)
   - `VITE_API_URL` = `https://fastrobox-backend.onrender.com`
6. **Create Static Site**
7. URL পাবে: `https://fastrobox-dashboard.onrender.com`

---

## ⚠️ Render Free Tier Gotchas

| সমস্যা | সমাধান |
|--------|---------|
| Free tier 15 min idle → sleep | Competition-এর আগে ping করো |
| Cold start = 30-50s | Call দেওয়ার আগে browser-এ open করো |
| Build timeout | requirements.txt-এ ভারী packages থাকলে হবে |

**Competition-এ যাওয়ার আগে**: Browser-এ backend URL open করে রাখো যাতে cold start না হয়।

---

## ✅ Deploy Verification

```bash
# Backend health check:
curl https://fastrobox-backend.onrender.com/
# → {"status":"ok","project":"fastrobox-classroom","version":"1.1.0"}

# API test:
curl https://fastrobox-backend.onrender.com/api/settings
# → settings JSON

# Dashboard test:
# Browser-এ https://fastrobox-dashboard.onrender.com খোলো
```

---

## 📌 Notes

- `render.yaml` repo-তে থাকলে Render automatically detect করে (Blueprint deploy)
- Free tier-এ 750 hours/month — competition-এর জন্য যথেষ্ট
- Render logs: Dashboard → Service → Logs tab (real-time)
- এ ENV vars Render-এর Dashboard-এ set করো, **কখনো code-এ না**

