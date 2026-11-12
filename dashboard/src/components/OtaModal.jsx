import React, { useState } from 'react';
import { X, UploadCloud, CheckCircle2, AlertCircle, FileCode } from 'lucide-react';
import { API_BASE } from '../lib/supabase';

export default function OtaModal({ isOpen, onClose }) {
  if (!isOpen) return null;

  const [file, setFile] = useState(null);
  const [version, setVersion] = useState('1.0.1');
  const [notes, setNotes] = useState('ESP32-S3 Camera and sensor update');
  const [status, setStatus] = useState({ type: '', text: '' });
  const [uploading, setUploading] = useState(false);

  const handleUpload = async (e) => {
    e.preventDefault();
    if (!file) {
      setStatus({ type: 'error', text: 'Please select a compiled firmware .bin file' });
      return;
    }

    setUploading(true);
    setStatus({ type: 'info', text: 'Uploading firmware binary to distribution server...' });

    const formData = new FormData();
    formData.append('file', file);
    formData.append('version', version);
    formData.append('notes', notes);

    try {
      const res = await fetch(`${API_BASE}/api/ota/upload`, {
        method: 'POST',
        body: formData
      });
      const data = await res.json();

      if (res.ok) {
        setStatus({
          type: 'success',
          text: `Version ${data.version} active. ESP32 will auto-flash on next poll or 3s BOOT button press.`
        });
        setFile(null);
      } else {
        setStatus({ type: 'error', text: data.detail || 'OTA upload failed' });
      }
    } catch (err) {
      setStatus({ type: 'error', text: err.message || 'Network communication failure' });
    } finally {
      setUploading(false);
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
            <UploadCloud className="w-5 h-5" />
          </div>
          <div>
            <h2 className="text-[18px] font-bold text-[#111827]">OTA Firmware Deployment</h2>
            <p className="text-[12px] text-[#6B7280]">Deliver over-the-air updates to ESP32-S3 node</p>
          </div>
        </div>

        {status.text && (
          <div className={`mb-4 p-3 rounded-xl border text-[12px] font-semibold flex items-center gap-2 ${
            status.type === 'success' 
              ? 'bg-[#ECFDF5] border-[#D1FAE5] text-[#059669]'
              : status.type === 'error'
              ? 'bg-[#FEF2F2] border-[#FEE2E2] text-[#EF4444]'
              : 'bg-[#EFF6FF] border-[#DBEAFE] text-[#2563EB]'
          }`}>
            {status.type === 'success' ? <CheckCircle2 className="w-4 h-4" /> : <AlertCircle className="w-4 h-4" />}
            <span>{status.text}</span>
          </div>
        )}

        <form onSubmit={handleUpload} className="space-y-4">
          <div>
            <label className="block text-[12px] font-bold text-[#374151] mb-1.5 uppercase tracking-wider">
              Firmware Binary (.bin)
            </label>
            <div className="relative border-2 border-dashed border-[#E5E7EB] rounded-2xl p-4 text-center hover:border-[#6366F1] transition-all bg-[#F9FAFB]">
              <input
                type="file"
                accept=".bin"
                onChange={(e) => setFile(e.target.files[0])}
                className="absolute inset-0 opacity-0 cursor-pointer"
              />
              <FileCode className="w-6 h-6 text-[#9CA3AF] mx-auto mb-1" />
              <p className="text-[12px] font-bold text-[#4B5563]">
                {file ? file.name : 'Click or drag firmware .bin file here'}
              </p>
              <p className="text-[10px] text-[#9CA3AF] font-mono mt-0.5">
                {file ? `${(file.size / 1024).toFixed(1)} KB` : 'Compiled ESP32-S3 Arduino Binary'}
              </p>
            </div>
          </div>

          <div className="grid grid-cols-2 gap-4">
            <div>
              <label className="block text-[12px] font-bold text-[#374151] mb-1.5 uppercase tracking-wider">
                Release Version
              </label>
              <input
                type="text"
                value={version}
                onChange={(e) => setVersion(e.target.value)}
                placeholder="1.0.1"
                className="w-full bg-[#F9FAFB] border border-[#E5E7EB] rounded-xl px-3.5 py-2 text-[13px] font-mono text-[#111827] focus:outline-none focus:border-[#6366F1]"
              />
            </div>
            <div>
              <label className="block text-[12px] font-bold text-[#374151] mb-1.5 uppercase tracking-wider">
                Release Notes
              </label>
              <input
                type="text"
                value={notes}
                onChange={(e) => setNotes(e.target.value)}
                placeholder="Bug fixes"
                className="w-full bg-[#F9FAFB] border border-[#E5E7EB] rounded-xl px-3.5 py-2 text-[13px] text-[#111827] focus:outline-none focus:border-[#6366F1]"
              />
            </div>
          </div>

          <button
            type="submit"
            disabled={uploading}
            className="w-full py-2.5 rounded-xl bg-[#6366F1] text-white text-[13px] font-bold hover:bg-[#4F46E5] transition-all flex items-center justify-center gap-2 shadow-sm disabled:opacity-50"
          >
            <UploadCloud className="w-4 h-4" />
            <span>{uploading ? 'Deploying...' : 'Deploy Over-the-Air'}</span>
          </button>
        </form>
      </div>
    </div>
  );
}
