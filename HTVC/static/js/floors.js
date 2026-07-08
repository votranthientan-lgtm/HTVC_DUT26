let currentFloor = 1;
const floorCharts = {};
const floorSchedules = { 1: [], 2: [], 3: [] };
const floorControlState = {
  1: { led: false, pump: false },
  2: { led: false, pump: false },
  3: { led: false, pump: false },
};

function toNumber(value) {
  const n = Number(value);
  return Number.isFinite(n) ? n : null;
}

function initFloorCharts() {
  for (let floor = 1; floor <= 3; floor += 1) {
    const canvas = document.getElementById(`f${floor}-chart`);
    if (!canvas || typeof Chart === "undefined") continue;

    floorCharts[floor] = new Chart(canvas.getContext("2d"), {
      type: "line",
      data: {
        labels: [],
        datasets: [
          {
            label: "Nhiệt độ",
            data: [],
            borderColor: "#00d4aa",
            tension: 0.3,
            pointRadius: 1.5,
          },
          {
            label: "Độ ẩm",
            data: [],
            borderColor: "#3b82f6",
            tension: 0.3,
            pointRadius: 1.5,
          },
        ],
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: { legend: { labels: { color: "#9aa5b5", boxWidth: 12 } } },
        scales: {
          x: {
            ticks: { color: "#7b8798", maxTicksLimit: 6 },
            grid: { color: "rgba(255,255,255,0.06)" },
          },
          y: {
            ticks: { color: "#7b8798" },
            grid: { color: "rgba(255,255,255,0.06)" },
          },
        },
      },
    });
  }
}

function updateFloorChart(floor, historyRows) {
  const chart = floorCharts[floor];
  if (!chart) return;

  let rows = Array.isArray(historyRows) ? historyRows.slice(-24) : [];
  if (!rows.length) {
    rows = Array.from({ length: 12 }).map((_, idx) => ({
      createdAt: new Date(
        Date.now() - (11 - idx) * 60 * 60 * 1000,
      ).toISOString(),
      tempAir: null,
      humidity: null,
    }));
  }

  const labels = rows.map((row) =>
    new Date(row.createdAt || row.timestamp).toLocaleTimeString("vi-VN", {
      hour: "2-digit",
      minute: "2-digit",
    }),
  );
  const temps = rows.map((row) =>
    typeof row.tempAir === "number" ? row.tempAir : null,
  );
  const hums = rows.map((row) =>
    typeof row.humidity === "number" ? row.humidity : null,
  );

  chart.data.labels = labels;
  chart.data.datasets[0].data = temps;
  chart.data.datasets[1].data = hums;
  chart.update();
}

function updateFloorUI(floor, data) {
  const prefix = `f${floor}`;
  const temp = document.getElementById(`${prefix}-temp`);
  const hum = document.getElementById(`${prefix}-hum`);
  const tempW = document.getElementById(`${prefix}-tempW`);
  const ph = document.getElementById(`${prefix}-ph`);
  const tds = document.getElementById(`${prefix}-tds`);

  if (temp) temp.textContent = `${data.tempAir ?? "--"} °C`;
  if (hum) hum.textContent = `${data.humidity ?? "--"} %`;
  if (tempW) tempW.textContent = `${data.tempWater ?? "--"} °C`;
  if (ph) ph.textContent = data.ph ?? "--";
  if (tds) tds.textContent = `${data.tds ?? "--"} ppm`;

  // Chỉ cập nhật state khi backend có trả về trường điều khiển,
  // tránh ghi đè sai khi dữ liệu chưa đầy đủ.
  if (Object.prototype.hasOwnProperty.call(data, "led")) {
    floorControlState[floor].led = Boolean(data.led);
  }
  if (Object.prototype.hasOwnProperty.call(data, "pump")) {
    floorControlState[floor].pump = Boolean(data.pump);
  }
  updateControlButtons(floor);
  updateEnvironmentStatus(floor, data);
}

function updateEnvironmentStatus(floor, data) {
  const overallEl = document.getElementById(`f${floor}-overall`);
  const recommendEl = document.getElementById(`f${floor}-recommend`);
  if (!overallEl || !recommendEl) return;

  // Simple heuristics
  const levels = [];
  if (
    typeof data.tempAir === "number" &&
    (data.tempAir < 20 || data.tempAir > 30)
  )
    levels.push("warn");
  if (
    typeof data.humidity === "number" &&
    (data.humidity < 50 || data.humidity > 80)
  )
    levels.push("warn");

  overallEl.classList.remove("env-good", "env-warn", "env-bad");
  if (levels.includes("bad")) {
    overallEl.classList.add("env-bad");
    overallEl.textContent = "Cần can thiệp";
    recommendEl.textContent =
      "Một hoặc nhiều chỉ số đang ngoài ngưỡng. Nên kiểm tra dung dịch, lưu lượng bơm và thông gió.";
    return;
  }
  if (levels.includes("warn")) {
    overallEl.classList.add("env-warn");
    overallEl.textContent = "Theo dõi thêm";
    recommendEl.textContent =
      "Chỉ số đang trong mức chấp nhận nhưng chưa tối ưu. Có thể tinh chỉnh dần để cây phát triển ổn định hơn.";
    return;
  }
  overallEl.classList.add("env-good");
  overallEl.textContent = "Môi trường tốt";
  recommendEl.textContent =
    "Các chỉ số chính đang ở vùng tối ưu theo bộ tham chiếu rau thủy canh.";
}

function loadFloorData(floor) {
  fetch(`/api/dashboard/floor/${floor}`)
    .then((res) => res.json())
    .then((data) => {
      if (data && (data.latest || data.history)) {
        updateFloorUI(floor, data.latest || {});
        updateFloorChart(floor, data.history || []);
        return;
      }

      updateFloorUI(floor, {});
      updateFloorChart(floor, []);
    })
    .catch((err) => {
      console.error("Load floor data error:", err);
      updateFloorUI(floor, {});
      updateFloorChart(floor, []);
    });
}

function updateControlButtons(floor) {
  const container = document.getElementById(`floor${floor}-content`);
  if (!container) return;
  const ledBtn = container.querySelector(
    `button[onclick*="toggleControl(${floor}, 'led')"]`,
  );
  const pumpBtn = container.querySelector(
    `button[onclick*="toggleControl(${floor}, 'pump')"]`,
  );
  const ledLabel = document.getElementById(`f${floor}-led-label`);
  const pumpLabel = document.getElementById(`f${floor}-pump-label`);

  const ledOn = Boolean(floorControlState[floor].led);
  const pumpOn = Boolean(floorControlState[floor].pump);

  if (ledBtn) {
    ledBtn.classList.toggle("active", ledOn);
    ledBtn.setAttribute("aria-pressed", String(ledOn));
  }
  if (pumpBtn) {
    pumpBtn.classList.toggle("active", pumpOn);
    pumpBtn.setAttribute("aria-pressed", String(pumpOn));
  }
  if (ledLabel) {
    ledLabel.textContent = `LED: ${ledOn ? "Bật" : "Tắt"}`;
  }
  if (pumpLabel) {
    pumpLabel.textContent = `Bơm: ${pumpOn ? "Bật" : "Tắt"}`;
  }
}

function sendControl(floor, target, value) {
  if (typeof window.sendMQTTControl === "function")
    window.sendMQTTControl(floor, target, value);
  fetch(`/api/dashboard/control`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      deviceId: `htvc-floor${floor}-001`,
      floor,
      target,
      value,
    }),
  }).catch(console.error);
}

function toggleControl(floor, target) {
  const state = floorControlState[floor]?.[target] || false;
  const newVal = !state;
  floorControlState[floor][target] = newVal;
  updateControlButtons(floor);
  sendControl(floor, target, newVal);
}

function addFloorSchedule(floor, cron, enabled) {
  /* placeholder - server interaction omitted */
}
function toggleFloorSchedule(id) {
  /* placeholder */
}
function removeFloorSchedule(id) {
  /* placeholder */
}

document.addEventListener("DOMContentLoaded", () => {
  initFloorCharts();
  loadFloorData(1);
  [1, 2, 3].forEach((f) => loadFloorData(f));
  setInterval(() => loadFloorData(currentFloor), 5000);
});

window.showFloor = function (floor) {
  currentFloor = floor;
  document.querySelectorAll(".floor-content").forEach((el, idx) => {
    const shouldShow = idx + 1 === floor;
    el.style.display = shouldShow ? "" : "none";
  });
  document.querySelectorAll(".floor-tab").forEach((btn) => {
    btn.classList.toggle("active", Number(btn.dataset.floor) === floor);
  });
  loadFloorData(floor);
};
window.sendControl = sendControl;
window.toggleControl = toggleControl;
window.addFloorSchedule = addFloorSchedule;
window.toggleFloorSchedule = toggleFloorSchedule;
window.removeFloorSchedule = removeFloorSchedule;
