# FastRobox Smart Classroom

**AI-Powered Smart Classroom: Safety, Access and Vision**
FastRobox 1.0 Techathon Grand Finale — BUBT

---

## Project Structure

```
firmware/main/     ESP32-S3-CAM Arduino firmware
backend/           Node.js REST API (Supabase)
dashboard/         Web dashboard (HTML/JS)
.skills/           Competition guidelines and checklists
plan.md            Full project plan (5 phases)
rulebook.pdf       Official competition rulebook
GEMINI.md          AI assistant project rules
```

## Setup

### Backend
```bash
cd backend
cp .env.example .env
# fill in SUPABASE_URL and SUPABASE_ANON_KEY
npm install
npm run dev
```

### Database
Run `backend/supabase/schema.sql` in Supabase SQL Editor.

### Firmware
1. Install Arduino libraries: `SmartProv`, `MFRC522`, `DHT sensor library`, `ArduinoJson`
2. Open `firmware/main/main.ino` in Arduino IDE
3. Board: `ESP32S3 Dev Module`
4. Flash and open Serial Monitor (115200 baud)
5. Connect to `FastRobox-Setup` WiFi → configure via captive portal

### Dashboard
Open `dashboard/index.html` or deploy to Vercel/Netlify.

## Git Tags

| Tag | Level |
|-----|-------|
| `level-01` | Level 1 |
| `level-02` | Level 2 |
| ... | ... |
