from pathlib import Path
import shutil, hashlib
root = Path(__file__).resolve().parent.parent
out = root / 'dist'
if out.exists():
    shutil.rmtree(out)
shutil.copytree(root / 'web', out / 'client')
# Identify the exact browser code in downloaded crash reports, independent of
# whether this build was made before or after its source commit was created.
identity=hashlib.sha256()
for name in ('app.js','diagnostics.js','controller-profile.js'):
    identity.update(name.encode()+b'\0'+(root/'web'/name).read_bytes())
index=out/'client'/'index.html'
html=index.read_text()
assert '</head>' in html
index.write_text(html.replace('</head>',f'<meta name="polarity-build" content="{identity.hexdigest()}"></head>',1))
(out / 'server').mkdir()
shutil.copyfile(root / 'hosting-worker.js', out / 'server' / 'index.js')
assert (out / 'client' / 'index.html').is_file()
assert (out / 'client' / 'vendor' / 'mgba' / 'mgba.wasm').is_file()
rom = (out / 'client' / 'polarity.gba').read_bytes()
assert 192 < len(rom) <= 32 * 1024 * 1024
assert (sum(rom[160:190]) + 0x19) & 255 == 0
assert (out / 'client' / 'color' / 'polarity.gbc').stat().st_size == 32768
print('Public game build ready: web player, emulator, and cartridge.')
