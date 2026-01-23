# Use Ubuntu as base image (has all the Linux syscalls you need)
FROM ubuntu:22.04

# Install build tools
RUN apt-get update && apt-get install -y \
    g++ \
    python3 \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy all files
COPY . .

# Compile the server
RUN g++ server.cpp -o server -pthread

# Railway will set PORT environment variable
# But your server hardcodes 8080, so we expose that
EXPOSE 8080

# Run the server
CMD ["./server"]
