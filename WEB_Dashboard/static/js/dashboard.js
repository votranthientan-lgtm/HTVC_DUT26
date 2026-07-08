const CONFIG = {
  apiBase: "/api",
  updateInterval: 5000,
  mqttPort: 8883,
  sse: null,
  charts: {
    temp: null,
    hum: null,
  },
  deviceId: localStorage.getItem("htvc-deviceId") || "floor1",
  floors: {
    1: {
      led: false,
      pump: false,
      fan: false,
      tempAir: null,
      humidity: null,
      tds: null,
      ph: null,
    },
    2: {
      led: false,
      pump: false,
      fan: false,
      tempAir: null,
      humidity: null,
      tds: null,
      ph: null,
    },
    3: {
      led: false,
      pump: false,
      fan: false,
      tempAir: null,
      humidity: null,
      tds: null,
      ph: null,
    },
  },
};

const PENDING_TIMEOUT_MS = 10000;
const pendingCommands = {};

function _pendingKey(floor, target) {
  return `${floor}:${target}`;
}

function _setPendingCommand(floor, target, commandId, value) {
  const key = _pendingKey(floor, target);
  pendingCommands[key] = {
    commandId,
    value: Boolean(value),
    expiresAt: Date.now() + PENDING_TIMEOUT_MS,
  };
}

function _getPendingValue(floor, target) {
  const key = _pendingKey(floor, target);
  const pending = pendingCommands[key];
  if (!pending) return null;
  if (Date.now() > pending.expiresAt) {
    delete pendingCommands[key];
    return null;
  }
  return pending.value;
}

function _clearPendingIfMatches(floor, target, value) {
  const key = _pendingKey(floor, target);
  const pending = pendingCommands[key];
  if (!pending) return;
  if (Boolean(value) === pending.value) {
    delete pendingCommands[key];
  }
}

function _dashboardStatsUrl() {
  return `${CONFIG.apiBase}/dashboard/stats`;
}

function _hasMeaningfulDashboardData(data) {
  if (!data || typeof data !== "object") return false;
  const floors = data.floors || {};
  return Object.values(floors).some((fd) => {
    if (!fd || typeof fd !== "object") return false;
    return ["tempAir", "humidity", "tds", "ph"].some(
      (k) => typeof fd[k] === "number" && fd[k] !== 0,
    );
  });
}

function initCharts() {
  if (typeof Chart === "undefined") return;
  const tempCtx = document.getElementById("tempChart")?.getContext("2d");
  const humCtx = document.getElementById("humChart")?.getContext("2d");
  if (!tempCtx || !humCtx) return;

  const commonOptions = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: { legend: { display: false } },
    scales: { x: { display: false }, y: { display: true } },
  };

  CONFIG.charts.temp = new Chart(tempCtx, {
    type: "line",
    data: {
      labels: [],
      datasets: [
        { label: "Tầng 1", data: [], borderColor: "#00d4aa", tension: 0.3 },
        { label: "Tầng 2", data: [], borderColor: "#3b82f6", tension: 0.3 },
        { label: "Tầng 3", data: [], borderColor: "#f59e0b", tension: 0.3 },
      ],
    },
    options: {
      ...commonOptions,
      scales: { y: { title: { display: true, text: "°C" } } },
    },
  });

  CONFIG.charts.hum = new Chart(humCtx, {
    type: "line",
    data: {
      labels: [],
      datasets: [
        { label: "Tầng 1", data: [], borderColor: "#00d4aa", tension: 0.3 },
        { label: "Tầng 2", data: [], borderColor: "#3b82f6", tension: 0.3 },
        { label: "Tầng 3", data: [], borderColor: "#f59e0b", tension: 0.3 },
      ],
    },
    options: {
      ...commonOptions,
      scales: { y: { title: { display: true, text: "%" } } },
    },
  });
}

async function loadInitialData() {
  try {
    const response = await fetch(_dashboardStatsUrl());
    const data = await response.json();
    if (_hasMeaningfulDashboardData(data)) {
      updateUI(data);
    } else {
      clearUINoData();
    }
  } catch (error) {
    console.error("Error loading initial data:", error);
    clearUINoData();
  }
}

function updateUI(data) {
  document.getElementById("lastUpdate").textContent =
    new Date().toLocaleTimeString("vi-VN");
  const floors = data.floors || {};
  let totalTemp = 0,
    totalHum = 0,
    totalTds = 0,
    count = 0;

  for (const [floorNum, floorData] of Object.entries(floors)) {
    const floor = parseInt(floorNum, 10);
    CONFIG.floors[floor] = { ...CONFIG.floors[floor], ...floorData };

    _clearPendingIfMatches(floor, "led", floorData.led);
    _clearPendingIfMatches(floor, "pump", floorData.pump);
      _clearPendingIfMatches(floor, "fan", floorData.fan);

    document.getElementById(`floor${floor}-temp`).textContent =
      `${floorData.tempAir?.toFixed(1) ?? "--"} °C`;
    document.getElementById(`floor${floor}-hum`).textContent =
      `${floorData.humidity?.toFixed(1) || "--"} %`;
    document.getElementById(`floor${floor}-tds`).textContent =
      `${floorData.tds?.toFixed(0) || "--"} ppm`;
    document.getElementById(`floor${floor}-ph`).textContent =
      floorData.ph?.toFixed(1) || "--";

    updateControlButton(floor, "led", floorData.led);
    updateControlButton(floor, "pump", floorData.pump);
      updateControlButton(floor, "fan", floorData.fan);

    if (typeof floorData.tempAir === "number") {
      totalTemp += floorData.tempAir;
      count += 1;
    }
    if (typeof floorData.humidity === "number") totalHum += floorData.humidity;
    if (typeof floorData.tds === "number") totalTds += floorData.tds;
  }

  document.getElementById("avgTemp").textContent =
    count > 0 ? (totalTemp / count).toFixed(1) : "--";
  document.getElementById("avgHum").textContent =
    count > 0 ? (totalHum / count).toFixed(1) : "--";
  document.getElementById("avgTds").textContent =
    count > 0 ? Math.round(totalTds / count) : "---";
  document.getElementById("deviceCount").textContent = `${count} thiet bi`;
  document.getElementById("deviceStatus").textContent =
    count > 0 ? "Online" : "Offline";

  updateConnectionStatus(true);
}

function clearUINoData() {
  document.getElementById("lastUpdate").textContent = "--:--:--";

  for (let floor = 1; floor <= 3; floor += 1) {
    CONFIG.floors[floor] = {
      ...CONFIG.floors[floor],
      tempAir: null,
      humidity: null,
      tds: null,
      ph: null,
      led: false,
      pump: false,
      fan: false,
    };

    document.getElementById(`floor${floor}-temp`).textContent = "-- °C";
    document.getElementById(`floor${floor}-hum`).textContent = "-- %";
    document.getElementById(`floor${floor}-tds`).textContent = "-- ppm";
    document.getElementById(`floor${floor}-ph`).textContent = "--";

    updateControlButton(floor, "led", false);
    updateControlButton(floor, "pump", false);
      updateControlButton(floor, "fan", false);
  }

  document.getElementById("avgTemp").textContent = "--";
  document.getElementById("avgHum").textContent = "--";
  document.getElementById("avgTds").textContent = "---";
  document.getElementById("deviceCount").textContent = "0 thiet bi";
  document.getElementById("deviceStatus").textContent = "Offline";

  document.getElementById("ledStatus").textContent = "Tat";
  document.getElementById("pumpStatus").textContent = "Tat";
  document.getElementById("fanStatus").textContent = "Tat";
  const ledToggle = document.getElementById("toggleLed");
  const pumpToggle = document.getElementById("togglePump");
  const fanToggle = document.getElementById("toggleFan");
  if (ledToggle) ledToggle.checked = false;
  if (pumpToggle) pumpToggle.checked = false;
  if (fanToggle) fanToggle.checked = false;

  updateConnectionStatus(false);
}

function updateControlButton(floor, type, value) {
  const button = document.querySelector(
     `.floor-card[data-floor="${floor}"] button[onclick*="${type === "led" ? "toggleLed" : type === "pump" ? "togglePump" : "toggleFan"}(${floor})"]`,
  );
  if (!button) return;
  const pending = _getPendingValue(floor, type);
  const showValue = pending !== null ? pending : value;
  button.classList.toggle("active", Boolean(showValue));
  button.setAttribute("aria-pressed", String(Boolean(showValue)));
}

async function postControl(floor, target, value) {
  try {
    const resp = await fetch(`${CONFIG.apiBase}/dashboard/control`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        deviceId: `htvc-floor${floor}-001`,
        floor,
        target,
        value,
      }),
    });
    if (resp && resp.ok) {
      try {
        const data = await resp.json();
        if (data && data.commandId)
          _setPendingCommand(floor, target, data.commandId, value);
      } catch (e) {}
    }
  } catch (error) {
    console.error(`Error toggling ${target}:`, error);
  }
}

async function toggleLed(floor) {
  const current = CONFIG.floors[floor]?.led || false;
  const newValue = !current;
  CONFIG.floors[floor].led = newValue;
  _setPendingCommand(floor, "led", `ui-${Date.now()}`, newValue);
  updateControlButton(floor, "led", newValue);
  if (typeof window.sendMQTTControl === "function")
    window.sendMQTTControl(floor, "led", newValue);
  await postControl(floor, "led", newValue);
}

async function togglePump(floor) {
  const current = CONFIG.floors[floor]?.pump || false;
  const newValue = !current;
  CONFIG.floors[floor].pump = newValue;
  _setPendingCommand(floor, "pump", `ui-${Date.now()}`, newValue);
  updateControlButton(floor, "pump", newValue);
  if (typeof window.sendMQTTControl === "function")
    window.sendMQTTControl(floor, "pump", newValue);
  await postControl(floor, "pump", newValue);
}

async function toggleFan(floor) {
  const current = CONFIG.floors[floor]?.fan || false;
  const newValue = !current;
  CONFIG.floors[floor].fan = newValue;
  _setPendingCommand(floor, "fan", `ui-${Date.now()}`, newValue);
  updateControlButton(floor, "fan", newValue);
  if (typeof window.sendMQTTControl === "function")
    window.sendMQTTControl(floor, "fan", newValue);
  await postControl(floor, "fan", newValue);
}

async function toggleMasterLed() {
  const value = document.getElementById("toggleLed").checked;
  document.getElementById("ledStatus").textContent = value ? "Bat" : "Tat";
  for (let floor = 1; floor <= 3; floor += 1)
    if ((CONFIG.floors[floor]?.led || false) !== value) await toggleLed(floor);
}

async function toggleMasterPump() {
  const value = document.getElementById("togglePump").checked;
  document.getElementById("pumpStatus").textContent = value ? "Bat" : "Tat";
  for (let floor = 1; floor <= 3; floor += 1)
    if ((CONFIG.floors[floor]?.pump || false) !== value)
      await togglePump(floor);
}

async function toggleMasterFan() {
  const value = document.getElementById("toggleFan").checked;
  document.getElementById("fanStatus").textContent = value ? "Bat" : "Tat";
  for (let floor = 1; floor <= 3; floor += 1)
    if ((CONFIG.floors[floor]?.fan || false) !== value) await toggleFan(floor);
}

function updateCharts() {
  if (!CONFIG.charts.temp || !CONFIG.charts.hum) return;
  const now = new Date().toLocaleTimeString("vi-VN", {
    hour: "2-digit",
    minute: "2-digit",
  });
  CONFIG.charts.temp.data.labels.push(now);
  CONFIG.charts.hum.data.labels.push(now);
  for (let floor = 1; floor <= 3; floor += 1) {
    const floorData = CONFIG.floors[floor] || {};
    CONFIG.charts.temp.data.datasets[floor - 1].data.push(
      floorData.tempAir ?? null,
    );
    CONFIG.charts.hum.data.datasets[floor - 1].data.push(
      floorData.humidity ?? null,
    );
  }
  if (CONFIG.charts.temp.data.labels.length > 20) {
    CONFIG.charts.temp.data.labels.shift();
    CONFIG.charts.hum.data.labels.shift();
    CONFIG.charts.temp.data.datasets.forEach((dataset) => dataset.data.shift());
    CONFIG.charts.hum.data.datasets.forEach((dataset) => dataset.data.shift());
  }
  CONFIG.charts.temp.update();
  CONFIG.charts.hum.update();
}

function _updateMasterToggleStatus() {
  let allLedOn = true,
    allLedOff = true;
  let allPumpOn = true,
    allPumpOff = true;
  let allFanOn = true,
    allFanOff = true;
  for (let floor = 1; floor <= 3; floor += 1) {
    const led = CONFIG.floors[floor]?.led || false;
    const pump = CONFIG.floors[floor]?.pump || false;
    const fan = CONFIG.floors[floor]?.fan || false;
    if (!led) allLedOn = false;
    else allLedOff = false;
    if (!pump) allPumpOn = false;
    else allPumpOff = false;
    if (!fan) allFanOn = false;
    else allFanOff = false;
  }
  const ledToggle = document.getElementById("toggleLed");
  const pumpToggle = document.getElementById("togglePump");
  const fanToggle = document.getElementById("toggleFan");
  if (ledToggle) ledToggle.checked = !allLedOff;
  if (pumpToggle) pumpToggle.checked = !allPumpOff;
  if (fanToggle) fanToggle.checked = !allFanOff;
  const ledStatusEl = document.getElementById("ledStatus");
  const pumpStatusEl = document.getElementById("pumpStatus");
  const fanStatusEl = document.getElementById("fanStatus");
  if (ledStatusEl)
    ledStatusEl.textContent = allLedOn ? "Bat" : allLedOff ? "Tat" : "Mix";
  if (pumpStatusEl)
    pumpStatusEl.textContent = allPumpOn ? "Bat" : allPumpOff ? "Tat" : "Mix";
  if (fanStatusEl)
    fanStatusEl.textContent = allFanOn ? "Bat" : allFanOff ? "Tat" : "Mix";
}

function _updateSummaryStats() {
  let totalTemp = 0,
    totalHum = 0,
    totalTds = 0,
    count = 0;
  for (let floor = 1; floor <= 3; floor += 1) {
    const fd = CONFIG.floors[floor] || {};
    if (fd.tempAir != null) {
      totalTemp += fd.tempAir;
      count += 1;
    }
    if (fd.humidity != null) totalHum += fd.humidity;
    if (fd.tds != null) totalTds += fd.tds;
  }
  const avgTempEl = document.getElementById("avgTemp");
  const avgHumEl = document.getElementById("avgHum");
  const avgTdsEl = document.getElementById("avgTds");
  const deviceCountEl = document.getElementById("deviceCount");
  const deviceStatusEl = document.getElementById("deviceStatus");
  if (avgTempEl)
    avgTempEl.textContent = count > 0 ? (totalTemp / count).toFixed(1) : "--";
  if (avgHumEl)
    avgHumEl.textContent = count > 0 ? (totalHum / count).toFixed(1) : "--";
  if (avgTdsEl)
    avgTdsEl.textContent = count > 0 ? Math.round(totalTds / count) : "---";
  if (deviceCountEl) deviceCountEl.textContent = `${count} thiet bi`;
  if (deviceStatusEl)
    deviceStatusEl.textContent = count > 0 ? "Online" : "Offline";
}

function updateConnectionStatus(connected) {
  const statusEl = document.getElementById("connectionStatus");
  const statusText = document.getElementById("statusText");
  const statusDetail = document.getElementById("statusDetail");
  if (!statusEl || !statusText) return;
  if (connected) {
    statusEl.classList.remove("disconnected");
    statusText.textContent = "Realtime HiveMQ";
  } else {
    statusEl.classList.add("disconnected");
    statusText.textContent = "Dang cho realtime";
  }
  if (statusDetail)
    statusDetail.textContent = connected
      ? "Subscribe qua SSE tu backend HiveMQ"
      : `Cong mac dinh: ${CONFIG.mqttPort}`;
}

function startPolling() {
  setInterval(async () => {
    try {
      const response = await fetch(_dashboardStatsUrl());
      if (response.ok) {
        const data = await response.json();
        if (_hasMeaningfulDashboardData(data)) updateUI(data);
        else clearUINoData();
      } else clearUINoData();
    } catch (_err) {
      clearUINoData();
    }
    updateCharts();
  }, CONFIG.updateInterval);
}

function startRealtimeStream() {
  if (!window.EventSource) {
    startPolling();
    return;
  }

  const streamUrl = `${CONFIG.apiBase}/dashboard/stream`;
  CONFIG.sse = new EventSource(streamUrl);

  CONFIG.sse.onopen = () => {
    updateConnectionStatus(true);
  };

  CONFIG.sse.onmessage = async (event) => {
    try {
      const message = JSON.parse(event.data);
      if (message && message.topic) {
        // Parse MQTT payload directly from SSE event for real-time display
        if (message.topic === "control" && message.payload) {
          const payload = message.payload;
          const floor = Number(payload.floor);
          const target = payload.target;
          if (
            payload.success !== false &&
            floor >= 1 &&
            floor <= 3 &&
            ["led", "pump", "fan"].includes(target)
          ) {
            const value = Boolean(payload.value);
            CONFIG.floors[floor] = {
              ...CONFIG.floors[floor],
              [target]: value,
            };
            _clearPendingIfMatches(floor, target, value);
            updateControlButton(floor, target, value);
            _updateMasterToggleStatus();
          }
          return;
        }

        if (message.topic === "sensors" && message.payload) {
          const payload = message.payload;
          const deviceId = message.deviceId || payload.deviceId || "";

          // Extract floor number from deviceId (e.g., "htvc-floor1-001" -> 1)
          let floor = 1;
          const floorMatch = deviceId.match(/floor(\d+)/i);
          if (floorMatch) {
            floor = parseInt(floorMatch[1], 10);
          }

          // Update CONFIG with sensor data
          if (floor >= 1 && floor <= 3) {
            const sensorData = payload.data || payload;
            CONFIG.floors[floor] = {
              ...CONFIG.floors[floor],
              tempAir: sensorData.tempAir ?? sensorData.tempA ?? null,
              humidity: sensorData.humidity ?? sensorData.hum ?? null,
              tds: sensorData.tds ?? null,
              ph: sensorData.ph ?? null,
              pump: Boolean(sensorData.pump),
              fan: Boolean(sensorData.fan),
              led: Boolean(sensorData.led),
              ledBrightness: sensorData.ledBrightness ?? 100,
              tempWater: sensorData.tempWater ?? sensorData.tempW ?? null,
            };

            // Update floor card UI
            const tempEl = document.getElementById(`floor${floor}-temp`);
            const humEl = document.getElementById(`floor${floor}-hum`);
            const tdsEl = document.getElementById(`floor${floor}-tds`);
            const phEl = document.getElementById(`floor${floor}-ph`);

            if (tempEl) {
              const temp = CONFIG.floors[floor].tempAir;
              tempEl.textContent =
                temp !== null ? `${temp.toFixed(1)} °C` : "-- °C";
            }
            if (humEl) {
              const hum = CONFIG.floors[floor].humidity;
              humEl.textContent = hum !== null ? `${hum.toFixed(1)} %` : "-- %";
            }
            if (tdsEl) {
              const tds = CONFIG.floors[floor].tds;
              tdsEl.textContent =
                tds !== null ? `${Math.round(tds)} ppm` : "-- ppm";
            }
            if (phEl) {
              const ph = CONFIG.floors[floor].ph;
              phEl.textContent = ph !== null ? ph.toFixed(1) : "--";
            }

            // Update control buttons
            updateControlButton(floor, "led", CONFIG.floors[floor].led);
            updateControlButton(floor, "pump", CONFIG.floors[floor].pump);
            updateControlButton(floor, "fan", CONFIG.floors[floor].fan);

            // Update master toggle status
            _clearPendingIfMatches(floor, "led", CONFIG.floors[floor].led);
            _clearPendingIfMatches(floor, "pump", CONFIG.floors[floor].pump);
            _clearPendingIfMatches(floor, "fan", CONFIG.floors[floor].fan);
            _updateMasterToggleStatus();
          }

          // Update summary stats and timestamp
          _updateSummaryStats();
          document.getElementById("lastUpdate").textContent =
            new Date().toLocaleTimeString("vi-VN");

          // Update charts
          updateCharts();
        }
      }
    } catch (error) {
      console.error("Error handling SSE message:", error);
    }
  };

  CONFIG.sse.onerror = () => {
    updateConnectionStatus(false);
  };
}

async function refreshData() {
  await loadInitialData();
  updateCharts();
}

document.addEventListener("DOMContentLoaded", () => {
  initCharts();
  loadInitialData();
  startRealtimeStream();
});

window.refreshData = refreshData;
window.toggleLed = toggleLed;
window.togglePump = togglePump;
window.toggleFan = toggleFan;
window.toggleMasterLed = toggleMasterLed;
window.toggleMasterPump = toggleMasterPump;
window.toggleMasterFan = toggleMasterFan;
