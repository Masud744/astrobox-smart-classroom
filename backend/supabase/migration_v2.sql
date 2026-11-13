-- ============================================================
-- FastRobox 1.0 — Schema Migration
-- Run this in Supabase SQL Editor (project: fastrobox-smart-classroom)
-- ============================================================

-- ── Fix 1: Add 'unknown_card' to alerts constraint ──────────
ALTER TABLE alerts DROP CONSTRAINT IF EXISTS alerts_alert_type_check;
ALTER TABLE alerts ADD CONSTRAINT alerts_alert_type_check
  CHECK (alert_type IN (
    'flame', 'panic', 'temp_high', 'hum_high',
    'offline', 'unknown_face', 'unknown_card'
  ));

-- ── Fix 2: New telemetry table (Level 2) ────────────────────
CREATE TABLE IF NOT EXISTS telemetry (
  id           bigserial primary key,
  device_id    text not null,
  room_id      text,
  seq          integer not null,
  ts           bigint,
  uptime_s     integer,
  temp_c       numeric(5,2),
  rh           numeric(5,2),
  heat_index_c numeric(5,2),
  flame        boolean default false,
  door         text,
  status       text,
  heat_trend   text,
  rssi         integer,
  heap_kb      integer,
  chip_temp_c  numeric(5,2),
  fw           text,
  quality      text default 'good',
  logged_at    timestamptz default now(),
  UNIQUE(device_id, seq)
);
ALTER TABLE telemetry ENABLE ROW LEVEL SECURITY;
-- Backend service role only (anon cannot read/write directly)
CREATE POLICY IF NOT EXISTS "service_only" ON telemetry USING (false);

-- ── Fix 3: New events table (Level 2) ───────────────────────
CREATE TABLE IF NOT EXISTS events (
  id        bigserial primary key,
  device_id text,
  room_id   text,
  seq       integer,
  ts        bigint,
  type      text not null,   -- BOOT, CARD_TAP, FIRE, PANIC, etc.
  detail    jsonb,
  logged_at timestamptz default now()
);
ALTER TABLE events ENABLE ROW LEVEL SECURITY;
CREATE POLICY IF NOT EXISTS "service_only" ON events USING (false);

-- ── Level 4 tables (run when starting Phase 4) ──────────────

-- Users table
CREATE TABLE IF NOT EXISTS users (
  id            uuid primary key default gen_random_uuid(),
  email         text unique not null,
  password_hash text not null,
  role          text not null check (role in ('admin','teacher','student','guardian')),
  locked_until  timestamptz,
  created_at    timestamptz default now()
);
ALTER TABLE users ENABLE ROW LEVEL SECURITY;

-- Login attempts (for lockout)
CREATE TABLE IF NOT EXISTS login_attempts (
  id      bigserial primary key,
  user_id uuid references users(id),
  ts      timestamptz default now(),
  success boolean default false,
  ip      text
);

-- Guardian → Student link
CREATE TABLE IF NOT EXISTS guardian_student (
  guardian_id uuid references users(id),
  student_id  uuid references users(id),
  primary key (guardian_id, student_id)
);

-- ── Level 5: Settings change log ────────────────────────────
CREATE TABLE IF NOT EXISTS settings_log (
  id         bigserial primary key,
  changed_by uuid references users(id),
  field      text,
  old_value  text,
  new_value  text,
  changed_at timestamptz default now()
);

-- ── Level 6: Weather cache ───────────────────────────────────
CREATE TABLE IF NOT EXISTS weather_cache (
  id          bigserial primary key,
  city        text,
  data        jsonb,
  fetched_at  timestamptz default now()
);

-- ── Level 7: Students + Face + Attendance ───────────────────
CREATE TABLE IF NOT EXISTS students (
  id             uuid primary key default gen_random_uuid(),
  name           text not null,
  face_embedding jsonb,  -- embedding vector only, NO images
  created_at     timestamptz default now()
);

CREATE TABLE IF NOT EXISTS card_links (
  card_uid      text primary key,
  student_id    uuid references students(id),
  registered_by uuid references users(id),
  registered_at timestamptz default now()
);

CREATE TABLE IF NOT EXISTS classes (
  id         bigserial primary key,
  room_id    text,
  teacher_id uuid references users(id),
  name       text,
  start_time time not null,
  end_time   time not null,
  days       text[],  -- e.g. {'MON','WED','FRI'}
  active     boolean default true
);

CREATE TABLE IF NOT EXISTS class_students (
  class_id   bigint references classes(id),
  student_id uuid references students(id),
  primary key (class_id, student_id)
);

-- Drop and recreate attendance to match Level 7 format
DROP TABLE IF EXISTS attendance;
CREATE TABLE attendance (
  id         bigserial primary key,
  class_id   bigint references classes(id),
  student_id uuid references students(id),
  date       date not null,
  status     text check (status in ('Present','Late','Absent')),
  checked_at timestamptz,
  logged_at  timestamptz default now(),
  UNIQUE(class_id, student_id, date)
);
ALTER TABLE attendance ENABLE ROW LEVEL SECURITY;
