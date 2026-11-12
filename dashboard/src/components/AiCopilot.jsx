import React, { useState } from 'react';
import { 
  Sparkles, 
  MoreVertical, 
  ArrowUp, 
  Mic, 
  Paperclip, 
  ShieldCheck, 
  Flame, 
  Lock, 
  Download
} from 'lucide-react';

export default function AiCopilot({ 
  temperature = 28.5, 
  humidity = 62, 
  doorLocked = true, 
  alertCount = 0, 
  flameDetected = false,
  onToggleDoor
}) {
  const [prompt, setPrompt] = useState('');
  const [responseMsg, setResponseMsg] = useState(
    flameDetected 
      ? 'CRITICAL ALERT: Flame sensor active in Room 01. Buzzer triggered. Immediate verification recommended.'
      : alertCount > 0
      ? `System warning: ${alertCount} unresolved incident(s) logged. Environmental metrics stable.`
      : 'All safety thresholds nominal. Environmental conditions optimal for classroom session.'
  );

  const handleSendPrompt = (e) => {
    e?.preventDefault();
    if (!prompt.trim()) return;

    const q = prompt.toLowerCase();
    if (q.includes('door') || q.includes('lock')) {
      onToggleDoor();
      setResponseMsg(`Simulated door state toggled. New state: ${!doorLocked ? 'LOCKED' : 'UNLOCKED'}.`);
    } else if (q.includes('status') || q.includes('temp')) {
      setResponseMsg(`Current temperature: ${temperature.toFixed(1)}°C, humidity: ${humidity.toFixed(1)}%. Safety sensors: ${flameDetected ? 'FLAME DETECTED' : 'SECURE'}.`);
    } else {
      setResponseMsg(`AI Assistant: Command received for Room 01. Telemetry verified.`);
    }
    setPrompt('');
  };

  const handleChipClick = (action) => {
    if (action === 'toggle_door') {
      onToggleDoor();
      setResponseMsg(`Command dispatched: Door ${doorLocked ? 'UNLOCKED' : 'LOCKED'}.`);
    } else if (action === 'status') {
      setResponseMsg(`Classroom 01 Report: ${temperature.toFixed(1)}°C | ${humidity.toFixed(1)}% | Dual-core active.`);
    } else if (action === 'export') {
      setResponseMsg('Attendance and telemetry logs formatted for report export.');
    }
  };

  return (
    <div className="bg-white border border-[#EBECEF] rounded-2xl p-6 flex flex-col justify-between">
      <div>
        {/* Header */}
        <div className="flex items-center justify-between mb-4">
          <div className="flex items-center gap-2">
            <div className="w-6 h-6 rounded-md bg-[#EEF2FF] flex items-center justify-center text-[#6366F1]">
              <Sparkles className="w-3.5 h-3.5" />
            </div>
            <h3 className="text-[14px] font-bold text-[#111827]">Classroom Copilot</h3>
          </div>
          <button className="text-[#9CA3AF] hover:text-[#4B5563]">
            <MoreVertical className="w-4 h-4" />
          </button>
        </div>

        {/* Ambient Glowing Orb */}
        <div className="flex justify-center my-4">
          <div className="relative w-24 h-24 flex items-center justify-center">
            <div className={`w-20 h-20 rounded-full ai-ambient-sphere flex items-center justify-center shadow-lg transition-all ${
              flameDetected 
                ? 'bg-gradient-to-tr from-[#DC2626] to-[#EF4444]' 
                : 'bg-gradient-to-tr from-[#6366F1] via-[#7C3AED] to-[#EC4899]'
            }`}>
              <div className="w-14 h-14 rounded-full bg-white/20 backdrop-blur-sm flex items-center justify-center">
                {flameDetected ? (
                  <Flame className="w-7 h-7 text-white animate-bounce" />
                ) : (
                  <ShieldCheck className="w-7 h-7 text-white" />
                )}
              </div>
            </div>
          </div>
        </div>

        {/* Message / Summary */}
        <div className="text-center mb-5 px-2">
          <p className="text-[13px] font-bold text-[#111827] mb-1">
            {flameDetected ? 'Hazard Alert Detected' : 'Safety Intelligence'}
          </p>
          <p className="text-[12px] text-[#6B7280] leading-relaxed">
            {responseMsg}
          </p>
        </div>

        {/* Quick Action Chips */}
        <div className="flex flex-wrap gap-1.5 justify-center mb-4">
          <button
            onClick={() => handleChipClick('toggle_door')}
            className="px-2.5 py-1 rounded-full border border-[#E5E7EB] text-[11px] font-semibold text-[#4B5563] hover:bg-[#F3F4F6] transition-all flex items-center gap-1"
          >
            <Lock className="w-3 h-3 text-[#6366F1]" />
            <span>{doorLocked ? 'Unlock Door' : 'Lock Door'}</span>
          </button>
          <button
            onClick={() => handleChipClick('status')}
            className="px-2.5 py-1 rounded-full border border-[#E5E7EB] text-[11px] font-semibold text-[#4B5563] hover:bg-[#F3F4F6] transition-all"
          >
            Room Status
          </button>
          <button
            onClick={() => handleChipClick('export')}
            className="px-2.5 py-1 rounded-full border border-[#E5E7EB] text-[11px] font-semibold text-[#4B5563] hover:bg-[#F3F4F6] transition-all flex items-center gap-1"
          >
            <Download className="w-3 h-3 text-[#10B981]" />
            <span>Export Log</span>
          </button>
        </div>
      </div>

      {/* Input Bar with Send Button */}
      <form onSubmit={handleSendPrompt} className="relative mt-2">
        <div className="flex items-center bg-[#F9FAFB] border border-[#E5E7EB] rounded-full px-3 py-1.5 focus-within:border-[#6366F1] focus-within:bg-white transition-all">
          <Paperclip className="w-4 h-4 text-[#9CA3AF] mr-2 cursor-pointer hover:text-[#4B5563]" />
          <input
            type="text"
            value={prompt}
            onChange={(e) => setPrompt(e.target.value)}
            placeholder="Ask Copilot or command door..."
            className="w-full bg-transparent text-[12px] font-medium text-[#111827] placeholder-[#9CA3AF] focus:outline-none"
          />
          <Mic className="w-4 h-4 text-[#9CA3AF] mx-2 cursor-pointer hover:text-[#4B5563]" />
          <button
            type="submit"
            className="w-7 h-7 rounded-full bg-[#6366F1] text-white flex items-center justify-center hover:bg-[#4F46E5] transition-all flex-shrink-0"
          >
            <ArrowUp className="w-3.5 h-3.5" />
          </button>
        </div>
      </form>
    </div>
  );
}
