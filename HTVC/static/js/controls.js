async function sendControl(floor, target, value) {
  try {
    const response = await fetch("/api/dashboard/control", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        deviceId: `htvc-floor${floor}-001`,
        floor,
        target,
        value,
      }),
    });

    const result = await response.json();
    if (result.status === "ok") {
      window.showToast?.(`Da gui lenh ${target} cho tang ${floor}`);
    }
  } catch (error) {
    console.error("Control error:", error);
    window.showToast?.("Loi khi gui lenh", "error");
  }
}

function addSchedule() {
  const floor = document.getElementById("schedule-floor").value;
  const start = document.getElementById("schedule-start").value;
  const end = document.getElementById("schedule-end").value;

  const list = document.getElementById("scheduleList");
  if (list.querySelector(".empty-state")) {
    list.innerHTML = "";
  }

  const item = document.createElement("div");
  item.className = "schedule-item";
  item.innerHTML = `
    <div class="schedule-info">
      <span class="schedule-floor">Tang ${floor}</span>
      <span class="schedule-time">${start} - ${end}</span>
    </div>
    <button class="btn btn-sm btn-danger" onclick="this.parentElement.remove()">Xoa</button>
  `;
  list.appendChild(item);

  window.showToast?.("Da them lich trinh moi");
}

document.addEventListener("DOMContentLoaded", () => {
  document.querySelectorAll('input[type="range"]').forEach((slider) => {
    slider.addEventListener("input", (event) => {
      const valueId = event.target.id.replace("-", "-value-");
      const valueDisplay = document.getElementById(valueId);
      if (valueDisplay) {
        valueDisplay.textContent = event.target.value + "%";
      }
    });
  });
});

window.sendControl = sendControl;
window.addSchedule = addSchedule;
