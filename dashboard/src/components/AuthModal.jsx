import React, { useState } from 'react';
import { X, Lock, Mail, UserCheck, ShieldCheck, ArrowRight } from 'lucide-react';
import { supabase } from '../lib/supabase';

export default function AuthModal({ isOpen, onClose, onAuthSuccess }) {
  if (!isOpen) return null;

  const [isSignUp, setIsSignUp] = useState(false);
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [errorMsg, setErrorMsg] = useState('');
  const [loading, setLoading] = useState(false);

  const handleSubmit = async (e) => {
    e.preventDefault();
    setErrorMsg('');
    setLoading(true);

    try {
      if (isSignUp) {
        const { data, error } = await supabase.auth.signUp({
          email,
          password,
          options: {
            data: { role: 'teacher', full_name: email.split('@')[0] }
          }
        });
        if (error) throw error;
        onAuthSuccess(data.user, 'teacher');
        onClose();
      } else {
        const { data, error } = await supabase.auth.signInWithPassword({
          email,
          password
        });
        if (error) throw error;
        onAuthSuccess(data.user, data.user.user_metadata?.role || 'teacher');
        onClose();
      }
    } catch (err) {
      setErrorMsg(err.message || 'Authentication failed');
    } finally {
      setLoading(false);
    }
  };

  // Instant 1-click Demo Teacher login for competition judges
  const handleDemoLogin = () => {
    const demoUser = {
      id: 'demo-teacher-01',
      email: 'teacher@fastrobox.edu',
      user_metadata: {
        full_name: 'Totok Michael',
        role: 'teacher'
      }
    };
    onAuthSuccess(demoUser, 'teacher');
    onClose();
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/40 backdrop-blur-sm select-none">
      <div className="bg-white rounded-3xl max-w-md w-full border border-[#EBECEF] p-8 shadow-2xl relative animate-in fade-in zoom-in-95 duration-200">
        {/* Close Button */}
        <button
          onClick={onClose}
          className="absolute right-6 top-6 w-8 h-8 rounded-full border border-[#E5E7EB] flex items-center justify-center text-[#9CA3AF] hover:text-[#111827] hover:bg-[#F9FAFB] transition-all"
        >
          <X className="w-4 h-4" />
        </button>

        {/* Modal Header */}
        <div className="text-center mb-6">
          <div className="w-12 h-12 rounded-2xl bg-[#EEF2FF] text-[#6366F1] flex items-center justify-center mx-auto mb-3 shadow-sm">
            <ShieldCheck className="w-6 h-6" />
          </div>
          <h2 className="text-[20px] font-extrabold text-[#111827] tracking-tight">
            {isSignUp ? 'Create Instructor Account' : 'Classroom Access Portal'}
          </h2>
          <p className="text-[13px] text-[#6B7280] mt-1">
            Authenticate to control door locks and system configurations
          </p>
        </div>

        {/* Error Alert */}
        {errorMsg && (
          <div className="mb-4 p-3 rounded-xl bg-[#FEF2F2] border border-[#FEE2E2] text-[12px] font-semibold text-[#EF4444]">
            {errorMsg}
          </div>
        )}

        {/* Form */}
        <form onSubmit={handleSubmit} className="space-y-4">
          <div>
            <label className="block text-[12px] font-bold text-[#374151] mb-1.5 uppercase tracking-wider">
              Email Address
            </label>
            <div className="relative">
              <Mail className="w-4 h-4 text-[#9CA3AF] absolute left-3.5 top-1/2 -translate-y-1/2" />
              <input
                type="email"
                required
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                placeholder="instructor@classroom.edu"
                className="w-full bg-[#F9FAFB] border border-[#E5E7EB] rounded-xl pl-10 pr-4 py-2.5 text-[13px] font-medium text-[#111827] focus:outline-none focus:border-[#6366F1] focus:bg-white transition-all"
              />
            </div>
          </div>

          <div>
            <label className="block text-[12px] font-bold text-[#374151] mb-1.5 uppercase tracking-wider">
              Password
            </label>
            <div className="relative">
              <Lock className="w-4 h-4 text-[#9CA3AF] absolute left-3.5 top-1/2 -translate-y-1/2" />
              <input
                type="password"
                required
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                placeholder="••••••••••••"
                className="w-full bg-[#F9FAFB] border border-[#E5E7EB] rounded-xl pl-10 pr-4 py-2.5 text-[13px] font-medium text-[#111827] focus:outline-none focus:border-[#6366F1] focus:bg-white transition-all"
              />
            </div>
          </div>

          <button
            type="submit"
            disabled={loading}
            className="w-full py-2.5 rounded-xl bg-[#6366F1] text-white text-[13px] font-bold hover:bg-[#4F46E5] transition-all flex items-center justify-center gap-2 shadow-sm disabled:opacity-50"
          >
            <span>{loading ? 'Processing...' : isSignUp ? 'Sign Up' : 'Sign In'}</span>
            <ArrowRight className="w-4 h-4" />
          </button>
        </form>

        {/* 1-Click Demo Teacher Access (Crucial for judging) */}
        <div className="mt-5 pt-5 border-t border-[#F3F4F6]">
          <p className="text-[11px] font-bold text-[#9CA3AF] uppercase tracking-wider text-center mb-3">
            Instant Judge / Evaluator Mode
          </p>
          <button
            onClick={handleDemoLogin}
            type="button"
            className="w-full py-2.5 rounded-xl bg-[#F3F4F6] hover:bg-[#E5E7EB] text-[#111827] text-[13px] font-bold transition-all flex items-center justify-center gap-2"
          >
            <UserCheck className="w-4 h-4 text-[#6366F1]" />
            <span>1-Click Demo Teacher Login</span>
          </button>
        </div>

        {/* Toggle Sign Up / Sign In */}
        <div className="text-center mt-5">
          <button
            type="button"
            onClick={() => {
              setIsSignUp(!isSignUp);
              setErrorMsg('');
            }}
            className="text-[12px] font-semibold text-[#6366F1] hover:underline"
          >
            {isSignUp ? 'Already have an account? Sign In' : "Don't have an account? Create one"}
          </button>
        </div>
      </div>
    </div>
  );
}
