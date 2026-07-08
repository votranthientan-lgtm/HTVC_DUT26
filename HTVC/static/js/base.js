/* Shared UI logic for all HTVC pages */

function initTheme() {
  const savedTheme = localStorage.getItem("htvc-theme");
  if (savedTheme) {
    document.body.dataset.theme = savedTheme;
    updateThemeIcon(savedTheme);
    return;
  }

  if (window.matchMedia("(prefers-color-scheme: light)").matches) {
    document.body.dataset.theme = "light";
  }

  updateThemeIcon(document.body.dataset.theme || "dark");
}

function updateThemeIcon(theme) {
  const icon = document.getElementById("themeIcon");
  if (!icon) {
    return;
  }

  const isLight = theme === "light";
  icon.src = isLight ? "/static/images/lightmode.svg" : "/static/images/darkmode.svg";
  icon.alt = isLight ? "Light mode" : "Dark mode";
}

function toggleTheme() {
  const currentTheme = document.body.dataset.theme || "dark";
  const newTheme = currentTheme === "dark" ? "light" : "dark";
  document.body.dataset.theme = newTheme;
  localStorage.setItem("htvc-theme", newTheme);
  updateThemeIcon(newTheme);
}

function openFirebaseSettings() {
  const modal = document.getElementById("firebaseModal");
  if (!modal) {
    return;
  }

  modal.classList.add("open");
  const savedDeviceId = localStorage.getItem("htvc-deviceId");
  const input = document.getElementById("firebaseDeviceId");
  if (savedDeviceId && input) {
    input.value = savedDeviceId;
  }
}

function closeFirebaseSettings() {
  document.getElementById("firebaseModal")?.classList.remove("open");
}

function saveFirebaseSettings() {
  const input = document.getElementById("firebaseDeviceId");
  const deviceId = input?.value?.trim();
  if (deviceId) {
    localStorage.setItem("htvc-deviceId", deviceId);
    showToast("Da luu cai dat Firebase");
  }
  closeFirebaseSettings();
}

function closeAllModals() {
  document.querySelectorAll(".modal.open").forEach((modal) => {
    modal.classList.remove("open");
  });
}

function createToastContainer() {
  const container = document.createElement("div");
  container.id = "toastContainer";
  container.innerHTML = `
    <style>
      #toastContainer {
        position: fixed;
        bottom: 20px;
        right: 20px;
        display: flex;
        flex-direction: column;
        gap: 8px;
        z-index: 10000;
      }
      .toast {
        display: flex;
        align-items: center;
        gap: 10px;
        padding: 12px 16px;
        border-radius: 8px;
        font-size: 0.875rem;
        font-weight: 500;
        opacity: 0;
        transform: translateX(100%);
        transition: all 0.3s ease;
        min-width: 200px;
        max-width: 350px;
      }
      .toast.show {
        opacity: 1;
        transform: translateX(0);
      }
      .toast-success {
        background: rgba(34, 197, 94, 0.15);
        color: var(--success, #22c55e);
        border: 1px solid rgba(34, 197, 94, 0.3);
      }
      .toast-error {
        background: rgba(239, 68, 68, 0.15);
        color: var(--danger, #ef4444);
        border: 1px solid rgba(239, 68, 68, 0.3);
      }
      .toast-info {
        background: rgba(59, 130, 246, 0.15);
        color: var(--info, #3b82f6);
        border: 1px solid rgba(59, 130, 246, 0.3);
      }
    </style>
  `;
  document.body.appendChild(container);
  return container;
}

function showToast(message, type = "success") {
  const container = document.getElementById("toastContainer") || createToastContainer();
  const toast = document.createElement("div");
  toast.className = `toast toast-${type}`;
  toast.innerHTML = `<span>${message}</span>`;
  container.appendChild(toast);

  requestAnimationFrame(() => toast.classList.add("show"));
  setTimeout(() => {
    toast.classList.remove("show");
    setTimeout(() => toast.remove(), 300);
  }, 3000);
}

document.addEventListener("DOMContentLoaded", () => {
  initTheme();
  document.getElementById("themeToggle")?.addEventListener("click", toggleTheme);
  document.getElementById("firebaseBtn")?.addEventListener("click", openFirebaseSettings);
  document.addEventListener("keydown", (event) => {
    if (event.key === "Escape") {
      closeAllModals();
    }
  });
});

window.toggleTheme = toggleTheme;
window.showToast = showToast;
window.openFirebaseSettings = openFirebaseSettings;
window.closeFirebaseSettings = closeFirebaseSettings;
window.saveFirebaseSettings = saveFirebaseSettings;
