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
assert (out / 'client' / 'vendor' / 'binjgb.wasm').is_file()
assert (out / 'client' / 'polarity.gbc').stat().st_size == 32768
print('Public game build ready: web player, emulator, and cartridge.')
