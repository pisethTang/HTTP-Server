// Auto-update stats every 2 seconds
function updateStats() {
    fetch('/stats')
        .then(response => response.json())
        .then(data => {
            document.getElementById('total-requests').textContent = data.requests;
            document.getElementById('active-connections').textContent = data.active_connections;
        })
        .catch(err => console.error('Stats update failed:', err));
}

// Initial update and set interval (faster polling during stress test)
updateStats();
let statsInterval = setInterval(updateStats, 2000); // Default: every 2 seconds

// Limit stress test options on Railway deployments
document.addEventListener('DOMContentLoaded', () => {
    const isRailway = window.location.hostname.includes('railway.app');
    if (!isRailway) return;

    const select = document.getElementById('stress-request-count');
    const warning = document.getElementById('railway-warning');

    if (warning) {
        warning.classList.remove('hidden');
    }

    if (select) {
        const allowed = new Set(['10', '100', '500']);
        Array.from(select.options).forEach(option => {
            if (!allowed.has(option.value)) {
                option.disabled = true;
                option.hidden = true;
            }
        });

        if (!allowed.has(select.value)) {
            select.value = '500';
        }
    }
});

// File Upload Demo
function uploadFile() {
    const content = document.getElementById('upload-content').value;
    const responseDiv = document.getElementById('upload-response');
    responseDiv.classList.remove('hidden');
    responseDiv.innerHTML = '<div class="inline-block w-5 h-5 border-4 border-gray-300 border-t-blue-600 rounded-full loading"></div> Sending POST request...';
    
    fetch('/upload', {
        method: 'POST',
        headers: {'Content-Type': 'text/plain'},
        body: content
    })
    .then(response => response.text())
    .then(data => {
        responseDiv.innerHTML = '<span class="text-green-600 font-bold">✅ SUCCESS</span>\n\n' + data;
    })
    .catch(err => {
        responseDiv.innerHTML = '<span class="text-red-600 font-bold">❌ ERROR</span>\n\n' + err.message;
    });
}

// CGI Execution Demo
function executeCGI() {
    const responseDiv = document.getElementById('cgi-response');
    responseDiv.classList.remove('hidden');
    responseDiv.innerHTML = '<div class="inline-block w-5 h-5 border-4 border-gray-300 border-t-blue-600 rounded-full loading"></div> Executing Python script...\n\n⏱️ This will take 10 seconds (demonstrating non-blocking thread pool)';
    
    const startTime = Date.now();
    
    fetch('/time')
        .then(response => response.text())
        .then(data => {
            const elapsed = ((Date.now() - startTime) / 1000).toFixed(1);
            responseDiv.innerHTML = '<span class="text-green-600 font-bold">✅ CGI EXECUTED</span> (took ' + elapsed + 's)\n\n' + data;
        })
        .catch(err => {
            responseDiv.innerHTML = '<span class="text-red-600 font-bold">❌ ERROR</span>\n\n' + err.message;
        });
}

// File Delete Demo
function deleteFile() {
    const path = document.getElementById('delete-path').value;
    const responseDiv = document.getElementById('delete-response');
    responseDiv.classList.remove('hidden');
    responseDiv.innerHTML = '<div class="inline-block w-5 h-5 border-4 border-gray-300 border-t-blue-600 rounded-full loading"></div> Sending DELETE request for: ' + path;
    
    fetch(path, {method: 'DELETE'})
        .then(response => response.text())
        .then(data => {
            responseDiv.innerHTML = '<span class="text-green-600 font-bold">✅ SUCCESS</span>\n\n' + data;
        })
        .catch(err => {
            responseDiv.innerHTML = '<span class="text-red-600 font-bold">❌ ERROR</span>\n\n' + err.message;
        });
}

// Cookie Check Demo
function checkCookies() {
    const cookieDiv = document.getElementById('cookie-display');
    cookieDiv.classList.remove('hidden');
    
    if (document.cookie) {
        cookieDiv.innerHTML = '🍪 <strong>Found Session:</strong> ' + document.cookie;
    } else {
        cookieDiv.innerHTML = '🍪 <strong>No cookies detected.</strong> The server sets "session_id" on first visit.';
    }
}

// Stress Test Variables
let stressTestActive = false;
let stressTestTarget = 0;

// Start Stress Test
function startStressTest() {
    if (stressTestActive) return;
    
    const requestCount = parseInt(document.getElementById('stress-request-count').value);
    stressTestTarget = requestCount;
    stressTestActive = true;
    
    // Speed up stats polling during stress test
    clearInterval(statsInterval);
    statsInterval = setInterval(updateStats, 50); // Poll every 50ms during test for instant updates
    updateStats(); // Immediate update before starting
    
    // Update UI
    document.getElementById('stress-start-btn').disabled = true;
    document.getElementById('stress-stop-btn').disabled = false;
    document.getElementById('stress-request-count').disabled = true;
    document.getElementById('stress-target').textContent = requestCount;
    document.getElementById('stress-progress').textContent = 'Triggering...';
    document.getElementById('stress-test-count').textContent = '⏳';
    
    // Trigger Python stress test server (running on port 8081)
    fetch(`http://localhost:8080/trigger-stress?count=${requestCount}`)
        .then(response => response.json())
        .then(data => {
            console.log('Stress test triggered:', data);
            document.getElementById('stress-progress').textContent = `Running ${requestCount} requests...`;
            
            // Auto-stop after reasonable time (based on request count)
            const estimatedTime = Math.max(3000, requestCount * 2); // Minimum 3 seconds
            setTimeout(() => {
                if (stressTestActive) {
                    stopStressTest();
                }
            }, estimatedTime);
        })
        .catch(err => {
            console.error('Failed to trigger stress test:', err);
            document.getElementById('stress-progress').textContent = '❌ Error triggering stress test';
            document.getElementById('stress-test-count').textContent = '0';
            setTimeout(() => stopStressTest(), 2000);
        });
}

// Stop Stress Test
function stopStressTest() {
    stressTestActive = false;
    
    // Restore normal stats polling speed
    clearInterval(statsInterval);
    statsInterval = setInterval(updateStats, 2000); // Back to 2 seconds
    updateStats(); // One final immediate update
    
    // Update UI
    document.getElementById('stress-start-btn').disabled = false;
    document.getElementById('stress-stop-btn').disabled = true;
    document.getElementById('stress-request-count').disabled = false;
    document.getElementById('stress-progress').textContent = 'Complete';
    document.getElementById('stress-test-count').textContent = '✓';
    
    // Reset after delay
    setTimeout(() => {
        if (!stressTestActive) {
            document.getElementById('stress-test-count').textContent = '0';
            document.getElementById('stress-progress').textContent = '0';
            document.getElementById('stress-target').textContent = '0';
        }
    }, 3000);
}
