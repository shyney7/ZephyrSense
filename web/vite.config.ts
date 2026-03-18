import { defineConfig } from "vite";
import cesium from "vite-plugin-cesium-build";
// import cesium from 'vite-plugin-cesium-build/engine' // when using @cesium/engine
//docs: https://github.com/s3xysteak/vite-plugin-cesium-build

export default defineConfig({
  plugins: [cesium()],
  base: "./",
  build: {
    outDir: "dist",
    emptyOutDir: true,
  },
});
