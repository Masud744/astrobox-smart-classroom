# Skill: Judging Call Strategy

## Purpose
পুরো ৭ ঘণ্টায় **মাত্র ৪টি judging call** আছে।
একটি call নষ্ট হলে সেটা ফেরত পাবে না।
তাই কখন call দেবে এটা সবচেয়ে গুরুত্বপূর্ণ সিদ্ধান্ত।

---

## Suggested Call Plan

| Call | Levels | Target Time | শর্ত |
|------|--------|-------------|------|
| **Call 1** | Level 1, 2, 3 | Hour 1.5–2 | Hardware setup + SmartProv কাজ করছে |
| **Call 2** | Level 4, 5, 6 | Hour 3–3.5 | সব sensor integrated |
| **Call 3** | Level 7, 8, 9 | Hour 5–5.5 | Dashboard + DB কাজ করছে |
| **Call 4** | Level 10, 11, 12 | Hour 6–6.5 | বাফার ৩০ মিনিট রেখে দাও |

---

## Call দেওয়ার আগে Checklist

**একটি call দেওয়ার আগে নিশ্চিত করো:**
- [ ] ঘোষণা করা সব level লোকালি কমপক্ষে **১০ বার** সফলভাবে টেস্ট হয়েছে
- [ ] সব **৫টি display surface** খোলা আছে:
  - Serial Monitor ✅
  - Local status page ✅
  - Web Dashboard ✅
  - Light & Buzzer ✅
  - Database records ✅
- [ ] Video recording শুরু করা আছে
- [ ] Stopwatch রেডি
- [ ] Team ID এবং Level নম্বর মুখে বলার জন্য রেডি

---

## Call দেওয়ার সময় কী বলবে

```
"Judge, আমরা Level [X], [Y], [Z] এর জন্য judging call দিচ্ছি।"
```

তারপর judge ক্রমানুসারে level test করবেন।

---

## মাঝপথে Level Fail করলে

```
উদাহরণ: Level 1, 2, 3 ঘোষণা করেছ।
Level 2-এ fail করলে → Level 3 আর এই call-এ judge হবে না।

তখন ২টা option:
  Option A: এখনই 2nd attempt নাও (code ঠিক না করে)
  Option B: এই call শেষ করো, পরের call-এ 2nd attempt নাও
            (কিন্তু সেটা একটা call খরচ হবে)
```

**Rule:** Attempt শুরু হলে মাঝপথে pause করা যাবে না।

---

## 2nd Attempt কখন নেবে?

```
✅ এখনই নাও যদি:
  → সমস্যাটা minor (timing issue, একটা variable)
  → ৩০ সেকেন্ডে ঠিক করা যাবে

❌ পরে নাও যদি:
  → সমস্যাটা বড় (logic error, sensor wiring)
  → সময় লাগবে debug করতে
  (কিন্তু পরে নিলে একটা call খরচ হবে মনে রেখো)
```

---

## Closure Rule — সবচেয়ে ভয়ঙ্কর নিয়ম

```
কোনো level-এ 1st + 2nd — দুটো attempt-ই fail
           ↓
সেই level + তার পরের সব level চিরতরে CLOSED
           ↓
আর কোনো level-এ অংশ নেওয়া যাবে না
```

**তাই: নিশ্চিত না হলে call দিও না। আরেকটু সময় নাও।**

---

## Time Management

```
৭ ঘণ্টা = ৪২০ মিনিট

ভাগ করলে:
  Hardware setup + SmartProv config → ৩০-৪৫ min
  প্রতি level group → ~৬০-৭৫ min
  Evidence per level → ৩০ min
  Buffer → ৩০ min

Call-এর মাঝে idle থাকবে না — পরের level-এর জন্য test করো।
```

---

## Emergency Situations

| সমস্যা | করণীয় |
|--------|--------|
| ESP32 crash করছে | Serial Monitor দেখো, watchdog reset কিনা |
| WiFi disconnect | SmartProv offline mode আছে কিনা চেক করো |
| Cloud down | Local status page দিয়ে চালিয়ে যাও |
| Call শেষে ভুলে গেছ evidence দিতে | ৩০ মিনিট আছে — rush করো! |
| ৪টি call শেষ | থামো, evidence ঠিকঠাক জমা দাও |

