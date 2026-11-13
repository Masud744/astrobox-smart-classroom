# Phase 6 — Level 6: Weather + History + AI (40 marks)
**Call 3**

---

## 📋 Task List

### Backend — `backend/main.py`

- [ ] **Weather service** (OpenWeatherMap free tier):
  ```python
  # OPENWEATHER_API_KEY env var (never reaches browser)
  GET /api/weather → cached response (refresh every 15 min)
  # Cache in weather_cache table
  # If stale (service down) → return last cached + "stale_since": "..."
  ```

- [ ] **WEATHER_ALERT push**: dangerous notice → `events`-এ insert → device পরের settings poll-এ পাবে

- [ ] **`GET /api/events`**: filter by `?type=FIRE&from=...&to=...`

- [ ] **`GET /api/report/daily`**: `?date=2026-11-13`
  - min/max/avg temp + humidity
  - alarm counts by type
  - attendance totals (Present/Late/Absent)

- [ ] **CSV export**:
  - `GET /api/export/events.csv`
  - `GET /api/export/telemetry.csv`
  - `GET /api/export/report.csv`

- [ ] **AI assistant**:
  ```python
  POST /api/ai/query {question: "highest temp today?"}
  # 1. Parse question → DB query (safe, parameterized)
  # 2. Format result
  # 3. Pass to Gemini API for natural language response
  # 4. NEVER invent data — if no data: return "No data available"
  # GEMINI_API_KEY env var
  ```

### Dashboard

- [ ] **Weather panel**: current temp, humidity, description + 24h forecast
- [ ] **Notice board**: warnings (storm/cyclone/rain/heatwave)
- [ ] **WEATHER_ALERT** → red/purple banner within 10s
- [ ] **Sensor chart**: temp/humidity/heat index (selectable: 1h / 6h / 24h)
- [ ] **Event history**: table with type + time filter
- [ ] **Daily report view**: summary table
- [ ] **CSV export buttons**: "Download CSV"
- [ ] **AI chat box**: English + Bangla input support

### Firmware

- [ ] `led.cpp` → Purple 2 Hz + 3 beeps = `WEATHER_ALERT`
- [ ] `local_server.cpp` → current weather one-liner on status page
- [ ] `cloud.cpp` → check for `weather_alert` field in settings response

---

## 🧪 Verification Test

```
[ ] Weather panel showing current data (not empty)
[ ] Admin adds TEST notice via API → purple LED + 3 beeps within 10s
[ ] Weather API key removed → panel shows "Data stale (Xm ago)" not blank/error
[ ] Chart shows last 24h data as a line graph
[ ] Event history → filter FIRE → CSV export matches page exactly (same rows)
[ ] AI: "highest temp today?" → correct number from DB
[ ] AI: "data for yesterday?" → "No data available" (no hallucination!)
[ ] AI: English + Bangla question → both work
```

## ✅ Done হলে

```bash
git add -A
git commit -m "level-06: weather panel, notice board, charts, event history, CSV export, AI assistant grounded on DB"
git tag level-06
git push origin main --tags
```

