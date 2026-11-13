# Level 8 — Anti-Spoof & Card Lockout Writeup
**Team: T07 | FastRobox 1.0 | BUBT Techathon Grand Finale**

---

## 1. Level Information

| Field | Value |
|-------|-------|
| **Level** | Level 8 |
| **Title** | Anti-Spoofing, Lockout & Secure Deletion |
| **Marks** | __ / 30 |
| **Call Number** | Call 4 |
| **Pass Time** | __ : __ |
| **Time Taken** | __ minutes |

---

## 2. What We Built

Level 8 required anti-spoofing (liveness detection to reject printed photos), automatic card lockout after 3 mismatches in 2 minutes, idempotent alert acknowledgment, and secure student deletion with an audit trail.

---

## 3. Features Implemented

| Feature | Status | Notes |
|---------|--------|-------|
| Anti-spoof: mismatch → private snapshot | ✅ Done | Saved server-side, never public |
| Teacher alert on mismatch within 10s | ✅ Done | Telegram |
| Liveness check (photo on phone rejected) | ✅ Done | Texture/reflection analysis |
| Card lockout: 3 mismatches in 2 min | ✅ Done | 5-min block |
| Admin unblock via dashboard | ✅ Done | |
| Alert Ack: race condition handled | ✅ Done | DB transaction, 1 ack only |
| Student deletion: all data removed | ✅ Done | Face, card, snapshots |
| Attendance records anonymized on delete | ✅ Done | Not deleted, just anonymized |
| Audit log for deletions | ✅ Done | who/when recorded |

---

## 4. How It Works

### Anti-Spoof

When a face mismatch occurs, the server saves a private JPEG snapshot (never accessible publicly) and sends a Telegram alert to the Teacher within 10 seconds. The snapshot is associated with the card UID that was used. For liveness detection, we use texture analysis — printed photos on phone screens have different reflection/texture characteristics than real faces. The check runs server-side on the received JPEG.

### Lockout

The backend tracks mismatches in the `card_mismatches` table with a timestamp. If 3 mismatches occur within 2 minutes for the same `card_uid`, the card is blocked for 5 minutes in `card_links` with a `blocked_until` field. The check-in endpoint checks this field first. Admins can unblock a card from the dashboard.

### Alert Ack Race Condition

Alert acknowledgment uses a database transaction with `UPDATE ... WHERE acked_at IS NULL RETURNING id`. Only the first UPDATE that matches returns a row; subsequent concurrent requests see no rows returned and return a 409. This ensures exactly one acknowledgment is recorded even under concurrent requests.

### Student Deletion

Deleting a student triggers: (1) face embedding cleared, (2) card_link removed, (3) private snapshots deleted from storage, (4) attendance records updated to `student_id = NULL` with `anonymized = true`. An audit log entry is created with the admin's user ID, student name, and timestamp.

---

## 5. Demo Steps Performed

1. Showed phone photo to camera during check-in → DENIED (liveness check)
2. Tried 3 mismatches in 2 minutes → card blocked for 5 min → denied even with correct card
3. Admin unblocked card → worked again
4. Two team members simultaneously acknowledged same alert → DB showed exactly 1 ack
5. Admin deleted student → face, card, snapshots gone → audit log showed deletion
6. Attendance history still showed anonymized record (not deleted)

---

## 6. Evidence

| Item | File Name |
|------|-----------|
| Video | `T07_L08_video.mp4` |
| Writeup PDF | `T07_L08_writeup.pdf` |
| Git Tag | `level-08` |
| Commit | [paste commit hash here] |

---

## 7. Any Deviations from Spec

- None

---

## 8. Lessons Learned

[Fill after competition]
