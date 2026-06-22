export {};

interface ServerStats {
    requests: number;
    active_connections: number;
}

type FeedbackType = 'success' | 'error' | 'loading';

const DEFAULT_POLL_INTERVAL_MS = 2000;
const STRESS_POLL_INTERVAL_MS = 50;

let statsInterval: number;
let stressTestActive = false;
let stressTestTarget = 0;

function getElement<T extends HTMLElement = HTMLElement>(id: string): T {
    const element = document.getElementById(id);
    if (!element) {
        throw new Error(`Element with id "${id}" not found`);
    }
    return element as T;
}

function setText(id: string, text: string | number): void {
    getElement(id).textContent = String(text);
}

function renderResponse(id: string, content: string, type: FeedbackType = 'loading'): void {
    const element = getElement(id);
    element.classList.remove('hidden');

    if (type === 'loading') {
        element.innerHTML = `<div class="inline-block w-5 h-5 border-4 border-gray-300 border-t-blue-600 rounded-full animate-spin"></div> ${escapeHtml(content)}`;
    } else if (type === 'success') {
        element.innerHTML = '<span class="text-green-600 font-bold">✅ SUCCESS</span>\n\n' + escapeHtml(content);
    } else {
        element.innerHTML = '<span class="text-red-600 font-bold">❌ ERROR</span>\n\n' + escapeHtml(content);
    }
}

function escapeHtml(text: string): string {
    return text
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#039;');
}

async function fetchStats(): Promise<ServerStats | null> {
    try {
        const response = await fetch('/stats');
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        return (await response.json()) as ServerStats;
    } catch (error) {
        console.error('Stats update failed:', error);
        return null;
    }
}

async function updateStats(): Promise<void> {
    const data = await fetchStats();
    if (!data) return;

    setText('total-requests', data.requests);
    setText('active-connections', data.active_connections);
}

function startStatsPolling(intervalMs: number): void {
    clearInterval(statsInterval);
    statsInterval = window.setInterval(updateStats, intervalMs);
}

function initRailwayLimits(): void {
    const isRailway = window.location.hostname.includes('railway.app');
    if (!isRailway) return;

    const warning = document.getElementById('railway-warning');
    const select = getElement<HTMLSelectElement>('stress-request-count');

    if (warning) {
        warning.classList.remove('hidden');
    }

    const allowed = new Set(['10', '100', '500']);
    Array.from(select.options).forEach((option) => {
        if (!allowed.has(option.value)) {
            option.disabled = true;
            option.hidden = true;
        }
    });

    if (!allowed.has(select.value)) {
        select.value = '500';
    }
}

async function uploadFile(): Promise<void> {
    const content = getElement<HTMLTextAreaElement>('upload-content').value;
    renderResponse('upload-response', 'Sending POST request...');

    try {
        const response = await fetch('/upload', {
            method: 'POST',
            headers: { 'Content-Type': 'text/plain' },
            body: content,
        });
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        const data = await response.text();
        renderResponse('upload-response', data, 'success');
    } catch (error) {
        renderResponse('upload-response', getErrorMessage(error), 'error');
    }
}

async function executeCGI(): Promise<void> {
    renderResponse('cgi-response', 'Executing Python script...\n\n⏱️ This will take 10 seconds (demonstrating non-blocking thread pool)');
    const startTime = Date.now();

    try {
        const response = await fetch('/time');
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        const data = await response.text();
        const elapsed = ((Date.now() - startTime) / 1000).toFixed(1);
        renderResponse('cgi-response', `CGI EXECUTED (took ${elapsed}s)\n\n${data}`, 'success');
    } catch (error) {
        renderResponse('cgi-response', getErrorMessage(error), 'error');
    }
}

async function deleteFile(): Promise<void> {
    const path = getElement<HTMLInputElement>('delete-path').value;
    renderResponse('delete-response', `Sending DELETE request for: ${path}`);

    try {
        const response = await fetch(path, { method: 'DELETE' });
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        const data = await response.text();
        renderResponse('delete-response', data, 'success');
    } catch (error) {
        renderResponse('delete-response', getErrorMessage(error), 'error');
    }
}

function checkCookies(): void {
    const cookieDiv = getElement('cookie-display');
    cookieDiv.classList.remove('hidden');

    if (document.cookie) {
        cookieDiv.innerHTML = `🍪 <strong>Found Session:</strong> ${escapeHtml(document.cookie)}`;
    } else {
        cookieDiv.innerHTML = '🍪 <strong>No cookies detected.</strong> The server sets "session_id" on first visit.';
    }
}

function updateStressUI(running: boolean): void {
    getElement<HTMLButtonElement>('stress-start-btn').disabled = running;
    getElement<HTMLButtonElement>('stress-stop-btn').disabled = !running;
    getElement<HTMLSelectElement>('stress-request-count').disabled = running;
}

async function startStressTest(): Promise<void> {
    if (stressTestActive) return;

    const requestCount = parseInt(getElement<HTMLSelectElement>('stress-request-count').value, 10);
    stressTestTarget = requestCount;
    stressTestActive = true;

    startStatsPolling(STRESS_POLL_INTERVAL_MS);
    updateStats();

    setText('stress-target', requestCount);
    setText('stress-progress', 'Triggering...');
    setText('stress-test-count', '⏳');
    updateStressUI(true);

    try {
        const response = await fetch(`http://localhost:8080/trigger-stress?count=${requestCount}`);
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        const data = (await response.json()) as unknown;
        console.log('Stress test triggered:', data);
        setText('stress-progress', `Running ${requestCount} requests...`);

        const estimatedTime = Math.max(3000, requestCount * 2);
        setTimeout(() => {
            if (stressTestActive) {
                stopStressTest();
            }
        }, estimatedTime);
    } catch (error) {
        console.error('Failed to trigger stress test:', error);
        setText('stress-progress', '❌ Error triggering stress test');
        setText('stress-test-count', '0');
        setTimeout(stopStressTest, 2000);
    }
}

function stopStressTest(): void {
    stressTestActive = false;

    startStatsPolling(DEFAULT_POLL_INTERVAL_MS);
    updateStats();

    updateStressUI(false);
    setText('stress-progress', 'Complete');
    setText('stress-test-count', '✓');

    setTimeout(() => {
        if (!stressTestActive) {
            setText('stress-test-count', '0');
            setText('stress-progress', '0');
            setText('stress-target', '0');
        }
    }, 3000);
}

function getErrorMessage(error: unknown): string {
    return error instanceof Error ? error.message : 'Unknown error';
}

function init(): void {
    updateStats();
    statsInterval = window.setInterval(updateStats, DEFAULT_POLL_INTERVAL_MS);

    document.addEventListener('DOMContentLoaded', () => {
        initRailwayLimits();

        getElement('upload-btn').addEventListener('click', uploadFile);
        getElement('cgi-btn').addEventListener('click', executeCGI);
        getElement('delete-btn').addEventListener('click', deleteFile);
        getElement('cookie-btn').addEventListener('click', checkCookies);
        getElement('stress-start-btn').addEventListener('click', startStressTest);
        getElement('stress-stop-btn').addEventListener('click', stopStressTest);
    });
}

init();
