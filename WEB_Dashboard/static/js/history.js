let historyChart = null;

function formatCompactTime(value) {
  const date = value instanceof Date ? value : new Date(value);
  if (Number.isNaN(date.getTime())) {
    return "--";
  }

  const hours = String(date.getHours()).padStart(2, "0");
  const minutes = String(date.getMinutes()).padStart(2, "0");
  return `${hours}:${minutes}`;
}


function buildDemoHistoryRows(floor, hours) {
  const now = new Date();
  const points = Math.max(12, Math.min(168, hours * 2));
  const baseTemp = 23 + floor * 0.6;
  const baseHum = 58 + floor * 2;
  const baseTds = 430 + floor * 35;

  return Array.from({ length: points }).map((_, idx) => {
    const stamp = new Date(now.getTime() - (points - 1 - idx) * 30 * 60 * 1000);
    return {
      createdAt: stamp.toISOString(),
      floor,
      tempAir: Number((baseTemp + Math.sin((idx + floor) / 4) * 1.2).toFixed(1)),
      humidity: Number((baseHum + Math.cos((idx + floor) / 5) * 2.8).toFixed(1)),
      tds: Math.round(baseTds + Math.sin(idx / 6) * 20),
      ph: Number((6.15 + floor * 0.05 + Math.cos(idx / 7) * 0.06).toFixed(2)),
      connected: idx % 5 !== 0,
    };
  });
}

function hasMeaningfulHistoryRows(rows) {
  return Array.isArray(rows) && rows.some((row) => {
    return ["tempAir", "humidity", "tds", "ph"].some((key) => typeof row?.[key] === "number" && row[key] !== 0);
  });
}

function renderHistoryTable(rows) {
  const body = document.getElementById("historyTableBody");
  if (!body) return;

  if (!rows.length) {
async function loadHistoryData() {
  const floor = document.getElementById("filterFloor")?.value || "";
  const hours = parseInt(document.getElementById("filterTime")?.value || "24", 10);
  const limit = hours <= 6 ? 100 : hours <= 24 ? 300 : 1000;

  const query = new URLSearchParams();
  query.set("limit", String(limit));
  if (floor) query.set("floor", floor);

  const response = await fetch(`/api/readings/history?${query.toString()}`);
  const rows = await response.json();
  return Array.isArray(rows) ? rows : [];
}
    .map((row) => {
      const timeValue = row.createdAt || (row.timestamp?.seconds ? row.timestamp.seconds * 1000 : null);
      const time = formatCompactTime(timeValue);
      return `
        <tr>
          <td>${time}</td>
          <td>${row.floor || "--"}</td>
          <td>${row.tempAir ?? "--"}</td>
          <td>${row.humidity ?? "--"}</td>
          <td>${row.tds ?? "--"}</td>
          <td>${row.ph ?? "--"}</td>
          <td>
            <span class="status-indicator ${row.connected ? "status-online" : "status-offline"}">
              ${row.connected ? "Online" : "Offline"}
            </span>
          </td>
        </tr>

          label: "Nhiet do",
          data: tempData,
          borderColor: "#00d4aa",
          tension: 0.3,
        },
        {
          label: "Do am",
          data: humData,
          borderColor: "#3b82f6",
          tension: 0.3,
        },
      ],
    },
    options: { responsive: true, maintainAspectRatio: false },
  });
}

async function loadHistoryData() {
  const floor = document.getElementById("filterFloor")?.value || "";
  const hours = parseInt(document.getElementById("filterTime")?.value || "24", 10);
  const limit = hours <= 6 ? 100 : hours <= 24 ? 300 : 1000;

  const query = new URLSearchParams();
  query.set("limit", String(limit));
  if (floor) query.set("floor", floor);

  // fetch real history data from backend

  const response = await fetch(`/api/readings/history?${query.toString()}`);
  const rows = await response.json();

  if (hasMeaningfulHistoryRows(rows)) {
    return Array.isArray(rows) ? rows : [];
  }

  const floors = floor ? [parseInt(floor, 10)] : [1, 2, 3];
  return floors.flatMap((itemFloor) => buildDemoHistoryRows(itemFloor, hours));
}

async function loadHistory() {
  try {
    const rows = await loadHistoryData();
    renderHistoryTable(Array.isArray(rows) ? rows : []);
    renderHistoryChart(Array.isArray(rows) ? rows.slice().reverse() : []);
  } catch (error) {
    console.error("Load history error:", error);
    const floor = document.getElementById("filterFloor")?.value || "";
    const hours = parseInt(document.getElementById("filterTime")?.value || "24", 10);
    const floors = floor ? [parseInt(floor, 10)] : [1, 2, 3];
    const rows = floors.flatMap((itemFloor) => buildDemoHistoryRows(itemFloor, hours));
    renderHistoryTable(rows);
    renderHistoryChart(rows.slice().reverse());
  }
}

document.addEventListener("DOMContentLoaded", () => {
  loadHistory();
});

window.loadHistory = loadHistory;
