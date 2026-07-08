// Usage: node screenshot.js <url> <output.png>
// Requires: npm install puppeteer

const puppeteer = require('puppeteer');
const url = process.argv[2] || 'http://127.0.0.1:5000';
const out = process.argv[3] || 'screenshots/page.png';

(async () => {
  const browser = await puppeteer.launch({ args: ['--no-sandbox', '--disable-setuid-sandbox'] });
  const page = await browser.newPage();
  await page.setViewport({ width: 1280, height: 900 });
  await page.goto(url, { waitUntil: 'networkidle2' });
  await page.screenshot({ path: out, fullPage: true });
  await browser.close();
  console.log('Saved:', out);
})();
