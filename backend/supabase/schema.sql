-- ============================================================
-- FastRobox Smart Classroom — Supabase Schema
-- Run this in: Supabase Dashboard → SQL Editor → New Query
-- ============================================================

-- ── 1. Settings (remote config for ESP32) ──────────────────
create table if not exists settings (
  id            serial primary key,
  room_id       text    not null default 'ROOM-01',
  class_start   time    not null default '08:00:00',
  temp_max      numeric not null default 35,
  hum_max       numeric not null default 80,
  buzzer_enabled boolean not null default true,
  door_locked   boolean not null default true,
  updated_at    timestamptz not null default now()
);

-- seed one default row
insert into settings (id) values (1)
  on conflict (id) do nothing;

-- ── 2. Sensor Logs ─────────────────────────────────────────
create table if not exists sensor_logs (
  id          bigserial primary key,
  room_id     text        not null,
  temperature numeric,
  humidity    numeric,
  flame       boolean     not null default false,
  wifi_rssi   integer,
  logged_at   timestamptz not null default now()
);

-- ── 3. Attendance ──────────────────────────────────────────
create table if not exists attendance (
  id          bigserial primary key,
  room_id     text        not null,
  card_uid    text        not null,
  person_type text        not null check (person_type in ('student','teacher','unknown')),
  action      text        not null check (action in ('check_in','check_out','denied')),
  method      text        not null check (method in ('rfid','face','manual')),
  face_label  text,
  logged_at   timestamptz not null default now()
);

-- ── 4. Door Events ─────────────────────────────────────────
create table if not exists door_events (
  id        bigserial primary key,
  room_id   text        not null,
  state     text        not null check (state in ('locked','unlocked')),
  trigger   text        not null check (trigger in ('rfid','dashboard','auto','panic')),
  logged_at timestamptz not null default now()
);

-- ── 5. Alerts ──────────────────────────────────────────────
create table if not exists alerts (
  id          bigserial primary key,
  room_id     text        not null,
  alert_type  text        not null check (alert_type in ('flame','panic','temp_high','hum_high','offline','unknown_face')),
  resolved    boolean     not null default false,
  logged_at   timestamptz not null default now()
);

-- ── 6. Device Heartbeat ────────────────────────────────────
create table if not exists heartbeat (
  id          bigserial primary key,
  room_id     text        not null,
  firmware_version text,
  ip_address  text,
  uptime_sec  integer,
  logged_at   timestamptz not null default now()
);

-- ── 7. OTA Firmware ────────────────────────────────────────
create table if not exists ota_firmware (
  id          serial primary key,
  version     text        not null,
  binary_url  text        not null,
  notes       text,
  active      boolean     not null default false,
  created_at  timestamptz not null default now()
);

-- ── Indexes for fast queries ────────────────────────────────
create index if not exists idx_sensor_room    on sensor_logs  (room_id, logged_at desc);
create index if not exists idx_attend_room    on attendance   (room_id, logged_at desc);
create index if not exists idx_door_room      on door_events  (room_id, logged_at desc);
create index if not exists idx_alerts_room    on alerts       (room_id, logged_at desc);

-- ── Enable Row Level Security (RLS) ────────────────────────
-- Keep data readable via anon key (for dashboard)
alter table settings       enable row level security;
alter table sensor_logs    enable row level security;
alter table attendance     enable row level security;
alter table door_events    enable row level security;
alter table alerts         enable row level security;
alter table heartbeat      enable row level security;
alter table ota_firmware   enable row level security;

-- Allow public read + insert via anon key
create policy "public read"  on settings     for select using (true);
create policy "public read"  on sensor_logs  for select using (true);
create policy "public insert" on sensor_logs for insert with check (true);
create policy "public read"  on attendance   for select using (true);
create policy "public insert" on attendance  for insert with check (true);
create policy "public read"  on door_events  for select using (true);
create policy "public insert" on door_events for insert with check (true);
create policy "public read"  on alerts       for select using (true);
create policy "public insert" on alerts      for insert with check (true);
create policy "public read"  on heartbeat    for select using (true);
create policy "public insert" on heartbeat   for insert with check (true);
create policy "public read"  on ota_firmware for select using (true);
create policy "settings update" on settings for update using (true);
