import { createClient } from '@supabase/supabase-js';

// Keys loaded from .env — never hardcode here (DQ risk!)
export const SUPABASE_URL = import.meta.env.VITE_SUPABASE_URL;
export const SUPABASE_ANON_KEY = import.meta.env.VITE_SUPABASE_ANON_KEY;

// Guard: fail fast if env vars are missing (misconfigured deploy)
if (!SUPABASE_URL || !SUPABASE_ANON_KEY) {
  console.error('[supabase] VITE_SUPABASE_URL or VITE_SUPABASE_ANON_KEY is missing. Check dashboard/.env');
}

export const supabase = createClient(SUPABASE_URL ?? '', SUPABASE_ANON_KEY ?? '');

// API base for backend endpoints (same origin in production/port 3000, or port 3000 in dev)
export const API_BASE = (typeof window !== 'undefined' && (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1'))
  ? (window.location.port === '3000' ? '' : (import.meta.env?.VITE_API_URL || 'http://localhost:3000'))
  : '';
