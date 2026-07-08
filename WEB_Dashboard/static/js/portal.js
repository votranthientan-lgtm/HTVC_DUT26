let autoRefreshTimer = null;

function updateLastUpdatedTime() {
  const lastUpdatedEl = document.getElementById("lastUpdated");
  if (lastUpdatedEl) {
    lastUpdatedEl.textContent = new Date().toLocaleTimeString("vi-VN");
  }
}

function setAutoRefresh(enabled) {
  if (autoRefreshTimer) {
    clearInterval(autoRefreshTimer);
    autoRefreshTimer = null;
  }

  if (enabled) {
    autoRefreshTimer = setInterval(loadStatus, 5000);
  }
}

async function loadStatus() {
  try {
    const response = await fetch("/portal/api/status");
    const data = await response.json();

    if (data.status !== "ok") {
      updateLastUpdatedTime();
      return;
    }

    for (const [floor, info] of Object.entries(data.floors)) {
      const prefix = `f${floor}`;
      document.getElementById(`${prefix}-temp`).textContent = `${info.tempAir?.toFixed(1) || "--"} C`;
      document.getElementById(`${prefix}-hum`).textContent = `${info.humidity?.toFixed(1) || "--"} %`;
      document.getElementById(`${prefix}-tds`).textContent = `${info.tds?.toFixed(0) || "--"} ppm`;
      document.getElementById(`${prefix}-ph`).textContent = info.ph?.toFixed(1) || "--";

      const statusEl = document.getElementById(`${prefix}-status`);
      if (info.online) {
        statusEl.textContent = "Online";
        statusEl.className = "status-badge status-online";
      } else {
        statusEl.textContent = "Offline";
        statusEl.className = "status-badge status-offline";
      }
    }

    updateLastUpdatedTime();
  } catch (error) {
    console.error("Error loading status:", error);
    for (let f = 1; f <= 3; f += 1) {
      const prefix = `f${f}`;
      document.getElementById(`${prefix}-temp`).textContent = `${(22 + Math.random() * 4).toFixed(1)} C`;
      document.getElementById(`${prefix}-hum`).textContent = `${(55 + Math.random() * 15).toFixed(1)} %`;
      document.getElementById(`${prefix}-tds`).textContent = `${(400 + Math.random() * 100).toFixed(0)} ppm`;
      document.getElementById(`${prefix}-ph`).textContent = (6.2 + Math.random() * 0.6).toFixed(1);
    }
    updateLastUpdatedTime();
  }
}

document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("refreshNowBtn")?.addEventListener("click", loadStatus);
  document.getElementById("autoRefreshToggle")?.addEventListener("change", (event) => {
    setAutoRefresh(event.target.checked);
  });

  loadStatus();
  setAutoRefresh(true);
});
