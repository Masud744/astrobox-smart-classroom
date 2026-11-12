require('dotenv').config();
const express = require('express');
const cors = require('cors');
const { createClient } = require('@supabase/supabase-js');

const app = express();
app.use(cors());
app.use(express.json());

const db = createClient(process.env.SUPABASE_URL, process.env.SUPABASE_ANON_KEY);

// ── Health check ───────────────────────────────────────────
app.get('/', (req, res) => res.json({ status: 'ok', project: 'fastrobox-classroom' }));

// ── Settings (ESP32 polls this every 10s) ──────────────────
app.get('/api/settings', async (req, res) => {
  const { data, error } = await db.from('settings').select('*').eq('id', 1).single();
  if (error) return res.status(500).json({ error: error.message });
  res.json(data);
});

app.patch('/api/settings', async (req, res) => {
  const allowed = ['room_id','class_start','temp_max','hum_max','buzzer_enabled','door_locked'];
  const updates = {};
  for (const key of allowed) {
    if (req.body[key] !== undefined) updates[key] = req.body[key];
  }
  updates.updated_at = new Date().toISOString();
  const { data, error } = await db.from('settings').update(updates).eq('id', 1).select().single();
  if (error) return res.status(500).json({ error: error.message });
  res.json(data);
});

// ── Sensor data ────────────────────────────────────────────
app.post('/api/sensor', async (req, res) => {
  const { room_id, temperature, humidity, flame, wifi_rssi } = req.body;
  const { error } = await db.from('sensor_logs').insert({ room_id, temperature, humidity, flame, wifi_rssi });
  if (error) return res.status(500).json({ error: error.message });
  res.json({ ok: true });
});

app.get('/api/sensor/latest', async (req, res) => {
  const room = req.query.room_id || 'ROOM-01';
  const { data, error } = await db
    .from('sensor_logs').select('*')
    .eq('room_id', room)
    .order('logged_at', { ascending: false })
    .limit(1).single();
  if (error) return res.status(500).json({ error: error.message });
  res.json(data);
});

app.get('/api/sensor/history', async (req, res) => {
  const room = req.query.room_id || 'ROOM-01';
  const limit = parseInt(req.query.limit) || 50;
  const { data, error } = await db
    .from('sensor_logs').select('*')
    .eq('room_id', room)
    .order('logged_at', { ascending: false })
    .limit(limit);
  if (error) return res.status(500).json({ error: error.message });
  res.json(data);
});

// ── Attendance ─────────────────────────────────────────────
app.post('/api/attendance', async (req, res) => {
  const { room_id, card_uid, person_type, action, method, face_label } = req.body;
  const { error } = await db.from('attendance').insert({ room_id, card_uid, person_type, action, method, face_label });
  if (error) return res.status(500).json({ error: error.message });
  res.json({ ok: true });
});

app.get('/api/attendance', async (req, res) => {
  const room = req.query.room_id || 'ROOM-01';
  const search = req.query.search || '';
  let query = db.from('attendance').select('*').eq('room_id', room).order('logged_at', { ascending: false });
  if (search) query = query.ilike('card_uid', `%${search}%`);
  const { data, error } = await query.limit(100);
  if (error) return res.status(500).json({ error: error.message });
  res.json(data);
});

// ── Door events ────────────────────────────────────────────
app.post('/api/door', async (req, res) => {
  const { room_id, state, trigger } = req.body;
  // update settings too
  await db.from('settings').update({ door_locked: state === 'locked' }).eq('id', 1);
  const { error } = await db.from('door_events').insert({ room_id, state, trigger });
  if (error) return res.status(500).json({ error: error.message });
  res.json({ ok: true });
});

app.get('/api/door/history', async (req, res) => {
  const room = req.query.room_id || 'ROOM-01';
  const { data, error } = await db
    .from('door_events').select('*')
    .eq('room_id', room)
    .order('logged_at', { ascending: false })
    .limit(50);
  if (error) return res.status(500).json({ error: error.message });
  res.json(data);
});

// ── Alerts ─────────────────────────────────────────────────
app.post('/api/alert', async (req, res) => {
  const { room_id, alert_type } = req.body;
  const { error } = await db.from('alerts').insert({ room_id, alert_type });
  if (error) return res.status(500).json({ error: error.message });
  res.json({ ok: true });
});

app.get('/api/alerts', async (req, res) => {
  const room = req.query.room_id || 'ROOM-01';
  const { data, error } = await db
    .from('alerts').select('*')
    .eq('room_id', room)
    .order('logged_at', { ascending: false })
    .limit(50);
  if (error) return res.status(500).json({ error: error.message });
  res.json(data);
});

// ── Heartbeat ──────────────────────────────────────────────
app.post('/api/heartbeat', async (req, res) => {
  const { room_id, firmware_version, ip_address, uptime_sec } = req.body;
  const { error } = await db.from('heartbeat').insert({ room_id, firmware_version, ip_address, uptime_sec });
  if (error) return res.status(500).json({ error: error.message });
  res.json({ ok: true });
});

// ── OTA firmware ───────────────────────────────────────────
app.get('/api/ota/latest', async (req, res) => {
  const { data, error } = await db
    .from('ota_firmware').select('*')
    .eq('active', true)
    .order('created_at', { ascending: false })
    .limit(1).single();
  if (error) return res.status(404).json({ update: false });
  res.json({ update: true, version: data.version, url: data.binary_url, notes: data.notes });
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`API running on port ${PORT}`));

