#!/usr/bin/env bash
# 在 macOS 上构建 SnailHeater Modern EL 的 .app 与 .dmg（等价于 Windows 下的 PyInstaller 流程）。
#
# 用法：
#   bash build_macos.sh                       # 默认使用 python3.8 + 官方 PyPI
#   PYTHON_BIN=python3.8 PIP_INDEX_URL=https://mirrors.aliyun.com/pypi/simple/ bash build_macos.sh
#
# 产物：
#   dist/SnailHeater_Modern_EL.app
#   dist/SnailHeater_Modern_EL_macOS.dmg

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

APP_NAME="SnailHeater_Modern_EL"
ENTRY="SnailHeater_Modern_EL.py"
VENV_DIR=".venv_webui_py38"
REQUIREMENTS="requirements/requirements_webui_venv_el.txt"
BUNDLE_ID="com.snailheater.modern.el"
ICON_SRC="images/SnailHeater_256.png"
ICON_ICNS="images/SnailHeater_256.icns"
DIST_DIR="dist"
DMG_NAME="${APP_NAME}_macOS.dmg"
PIP_INDEX_URL="${PIP_INDEX_URL:-https://pypi.org/simple}"

log() { printf '\n==> %s\n' "$*"; }

# 由 PNG 生成 .icns（iconutil 要求十个尺寸齐全，源图不足时用 sips 放大）
make_icns() {
    local src="$1" out="$2" iconset
    if [ ! -f "$src" ]; then
        echo "缺少图标源文件：$src" >&2
        return 1
    fi
    iconset="$(mktemp -d)/AppIcon.iconset"
    mkdir -p "$iconset"
    # 兼容 macOS 自带的 bash 3.2：不使用数组
    local pairs size name
    pairs="16 icon_16x16.png
32 icon_16x16@2x.png
32 icon_32x32.png
64 icon_32x32@2x.png
128 icon_128x128.png
256 icon_128x128@2x.png
256 icon_256x256.png
512 icon_256x256@2x.png
512 icon_512x512.png
1024 icon_512x512@2x.png"
    while read -r size name; do
        [ -n "$size" ] || continue
        sips -z "$size" "$size" "$src" --out "$iconset/$name" >/dev/null
    done <<< "$pairs"
    rm -f "$out"
    iconutil -c icns "$iconset" -o "$out"
    rm -rf "$(dirname "$iconset")"
    log "已生成图标 $out"
}

PYTHON_BIN="${PYTHON_BIN:-python3.8}"
if ! command -v "$PYTHON_BIN" >/dev/null 2>&1; then
    echo "未找到 $PYTHON_BIN，回退到 python3（建议安装 3.8 以匹配 Windows 构建）" >&2
    PYTHON_BIN="python3"
fi
log "使用解释器：$("$PYTHON_BIN" -c 'import sys; print(sys.executable, sys.version.split()[0])')"

log "创建虚拟环境 $VENV_DIR"
rm -rf "$VENV_DIR"
"$PYTHON_BIN" -m venv "$VENV_DIR"
# shellcheck disable=SC1090
source "$VENV_DIR/bin/activate"

log "安装 Python 依赖（$PIP_INDEX_URL）"
python -m pip install --upgrade pip
python -m pip install -r "$REQUIREMENTS" -i "$PIP_INDEX_URL"

log "构建 WebUI"
pushd webui >/dev/null
npm install
npm run build
popd >/dev/null
if [ ! -f "webui/dist/index.html" ]; then
    echo "WebUI 构建产物缺失：webui/dist/index.html" >&2
    exit 1
fi

make_icns "$ICON_SRC" "$ICON_ICNS"

log "PyInstaller 打包（macOS 下 -w 会自动生成 .app）"
rm -rf build "$DIST_DIR"
# macOS 的 --add-data 使用 ':' 作为分隔符，Windows 使用 ';'
python -m PyInstaller --noconfirm --clean \
    --icon "$ICON_ICNS" -w -F \
    --name "$APP_NAME" \
    --osx-bundle-identifier "$BUNDLE_ID" \
    --add-data "webui/dist:webui/dist" \
    --collect-data webview \
    --collect-submodules webview \
    --hidden-import uvicorn.logging \
    --hidden-import uvicorn.loops.auto \
    --hidden-import uvicorn.protocols.http.auto \
    --hidden-import webview.platforms.cocoa \
    "$ENTRY"

APP_PATH="$DIST_DIR/$APP_NAME.app"
if [ ! -d "$APP_PATH" ]; then
    echo "未生成 .app 包，请检查 PyInstaller 输出：$APP_PATH" >&2
    exit 1
fi

log "临时签名（ad-hoc），便于本地运行"
codesign --force --deep --sign - "$APP_PATH" >/dev/null 2>&1 || \
    echo "提示：ad-hoc 签名失败，可忽略（未签名应用需右键打开或执行 xattr -cr）" >&2

log "制作 DMG"
STAGE_DIR="$(mktemp -d)"
cp -R "$APP_PATH" "$STAGE_DIR/"
ln -s /Applications "$STAGE_DIR/Applications"
rm -f "$DIST_DIR/$DMG_NAME"
hdiutil create -volname "$APP_NAME" -srcfolder "$STAGE_DIR" -ov -format UDZO \
    "$DIST_DIR/$DMG_NAME" >/dev/null
rm -rf "$STAGE_DIR"

log "构建完成"
ls -lh "$DIST_DIR/$DMG_NAME"
