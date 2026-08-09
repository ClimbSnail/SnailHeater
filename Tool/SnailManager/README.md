# SnailHeater Windows 管理工具

SnailHeater Windows 管理工具用于设备识别、固件刷写、激活、壁纸与背景素材处理、UI 前景颜色配置、RTTTL/MIDI 转换和运行日志查看。

本工程提供两套相互独立的桌面界面：

- **现代 WebUI**：React + TypeScript + Vite + FastAPI + pywebview；
- **经典 Qt UI**：PySide2 / PyQt5 入口。

两套界面共享设备、固件、媒体和产品配置服务，但不会互相导入或启动。

> 本项目按 **Python 3.8** 维护。以下 Python 命令均应在 Python 3.8 虚拟环境中执行。

## 主要功能

### 现代 WebUI

- 标准款 SnailHeater 与 EL 款产品支持；
- 串口扫描、机器码、SN、软件版本、芯片和 Flash 容量查询；
- 设备激活、硬复位、UI 前景颜色读取与写入；
- 更新式、清空式固件刷写；
- 图片、视频、BIN、LSW、RTTTL 素材转换、打包和写入；
- 独立 MP4 → RTTTL 转换；
- Coredump 读取、实时日志和 SSE 任务状态；
- 深色、浅色和跟随系统主题。

### 经典 Qt UI

- PySide2 标准款和 EL 款；
- PyQt5 标准款和 EL 款；
- 保留原 QT_Tool 的设备通信、刷机、壁纸、背景和颜色配置功能；
- 刷机“中止”操作按原 QT_Tool 行为立即终止刷机线程。

## 环境要求

- Windows 10 / Windows 11；
- **Python 3.8.x**；
- Node.js 与 npm，用于构建 WebUI；
- Microsoft Edge WebView2 Runtime，用于运行现代 WebUI；
- CH340 等设备串口驱动；
- `ffmpeg.exe`：项目根目录已提供，视频和 MP4→RTTTL 转换需要它。

PowerShell 示例使用 `npm.cmd`，可以避免部分系统禁止执行 `npm.ps1` 的问题。

## 目录结构

```text
SnailManager/
├─ snailheater_tool/
│  ├─ controllers/        # 经典 Qt 控制器
│  ├─ webapp/             # FastAPI、SSE 和 WebUI 桌面服务
│  ├─ device_service.py   # 串口和设备设置
│  ├─ firmware_service.py # 固件扫描、识别和刷写
│  ├─ media_service.py    # 媒体转换和壁纸打包
│  ├─ rtttl_service.py    # MP4 → RTTTL 共享服务
│  ├─ profiles.py         # 产品能力、地址和版本规划
│  └─ paths.py            # 运行路径管理
├─ webui/                 # React + TypeScript + Vite 前端
├─ requirements/          # Python 依赖清单
├─ tests/                 # Python 自动化测试
├─ tools/
│  ├─ cli/                # 命令行刷机工具
│  ├─ filesystem/         # SPIFFS 工具
│  └─ wallpaper/          # 壁纸工具
├─ docs/                  # 使用说明
├─ images/                # 图标资源
├─ stub_flasher/          # esptool Stub 资源
├─ base_data/             # EL 固件资源
├─ base_data_2117/        # 旧版标准款固件资源
├─ base_data_new/         # 新版标准款固件资源
├─ Generate/              # 运行时生成目录
├─ ffmpeg.exe             # 视频处理程序
├─ SnailHeater_Tool.yaml  # 标准款外部配置
└─ SnailHeater_Tool_EL.yaml # EL 外部配置
```

## 创建 Python 3.8 环境并安装依赖

推荐使用 PySide2 / WebUI 共用的 Python 3.8 依赖清单：

```powershell
cd D:\Workspace\OpenWorkspace\SnailHeater\Tool\SnailManager

python --version
# 应显示 Python 3.8.x

python -m venv .venv_webui_py38
.\.venv_webui_py38\Scripts\Activate
python -m pip install --upgrade pip
python -m pip install -r requirements\requirements_webui_venv.txt -i https://mirrors.aliyun.com/pypi/simple/

python -m venv .venv_pyside_py38
.\.venv_pyside_py38\Scripts\Activate
python -m pip install --upgrade pip
python -m pip install -r requirements\requirements_pyside_py38_venv.txt -i https://mirrors.aliyun.com/pypi/simple/
```

如果使用经典 PyQt5 入口，则在 Python 3.8 环境中安装：

```powershell
python -m venv .venv_pyqt5
.\.venv_pyqt5\Scripts\Activate
python -m pip install --upgrade pip
python -m pip install -r requirements\requirements_pyqt5_venv.txt -i https://mirrors.aliyun.com/pypi/simple/
```

不要使用 `pip freeze` 覆盖项目中的 requirements 文件，以免将本机无关依赖写入项目清单。

## 构建 WebUI 前端

现代 WebUI 启动和打包前必须生成 `webui/dist/`：

```powershell
cd webui; npm.cmd install; npm.cmd run build; cd ..
分布执行如下
cd webui
npm.cmd install
npm.cmd run build
cd ..
```

前端开发模式：

```powershell
cd webui
npm.cmd run dev
```

开发模式只启动 Vite 前端服务；完整桌面功能仍需启动 Python 本地 API。

## 启动程序

### 现代 WebUI

在已经激活 Python 3.8 虚拟环境、并完成 `webui/dist` 构建后执行：

```powershell
# 标准款
python SnailHeater_Modern.py

# EL 款
python SnailHeater_Modern_EL.py
```

### 经典 Qt UI

```powershell
# PySide2 标准款 / EL 款
python SnailHeater_WinTool_pyside2.py
python SnailHeater_WinTool_pyside2_EL.py

# PyQt5 标准款 / EL 款
python SnailHeater_WinTool_pyqt5.py
python SnailHeater_WinTool_pyqt5_EL.py
```

### 命令行刷机工具

```powershell
python SnailHeater_Tool.py
```

根目录的 `SnailHeater_Tool.py` 是兼容入口，实际实现位于 `tools/cli/SnailHeater_Tool.py`。

## RTTTL / MIDI 工具用法

MIDI 工具需要 `mido`，已包含在 `requirements_pyside_py38_venv.txt` 中。MP4→RTTTL 工具需要 `ffmpeg.exe`、`numpy` 和 `librosa`。

## 打包前准备

所有打包命令都应在 **SnailManager 根目录**执行，并使用已安装 PyInstaller 的 Python 3.8 环境：

```powershell
cd D:\Workspace\OpenWorkspace\SnailHeater\Tool\SnailManager
python -m pip install -r requirements\requirements_pyside_py38_venv.txt -i https://mirrors.aliyun.com/pypi/simple/
cd webui; npm.cmd install; npm.cmd run build; cd ..
```

### 打包现代 WebUI 标准款

```powershell
python -m PyInstaller --noconfirm --clean --icon .\images\SnailHeater_256.ico -w -F `
  --add-data ".\webui\dist;webui\dist" `
  --collect-data webview `
  --collect-submodules webview `
  --hidden-import uvicorn.logging `
  --hidden-import uvicorn.loops.auto `
  --hidden-import uvicorn.protocols.http.auto `
  .\SnailHeater_Modern.py
```

python -m PyInstaller --noconfirm --clean --icon .\images\SELO_256.ico -w -F `
  --add-data ".\webui\dist;webui\dist" `
  --collect-data webview `
  --collect-submodules webview `
  --hidden-import uvicorn.logging `
  --hidden-import uvicorn.loops.auto `
  --hidden-import uvicorn.protocols.http.auto `
  .\SnailHeater_Modern_EL.py

### 打包经典 PySide2 标准款

```powershell
python -m PyInstaller --noconfirm --clean --icon .\images\SnailHeater_256.ico -w -F `
  --add-data ".\images;images" `
  .\SnailHeater_WinTool_pyside2.py
```

### 打包经典 PySide2 EL 款

```powershell
python -m PyInstaller --noconfirm --clean --icon .\images\SELO_256.ico -w -F `
  --add-data ".\images;images" `
  .\SnailHeater_WinTool_pyside2_EL.py
```

### 打包命令行工具

```powershell
python -m PyInstaller --noconfirm --clean --icon .\images\SnailHeater_256.ico -F `
  --add-data ".\images;images" `
  --collect-all esptool `
  .\SnailHeater_Tool.py
```

所有产物默认输出到：

```text
dist\
```

## 打包后部署目录

以标准现代 WebUI 为例，`dist` 中至少需要：

```text
dist/
├─ SnailHeater_Modern.exe
├─ SnailHeater_Tool.yaml       # 外部配置，不在 EXE 内
├─ SnailHeater_Tool_EL.yaml    # 外部配置，不在 EXE 内
└─ 其他 PyInstaller 运行文件
```

如果使用 `-F`，PyInstaller 会生成单文件 EXE；配置文件仍必须放在 EXE 旁边。不要把 YAML 通过 `--add-data` 加入命令。

如果没有随程序打包 `ffmpeg.exe`，则必须保证系统 PATH 中存在 `ffmpeg`。


## 运行数据与配置

- 标准款配置：`SnailHeater_Tool.yaml`；
- EL 配置：`SnailHeater_Tool_EL.yaml`；
- 配置文件在源码运行时位于项目根目录，打包运行时位于 EXE 同目录；
- 主题偏好：`%APPDATA%\SnailHeaterTool\webui-preferences.json`；
- 视频转换、壁纸、背景和 Coredump：`Generate\`；
- FFmpeg：项目根目录的 `ffmpeg.exe`，或系统 PATH 中的 `ffmpeg`；
- 刷写 Stub：`stub_flasher\`。

YAML 配置文件属于外部运行配置，不应打包进 EXE。修改 YAML 后无需重新构建程序。

## 架构说明

```text
现代 WebUI
React / TypeScript
        │ REST + SSE
        ▼
FastAPI + pywebview
        │
        ▼
WebToolSession
        │
        ├─ DeviceService
        ├─ FirmwareService
        ├─ MediaService
        ├─ ActivationService
        └─ ProductProfile

经典 Qt UI
PySide2 / PyQt5
        │
        ▼
BaseToolController
        │
        └─ 复用上述共享服务
```

FastAPI 只监听 `127.0.0.1`，每次启动生成随机访问令牌。WebUI 不提供切换经典 Qt UI 的按钮，经典 Qt UI 需要直接运行对应的入口文件。
