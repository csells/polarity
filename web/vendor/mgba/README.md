# mGBA WebAssembly runtime

Unmodified `mgba.js` and `mgba.wasm` from `@wasm-gaming/mgba-wasm@0.1.1`, pinned in the root package-lock.json. MPL-2.0 license is in LICENSE.

- Wrapper and build/shim source: https://github.com/wasm-gaming/mGBA-wasm
- Emulator source: https://github.com/mgba-emu/mgba
- Published package and source files: https://www.npmjs.com/package/@wasm-gaming/mgba-wasm/v/0.1.1

To restore the exact binaries after `npm ci`, copy `node_modules/@wasm-gaming/mgba-wasm/dist/mgba/mgba.{js,wasm}` here. The player drives the upstream C shim directly so keyboard, calibrated controllers, touch, saves, and deterministic tests share one input path. No proprietary BIOS is distributed; mGBA supplies its HLE implementation.
