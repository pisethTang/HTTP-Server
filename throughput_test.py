#!/usr/bin/env python3
"""
High-Performance Throughput Test for Titan HTTP Server
Tests requests per second (not just concurrent connections)
"""

import socket
import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed

# CONFIGURATION
TARGET_IP = "127.0.0.1"
TARGET_PORT = 8080
TOTAL_REQUESTS = 10000
CONCURRENT_WORKERS = 100  # Number of threads sending requests

def send_request():
    """Send a single HTTP GET request and measure response time"""
    try:
        start_time = time.time()
        
        # Create socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5)
        s.connect((TARGET_IP, TARGET_PORT))
        
        # Send complete HTTP request
        request = f"GET / HTTP/1.1\r\nHost: {TARGET_IP}\r\nConnection: close\r\n\r\n"
        s.sendall(request.encode('utf-8'))
        
        # Read response (don't care about content, just that it responds)
        response = s.recv(4096)
        s.close()
        
        elapsed = time.time() - start_time
        return True, elapsed
        
    except Exception as e:
        return False, 0

def main():
    print(f"🚀 Throughput Test: {TOTAL_REQUESTS} requests with {CONCURRENT_WORKERS} workers")
    print(f"Target: {TARGET_IP}:{TARGET_PORT}")
    print("-" * 60)
    
    successful = 0
    failed = 0
    total_time_sum = 0
    
    start = time.time()
    
    # Use ThreadPoolExecutor for high concurrency
    with ThreadPoolExecutor(max_workers=CONCURRENT_WORKERS) as executor:
        # Submit all requests
        futures = [executor.submit(send_request) for _ in range(TOTAL_REQUESTS)]
        
        # Track progress
        for i, future in enumerate(as_completed(futures), 1):
            success, elapsed = future.result()
            
            if success:
                successful += 1
                total_time_sum += elapsed
            else:
                failed += 1
            
            # Progress update every 1000 requests
            if i % 1000 == 0:
                print(f"Progress: {i}/{TOTAL_REQUESTS} requests completed...")
    
    total_elapsed = time.time() - start
    
    # Results
    print("\n" + "=" * 60)
    print("📊 RESULTS")
    print("=" * 60)
    print(f"Total Requests:        {TOTAL_REQUESTS}")
    print(f"Successful:            {successful}")
    print(f"Failed:                {failed}")
    print(f"Total Time:            {total_elapsed:.2f} seconds")
    print(f"Requests/Second:       {successful / total_elapsed:.2f} req/s")
    print(f"Avg Response Time:     {(total_time_sum / successful * 1000):.2f} ms")
    print("=" * 60)
    print(f"\n✅ Server handled {successful / total_elapsed:.0f} requests per second!")
    print(f"💡 Check http://{TARGET_IP}:{TARGET_PORT}/stats to verify server-side counters\n")

if __name__ == "__main__":
    main()
