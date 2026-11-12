import React from 'react';
import { 
  ArrowUpRight, 
  Thermometer, 
  Droplets, 
  Lock, 
  Unlock, 
  Users,
  CheckCircle2,
  AlertCircle
} from 'lucide-react';

export default function MetricCards({ 
  temperature = 0, 
  humidity = 0, 
  tempMax = 35, 
  humMax = 80, 
  doorLocked = true, 
  onToggleDoor, 
  attendanceCount = 0, 
  totalExpected = 50 
}) {
  const isTempHigh = temperature > tempMax;
  const isHumHigh = humidity > humMax;
  const attendanceRate = totalExpected > 0 ? Math.round((attendanceCount / totalExpected) * 100) : 0;

  return (
    <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-5 mb-8">
      {/* 1. Temperature Card (Lavender) */}
      <div className="bg-white border border-[#EBECEF] rounded-2xl p-5 hover:border-[#D1D5DB] transition-all">
        <div className="flex items-center justify-between mb-4">
          <div className="flex items-center gap-2.5">
            <div className="w-8 h-8 rounded-lg bg-[#EEF2FF] flex items-center justify-center text-[#6366F1]">
              <Thermometer className="w-4 h-4" />
            </div>
            <span className="text-[13px] font-bold text-[#4B5563]">Temperature</span>
          </div>
          <ArrowUpRight className="w-4 h-4 text-[#9CA3AF]" />
        </div>
        <div className="mb-3">
          <span className="text-[28px] font-extrabold text-[#111827] tracking-tight font-mono">
            {typeof temperature === 'number' ? temperature.toFixed(1) : '--'}
          </span>
          <span className="text-[18px] font-bold text-[#6B7280] ml-1">°C</span>
        </div>
        <div className="flex items-center gap-2">
          <span className={`inline-flex items-center gap-1 px-2 py-0.5 rounded-md text-[11px] font-bold font-mono ${
            isTempHigh ? 'bg-[#FEF2F2] text-[#EF4444]' : 'bg-[#ECFDF5] text-[#059669]'
          }`}>
            {isTempHigh ? <AlertCircle className="w-3 h-3" /> : <CheckCircle2 className="w-3 h-3" />}
            {isTempHigh ? 'High Alert' : 'Normal Range'}
          </span>
          <span className="text-[11px] font-medium text-[#9CA3AF]">Max: {tempMax}°C</span>
        </div>
      </div>

      {/* 2. Humidity Card (Orange/Amber) */}
      <div className="bg-white border border-[#EBECEF] rounded-2xl p-5 hover:border-[#D1D5DB] transition-all">
        <div className="flex items-center justify-between mb-4">
          <div className="flex items-center gap-2.5">
            <div className="w-8 h-8 rounded-lg bg-[#FEF3C7] flex items-center justify-center text-[#D97706]">
              <Droplets className="w-4 h-4" />
            </div>
            <span className="text-[13px] font-bold text-[#4B5563]">Humidity</span>
          </div>
          <ArrowUpRight className="w-4 h-4 text-[#9CA3AF]" />
        </div>
        <div className="mb-3">
          <span className="text-[28px] font-extrabold text-[#111827] tracking-tight font-mono">
            {typeof humidity === 'number' ? humidity.toFixed(1) : '--'}
          </span>
          <span className="text-[18px] font-bold text-[#6B7280] ml-1">%</span>
        </div>
        <div className="flex items-center gap-2">
          <span className={`inline-flex items-center gap-1 px-2 py-0.5 rounded-md text-[11px] font-bold font-mono ${
            isHumHigh ? 'bg-[#FEF2F2] text-[#EF4444]' : 'bg-[#ECFDF5] text-[#059669]'
          }`}>
            <CheckCircle2 className="w-3 h-3" />
            {isHumHigh ? 'Exceeded' : 'Optimal'}
          </span>
          <span className="text-[11px] font-medium text-[#9CA3AF]">Limit: {humMax}%</span>
        </div>
      </div>

      {/* 3. Door State Card (Cyan/Blue) with 1-Click Toggle */}
      <div className="bg-white border border-[#EBECEF] rounded-2xl p-5 hover:border-[#D1D5DB] transition-all">
        <div className="flex items-center justify-between mb-4">
          <div className="flex items-center gap-2.5">
            <div className={`w-8 h-8 rounded-lg flex items-center justify-center ${
              doorLocked ? 'bg-[#FEE2E2] text-[#EF4444]' : 'bg-[#DCFCE7] text-[#16A34A]'
            }`}>
              {doorLocked ? <Lock className="w-4 h-4" /> : <Unlock className="w-4 h-4" />}
            </div>
            <span className="text-[13px] font-bold text-[#4B5563]">Simulated Door</span>
          </div>
          <ArrowUpRight className="w-4 h-4 text-[#9CA3AF]" />
        </div>
        <div className="flex items-center justify-between mb-3">
          <span className={`text-[24px] font-extrabold tracking-tight font-mono ${
            doorLocked ? 'text-[#EF4444]' : 'text-[#16A34A]'
          }`}>
            {doorLocked ? 'LOCKED' : 'UNLOCKED'}
          </span>
        </div>
        <button
          onClick={onToggleDoor}
          className={`w-full py-1.5 px-3 rounded-lg text-[12px] font-bold transition-all flex items-center justify-center gap-1.5 ${
            doorLocked 
              ? 'bg-[#111827] text-white hover:bg-[#1F2937]' 
              : 'bg-[#EF4444] text-white hover:bg-[#DC2626]'
          }`}
        >
          {doorLocked ? <Unlock className="w-3.5 h-3.5" /> : <Lock className="w-3.5 h-3.5" />}
          <span>{doorLocked ? 'Click to Unlock' : 'Click to Lock'}</span>
        </button>
      </div>

      {/* 4. Attendance Card (Mint Green) */}
      <div className="bg-white border border-[#EBECEF] rounded-2xl p-5 hover:border-[#D1D5DB] transition-all">
        <div className="flex items-center justify-between mb-4">
          <div className="flex items-center gap-2.5">
            <div className="w-8 h-8 rounded-lg bg-[#DCFCE7] flex items-center justify-center text-[#16A34A]">
              <Users className="w-4 h-4" />
            </div>
            <span className="text-[13px] font-bold text-[#4B5563]">Attendance</span>
          </div>
          <ArrowUpRight className="w-4 h-4 text-[#9CA3AF]" />
        </div>
        <div className="mb-3">
          <span className="text-[28px] font-extrabold text-[#111827] tracking-tight font-mono">
            {attendanceCount}
          </span>
          <span className="text-[14px] font-semibold text-[#6B7280] ml-1.5">/ {totalExpected}</span>
        </div>
        <div className="flex items-center gap-2">
          <span className="inline-flex items-center gap-1 px-2 py-0.5 rounded-md text-[11px] font-bold bg-[#ECFDF5] text-[#059669] font-mono">
            <ArrowUpRight className="w-3 h-3" />
            {attendanceRate}% Present
          </span>
          <span className="text-[11px] font-medium text-[#9CA3AF]">Today</span>
        </div>
      </div>
    </div>
  );
}
