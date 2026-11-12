import React from 'react';
import { 
  LayoutDashboard, 
  Users, 
  Camera, 
  AlertTriangle, 
  Sliders, 
  UploadCloud, 
  Cpu, 
  ShieldCheck, 
  FileText,
  Sparkles
} from 'lucide-react';

export default function Sidebar({ activeTab, setActiveTab, alertCount = 0, onOpenSettings, onOpenOta }) {
  const menuItems = [
    { id: 'overview', label: 'Dashboard', icon: LayoutDashboard },
    { id: 'attendance', label: 'Attendance', icon: Users, badge: null },
    { id: 'camera', label: 'Camera Feed', icon: Camera },
    { id: 'alerts', label: 'Incident Alerts', icon: AlertTriangle, badge: alertCount > 0 ? alertCount : null, badgeColor: 'bg-red-500' },
  ];

  const systemItems = [
    { id: 'settings', label: 'Remote Settings', icon: Sliders, action: onOpenSettings },
    { id: 'ota', label: 'OTA Firmware', icon: UploadCloud, action: onOpenOta },
    { id: 'docs', label: 'System Telemetry', icon: FileText, action: null },
  ];

  const handleItemClick = (item) => {
    if (item.action) {
      item.action();
    } else {
      setActiveTab(item.id);
    }
  };

  return (
    <aside className="w-64 bg-white border-r border-[#EBECEF] p-6 flex flex-col justify-between select-none">
      <div>
        {/* Brand Header */}
        <div className="flex items-center gap-3 mb-8">
          <div className="w-10 h-10 rounded-xl bg-[#6366F1] flex items-center justify-center text-white shadow-sm">
            <Cpu className="w-5 h-5" />
          </div>
          <div>
            <h1 className="font-bold text-[17px] text-[#111827] leading-tight tracking-tight">FastRobox</h1>
            <p className="text-[12px] font-medium text-[#6B7280]">Smart Classroom Node</p>
          </div>
        </div>

        {/* Menu Section */}
        <div className="mb-6">
          <p className="text-[11px] font-bold text-[#9CA3AF] tracking-wider uppercase mb-3 px-3">Menu</p>
          <nav className="space-y-1">
            {menuItems.map((item) => {
              const Icon = item.icon;
              const isActive = activeTab === item.id;
              return (
                <button
                  key={item.id}
                  onClick={() => handleItemClick(item)}
                  className={`w-full flex items-center justify-between px-3 py-2.5 rounded-xl text-[14px] font-semibold transition-all relative ${
                    isActive 
                      ? 'bg-[#EEF2FF] text-[#6366F1]' 
                      : 'text-[#6B7280] hover:text-[#111827] hover:bg-[#F9FAFB]'
                  }`}
                >
                  {/* Left purple indicator bar for active item */}
                  {isActive && (
                    <span className="absolute left-0 top-2 bottom-2 w-1 bg-[#6366F1] rounded-r-full" />
                  )}
                  <div className="flex items-center gap-3">
                    <Icon className={`w-4 h-4 ${isActive ? 'text-[#6366F1]' : 'text-[#6B7280]'}`} />
                    <span>{item.label}</span>
                  </div>
                  {item.badge && (
                    <span className={`px-2 py-0.5 rounded-full text-[11px] font-bold text-white ${item.badgeColor || 'bg-[#6366F1]'}`}>
                      {item.badge}
                    </span>
                  )}
                </button>
              );
            })}
          </nav>
        </div>

        {/* System & Configuration Section */}
        <div>
          <p className="text-[11px] font-bold text-[#9CA3AF] tracking-wider uppercase mb-3 px-3">System</p>
          <nav className="space-y-1">
            {systemItems.map((item) => {
              const Icon = item.icon;
              return (
                <button
                  key={item.id}
                  onClick={() => handleItemClick(item)}
                  className="w-full flex items-center justify-between px-3 py-2.5 rounded-xl text-[14px] font-semibold text-[#6B7280] hover:text-[#111827] hover:bg-[#F9FAFB] transition-all"
                >
                  <div className="flex items-center gap-3">
                    <Icon className="w-4 h-4 text-[#6B7280]" />
                    <span>{item.label}</span>
                  </div>
                </button>
              );
            })}
          </nav>
        </div>
      </div>

      {/* Bottom Pro / Hardware Status Card */}
      <div className="bg-[#1E1B4B] rounded-2xl p-4 text-white relative overflow-hidden shadow-sm">
        <div className="w-7 h-7 rounded-lg bg-white/10 flex items-center justify-center mb-3">
          <ShieldCheck className="w-4 h-4 text-[#A5B4FC]" />
        </div>
        <h4 className="text-[14px] font-bold mb-1">Dual-Core Guard</h4>
        <p className="text-[12px] text-[#C7D2FE] leading-relaxed mb-3">
          Core 1 handles MJPEG vision. Core 0 manages sensor telemetry.
        </p>
        <div className="flex items-center gap-2">
          <span className="w-2 h-2 rounded-full bg-[#10B981] animate-pulse"></span>
          <span className="text-[11px] font-semibold font-mono text-[#A5B4FC]">LX7 @ 240MHz Active</span>
        </div>
      </div>
    </aside>
  );
}
