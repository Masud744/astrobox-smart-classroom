# Level 6 — Weather Service, Analytics & AI Assistant Writeup
**Team: T07 | FastRobox 1.0 | BUBT Techathon Grand Finale**

---

## 1. Level Information

| Field | Value |
|-------|-------|
| **Level** | Level 6 |
| **Title** | Weather Integration, Data Analytics & AI Assistant |
| **Marks** | __ / 40 |
| **Call Number** | Call 3 |
| **Pass Time** | __ : __ |
| **Time Taken** | __ minutes |

---

## 2. What We Built

Level 6 required integration with a weather service, data charts, event history with filtering, daily reports, CSV export, and a grounded AI assistant that answers questions strictly from database data.

---

## 3. Features Implemented

| Feature | Status | Notes |
|---------|--------|-------|
| Weather service (OpenWeatherMap) | ✅ Done | 15-min cache, server-side key |
| Weather panel (current + 24h forecast) | ✅ Done | |
| Stale data notice | ✅ Done | "Data stale (Xm ago)" |
| WEATHER_ALERT push to device | ✅ Done | Purple LED + 3 beeps |
| Sensor chart (1h/6h/24h) | ✅ Done | Temp, humidity, heat index |
| Event history with filter | ✅ Done | Type + time filter |
| Daily report endpoint | ✅ Done | Min/max/avg + counts |
| CSV export (events, telemetry, report) | ✅ Done | |
| AI assistant (Gemini API) | ✅ Done | Grounded on DB, no hallucination |
| AI: English + Bangla input | ✅ Done | |
| Firmware WEATHER_ALERT handling | ✅ Done | Purple 2 Hz + 3 beeps |
| Local status page weather line | ✅ Done | |

---

## 4. How It Works

### Backend

The weather service fetches data from OpenWeatherMap every 15 minutes and caches it in the `weather_cache` table. The API key is server-side only and never sent to the browser. If the external service is down, the last cached response is returned with a `stale_since` timestamp so the UI can show "Data stale".

The AI assistant (`/api/ai/query`) parses the question, runs a safe parameterized DB query, formats the result into a prompt, and passes it to the Gemini API. If no data is found, it returns "No data available" — it never invents data. Both English and Bangla questions are accepted.

---

## 5. Demo Steps Performed

1. Weather panel showed current temperature, humidity, and weather description
2. Admin triggered TEST weather alert via API → device showed purple LED + 3 beeps within 10s
3. Removed OpenWeatherMap API key → panel showed "Data stale (Xm ago)" instead of error
4. Sensor chart showed last 24h temp/humidity/heat index as line graphs
5. Filtered event history to FIRE → exported CSV → rows matched the page exactly
6. AI: asked "highest temp today?" → got correct number from DB
7. AI: asked "data for yesterday?" → got "No data available" (no hallucination)
8. AI: asked question in Bangla → got correct answer

---

## 6. Evidence

| Item | File Name |
|------|-----------|
| Video | `T07_L06_video.mp4` |
| Writeup PDF | `T07_L06_writeup.pdf` |
| Git Tag | `level-06` |
| Commit | [paste commit hash here] |

---

## 7. Any Deviations from Spec

- None

---

## 8. Lessons Learned

[Fill after competition]
