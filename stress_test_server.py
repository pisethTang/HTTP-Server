#!/usr/bin/env python3
"""
Stress Test Trigger Server
Runs on port 8081 and accepts requests from the browser to trigger high-performance stress tests
Uses curl (pre-installed on most Linux systems) instead of Python sockets
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import subprocess
import threading
import json
import time
from urllib.parse import urlparse, parse_qs

# Configuration
LISTEN_PORT = 8081
TARGET_SERVER = "http://127.0.0.1:8080"

class StressTestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        parsed = urlparse(self.path)
        
        # CORS headers for browser
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        
        if parsed.path == '/trigger-stress':
            # Parse request count from query params
            params = parse_qs(parsed.query)
            count = int(params.get('count', [100])[0])
            
            print(f"🚀 Triggering stress test: {count} requests using curl")
            
            # Run stress test in background thread
            def run_test():
                start = time.time()
                
                # Use xargs to run curl in parallel (GNU parallel would be better but may not be installed)
                # This creates N processes that each run curl once
                # seq generates numbers 1 to count, xargs runs them in parallel with -P flag
                cmd = f'seq {count} | xargs -P 100 -I {{}} curl -s -o /dev/null {TARGET_SERVER}/'
                
                try:
                    subprocess.run(cmd, shell=True, timeout=60)
                    elapsed = time.time() - start
                    print(f"✅ Completed {count} requests in {elapsed:.2f}s ({count/elapsed:.0f} req/s)")
                except subprocess.TimeoutExpired:
                    print(f"⚠️ Stress test timed out after 60 seconds")
                except Exception as e:
                    print(f"❌ Error running stress test: {e}")
            
            threading.Thread(target=run_test, daemon=True).start()
            
            response = {'status': 'started', 'count': count}
            self.wfile.write(json.dumps(response).encode())
        else:
            response = {'error': 'Unknown endpoint'}
            self.wfile.write(json.dumps(response).encode())
    
    def do_OPTIONS(self):
        # Handle CORS preflight
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()
    
    def log_message(self, format, *args):
        # Suppress default logging
        pass

def main():
    server = HTTPServer(('0.0.0.0', LISTEN_PORT), StressTestHandler)
    print(f"🔥 Stress Test Server running on port {LISTEN_PORT}")
    print(f"   Target: {TARGET_SERVER}")
    print(f"   Using: curl + xargs (pre-installed tools)")
    print(f"   Ready to accept browser triggers...\n")
    server.serve_forever()

if __name__ == "__main__":
    main()
