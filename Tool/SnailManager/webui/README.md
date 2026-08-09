# WebUI 现代界面

现代界面由 `webui/` React + TypeScript + Vite 工程构建，Python 侧通过 FastAPI 和 pywebview 提供本地桌面容器。

- `SnailHeater_Modern.py`：标准款现代界面入口。
- `SnailHeater_Modern_EL.py`：EL 款现代界面入口。
- `SnailHeater_Launcher.py`：现代 WebUI 的产品选择入口。
- `snailheater_tool/webapp/session.py`：复用现有设备、固件、激活和媒体服务的应用层。
- `snailheater_tool/webapp/api.py`：本机令牌保护的 REST/SSE 接口。

现代界面不导入或启动原有 Qt 控制器；原有 PyQt5/PySide2 入口也保持独立运行。
