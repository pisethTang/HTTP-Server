# Use Ubuntu as base image (has all the Linux syscalls you need)
FROM ubuntu:22.04

# Install build tools and Node.js (for TypeScript compilation)
RUN apt-get update && apt-get install -y \
    g++ \
    python3 \
    curl \
    && curl -fsSL https://deb.nodesource.com/setup_22.x | bash - \
    && apt-get install -y nodejs \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy dependency files and install Node dependencies first (better caching)
COPY package.json package-lock.json ./
RUN npm ci

# Copy all remaining files
COPY . .

# Compile TypeScript sources to dist/ and build the C++ server
RUN npm run build && g++ server.cpp -o server -pthread

# Railway will set PORT environment variable
# But your server hardcodes 8080, so we expose that
EXPOSE 8080

# Run the server
CMD ["./server"]
