import React, { useState } from 'react';
import { 
  Search, 
  Bell, 
  Wifi, 
  ChevronDown, 
  User, 
  LogOut, 
  LogIn, 
  Radio, 
  ShieldAlert
} from 'lucide-react';

export default function Header({ 
  searchQuery, 
  setSearchQuery, 
  wifiRssi, 
  wifiConnected, 
  currentUser, 
  userRole, 
  onOpenAuth, 
  onSignOut 
}) {
  const [profileOpen, setProfileOpen] = useState(false);

  return (
    <header className="h-20 bg-white border-b border-[#EBECEF] px-8 flex items-center justify-between select-none">
      {/* Search Input Bar */}
      <div className="relative w-96">
        <Search className="w-4 h-4 text-[#9CA3AF] absolute left-4 top-1/2 -translate-y-1/2" />
        <input
          type="text"
          value={searchQuery}
          onChange={(e) => setSearchQuery(e.target.value)}
          placeholder="Search card UID, student name..."
          className="w-full bg-[#F9FAFB] border border-[#E5E7EB] rounded-full pl-11 pr-4 py-2 text-[13px] font-medium text-[#111827] placeholder-[#9CA3AF] focus:outline-none focus:border-[#6366F1] focus:bg-white transition-all"
        />
      </div>

      {/* Right Controls */}
      <div className="flex items-center gap-4">
        {/* Node Telemetry / WiFi Status */}
        <div className="hidden sm:flex items-center gap-2 px-3 py-1.5 rounded-full bg-[#F3F4F6] border border-[#E5E7EB] text-[12px] font-semibold text-[#4B5563]">
          <span className={`w-2 h-2 rounded-full ${wifiConnected ? 'bg-[#10B981] animate-pulse' : 'bg-[#EF4444]'}`} />
          <Wifi className="w-3.5 h-3.5 text-[#6B7280]" />
          <span className="font-mono">{wifiConnected ? `${wifiRssi || -55} dBm` : 'OFFLINE'}</span>
        </div>

        {/* Notification Bell */}
        <button 
          title="System Notifications"
          className="w-10 h-10 rounded-full border border-[#E5E7EB] flex items-center justify-center text-[#6B7280] hover:text-[#111827] hover:bg-[#F9FAFB] transition-all relative"
        >
          <Bell className="w-4 h-4" />
          <span className="w-2 h-2 rounded-full bg-[#EF4444] absolute top-2.5 right-2.5" />
        </button>

        {/* User Profile Pill / Auth Button */}
        <div className="relative">
          {currentUser ? (
            <button
              onClick={() => setProfileOpen(!profileOpen)}
              className="flex items-center gap-3 pl-2 pr-3 py-1.5 rounded-full border border-[#E5E7EB] hover:border-[#D1D5DB] transition-all bg-white"
            >
              <div className="w-8 h-8 rounded-full bg-[#6366F1] text-white flex items-center justify-center font-bold text-[13px]">
                {currentUser.email ? currentUser.email[0].toUpperCase() : 'T'}
              </div>
              <div className="text-left hidden md:block">
                <p className="text-[13px] font-bold text-[#111827] leading-tight">
                  {currentUser.user_metadata?.full_name || 'Totok Michael'}
                </p>
                <p className="text-[11px] font-medium text-[#6B7280] leading-tight truncate max-w-[140px]">
                  {currentUser.email}
                </p>
              </div>
              <ChevronDown className="w-3.5 h-3.5 text-[#9CA3AF]" />
            </button>
          ) : (
            <button
              onClick={onOpenAuth}
              className="flex items-center gap-2 px-4 py-2 rounded-full bg-[#111827] text-white text-[13px] font-bold hover:bg-[#1F2937] transition-all shadow-sm"
            >
              <LogIn className="w-3.5 h-3.5" />
              <span>Sign In / Demo</span>
            </button>
          )}

          {/* Profile Dropdown */}
          {profileOpen && currentUser && (
            <div className="absolute right-0 mt-2 w-56 bg-white border border-[#E5E7EB] rounded-2xl shadow-xl p-2 z-50">
              <div className="px-3 py-2 border-b border-[#F3F4F6] mb-1">
                <p className="text-[11px] font-bold uppercase text-[#9CA3AF] tracking-wider">Signed in as</p>
                <p className="text-[13px] font-bold text-[#111827] truncate">{currentUser.email}</p>
                <span className="inline-block mt-1 px-2 py-0.5 rounded-md text-[10px] font-bold bg-[#EEF2FF] text-[#6366F1] uppercase">
                  {userRole || 'Teacher'}
                </span>
              </div>
              <button
                onClick={() => {
                  setProfileOpen(false);
                  onSignOut();
                }}
                className="w-full flex items-center gap-2 px-3 py-2 rounded-xl text-[13px] font-semibold text-[#EF4444] hover:bg-[#FEF2F2] transition-all text-left"
              >
                <LogOut className="w-4 h-4" />
                <span>Sign Out</span>
              </button>
            </div>
          )}
        </div>
      </div>
    </header>
  );
}
