import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vite.dev/config/
export default defineConfig({
  plugins: [react()],
  // GitHub Pages 部署时的路径前缀
  // 如果仓库名是 ffmpeg-practices，则访问地址是 https://username.github.io/ffmpeg-practices/
  base: process.env.GITHUB_ACTIONS ? '/ffmpeg-practices/' : '/',
})
