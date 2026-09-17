"""Convert the checked-in original artwork to GBA color/tile formats.

Optional authoring step: pip install Pillow==12.3.0. Ordinary ROM builds use
the checked-in packed files and need only Python's standard library.
"""
from pathlib import Path
from PIL import Image, ImageOps
import struct

ROOT = Path('assets/gba')
OUT = ROOT / 'packed'
OUT.mkdir(exist_ok=True)

def rgb15(rgb):
    r,g,b = rgb[:3]
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)

def palette(im, count):
    p = im.getpalette()
    return [tuple(p[i*3:i*3+3]) for i in range(count)]

def write_palette(name, colors):
    (OUT / (name+'.pal')).write_bytes(struct.pack('<'+'H'*len(colors), *map(rgb15,colors)))

def tiled(im, bpp):
    p = list(im.get_flattened_data()); w,h = im.size; out = bytearray()
    for ty in range(0,h,8):
        for tx in range(0,w,8):
            for y in range(8):
                for x in range(0,8,2 if bpp==4 else 1):
                    v = p[(ty+y)*w+tx+x]
                    if bpp==4: v |= p[(ty+y)*w+tx+x+1]<<4
                    out.append(v)
    return out

atlas = Image.open(ROOT/'source/districts.png').convert('RGB')
for i in range(7):
    if i<6:
        x,y=i%2,i//2; w,h=atlas.size
        source=atlas.crop((x*w//2,y*h//3,(x+1)*w//2,(y+1)*h//3))
    else: source=Image.open(ROOT/'source/city-map.png').convert('RGB')
    im=source.resize((256,160),Image.Resampling.LANCZOS).quantize(colors=128,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE)
    (OUT/f'scene-{i}.tiles').write_bytes(tiled(im,8))
    write_palette(f'scene-{i}',palette(im,128))
    im.resize((768,480),Image.Resampling.NEAREST).save(OUT/f'scene-{i}-preview.png')
    print(f'[{i+1}/7] Packed 256x160 background, 128 colors',flush=True)

def cell(sheet,x,y,cols,rows):
    w,h=sheet.size
    return sheet.crop((x*w//cols,y*h//rows,(x+1)*w//cols,(y+1)*h//rows))

def fit_sprite(im, size, height):
    # Alpha is supplied by the generator; preserve it in palette index zero.
    alpha=im.getchannel('A').point(lambda p:255 if p>160 else 0)
    box=alpha.getbbox()
    if not box: raise ValueError('Missing sprite')
    im=im.crop(box)
    scale=min((size-2)/im.width,height/im.height)
    im=im.resize((max(1,round(im.width*scale)),max(1,round(im.height*scale))),Image.Resampling.LANCZOS)
    out=Image.new('RGBA',(size,size))
    out.alpha_composite(im,((size-im.width)//2,size-1-im.height))
    return out

def quantize_sprites(frames, name):
    pixels=[]
    for im in frames:
        pixels.extend([p[:3] for p in im.get_flattened_data() if p[3]>160])
    strip=Image.new('RGB',(len(pixels),1));strip.putdata(pixels)
    colors=palette(strip.quantize(colors=15,method=Image.Quantize.MEDIANCUT),15)
    pal=Image.new('P',(1,1));pal.putpalette(sum((list(c) for c in colors),[])+list(colors[-1])*(256-15))
    result=[]
    for im in frames:
        q=im.convert('RGB').quantize(palette=pal,dither=Image.Dither.NONE)
        q.putdata([min(n,14)+1 if a>160 else 0 for n,a in zip(q.get_flattened_data(),im.getchannel('A').get_flattened_data())])
        q.putpalette([0,0,0]+sum((list(c) for c in colors),[])+[0]*720)
        result.append(q)
    (OUT/(name+'.tiles')).write_bytes(b''.join(tiled(im,4) for im in result))
    write_palette(name,[(0,0,0)]+colors)
    return result,colors

sheet=Image.open(ROOT/'source/courier.png').convert('RGBA')
# Use the same scale for all animation poses; a crouch must not grow to standing height.
frames=[]
for i in range(16):
    im=cell(sheet,i%4,i//4,4,4).resize((32,32),Image.Resampling.LANCZOS)
    frames.append(im)
qs,colors=quantize_sprites(frames,'courier')
amber=[]
for r,g,b in colors:
    amber.append((min(255,int(g*1.22)),min(225,int(g*.79)),int(b*.38)) if b>r*1.25 and g>r*1.15 else (r,g,b))
write_palette('courier-amber',[(0,0,0)]+amber)
sheet=Image.open(ROOT/'source/neighbors.png').convert('RGBA')
for i in range(6):
    im=cell(sheet,i%3,i//3,3,2)
    body=fit_sprite(im,32,30)
    # The portraits are independently sampled from the head and shoulders,
    # rather than enlargements of the tiny in-world sprite.
    portrait=ImageOps.fit(im.crop((64,0,448,340)),(64,64),method=Image.Resampling.LANCZOS)
    quantize_sprites([body,portrait],f'neighbor-{i}')
print('Packed 16 courier poses and six resident sprites / 64px portraits',flush=True)

sheet=Image.open(ROOT/'source/terrain.png').convert('RGBA')
for i in range(6):
    roof=cell(sheet,i,0,6,2).resize((16,16),Image.Resampling.LANCZOS)
    wall=cell(sheet,i,1,6,2).resize((16,16),Image.Resampling.LANCZOS)
    quantize_sprites([roof,wall],f'terrain-{i}')
print('Packed six district-specific roof and wall materials',flush=True)
