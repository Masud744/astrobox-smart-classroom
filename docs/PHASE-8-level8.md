# Phase 8 — Level 8: Anti-Spoof + Lockout (30 marks)
**Call 4 — যদি সময় থাকে**

---

## 📋 Task List

- [ ] **Anti-spoof detection**:
  - Face mismatch → save private snapshot + alert Teacher within 10s
  - Liveness check: photo shown on phone → system refuses

- [ ] **Card lockout**:
  - 3 mismatches in 2 min → card blocked 5 min
  - Admin can unblock via dashboard

- [ ] **Alert Ack race condition**:
  - Multiple teachers ack same alert → exactly 1 ack recorded (DB transaction)

- [ ] **Student deletion**:
  - Delete student → face embedding + card link + snapshots → all gone
  - Audit log: who deleted, when
  - Attendance records: anonymized (not deleted)

---

## 🧪 Verification Test

```
[ ] Mismatch 3x within 2 min → card blocked 5 min
[ ] Admin unblocks → works again
[ ] Show phone photo to camera → denied (liveness check)
[ ] Two teachers ack same alert simultaneously → exactly 1 ack in DB
[ ] Delete student → face, card, snapshots gone → audit log has entry
[ ] Attendance still shows anonymized record
```

## ✅ Done হলে

```bash
git add -A
git commit -m "level-08: anti-spoof liveness, card lockout 3fails 2min, alert ack idempotent, student deletion audit"
git tag level-08
git push origin main --tags
```

