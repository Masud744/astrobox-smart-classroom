/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      fontFamily: {
        sans: ['Manrope', 'system-ui', '-apple-system', 'sans-serif'],
        mono: ['"Geist Mono"', 'monospace'],
      },
      colors: {
        canvas: '#F4F5F7',
        surface: '#FFFFFF',
        subtle: '#EBECEF',
        brand: {
          50: '#EEF2FF',
          100: '#E0E7FF',
          500: '#6366F1',
          600: '#4F46E5',
          700: '#4338CA',
        }
      }
    },
  },
  plugins: [],
}
