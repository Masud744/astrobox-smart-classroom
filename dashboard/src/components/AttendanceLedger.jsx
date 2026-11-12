import React from 'react';
import { 
  ArrowUpDown, 
  Filter, 
  CreditCard, 
  ScanFace, 
  CheckCircle2, 
  XCircle,
  Clock
} from 'lucide-react';

export default function AttendanceLedger({ logs = [], searchQuery = '' }) {
  // Filter logs by search query
  const filtered = logs.filter((log) => {
    if (!searchQuery) return true;
    const q = searchQuery.toLowerCase();
    return (
      log.card_uid?.toLowerCase().includes(q) ||
      log.person_type?.toLowerCase().includes(q) ||
      log.face_label?.toLowerCase().includes(q) ||
      log.method?.toLowerCase().includes(q)
    );
  });

  return (
    <div className="bg-white border border-[#EBECEF] rounded-2xl p-6">
      {/* Table Header Controls */}
      <div className="flex items-center justify-between mb-5">
        <div>
          <h3 className="text-[16px] font-bold text-[#111827]">Attendance & Access Ledger</h3>
          <p className="text-[12px] text-[#6B7280]">Real-time student card swipes and biometric face scans</p>
        </div>
        <div className="flex items-center gap-2">
          <button className="flex items-center gap-1.5 px-3 py-1.5 rounded-lg border border-[#E5E7EB] text-[12px] font-bold text-[#4B5563] hover:bg-[#F9FAFB] transition-all">
            <ArrowUpDown className="w-3.5 h-3.5" />
            <span>Sort</span>
          </button>
          <button className="flex items-center gap-1.5 px-3 py-1.5 rounded-lg border border-[#E5E7EB] text-[12px] font-bold text-[#4B5563] hover:bg-[#F9FAFB] transition-all">
            <Filter className="w-3.5 h-3.5" />
            <span>Filter</span>
          </button>
        </div>
      </div>

      {/* Table Container */}
      <div className="overflow-x-auto">
        <table className="w-full text-left text-[13px]">
          <thead>
            <tr className="border-b border-[#F3F4F6] text-[#6B7280] font-semibold text-[11px] uppercase tracking-wider">
              <th className="pb-3 px-3">Student / Personnel</th>
              <th className="pb-3 px-3">Role</th>
              <th className="pb-3 px-3">Verification</th>
              <th className="pb-3 px-3">Timestamp</th>
              <th className="pb-3 px-3 text-right">Status</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-[#F9FAFB]">
            {filtered.length > 0 ? (
              filtered.map((log, index) => {
                const isStudent = log.person_type === 'student';
                const isFace = log.method === 'face';
                const isDenied = log.action === 'denied';
                const name = log.face_label || (isStudent ? 'Enrolled Student' : 'Class Instructor');
                const timeStr = log.logged_at ? new Date(log.logged_at).toLocaleTimeString() : '--';

                return (
                  <tr key={log.id || index} className="hover:bg-[#F9FAFB] transition-colors">
                    {/* Identity Avatar + Name + Card UID */}
                    <td className="py-3 px-3">
                      <div className="flex items-center gap-3">
                        <div className={`w-8 h-8 rounded-full flex items-center justify-center font-bold text-[12px] ${
                          isStudent ? 'bg-[#EEF2FF] text-[#6366F1]' : 'bg-[#FEF3C7] text-[#D97706]'
                        }`}>
                          {name[0]}
                        </div>
                        <div>
                          <p className="font-bold text-[#111827] leading-tight">{name}</p>
                          <p className="text-[11px] font-mono text-[#9CA3AF] leading-tight">{log.card_uid}</p>
                        </div>
                      </div>
                    </td>

                    {/* Role Pill */}
                    <td className="py-3 px-3 font-semibold text-[#4B5563]">
                      <span className={`inline-block px-2.5 py-0.5 rounded-full text-[11px] font-bold capitalize ${
                        isStudent ? 'bg-[#F3F4F6] text-[#4B5563]' : 'bg-[#FEF3C7] text-[#B45309]'
                      }`}>
                        {log.person_type}
                      </span>
                    </td>

                    {/* Method with SVG Icon */}
                    <td className="py-3 px-3 text-[#4B5563]">
                      <div className="flex items-center gap-1.5 font-medium">
                        {isFace ? (
                          <ScanFace className="w-3.5 h-3.5 text-[#10B981]" />
                        ) : (
                          <CreditCard className="w-3.5 h-3.5 text-[#6366F1]" />
                        )}
                        <span className="capitalize">{log.method}</span>
                      </div>
                    </td>

                    {/* Time (Geist Mono) */}
                    <td className="py-3 px-3 font-mono text-[12px] text-[#6B7280]">
                      <div className="flex items-center gap-1">
                        <Clock className="w-3 h-3 text-[#9CA3AF]" />
                        <span>{timeStr}</span>
                      </div>
                    </td>

                    {/* Status Pill */}
                    <td className="py-3 px-3 text-right">
                      <span className={`inline-flex items-center gap-1 px-2.5 py-0.5 rounded-full text-[11px] font-bold ${
                        isDenied 
                          ? 'bg-[#FEF2F2] text-[#EF4444]' 
                          : 'bg-[#ECFDF5] text-[#059669]'
                      }`}>
                        {isDenied ? <XCircle className="w-3 h-3" /> : <CheckCircle2 className="w-3 h-3" />}
                        {isDenied ? 'Denied' : 'Verified'}
                      </span>
                    </td>
                  </tr>
                );
              })
            ) : (
              <tr>
                <td colSpan="5" className="py-8 text-center text-[#9CA3AF] text-[13px]">
                  No attendance records found
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
}
