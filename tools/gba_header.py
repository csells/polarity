from pathlib import Path
p=Path('build/polarity.gba');b=bytearray(p.read_bytes())
# The fixed cartridge identification bytes required by the GBA boot ROM.
b[4:160]=bytes.fromhex('24ffae51699aa2213d84820a84e409ad11248b98c0817f21a352be199309ce2010464a4af82731ec58c7e83382e3cebf85f4df94ce4b09c194568ac01372a7fc9f844d73a3ca9a615897a327fc039876231dc7610304ae56bf38840040a70efdff52fe036f9530f197fbc08560d68025a963be03014e38e2f9a234ffbb3e0344780090cb88113a9465c07c6387f03cafd625e48b380aac7221d4f807')
b[160:172]=b'POLARITY ADV';b[172:176]=b'PLRE';b[176:178]=b'CS';b[178]=0x96;b[179:189]=bytes(10);b[189]=(-sum(b[160:189])-0x19)&255;b[190:192]=bytes(2)
b.extend(bytes((-len(b))%4));p.write_bytes(b)
assert (sum(b[160:190])+0x19)&255==0
print(f'GBA cartridge: {len(b):,} bytes, header checksum verified')
