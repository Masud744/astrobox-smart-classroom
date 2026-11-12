import React, { useState } from 'react';
import { 
  ArrowUpRight, 
  Camera, 
  Radio, 
  Maximize2, 
  Activity, 
  ShieldCheck, 
  Flame
} from 'lucide-react';

export default function VisualWidgets({ 
  sensorHistory = [], 
  attendanceStats = { rfid: 0, face: 0, denied: 0 }, 
  cameraStreamUrl = 'http://192.168.1.100:81/stream',
  flameDetected = false
}) {
  const [streamUrl, setStreamUrl] = useState(cameraStreamUrl);
  const [isStreaming, setIsStreaming] = useState(false);

  // Normalize recent 7 data points for telemetry bars
  const recentLogs = sensorHistory.slice(0, 7).reverse();
  const defaultBars = [
    { day: '08:00', val: 26.5 },
    { day: '08:15', val: 27.0 },
    { day: '08:30', val: 27.8 },
    { day: '08:45', val: 28.2 },
    { day: '09:00', val: 28.5 },
    { day: '09:15', val: 28.4 },
    { day: 'Live',  val: 28.6 }
  ];

  const bars = recentLogs.length > 0
    ? recentLogs.map((l) => ({
        day: new Date(l.logged_at).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }),
        val: l.temperature || 26.0
      }))
    : defaultBars;

  const totalVerifications = attendanceStats.rfid + attendanceStats.face + attendanceStats.denied || 1;
  const rfidPct = Math.round((attendanceStats.rfid / totalVerifications) * 100);
  const facePct = Math.round((attendanceStats.face / totalVerifications) * 100);
  const deniedPct = 100 - rfidPct - facePct;

  return (
    <div className="grid grid-cols-1 lg:grid-cols-3 gap-5 mb-8">
      {/* 1. Sensor Telemetry Trend (Bar chart) */}
      <div className="bg-white border border-[#EBECEF] rounded-2xl p-5 flex flex-col justify-between">
        <div>
          <div className="flex items-center justify-between mb-2">
            <h3 className="text-[14px] font-bold text-[#111827]">Sensor Telemetry</h3>
            <ArrowUpRight className="w-4 h-4 text-[#9CA3AF]" />
          </div>
          <div className="flex items-baseline gap-2 mb-6">
            <span className="text-[26px] font-extrabold text-[#111827] font-mono">
              {bars[bars.length - 1]?.val?.toFixed(1) || '28.4'}°C
            </span>
            <span className="text-[12px] font-semibold text-[#10B981] bg-[#ECFDF5] px-2 py-0.5 rounded-md font-mono">
              Stable
            </span>
          </div>
        </div>

        {/* Bar Visualization */}
        <div className="h-32 flex items-end justify-between gap-2 pt-4 px-2 border-t border-[#F3F4F6]">
          {bars.map((bar, i) => {
            const heightPct = Math.min(Math.max(((bar.val - 20) / 15) * 100, 20), 95);
            const isLatest = i === bars.length - 1;
            return (
              <div key={i} className="flex flex-col items-center gap-2 flex-1">
                <div className="w-full flex items-end justify-center h-24">
                  <div 
                    style={{ height: `${heightPct}%` }}
                    className={`w-full max-w-[20px] rounded-t-md transition-all ${
                      isLatest ? 'bg-[#6366F1]' : 'bg-[#E0E7FF] hover:bg-[#C7D2FE]'
                    }`}
                  />
                </div>
                <span className="text-[10px] font-semibold text-[#9CA3AF] font-mono truncate max-w-[36px]">
                  {bar.day}
                </span>
              </div>
            );
          })}
        </div>
      </div>

      {/* 2. Verification Method Breakdown (Donut Chart) */}
      <div className="bg-white border border-[#EBECEF] rounded-2xl p-5 flex flex-col justify-between">
        <div>
          <div className="flex items-center justify-between mb-4">
            <h3 className="text-[14px] font-bold text-[#111827]">Access Verification</h3>
            <ArrowUpRight className="w-4 h-4 text-[#9CA3AF]" />
          </div>

          {/* Donut representation */}
          <div className="flex items-center justify-center py-4">
            <div className="relative w-32 h-32 flex items-center justify-center">
              <svg className="w-full h-full -rotate-90" viewBox="0 0 36 36">
                <circle cx="18" cy="18" r="15.915" fill="transparent" stroke="#E5E7EB" strokeWidth="4" />
                {/* RFID Slice */}
                <circle
                  cx="18" cy="18" r="15.915" fill="transparent"
                  stroke="#6366F1" strokeWidth="4.5"
                  strokeDasharray={`${rfidPct} ${100 - rfidPct}`}
                  strokeDashoffset="0"
                />
                {/* Face Slice */}
                <circle
                  cx="18" cy="18" r="15.915" fill="transparent"
                  stroke="#10B981" strokeWidth="4.5"
                  strokeDasharray={`${facePct} ${100 - facePct}`}
                  strokeDashoffset={`${-rfidPct}`}
                />
              </svg>
              <div className="absolute text-center">
                <span className="text-[18px] font-extrabold text-[#111827] font-mono leading-none block">
                  {totalVerifications}
                </span>
                <span className="text-[10px] font-semibold text-[#9CA3AF] uppercase">Logs</span>
              </div>
            </div>
          </div>
        </div>

        {/* Legend */}
        <div className="flex items-center justify-center gap-4 pt-3 border-t border-[#F3F4F6] text-[12px] font-semibold text-[#4B5563]">
          <div className="flex items-center gap-1.5">
            <span className="w-2.5 h-2.5 rounded-full bg-[#6366F1]"></span>
            <span>RFID ({rfidPct}%)</span>
          </div>
          <div className="flex items-center gap-1.5">
            <span className="w-2.5 h-2.5 rounded-full bg-[#10B981]"></span>
            <span>Face ({facePct}%)</span>
          </div>
        </div>
      </div>

      {/* 3. Live OV2640 Camera Feed Viewport */}
      <div className="bg-white border border-[#EBECEF] rounded-2xl p-5 flex flex-col justify-between">
        <div>
          <div className="flex items-center justify-between mb-3">
            <div className="flex items-center gap-2">
              <h3 className="text-[14px] font-bold text-[#111827]">Classroom Vision</h3>
              {flameDetected && (
                <span className="inline-flex items-center gap-1 px-2 py-0.5 rounded-md text-[10px] font-bold bg-[#FEE2E2] text-[#EF4444]">
                  <Flame className="w-3 h-3" /> Flame Detected
                </span>
              )}
            </div>
            <span className="inline-flex items-center gap-1.5 px-2 py-0.5 rounded-full text-[11px] font-bold bg-[#ECFDF5] text-[#059669]">
              <span className="w-1.5 h-1.5 rounded-full bg-[#10B981] animate-pulse"></span>
              PORT 81
            </span>
          </div>

          {/* Camera Frame / Stream Container */}
          <div className="relative w-full h-36 bg-[#0F172A] rounded-xl overflow-hidden flex items-center justify-center border border-[#E2E8F0]">
            {isStreaming ? (
              <img 
                src={streamUrl} 
                alt="Live Camera Feed"
                className="w-full h-full object-cover"
                onError={() => setIsStreaming(false)}
              />
            ) : (
              <div className="text-center p-3">
                <Camera className="w-8 h-8 text-[#475569] mx-auto mb-2" />
                <p className="text-[11px] text-[#94A3B8]">ESP32-S3 OV2640 Stream Ready</p>
                <button
                  onClick={() => setIsStreaming(true)}
                  className="mt-2 px-3 py-1 rounded-lg bg-[#6366F1] text-white text-[11px] font-bold hover:bg-[#4F46E5] transition-all"
                >
                  Connect Stream
                </button>
              </div>
            )}
          </div>
        </div>

        {/* Input for ESP32 IP */}
        <div className="mt-3 flex items-center gap-2">
          <input
            type="text"
            value={streamUrl}
            onChange={(e) => setStreamUrl(e.target.value)}
            placeholder="http://192.168.x.x:81/stream"
            className="flex-1 bg-[#F9FAFB] border border-[#E5E7EB] rounded-lg px-2.5 py-1 text-[11px] font-mono text-[#111827] focus:outline-none focus:border-[#6366F1]"
          />
          <button
            onClick={() => setIsStreaming(!isStreaming)}
            className="px-2.5 py-1 rounded-lg border border-[#E5E7EB] text-[11px] font-semibold text-[#4B5563] hover:bg-[#F3F4F6] transition-all"
          >
            {isStreaming ? 'Stop' : 'Play'}
          </button>
        </div>
      </div>
    </div>
  );
}
