# Level 4 — Platform, Roles & Alerts Writeup
**Team: T07 | FastRobox 1.0 | BUBT Techathon Grand Finale**

---

## 1. Level Information

| Field | Value |
|-------|-------|
| **Level** | Level 4 |
| **Title** | Multi-Role Platform, Auth & Phone Alerts |
| **Marks** | __ / 40 |
| **Call Number** | Call 2 |
| **Pass Time** | __ : __ |
| **Time Taken** | __ minutes |

---

## 2. What We Built

Level 4 required a full authentication system with 4 roles (Admin, Teacher, Student, Guardian), login lockout after 5 fails (30s), role-based access control, Telegram phone alerts for FIRE/PANIC/DANGER, English/Bangla language toggle, and a device health panel.

---

## 3. Features Implemented

| Feature | Status | Notes |
|---------|--------|-------|
| 4 roles: admin/teacher/student/guardian | ✅ Done | |
| `POST /api/auth/login` with bcrypt | ✅ Done | JWT token returned |
| Login lockout: 5 fails → 30s | ✅ Done | `locked_until` field |
| JWT middleware on all protected routes | ✅ Done | |
| Role-based routing in dashboard | ✅ Done | |
| Guardian → Student link | ✅ Done | `guardian_student` table |
| Telegram Bot alerts | ✅ Done | FIRE/PANIC within 10s |
| English/Bangla toggle (i18n) | ✅ Done | 2+ pages |
| Device health panel | ✅ Done | uptime, RSSI, heap, chip temp |
| FIRE/PANIC red banner (all users) | ✅ Done | Realtime via Supabase |
| Reset fire button (Teacher/Admin) | ✅ Done | |
| Login history page (Admin) | ✅ Done | |

---

## 4. How It Works

### Backend

Authentication uses bcrypt password hashing with JWT tokens (HS256). Failed login attempts are tracked in a `login_attempts` table. After 5 consecutive failures, `locked_until` is set to `now() + 30s`. A locked account returns 423 Locked. All endpoints check the JWT and verify the user's role before serving data.

Telegram alerts are sent via Bot API using `httpx` async requests within 10 seconds of a FIRE, PANIC, or DANGER event. The bot token and chat IDs are stored in environment variables.

### Dashboard

The dashboard uses a `useTranslation()` hook with `src/translations/en.js` and `src/translations/bn.js`. Switching language updates all UI strings instantly using React context. The device health panel polls `/api/settings` and `/api/sensor/latest` every 5s and displays uptime, WiFi RSSI, heap memory, chip temperature, and firmware version.

---

## 5. Demo Steps Performed

1. Logged in as Admin, Teacher, Student, Guardian in 4 different tabs
2. Tried to access Teacher page as Student → got "Access Denied"
3. Guardian tab → only saw linked student's attendance, not all students
4. Triggered PANIC → red banner appeared on all tabs within 3s + Telegram message within 10s
5. Entered wrong password 5 times → got 30s lockout → correct pass rejected → waited 30s → accepted
6. Switched to Bangla → all labels on 2+ pages changed to Bangla
7. Admin → Login History page → all attempt records visible

---

## 6. Evidence

| Item | File Name |
|------|-----------|
| Video | `T07_L04_video.mp4` |
| Writeup PDF | `T07_L04_writeup.pdf` |
| Git Tag | `level-04` |
| Commit | [paste commit hash here] |

---

## 7. Any Deviations from Spec

- None

---

## 8. Lessons Learned

[Fill after competition]
