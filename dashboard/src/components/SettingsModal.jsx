import React, { useState, useEffect } from 'react';
import { X, Sliders, CheckCircle2, Save, BellRing } from 'lucide-react';
import { API_BASE } from '../lib/supabase';

export default function SettingsModal({ isOpen, onClose, currentSettings, onSaveSuccess }) {
  if (!isOpen) return null;

  const [roomId, setRoomId] = useState(currentSettings.room_id || 'ROOM-01');
  const [classStart, setClassStart] = useState(currentSettings.class_start?.slice(0, 5) || '08:00');
  const [tempMax, setTempMax] = useState(currentSettings.temp_max || 35);
  const [humMax, setHumMax] = useState(currentSettings.hum_max || 80);
  const [buzzerEnabled, setBuzzerEnabled] = useState(currentSettings.buzzer_enabled ?? true);
  const [isSaving, setIsSaving] = useState(false);
  const [success, setSuccess] = useState(false);

  useEffect(() => {
    if (currentSettings) {
      setRoomId(currentSettings.room_id || 'ROOM-01');
      setClassStart(currentSettings.class_start?.slice(0, 5) || '08:00');
      setTempMax(currentSettings.temp_max || 35);
      setHumMax(currentSettings.hum_max || 80);
      setBuzzerEnabled(currentSettings.buzzer_enabled ?? true);
    }
  }, [currentSettings]);

  const handleSubmit = async (e) => {
    e.preventDefault();
    setIsSaving(true);
    setSuccess(false);

    try {
      const res = await fetch(`${API_BASE}/api/settings`, {
        method: 'PATCH',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          room_id: roomId,
          class_start: `${classStart}:00`,
          temp_max: parseFloat(tempMax),
          hum_max: parseFloat(humMax),
          buzzer_enabled: buzzerEnabled
        })
      });

      if (res.ok) {
        setSuccess(true);
        onSaveSuccess({
          room_id: roomId,
          class_start: `${classStart}:00`,
          temp_max: parseFloat(tempMax),
          hum_max: parseFloat(humMax),
          buzzer_enabled: buzzerEnabled
        });
        setTimeout(() => {
          setSuccess(false);
          onClose();
        }, 1200);
      }
    } catch (err) {
      console.error(err);
    } finally {
      setIsSaving(false);
    }
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/40 backdrop-blur-sm select-none">
      <div className="bg-white rounded-3xl max-w-lg w-full border border-[#EBECEF] p-8 shadow-2xl relative animate-in fade-in zoom-in-95 duration-200">
        <button
          onClick={onClose}
          className="absolute right-6 top-6 w-8 h-8 rounded-full border border-[#E5E7EB] flex items-center justify-center text-[#9CA3AF] hover:text-[#111827] hover:bg-[#F9FAFB] transition-all"
        >
          <X className="w-4 h-4" />
        </button>

        <div className="flex items-center gap-3 mb-6">
          <div className="w-10 h-10 rounded-xl bg-[#EEF2FF] text-[#6366F1] flex items-center justify-center">
            <Sliders className="w-5 h-5" />
          </div>
          <div>
            <h2 className="text-[18px] font-bold text-[#111827]">Classroom Thresholds</h2>
            <p className="text-[12px] text-[#6B7280]">Remote parameters synced to ESP32 node</p>
          </div>
        </div>

        {success && (
          <div className="mb-4 p-3 rounded-xl bg-[#ECFDF5] border border-[#D1FAE5] text-[12px] font-bold text-[#059669] flex items-center gap-2">
            <CheckCircle2 className="w-4 h-4" />
            <span>Settings synced successfully across edge devices!</span>
          </div>
        )}

        <form onSubmit={handleSubmit} className="space-y-4">
          <div className="grid grid-cols-2 gap-4">
            <div>
              <label className="block text-[12px] font-bold text-[#374151] mb-1.5 uppercase tracking-wider">
                Room Identifier
              </label>
              <input
                type="text"
                value={roomId}
                onChange={(e) => setRoomId(e.target.value)}
                className="w-full bg-[#F9FAFB] border border-[#E5E7EB] rounded-xl px-3.5 py-2 text-[13px] font-mono text-[#111827] focus:outline-none focus:border-[#6366F1]"
              />
            </div>
            <div>
              <label className="block text-[12px] font-bold text-[#374151] mb-1.5 uppercase tracking-wider">
                Class Schedule Start
              </label>
              <input
                type="time"
                value={classStart}
                onChange={(e) => setClassStart(e.target.value)}
                className="w-full bg-[#F9FAFB] border border-[#E5E7EB] rounded-xl px-3.5 py-2 text-[13px] font-mono text-[#111827] focus:outline-none focus:border-[#6366F1]"
              />
            </div>
          </div>

          <div className="grid grid-cols-2 gap-4">
            <div>
              <label className="block text-[12px] font-bold text-[#374151] mb-1.5 uppercase tracking-wider">
                Max Temperature (°C)
              </label>
              <input
                type="number"
                step="0.5"
                value={tempMax}
                onChange={(e) => setTempMax(e.target.value)}
                className="w-full bg-[#F9FAFB] border border-[#E5E7EB] rounded-xl px-3.5 py-2 text-[13px] font-mono text-[#111827] focus:outline-none focus:border-[#6366F1]"
              />
            </div>
            <div>
              <label className="block text-[12px] font-bold text-[#374151] mb-1.5 uppercase tracking-wider">
                Max Humidity (%)
              </label>
              <input
                type="number"
                value={humMax}
                onChange={(e) => setHumMax(e.target.value)}
                className="w-full bg-[#F9FAFB] border border-[#E5E7EB] rounded-xl px-3.5 py-2 text-[13px] font-mono text-[#111827] focus:outline-none focus:border-[#6366F1]"
              />
            </div>
          </div>

          <div className="p-3.5 rounded-xl bg-[#F9FAFB] border border-[#E5E7EB] flex items-center justify-between">
            <div className="flex items-center gap-2.5">
              <BellRing className="w-4 h-4 text-[#6366F1]" />
              <div>
                <p className="text-[13px] font-bold text-[#111827]">Hardware Buzzer</p>
                <p className="text-[11px] text-[#6B7280]">Audio feedback for card check-ins and alarms</p>
              </div>
            </div>
            <input
              type="checkbox"
              checked={buzzerEnabled}
              onChange={(e) => setBuzzerEnabled(e.target.checked)}
              className="w-4 h-4 text-[#6366F1] rounded border-[#D1D5DB] focus:ring-[#6366F1]"
            />
          </div>

          <button
            type="submit"
            disabled={isSaving}
            className="w-full py-2.5 rounded-xl bg-[#6366F1] text-white text-[13px] font-bold hover:bg-[#4F46E5] transition-all flex items-center justify-center gap-2 shadow-sm"
          >
            <Save className="w-4 h-4" />
            <span>{isSaving ? 'Syncing...' : 'Save & Propagate'}</span>
          </button>
        </form>
      </div>
    </div>
  );
}
