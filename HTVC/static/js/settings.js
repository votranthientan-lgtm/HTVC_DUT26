function saveDeviceSettings() {
  const deviceId = document.getElementById("deviceId")?.value;
  if (deviceId) {
    localStorage.setItem("htvc-deviceId", deviceId);
    window.showToast?.("Da luu cai dat thiet bi");
  }
}

function saveServerSettings() {
  const serverUrl = document.getElementById("serverUrl")?.value;
  const wsUrl = document.getElementById("wsUrl")?.value;
  const refreshInterval = document.getElementById("refreshInterval")?.value;

  localStorage.setItem("htvc-serverUrl", serverUrl || "");
  localStorage.setItem("htvc-wsUrl", wsUrl || "");
  localStorage.setItem("htvc-refreshInterval", refreshInterval || "5000");

  window.showToast?.("Da luu cai dat server");
}

function checkUpdates() {
  window.showToast?.("Ban dang su dung phien ban moi nhat");
}

document.addEventListener("DOMContentLoaded", () => {
  const savedDeviceId = localStorage.getItem("htvc-deviceId");
  const savedServerUrl = localStorage.getItem("htvc-serverUrl");
  const savedWsUrl = localStorage.getItem("htvc-wsUrl");
  const savedRefresh = localStorage.getItem("htvc-refreshInterval");

  if (savedDeviceId) document.getElementById("deviceId").value = savedDeviceId;
  if (savedServerUrl) document.getElementById("serverUrl").value = savedServerUrl;
  if (savedWsUrl) document.getElementById("wsUrl").value = savedWsUrl;
  if (savedRefresh) document.getElementById("refreshInterval").value = savedRefresh;
});

window.saveDeviceSettings = saveDeviceSettings;
window.saveServerSettings = saveServerSettings;
window.checkUpdates = checkUpdates;
