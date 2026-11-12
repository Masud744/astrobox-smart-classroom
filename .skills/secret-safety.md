# Skill: Secret Safety (কোডে কোনো Secret রাখবে না)

## Purpose
রুলবুক স্পষ্ট বলেছে:
> *"ফার্মওয়্যার কোড, ওয়েবসাইট কোড বা গিট রিপোজিটরিতে কোনো পাসওয়ার্ড, API Key বা সিকিউরিটি টোকেন লিখে রাখা সম্পূর্ণ নিষিদ্ধ।"*

ধরা পড়লে → **Disqualification।**

---

## কোনগুলো Secret?

```
❌ কখনো code বা git-এ রাখবে না:
  - Supabase URL + anon key
  - Supabase service role key
  - API keys (যেকোনো)
  - WiFi SSID / Password (hardcode)
  - Database password
  - JWT secret
  - GitHub token
```

---

## কীভাবে সঠিকভাবে রাখবে?

### ESP32 Firmware-এ:
```cpp
// ❌ WRONG — কখনো এভাবে করবে না
const char* SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5c...";

// ✅ CORRECT — SmartProv দিয়ে runtime-এ নাও
// বা config.h-তে placeholder রাখো (actual value SmartProv portal থেকে)
```

### Backend / Dashboard-এ:
```bash
# .env ফাইলে রাখো (কখনো git-এ push করবে না)
SUPABASE_URL=https://xxxx.supabase.co
SUPABASE_ANON_KEY=eyJhbGci...
```

```bash
# .env.example ফাইল বানাও (এটা git-এ push করো — কিন্তু actual value ছাড়া)
SUPABASE_URL=your_supabase_url_here
SUPABASE_ANON_KEY=your_anon_key_here
```

---

## Git Commit করার আগে চেক করো

```bash
# .env git-এ নেই তো?
git status

# কোনো key accidentally staged হয়নি তো?
git diff --cached | grep -i "key\|token\|password\|secret"
```

---

## .gitignore-এ এগুলো থাকতে হবে (ইতিমধ্যে আছে)

```
.env
.env.local
.env.production
*.secret
```

---

## Deployment-এ Secret দেওয়ার সঠিক উপায়

| Platform | কীভাবে |
|----------|--------|
| Vercel | Dashboard → Settings → Environment Variables |
| Railway | Variables tab-এ add করো |
| Render | Environment tab-এ add করো |
| Supabase Edge Functions | `supabase secrets set KEY=value` |

---

## মনে রাখো

```
Code-এ secret দেখলে judge → সরাসরি DQ
Git history-তে secret থাকলেও ধরা যায়
"আমি delete করেছি" — git history-তে থেকে যায়!

তাই শুরু থেকেই সঠিকভাবে করো।
```
