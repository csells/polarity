"""Produce a reviewable atlas without running or changing the game."""
from pathlib import Path
from html import escape
from rooms import rooms,names,intent,locations,W,H
colors={'#':'#52677d','^':'#ff6a76','a':'#61f4d4','b':'#ffc457','r':'#bb925a','w':'#397f9e','j':'#ceee82','c':'#fff','E':'#d5f997','u':'#20e4dc','v':'#f9ac32','l':'#fff1d0','m':'#ef606b','h':'#b581d0','=':'#265a61','+':'#67521f','o':'#d5f997'}
out=['<!doctype html><meta charset="utf-8"><title>Polarity room atlas</title><style>body{background:#111824;color:#e3ecd9;font:14px system-ui;margin:32px}main{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:24px}h1,h2{grid-column:1/-1}svg{width:100%;background:#172038}p{line-height:1.5}article{min-width:0}b{font-size:18px}</style><h1>POLARITY / CITY RESTORATION</h1><p>Cyan & amber squares: sockets · white flags: checkpoints · cream envelope: optional letter · lime ring: delivery</p><main>']
for i,(g,name,idea) in enumerate(zip(rooms,names,intent)):
 if i%3==0:out.append('<h2>'+locations[i//3]+'</h2>')
 out.append('<article><b>'+escape(name)+'</b><p>'+escape(idea)+'</p><svg viewBox="0 0 640 144" xmlns="http://www.w3.org/2000/svg">')
 for y,row in enumerate(g):
  for x,c in enumerate(row):
   if c in colors:
    out.append(f'<rect x="{x*8}" y="{y*8}" width="7" height="7" fill="{colors[c]}"/>')
    if c in 'uvlcE':out.append(f'<text x="{x*8+1}" y="{y*8+6}" fill="#10151b" font-size="6">{c.upper()}</text>')
 out.append('</svg></article>')
out.append('</main>');Path('artifacts').mkdir(exist_ok=True);Path('artifacts/room-atlas.html').write_text(''.join(out));print('Wrote artifacts/room-atlas.html: 18 authored routes with design intent.')
