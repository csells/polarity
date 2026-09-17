from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler
import os
os.chdir(os.path.join(os.path.dirname(__file__),'web'))
class Handler(SimpleHTTPRequestHandler):
 def end_headers(self):
  self.send_header('Cache-Control','no-cache')
  self.send_header('X-Content-Type-Options','nosniff')
  super().end_headers()
print('Polarity web emulator listening on port 8787',flush=True)
ThreadingHTTPServer(('0.0.0.0',8787),Handler).serve_forever()
