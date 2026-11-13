# Phase 7 — Level 7: Card + Face Check-In (50 marks)
**Call 3 — সবচেয়ে complex phase | সবাই একসাথে করো**

---

## 📋 Task List

### Firmware — `firmware/main/camera.cpp`

- [ ] **Face detection only** (on-device — full recognition নয়):
  ```cpp
  // Detect face → crop to 112×112 → JPEG encode → POST to server
  // [CAM] FACE w=112 h=112
  // 3s-এ face না পেলে → [EVENT] NO_FACE
  ```

### Firmware — `firmware/main/cloud.cpp`

- [ ] **`POST /api/checkin`** payload:
  ```json
  { "card_uid": "A3:4F:2B:11", "face_jpg": "<base64>" }
  ```
- [ ] Response handle করো:
  - `GRANTED` → LED solid green 5s, 1 beep, door open 5s, auto-lock
  - `DENIED`  → 2 beeps, 3 red flashes, door stays locked

### Backend — `backend/main.py`

- [ ] **`/api/students/enrol`** (POST):
  - 3+ photos → extract embedding (DeepFace) → store embedding only
  - Delete original photos immediately

- [ ] **`/api/cards/register`** (POST, Admin):
  - `{card_uid, student_id}` → `card_links` table

- [ ] **Class schedule** (`classes` + `class_students` tables)

- [ ] **`/api/checkin`** logic:
  ```
  1. card_uid → student lookup (card_links)
  2. Face embedding compare (cosine similarity > threshold)
  3. Student in running class? (time check)
  4. Mark attendance:
     - Within 10 min of start → "Present"
     - After 10 min          → "Late"
  5. Return: {result: "GRANTED"/"DENIED", reason: "..."}
  ```

- [ ] **Absent marking** (background job, runs every 5 min):
  - 30 min after class start → not checked-in → mark "Absent"
  - → Telegram to Guardian

### Dashboard

- [ ] **Student enrolment UI**: upload 3+ photos → enrol
- [ ] **Card registration UI**: enter UID → link to student
- [ ] **Class schedule management**: add/edit/delete
- [ ] **Attendance table**: Present / Late / Absent per class
- [ ] **Guardian view**: only own children's data
- [ ] **Face image log**: size in pixels only (no actual images stored)

---

## 🧪 Verification Test

```
[ ] Card tap → face detected → server grants → door 5s green → auto-lock
[ ] No face in 3s → "NO_FACE" (2 beeps, door stays locked)
[ ] Wrong class enrolled → "NOT IN THIS CLASS"
[ ] No class running → "NO CLASS NOW"
[ ] Unknown card → "UNKNOWN CARD"
[ ] Check-in A twice same class → door opens but only 1 attendance record
[ ] 30+ min pass without check-in → Guardian gets Telegram: "Student X is Absent"
[ ] Attendance table shows correct Present/Late/Absent
```

## ✅ Done হলে

```bash
git add -A
git commit -m "level-07: face crop on-device, /api/checkin, attendance Present/Late, absent job, guardian notify"
git tag level-07
git push origin main --tags
```
