# Skill: Evidence Checklist (প্রতিটি Level-এর পর)

## Purpose
প্রতিটি level পাসের পর **৩০ মিনিটের মধ্যে** সব evidence জমা দিতে হবে।
এই checklist follow না করলে সেই level-এর নম্বর **০ হয়ে যাবে।**

---

## ⚡ Level পাসের সাথে সাথে (live-এ):

- [ ] জাজ চলে যাওয়ার আগেই Git commit করো
- [ ] Level tag করো:
  ```bash
  git add .
  git commit -m "level-NN: <কী করলে>"
  git tag level-NN
  git push origin main --tags
  ```
- [ ] README আপডেট করো (Level 2 থেকে বাধ্যতামূলক)
- [ ] Video recording বন্ধ করো (এখনই!)

---

## 📹 Video Checklist (max 10 min, max 3 clips, min 720p)

ভিডিওতে অবশ্যই থাকতে হবে:
- [ ] শুরুতে দলের একজন **Team ID ও Level নম্বর মুখে বলছে**
- [ ] সব **"Video must show"** আইটেম দেখানো হয়েছে
- [ ] **Serial Monitor** দেখা যাচ্ছে
- [ ] **Local status page** দেখা যাচ্ছে
- [ ] **Web Dashboard** দেখা যাচ্ছে
- [ ] **Buzzer / Light** দেখা যাচ্ছে
- [ ] **Database records** দেখা যাচ্ছে
- [ ] **Stopwatch / ঘড়ি** দেখা যাচ্ছে (timing-এর প্রমাণ)
- [ ] ভিডিও unedited (কাটছাঁট নেই, শুধু ক্লিপ আলাদা করা যাবে)

**ফাইলনাম:** `<TeamID>_L<NN>_video.mp4`
*(উদাহরণ: `T07_L03_video.mp4`)*

---

## 📄 Write-up Checklist (max 2 pages, PDF)

৫টি section অবশ্যই থাকতে হবে:

### (1) Design
- [ ] কী তৈরি করা হয়েছে তার বিবরণ
- [ ] Architecture diagram বা flow chart

### (2) Where it shows
- [ ] Serial Monitor screenshot
- [ ] Local status page screenshot
- [ ] Web Dashboard screenshot
- [ ] Database records screenshot

### (3) Test results
- [ ] প্রতিটি live check-এর ফলাফল
- [ ] Video timestamp উল্লেখ (ভিডিওতে কত সেকেন্ডে দেখা গেছে)
- [ ] Measurement table (measured value vs required limit)

### (4) Problems and fixes
- [ ] কোন সমস্যা হয়েছিল?
- [ ] কীভাবে সমাধান করা হয়েছে?

### (5) Extra item
- [ ] সেই level-এর নির্দিষ্ট অতিরিক্ত বিষয় (প্রশ্নপত্রে দেওয়া থাকবে)

**ফাইলনাম:** `<TeamID>_L<NN>_writeup.pdf`
*(উদাহরণ: `T07_L03_writeup.pdf`)*

---

## 📤 Upload Checklist

- [ ] Video file আপলোড হয়েছে (সঠিক নামে)
- [ ] Write-up PDF আপলোড হয়েছে (সঠিক নামে)
- [ ] Git tag push হয়েছে (`git push --tags`)
- [ ] সব কিছু **৩০ মিনিটের মধ্যে** জমা দেওয়া হয়েছে

---

## ⏱️ Time Budget per Level

```
Level pass হওয়ার পর:
  0-5 min  → Git commit + tag + push
  5-15 min → Write-up fill করো (সাথে সাথে মনে থাকবে)
  15-25 min → Video export + rename
  25-30 min → সব upload
  
  30 min পার হলে → সেই level-এর নম্বর 0!
```

