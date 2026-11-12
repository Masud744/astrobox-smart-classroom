import React, { useState, useEffect, useCallback } from 'react';
import Sidebar from './components/Sidebar';
import Header from './components/Header';
import MetricCards from './components/MetricCards';
import VisualWidgets from './components/VisualWidgets';
import AttendanceLedger from './components/AttendanceLedger';
import AiCopilot from './components/AiCopilot';
import AuthModal from './components/AuthModal';
import SettingsModal from './components/SettingsModal';
import OtaModal from './components/OtaModal';
import { supabase, API_BASE } from './lib/supabase';
import { Calendar, RefreshCw } from 'lucide-react';

export default function App() {
  const [activeTab, setActiveTab] = useState('overview');
  const [searchQuery, setSearchQuery] = useState('');
  
  // Auth state
  const [currentUser, setCurrentUser] = useState(null);
  const [userRole, setUserRole] = useState('guest');
  const [isAuthModalOpen, setIsAuthModalOpen] = useState(false);
  const [isSettingsModalOpen, setIsSettingsModalOpen] = useState(false);
  const [isOtaModalOpen, setIsOtaModalOpen] = useState(false);

  // Live Telemetry & Settings
  const [settings, setSettings] = useState({
    room_id: 'ROOM-01',
    class_start: '08:00:00',
    temp_max: 35.0,
    hum_max: 80.0,
    buzzer_enabled: true,
    door_locked: true,
  });

  const [sensor, setSensor] = useState({
    temperature: 28.4,
    humidity: 61.5,
    flame: false,
    wifi_rssi: -54,
  });

  const [sensorHistory, setSensorHistory] = useState([]);
  const [attendanceLogs, setAttendanceLogs] = useState([]);
  const [alerts, setAlerts] = useState([]);
  const [wifiConnected, setWifiConnected] = useState(true);
  const [isRefreshing, setIsRefreshing] = useState(false);

  // ── Supabase Auth Listener ──────────────────────────────
  useEffect(() => {
    supabase.auth.getSession().then(({ data: { session } }) => {
      if (session?.user) {
        setCurrentUser(session.user);
        setUserRole(session.user.user_metadata?.role || 'teacher');
      }
    });

    const { data: { subscription } } = supabase.auth.onAuthStateChange((_event, session) => {
      if (session?.user) {
        setCurrentUser(session.user);
        setUserRole(session.user.user_metadata?.role || 'teacher');
      } else {
        // Fall back to guest if no session
        setCurrentUser(null);
        setUserRole('guest');
      }
    });

    return () => subscription.unsubscribe();
  }, []);

  // ── Polling & Data Synchronization ──────────────────────
  const fetchData = useCallback(async () => {
    setIsRefreshing(true);
    try {
      // 1. Settings
      const resSettings = await fetch(`${API_BASE}/api/settings`);
      if (resSettings.ok) {
        const s = await resSettings.json();
        setSettings(s);
      }

      // 2. Latest Sensor Reading
      const resSensor = await fetch(`${API_BASE}/api/sensor/latest?room_id=${settings.room_id || 'ROOM-01'}`);
      if (resSensor.ok) {
        const sen = await resSensor.json();
        setSensor({
          temperature: parseFloat(sen.temperature) || 28.4,
          humidity: parseFloat(sen.humidity) || 61.5,
          flame: !!sen.flame,
          wifi_rssi: sen.wifi_rssi ?? -54,
        });
        setWifiConnected(true);
      }

      // 3. Sensor History
      const resHistory = await fetch(`${API_BASE}/api/sensor/history?room_id=${settings.room_id || 'ROOM-01'}&limit=10`);
      if (resHistory.ok) {
        const hist = await resHistory.json();
        setSensorHistory(hist);
      }

      // 4. Attendance Ledger
      const resAttendance = await fetch(`${API_BASE}/api/attendance?room_id=${settings.room_id || 'ROOM-01'}&limit=50`);
      if (resAttendance.ok) {
        const att = await resAttendance.json();
        setAttendanceLogs(att);
      }

      // 5. Alerts
      const resAlerts = await fetch(`${API_BASE}/api/alerts?room_id=${settings.room_id || 'ROOM-01'}`);
      if (resAlerts.ok) {
        const alt = await resAlerts.json();
        setAlerts(alt);
      }
    } catch (err) {
      console.warn('Backend sync poll:', err.message);
      setWifiConnected(false);
    } finally {
      setIsRefreshing(false);
    }
  }, [settings.room_id]);

  useEffect(() => {
    fetchData();
    const timer = setInterval(fetchData, 4000);
    return () => clearInterval(timer);
  }, [fetchData]);

  // ── Toggle Door Action ──────────────────────────────────
  const handleToggleDoor = async () => {
    const newState = !settings.door_locked ? 'locked' : 'unlocked';
    // Optimistic UI update
    setSettings(prev => ({ ...prev, door_locked: newState === 'locked' }));

    try {
      await fetch(`${API_BASE}/api/door`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          room_id: settings.room_id || 'ROOM-01',
          state: newState,
          trigger: currentUser ? 'dashboard_teacher' : 'dashboard_quick'
        })
      });
    } catch (err) {
      console.error('Door toggle error:', err);
    }
  };

  const handleSignOut = async () => {
    await supabase.auth.signOut();
    setCurrentUser(null);
    setUserRole('guest');
  };

  // Stats calculation
  const attendanceStats = {
    rfid: attendanceLogs.filter(l => l.method === 'rfid').length,
    face: attendanceLogs.filter(l => l.method === 'face').length,
    denied: attendanceLogs.filter(l => l.action === 'denied').length,
  };

  return (
    <div className="min-h-screen bg-[#F4F5F7] p-4 md:p-8 flex items-center justify-center">
      {/* Master Application Window Container (Matching screenshot rounded-3xl aesthetic) */}
      <div className="w-full max-w-[1520px] bg-white rounded-3xl border border-[#EBECEF] shadow-xl overflow-hidden flex flex-col md:flex-row min-h-[920px]">
        {/* Left Sidebar */}
        <Sidebar
          activeTab={activeTab}
          setActiveTab={setActiveTab}
          alertCount={alerts.length}
          onOpenSettings={() => setIsSettingsModalOpen(true)}
          onOpenOta={() => setIsOtaModalOpen(true)}
        />

        {/* Main Content Area */}
        <div className="flex-1 flex flex-col overflow-y-auto">
          {/* Top Navigation Bar */}
          <Header
            searchQuery={searchQuery}
            setSearchQuery={setSearchQuery}
            wifiRssi={sensor.wifi_rssi}
            wifiConnected={wifiConnected}
            currentUser={currentUser}
            userRole={userRole}
            onOpenAuth={() => setIsAuthModalOpen(true)}
            onSignOut={handleSignOut}
          />

          {/* Subheader & Page Context */}
          <main className="p-8 flex-1">
            <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 mb-7 select-none">
              <div>
                <h2 className="text-[26px] font-extrabold text-[#111827] tracking-tight">
                  Dashboard
                </h2>
                <p className="text-[14px] text-[#6B7280] font-medium mt-0.5">
                  Real-time safety telemetry, access vision, and student attendance for {settings.room_id}.
                </p>
              </div>

              {/* Action Pills */}
              <div className="flex items-center gap-3">
                <button
                  onClick={fetchData}
                  disabled={isRefreshing}
                  className="px-3 py-2 rounded-xl border border-[#E5E7EB] bg-white text-[12px] font-bold text-[#4B5563] hover:bg-[#F9FAFB] transition-all flex items-center gap-1.5 shadow-sm"
                >
                  <RefreshCw className={`w-3.5 h-3.5 ${isRefreshing ? 'animate-spin text-[#6366F1]' : ''}`} />
                  <span>Refresh</span>
                </button>
                <div className="flex items-center gap-2 px-3.5 py-2 rounded-xl bg-[#111827] text-white text-[12px] font-bold shadow-sm">
                  <Calendar className="w-3.5 h-3.5 text-[#9CA3AF]" />
                  <span>Realtime Stream</span>
                </div>
              </div>
            </div>

            {/* Top 4 KPI Metrics Row */}
            <MetricCards
              temperature={sensor.temperature}
              humidity={sensor.humidity}
              tempMax={settings.temp_max}
              humMax={settings.hum_max}
              doorLocked={settings.door_locked}
              onToggleDoor={handleToggleDoor}
              attendanceCount={attendanceLogs.length}
              totalExpected={50}
            />

            {/* Middle Row: Telemetry Trend, Donut Breakdown, Live Camera */}
            <VisualWidgets
              sensorHistory={sensorHistory}
              attendanceStats={attendanceStats}
              flameDetected={sensor.flame}
              cameraStreamUrl="http://192.168.1.100:81/stream"
            />

            {/* Bottom Row: Attendance Ledger (2/3) + AI Copilot (1/3) */}
            <div className="grid grid-cols-1 lg:grid-cols-3 gap-5">
              <div className="lg:col-span-2">
                <AttendanceLedger
                  logs={attendanceLogs}
                  searchQuery={searchQuery}
                />
              </div>
              <div className="lg:col-span-1">
                <AiCopilot
                  temperature={sensor.temperature}
                  humidity={sensor.humidity}
                  doorLocked={settings.door_locked}
                  alertCount={alerts.length}
                  flameDetected={sensor.flame}
                  onToggleDoor={handleToggleDoor}
                />
              </div>
            </div>
          </main>
        </div>
      </div>

      {/* Modals */}
      <AuthModal
        isOpen={isAuthModalOpen}
        onClose={() => setIsAuthModalOpen(false)}
        onAuthSuccess={(user, role) => {
          setCurrentUser(user);
          setUserRole(role);
        }}
      />

      <SettingsModal
        isOpen={isSettingsModalOpen}
        onClose={() => setIsSettingsModalOpen(false)}
        currentSettings={settings}
        onSaveSuccess={(newS) => setSettings(prev => ({ ...prev, ...newS }))}
      />

      <OtaModal
        isOpen={isOtaModalOpen}
        onClose={() => setIsOtaModalOpen(false)}
      />
    </div>
  );
}
