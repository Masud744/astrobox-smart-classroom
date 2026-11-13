# Phase 4 — Level 4: Platform, Roles, Alerts (40 marks)
**Call 2**

---

## 📋 Task List

### Backend — `backend/main.py`

- [ ] **Users table** (Supabase SQL):
  ```sql
  CREATE TABLE users (
    id uuid primary key default gen_random_uuid(),
    email text unique not null,
    password_hash text not null,
    role text check (role in ('admin','teacher','student','guardian')),
    locked_until timestamptz,
    created_at timestamptz default now()
  );
  ```

- [ ] **`POST /api/auth/login`** → bcrypt verify → JWT token return
  - 5 fails → `locked_until = now() + 30s`
  - locked → 423 error (Locked)

- [ ] **`GET /api/auth/me`** → JWT decode → user info

- [ ] **JWT middleware** → সব protected route-এ

- [ ] **Login attempts table** tracking

- [ ] **`guardian_student` table** (Guardian → Student link)

- [ ] **Telegram Bot API** integration:
  ```python
  # TELEGRAM_BOT_TOKEN env var
  # FIRE/PANIC/DANGER → send message within 10s
  import httpx
  async def telegram_alert(msg: str):
      url = f"https://api.telegram.org/bot{TOKEN}/sendMessage"
      await httpx.post(url, json={"chat_id": CHAT_ID, "text": msg})
  ```

- [ ] **Login history**: `GET /api/auth/history` (Admin only)

### Dashboard — `dashboard/src/`

- [ ] **Login page** (`LoginPage.jsx`): email + password form

- [ ] **JWT token** → localStorage, সব API call-এ `Authorization: Bearer <token>` header

- [ ] **Role-based routing**:
  - `admin` → সব page
  - `teacher` → room data + alerts + RFID register
  - `student` → শুধু নিজের attendance
  - `guardian` → linked children-এর status only

- [ ] **English/Bangla toggle** (i18n):
  - `src/translations/en.js` → সব UI string
  - `src/translations/bn.js` → বাংলা strings
  - `useTranslation()` hook সব component-এ

- [ ] **Device health panel**: uptime, RSSI, heap, chip temp, fw version, reset reason

- [ ] **FIRE/PANIC banner**: সব logged-in user-এর screen-এ red banner, 3s-এর মধ্যে

- [ ] **Reset fire button** (Teacher + Admin only)

- [ ] **Login history page** (Admin only)

---

## 🧪 Verification Test

```
[ ] 4টা আলাদা browser tab → 4টা role দিয়ে login
[ ] Student → Teacher page-এ যেতে চাইলে → "Access Denied"
[ ] Dashboard → 3s-এর মধ্যে নতুন data দেখায়
[ ] PANIC trigger → red banner 3s + Telegram message 10s-এর মধ্যে
[ ] 5 wrong passwords → 30s lock → correct pass rejected → 30s পরে accepted
[ ] Language toggle → Bangla-তে 2+ page-এর সব label বাংলায়
[ ] Admin login history page → সব login attempt দেখাচ্ছে
```

## ✅ Done হলে

```bash
git add -A
git commit -m "level-04: auth roles JWT, login lockout 5fails 30s, Telegram alerts, i18n EN/BN, guardian role, login history"
git tag level-04
git push origin main --tags
```
