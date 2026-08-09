import { describe, expect, it } from "vitest";
import { extractExternalUrl, fromColorInputValue, isProgressOperation, nextSimulatedProgress, progressDisplayLabel, resolveResponsiveLayout, resolveSelectedPort, resolveTheme, sanitizeUserMessage, settleSimulatedProgress, shouldDisableDeviceAction, toColorInputValue } from "./ui-utils";

describe("现代界面状态工具", () => {
  it("系统主题会根据操作系统偏好解析为具体色彩方案", () => {
    expect(resolveTheme("system", true)).toBe("dark");
    expect(resolveTheme("system", false)).toBe("light");
    expect(resolveTheme("light", true)).toBe("light");
  });

  it("设备操作在未选串口或任务执行期间会被禁用", () => {
    expect(shouldDisableDeviceAction("", false)).toBe(true);
    expect(shouldDisableDeviceAction("COM3", true)).toBe(true);
    expect(shouldDisableDeviceAction("COM3", false)).toBe(false);
  });

  it("用户提示会隐藏后端 IP、端口和接口地址", () => {
    expect(sanitizeUserMessage("请求 http://127.0.0.1:54321/api/device/query 失败")).toBe("请求 服务接口 失败");
    expect(sanitizeUserMessage("HTTPConnectionPool(host='api.example.com', port=80) 连接失败")).toBe("服务连接 连接失败");
    expect(sanitizeUserMessage("无法连接 192.168.1.20:8080")).toBe("无法连接 服务地址");
    expect(sanitizeUserMessage("设备未返回有效的机器码")).toBe("设备未返回有效的机器码");
  });

  it("只为固件、壁纸和背景图任务显示模拟进度", () => {
    expect(isProgressOperation("flash")).toBe(true);
    expect(isProgressOperation("wallpaper")).toBe(true);
    expect(isProgressOperation("background")).toBe(true);
    expect(isProgressOperation("pack-wallpaper")).toBe(false);
    expect(isProgressOperation("coredump")).toBe(false);
  });

  it("模拟进度逐秒增长且最高停留在 99%", () => {
    expect(nextSimulatedProgress(0)).toBe(1);
    expect(nextSimulatedProgress(1)).toBe(2);
    expect(nextSimulatedProgress(98)).toBe(99);
    expect(nextSimulatedProgress(99)).toBe(99);
  });

  it("任务成功到 100%，失败和取消保留当前进度", () => {
    expect(settleSimulatedProgress("completed", 36)).toBe(100);
    expect(settleSimulatedProgress("failed", 36)).toBe(36);
    expect(settleSimulatedProgress("cancelled", 36)).toBe(36);
    expect(progressDisplayLabel("flash", "running")).toBe("正在刷写固件");
    expect(progressDisplayLabel("wallpaper", "completed")).toBe("写入壁纸已完成");
    expect(progressDisplayLabel("background", "cancelled")).toBe("写入背景图已取消");
  });

  it("重新扫描串口时保留有效选择，并对失效选择执行回退", () => {
    expect(resolveSelectedPort("COM5", ["COM3", "COM5"])).toBe("COM5");
    expect(resolveSelectedPort("COM9", ["COM3", "COM5"])).toBe("COM3");
    expect(resolveSelectedPort("", ["COM3", "COM5"])).toBe("COM3");
    expect(resolveSelectedPort("COM5", [])).toBe("");
  });

  it("调色盘颜色与设备十六进制格式可以双向转换", () => {
    expect(toColorInputValue("7AFE89")).toBe("#7AFE89");
    expect(toColorInputValue("BAD")).toBe("#000000");
    expect(fromColorInputValue("#7afe89")).toBe("7AFE89");
  });

  it("能将旧 Qt 富文本链接转换为安全的外部链接", () => {
    expect(extractExternalUrl('<a href="https://example.com/guide">教程</a>')).toBe("https://example.com/guide");
    expect(extractExternalUrl("https://www.bilibili.com/video/BV1oG4y1h7A8?p=2")).toBe("https://www.bilibili.com/video/BV1oG4y1h7A8?p=2");
    expect(extractExternalUrl("https://docs.qq.com/doc/DQWdkTFpvV2ZSR29G")).toBe("https://docs.qq.com/doc/DQWdkTFpvV2ZSR29G");
    expect(extractExternalUrl("未配置")).toBe("");
  });

  it("按窗口宽度选择完整、紧凑和窄屏布局", () => {
    expect(resolveResponsiveLayout(1920)).toBe("full");
    expect(resolveResponsiveLayout(1280)).toBe("full");
    expect(resolveResponsiveLayout(1279)).toBe("compact");
    expect(resolveResponsiveLayout(960)).toBe("compact");
    expect(resolveResponsiveLayout(959)).toBe("narrow");
  });
});
