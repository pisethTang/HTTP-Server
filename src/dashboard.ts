export {};

function getErrorMessage(error: unknown): string {
    return error instanceof Error ? error.message : 'Unknown error';
}

async function deleteFile(): Promise<void> {
    const statusElement = document.getElementById('status');
    if (!statusElement) return;

    try {
        const response = await fetch('/uploaded_data.txt', { method: 'DELETE' });
        if (!response.ok) {
            throw new Error('Delete failed (404?)');
        }
        const message = await response.text();
        statusElement.innerText = `✅ Server says: ${message}`;
    } catch (error) {
        statusElement.innerText = `❌ Error: ${getErrorMessage(error)}`;
    }
}

function checkCookie(): void {
    const display = document.getElementById('cookie_display');
    if (!display) return;

    display.innerText = document.cookie
        ? `Found: ${document.cookie}`
        : 'No cookies found (Did you enable them in server.cpp?)';
}

document.addEventListener('DOMContentLoaded', () => {
    const deleteButton = document.getElementById('delete-btn');
    const cookieButton = document.getElementById('cookie-btn');

    deleteButton?.addEventListener('click', deleteFile);
    cookieButton?.addEventListener('click', checkCookie);
});
