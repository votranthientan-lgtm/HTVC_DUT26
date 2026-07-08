Automated screenshots (Puppeteer)

1) Install Node.js (if not installed).

2) From project root, install puppeteer:

   npm install puppeteer

3) Create a folder for screenshots:

   mkdir screenshots

4) Run the script (example):

   node tools/screenshot.js http://127.0.0.1:5000 screenshots/home.png

This captures the full page and saves to `screenshots/home.png`.

Alternative: use Chrome headless (Windows example):

"C:\Program Files\Google\Chrome\Application\chrome.exe" --headless --disable-gpu --screenshot="screenshots/home.png" --window-size=1280,900 http://127.0.0.1:5000

Manual: open the app at http://127.0.0.1:5000 and use browser DevTools -> Run command -> "Capture full size screenshot" or use Snipping Tool for ad-hoc images.
