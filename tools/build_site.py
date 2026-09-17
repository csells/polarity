from pathlib import Path
import shutil
root = Path(__file__).resolve().parent.parent
out = root / 'dist'
if out.exists():
    shutil.rmtree(out)
shutil.copytree(root / 'web', out / 'client')
(out / 'server').mkdir()
shutil.copyfile(root / 'hosting-worker.js', out / 'server' / 'index.js')
assert (out / 'client' / 'index.html').is_file()
assert (out / 'client' / 'vendor' / 'mgba' / 'mgba.wasm').is_file()
rom = (out / 'client' / 'polarity.gba').read_bytes()
assert 192 < len(rom) <= 32 * 1024 * 1024
assert (sum(rom[160:190]) + 0x19) & 255 == 0
assert (out / 'client' / 'color' / 'polarity.gbc').stat().st_size == 32768
print('Public game build ready: web player, emulator, and cartridge.')
