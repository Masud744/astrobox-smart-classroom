import { createClient } from '@supabase/supabase-js';

export const SUPABASE_URL = 'https://oxxfnnecwvslmkxqhfgb.supabase.co';
export const SUPABASE_ANON_KEY = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im94eGZubmVjd3ZzbG1reHFoZmdiIiwicm9sZSI6ImFub24iLCJpYXQiOjE3OTQ1MDM5MzgsImV4cCI6MjExMDA3OTkzOH0.VNpqjyTHanZ4wgwZU9VT0aATGFsyppEWr8m7__x_UZc';

export const supabase = createClient(SUPABASE_URL, SUPABASE_ANON_KEY);

// API base for backend endpoints (same origin in production/port 3000, or port 3000 in dev)
export const API_BASE = (typeof window !== 'undefined' && (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1'))
  ? (window.location.port === '3000' ? '' : (import.meta.env?.VITE_API_URL || 'http://localhost:3000'))
  : '';
