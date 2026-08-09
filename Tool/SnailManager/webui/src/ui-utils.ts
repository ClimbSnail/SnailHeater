export type Theme = "system" | "light" | "dark";

export function resolveTheme(theme: Theme, systemPrefersDark: boolean): "light" | "dark" {
  if (theme === "system") return systemPrefersDark ? "dark" : "light";
  return theme;
}

export function shouldDisableDeviceAction(port: string, busy: boolean): boolean {
  return !port || busy;
}

export function sanitizeUserMessage(value: string): string {
  return value
    .replace(/HTTPConnectionPool\(host=["'][^"']+["'],\s*port=\d+\)/gi, "服务连接")
    .replace(/\bhost=["'][^"']+["'],?\s*port=\d+\b/gi, "服务连接")
    .replace(/https?:\/\/(?:\[[0-9a-f:]+\]|(?:\d{1,3}\.){3}\d{1,3}|localhost|[a-z0-9.-]+)(?::\d{1,5})?(?:\/[^\s)"']*)?/gi, "服务接口")
    .replace(/\b(?:\d{1,3}\.){3}\d{1,3}(?::\d{1,5})?\b/g, "服务地址")
    .replace(/\blocalhost(?::\d{1,5})?\b/gi, "本地服务");
}

export function resolveSelectedPort(current: string, availablePorts: string[]): string {
  if (current && availablePorts.includes(current)) return current;
  return availablePorts[0] ?? "";
}

export type ProgressOperationKind = "flash" | "wallpaper" | "background";
export type ProgressDisplayStatus = "running" | "completed" | "failed" | "cancelled";

const progressOperationLabels: Record<ProgressOperationKind, string> = {
  flash: "刷写固件",
  wallpaper: "写入壁纸",
  background: "写入背景图",
};

export function isProgressOperation(kind: string): kind is ProgressOperationKind {
  return kind === "flash" || kind === "wallpaper" || kind === "background";
}

export function nextSimulatedProgress(value: number): number {
  return Math.min(99, Math.max(1, Math.floor(value) + 1));
}

export function settleSimulatedProgress(status: ProgressDisplayStatus, value: number): number {
  if (status === "completed") return 100;
  return Math.min(99, Math.max(1, Math.floor(value)));
}

export function progressDisplayLabel(kind: ProgressOperationKind, status: ProgressDisplayStatus): string {
  const label = progressOperationLabels[kind];
  if (status === "running") return `正在${label}`;
  if (status === "completed") return `${label}已完成`;
  if (status === "failed") return `${label}失败`;
  return `${label}已取消`;
}

export function toColorInputValue(value: string): string {
  return /^[0-9A-F]{6}$/i.test(value) ? `#${value}` : "#000000";
}

export function fromColorInputValue(value: string): string {
  return value.replace(/^#/, "").toUpperCase();
}

export function extractExternalUrl(value: string): string {
  const href = value.match(/href\s*=\s*["']([^"']+)["']/i)?.[1];
  const candidate = (href ?? value.replace(/<[^>]*>/g, "")).trim();
  return /^https?:\/\/\S+$/i.test(candidate) ? candidate : "";
}

export type ResponsiveLayout = "full" | "compact" | "narrow";

export function resolveResponsiveLayout(viewportWidth: number): ResponsiveLayout {
  if (viewportWidth < 960) return "narrow";
  if (viewportWidth < 1280) return "compact";
  return "full";
}

