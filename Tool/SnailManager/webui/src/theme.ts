export type Theme = "system" | "light" | "dark";

/** 将用户主题偏好解析为实际应用到页面的浅色或深色方案。 */
export function resolveTheme(theme: Theme, systemPrefersDark: boolean): "light" | "dark" {
  if (theme === "system") return systemPrefersDark ? "dark" : "light";
  return theme;
}
