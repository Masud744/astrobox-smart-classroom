# Level 7 — Card + Face Check-In & Attendance Writeup
**Team: T07 | FastRobox 1.0 | BUBT Techathon Grand Finale**

---

## 1. Level Information

| Field | Value |
|-------|-------|
| **Level** | Level 7 |
| **Title** | RFID + Face Biometric Check-In & Attendance |
| **Marks** | __ / 50 |
| **Call Number** | Call 3 |
| **Pass Time** | __ : __ |
| **Time Taken** | __ minutes |

---

## 2. What We Built

Level 7 required a dual-factor check-in system (RFID card + face recognition) with server-side face matching, class-aware attendance marking (Present/Late/Absent), and automatic Guardian notifications for absent students.

---

## 3. Features Implemented

| Feature | Status | Notes |
|---------|--------|-------|
| On-device face detection (crop only) | ✅ Done | 112×112 JPEG to server |
| `POST /api/checkin` {card_uid, face_jpg} | ✅ Done | |
| Server-side face matching | ✅ Done | DeepFace cosine similarity |
| 5s door unlock + auto-lock | ✅ Done | Solid green LED |
| Denied: 2 beeps + 3 red flashes | ✅ Done | |
| No face in 3s → NO_FACE denial | ✅ Done | |
| Students table (embedding only, no photos) | ✅ Done | |
| Card-student link table | ✅ Done | |
| Class schedule management | ✅ Done | |
| Attendance: Present (≤10 min) | ✅ Done | |
| Attendance: Late (>10 min) | ✅ Done | |
| Absent job (30 min after class start) | ✅ Done | Background task |
| Guardian Telegram on Late/Absent | ✅ Done | |
| Student enrolment UI | ✅ Done | 3+ photos |
| Card registration UI | ✅ Done | |
| Attendance table in dashboard | ✅ Done | |
| Guardian view: own children only | ✅ Done | |

---

## 4. How It Works

### Firmware

When a student taps their RFID card, the camera activates and tries to detect a face within 3 seconds. If a face is detected, it's cropped to 112×112 pixels and JPEG-encoded, then sent to `POST /api/checkin` along with the card UID. If no face is detected in 3 seconds, a `NO_FACE` denial is returned. No face recognition runs on the device — only detection and cropping.

### Backend

The check-in endpoint looks up the card UID in `card_links` to find the student, then compares the received face embedding against the stored embedding using cosine similarity with DeepFace. It then checks if the student is enrolled in a class running at the current time in the current room. If all checks pass, attendance is marked as Present (within 10 minutes of class start) or Late (after 10 minutes). A background job runs every 5 minutes; 30 minutes after a class starts, any enrolled student who hasn't checked in is marked Absent and a Telegram notification is sent to their linked Guardian.

---

## 5. Demo Steps Performed

1. Enrolled student via dashboard (3 photos) + registered RFID card
2. Student tap card → face detected → `[CAM] FACE w=112 h=112` → GRANTED → door 5s green → auto-lock
3. Covered camera for 3s → NO_FACE denial (2 beeps)
4. Used different student card (not in this class) → NOT IN THIS CLASS
5. Tried outside class hours → NO CLASS NOW
6. Used unregistered card → UNKNOWN CARD
7. Student checked in twice → attendance table showed only 1 record
8. Waited 30+ min without check-in → Guardian received Telegram: "Student is Absent"

---

## 6. Evidence

| Item | File Name |
|------|-----------|
| Video | `T07_L07_video.mp4` |
| Writeup PDF | `T07_L07_writeup.pdf` |
| Git Tag | `level-07` |
| Commit | [paste commit hash here] |

---

## 7. Any Deviations from Spec

- None

---

## 8. Lessons Learned

[Fill after competition]

