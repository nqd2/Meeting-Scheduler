import { app, BrowserWindow } from 'electron';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Xác định môi trường (Dev hay Prod)
const isDev = !app.isPackaged;

function createWindow() {
  const mainWindow = new BrowserWindow({
    width: 1200,
    height: 800,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false, // Để đơn giản hóa việc tích hợp ban đầu
      // preload: path.join(__dirname, 'preload.js'), // Bạn có thể thêm preload nếu cần IPC
    },
  });

  // Ẩn menu mặc định của Electron (File, Edit...) nếu muốn giao diện sạch hơn
  mainWindow.setMenuBarVisibility(false);

  if (isDev) {
    // Nếu là Dev, load từ Vite Server
    mainWindow.loadURL('http://localhost:5173');
    // Mở DevTools để debug
    mainWindow.webContents.openDevTools();
  } else {
    // Nếu là Prod, load file index.html đã build
    mainWindow.loadFile(path.join(__dirname, '../dist/index.html'));
  }
}

app.whenReady().then(() => {
  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});