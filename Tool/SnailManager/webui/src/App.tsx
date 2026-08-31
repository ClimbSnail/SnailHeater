import { useEffect, useMemo, useRef, useState, type ReactElement, type ReactNode } from "react";
import { extractExternalUrl, fromColorInputValue, isProgressOperation, nextSimulatedProgress, progressDisplayLabel, resolveResponsiveLayout, resolveSelectedPort, resolveTheme, sanitizeUserMessage, settleSimulatedProgress, toColorInputValue, type ProgressOperationKind, type ResponsiveLayout, type Theme as ThemePreference } from "./ui-utils";
import {
  Activity,
  Cable,
  CheckCircle2,
  CircleAlert,
  CircleStop,
  ClipboardList,
  Copy,
  Cpu,
  ExternalLink,
  FileDown,
  FolderOpen,
  HardDrive,
  Image,
  Info,
  LoaderCircle,
  Monitor,
  Music2,
  Moon,
  Palette,
  RefreshCw,
  Settings,
  Sun,
  Terminal,
  UploadCloud,
  X,
  Zap,
} from "lucide-react";

type Product = "snailheater" | "el";
type Page = "device" | "firmware" | "media" | "logs" | "settings";
type Theme = ThemePreference;
type InfoLink = { name: string; url: string };

type Bootstrap = {
  product: Product;
  toolName: string;
  toolVersion: string;
  resolutions: string[];
  supportsActivation: boolean;
  supportsRtttl: boolean;
  defaultPage: Page;
  links: { guide: InfoLink; updates: InfoLink };
  qq: string[];
};

type Port = { name: string; description: string; display: string };
type Preferences = { theme: Theme };
type VersionInfo = { tool: string; firmware: string | null };
type Operation = {
  operationId: string;
  kind: string;
  status: "running" | "completed" | "failed" | "cancelled";
  result?: unknown;
  error?: string;
  cancellable: boolean;
};

type ProgressState = {
  operationId: string;
  kind: ProgressOperationKind;
  status: Operation["status"];
  value: number;
};

type ApiResult<T> = { ok: true; data: T } | { ok: false; error: { code: string; message: string } };

declare global {
  interface Window {
    pywebview?: {
      api?: {
        select_files?: (multiple?: boolean, backgroundOnly?: boolean) => Promise<string[]>;
      };
    };
  }
}

const token = new URLSearchParams(window.location.search).get("token") ?? "";

async function api<T>(path: string, options: RequestInit = {}): Promise<T> {
  const response = await fetch(path, {
    ...options,
    headers: {
      "X-Snail-Token": token,
      ...(options.body ? { "Content-Type": "application/json" } : {}),
      ...(options.headers ?? {}),
    },
  });
  const result = (await response.json()) as ApiResult<T>;
  if (!result.ok) throw new Error(result.error.message);
  return result.data;
}

function setDocumentTheme(theme: Theme): void {
  const resolved = resolveTheme(theme, window.matchMedia("(prefers-color-scheme: dark)").matches);
  document.documentElement.dataset.theme = resolved;
}

function App(): ReactElement {
  const [bootstrap, setBootstrap] = useState<Bootstrap | null>(null);
  const [page, setPage] = useState<Page>("device");
  const [theme, setTheme] = useState<Theme>("system");
  const [ports, setPorts] = useState<Port[]>([]);
  const [port, setPort] = useState("");
  const [firmwares, setFirmwares] = useState<string[]>([]);
  const [firmware, setFirmware] = useState("");
  const [normalColor, setNormalColor] = useState("FFFFFF");
  const [pressedColor, setPressedColor] = useState("7AFE89");
  const [logs, setLogs] = useState<string[]>(["现代控制台已就绪，等待设备连接。"]);
  const [operation, setOperation] = useState<Operation | null>(null);
  const [progress, setProgress] = useState<ProgressState | null>(null);
  const [notice, setNotice] = useState<{ kind: "success" | "error" | "info"; text: string } | null>(null);
  const [coredumpSavedPath, setCoredumpSavedPath] = useState<string | null>(null);
  const [confirmationMessage, setConfirmationMessage] = useState<string | null>(null);
  const confirmationResolver = useRef<((accepted: boolean) => void) | null>(null);
  const [busy, setBusy] = useState(false);
  const [versions, setVersions] = useState<VersionInfo | null>(null);
  const [versionLoading, setVersionLoading] = useState(false);
  const [layoutMode, setLayoutMode] = useState<ResponsiveLayout>(() => resolveResponsiveLayout(window.innerWidth));

  const productLabel = bootstrap?.product === "el" ? "SnailHeater EL" : "SnailHeater";
  const isRunning = operation?.status === "running";

  const appendLog = (message: string): void => {
    const safeMessage = sanitizeUserMessage(message);
    setLogs((current) => [...current, `[${new Date().toLocaleTimeString("zh-CN", { hour12: false })}] ${safeMessage}`].slice(-1000));
  };

  const showError = (error: unknown): void => {
    const rawText = error instanceof Error ? error.message : String(error);
    const text = sanitizeUserMessage(rawText);
    setNotice({ kind: "error", text });
    appendLog(`错误：${text}`);
  };

  const refreshPorts = async (): Promise<void> => {
    try {
      const values = await api<Port[]>("/api/ports");
      setPorts(values);
      setPort((current) => resolveSelectedPort(current, values.map((item) => item.name)));
      appendLog(values.length ? `检测到 ${values.length} 个串口。` : "未检测到可用串口。");
    } catch (error) {
      showError(error);
    }
  };

  const refreshFirmware = async (): Promise<void> => {
    try {
      const values = await api<string[]>("/api/firmware");
      setFirmwares(values);
      setFirmware((current) => current && values.includes(current) ? current : values[0] || "");
      appendLog(values.length ? `已扫描到 ${values.length} 个固件。` : "当前目录没有匹配的固件。");
    } catch (error) {
      showError(error);
    }
  };

  const checkVersions = async (): Promise<void> => {
    try {
      setVersionLoading(true);
      const value = await api<VersionInfo>("/api/versions");
      setVersions(value);
      appendLog("已完成在线版本检查。");
    } catch (error) {
      showError(error);
    } finally {
      setVersionLoading(false);
    }
  };

  useEffect(() => {
    const updateLayout = (): void => setLayoutMode(resolveResponsiveLayout(window.innerWidth));
    window.addEventListener("resize", updateLayout);
    return () => window.removeEventListener("resize", updateLayout);
  }, []);

  useEffect(() => () => {
    confirmationResolver.current?.(false);
    confirmationResolver.current = null;
  }, []);

  useEffect(() => {
    const initialize = async (): Promise<void> => {
      try {
        const [app, preferences] = await Promise.all([
          api<Bootstrap>("/api/bootstrap"),
          api<Preferences>("/api/preferences"),
        ]);
        setBootstrap(app);
        setPage(app.product === "el" ? "firmware" : app.defaultPage);
        setTheme(preferences.theme);
        setDocumentTheme(preferences.theme);
        await Promise.all([refreshPorts(), refreshFirmware()]);
        // 在线版本检查不参与首屏初始化，网络较慢或服务不可用时不会拖慢界面启动。
        void checkVersions();
      } catch (error) {
        showError(error);
      }
    };
    void initialize();
  }, []);

  useEffect(() => {
    if (!operation || !isProgressOperation(operation.kind)) {
      setProgress(null);
      return;
    }

    const operationId = operation.operationId;
    const kind = operation.kind;
    const status = operation.status;

    if (status === "running") {
      setProgress((current) => current?.operationId === operationId
        ? { ...current, kind, status }
        : { operationId, kind, status, value: 1 });
      const timer = window.setInterval(() => {
        setProgress((current) => current?.operationId === operationId && current.status === "running"
          ? { ...current, value: nextSimulatedProgress(current.value) }
          : current);
      }, 1000);
      return () => window.clearInterval(timer);
    }

    setProgress((current) => {
      const currentValue = current?.operationId === operationId ? current.value : 1;
      return { operationId, kind, status, value: settleSimulatedProgress(status, currentValue) };
    });
    const hideDelay = status === "completed" ? 1400 : 1800;
    const timeout = window.setTimeout(() => {
      setProgress((current) => current?.operationId === operationId ? null : current);
    }, hideDelay);
    return () => window.clearTimeout(timeout);
  }, [operation?.operationId, operation?.kind, operation?.status]);

  const saveTheme = async (nextTheme: Theme): Promise<void> => {
    setTheme(nextTheme);
    setDocumentTheme(nextTheme);
    try {
      await api<Preferences>("/api/preferences", {
        method: "PUT",
        body: JSON.stringify({ theme: nextTheme }),
      });
      setNotice({ kind: "success", text: "主题偏好已保存。" });
    } catch (error) {
      showError(error);
    }
  };

  const requestConfirmation = (message: string): Promise<boolean> => new Promise((resolve) => {
    confirmationResolver.current?.(false);
    confirmationResolver.current = resolve;
    setConfirmationMessage(message);
  });

  const resolveConfirmation = (accepted: boolean): void => {
    const resolve = confirmationResolver.current;
    confirmationResolver.current = null;
    setConfirmationMessage(null);
    resolve?.(accepted);
  };

  const startOperation = async (kind: string, payload: Record<string, unknown>, confirmation: string): Promise<void> => {
    if (isRunning) {
      setNotice({ kind: "info", text: "当前已有任务正在执行，请先等待或取消。" });
      return;
    }
    if (!(await requestConfirmation(confirmation))) return;
    try {
      setBusy(true);
      const next = await api<Operation>(`/api/operations/${kind}`, {
        method: "POST",
        body: JSON.stringify(payload),
      });
      setOperation(next);
      appendLog(`已启动任务：${kind}`);
      const stream = new EventSource(`/api/operations/${next.operationId}/events?token=${encodeURIComponent(token)}`);
      stream.onmessage = (event) => {
        const data = JSON.parse(event.data) as { type: string; message?: string; error?: { message: string }; result?: unknown };
        if (data.type === "log" && data.message) appendLog(data.message);
        if (data.type === "completed") {
          setOperation((current) => current ? { ...current, status: "completed", cancellable: false, result: data.result } : current);
          const coredumpPath = next.kind === "coredump" && data.result && typeof data.result === "object"
            ? (data.result as { path?: unknown }).path
            : undefined;
          const completionMessage = typeof coredumpPath === "string"
            ? `Coredump 已保存至：${coredumpPath}`
            : "任务已完成。";
          if (typeof coredumpPath === "string") setCoredumpSavedPath(coredumpPath);
          setNotice({ kind: "success", text: completionMessage });
          appendLog(completionMessage);
          stream.close();
        }
        if (data.type === "failed") {
          setOperation((current) => current ? { ...current, status: "failed", cancellable: false, error: data.error?.message } : current);
          showError(data.error?.message ?? "任务执行失败");
          stream.close();
        }
        if (data.type === "cancelled") {
          setOperation((current) => current ? { ...current, status: "cancelled", cancellable: false } : current);
          setNotice({ kind: "info", text: "任务已取消。" });
          appendLog("任务已取消。");
          stream.close();
        }
      };
      stream.onerror = () => {
        stream.close();
      };
    } catch (error) {
      showError(error);
    } finally {
      setBusy(false);
    }
  };

  const cancelOperation = async (): Promise<void> => {
    if (!operation?.cancellable) return;
    try {
      await api<Operation>(`/api/operations/${operation.operationId}/cancel`, { method: "POST" });
      appendLog("已向后台任务发送取消请求。");
      setNotice({ kind: "info", text: "已请求取消，正在等待当前硬件步骤结束。" });
    } catch (error) {
      showError(error);
    }
  };

  if (!bootstrap) {
    return <div className="startup"><LoaderCircle className="spin" size={30} /> 正在启动本地控制台…</div>;
  }

  return (
    <div className="app-shell" data-layout={layoutMode}>
      <aside className="sidebar">
        <div className="brand"><div className="brand-mark"><Zap size={20} /></div><div><strong>Snail<span>Console</span></strong><small>DEVICE STUDIO</small></div></div>
        <nav>
          {bootstrap.product !== "el" && <NavItem icon={<Cable size={18} />} label="设备连接" active={page === "device"} onClick={() => setPage("device")} />}
          <NavItem icon={<UploadCloud size={18} />} label="固件刷写" active={page === "firmware"} onClick={() => setPage("firmware")} />
          <NavItem icon={<Image size={18} />} label="媒体资源" active={page === "media"} onClick={() => setPage("media")} />
          <NavItem icon={<Terminal size={18} />} label="操作日志" active={page === "logs"} onClick={() => setPage("logs")} />
          <NavItem icon={<Settings size={18} />} label="应用设置" active={page === "settings"} onClick={() => setPage("settings")} />
        </nav>
        <div className="sidebar-footer" title="React 界面已连接到本机控制服务，不代表设备已经连接。"><span className="status-dot" /> 本地界面服务运行中<small>{bootstrap.toolVersion}</small></div>
      </aside>

      <main className="main-area">
        <header className="topbar">
          <div><p className="eyebrow">设备管理工作台</p><h1>{productLabel}</h1></div>
          <div className="top-actions">
            <label className="port-select" title="选择目标串口">
              <Cable size={15} />
              <select value={port} onPointerDown={() => void refreshPorts()} onKeyDown={(event) => { if (["ArrowDown", "Enter", " "].includes(event.key)) void refreshPorts(); }} onChange={(event) => setPort(event.target.value)} aria-label="目标串口">
                <option value="">未选择串口</option>
                {ports.map((item) => <option value={item.name} key={item.name}>{item.display}</option>)}
              </select>
            </label>
            <button className="icon-button" title="切换主题" onClick={() => void saveTheme(theme === "dark" ? "light" : "dark")}>{theme === "dark" ? <Sun size={17} /> : <Moon size={17} />}</button>
          </div>
        </header>

        <section className="page-content">
          {notice && <div className={`notice ${notice.kind}`}><span>{notice.kind === "error" ? <CircleAlert size={17} /> : notice.kind === "success" ? <CheckCircle2 size={17} /> : <Info size={17} />}</span>{notice.text}<button onClick={() => setNotice(null)}><X size={16} /></button></div>}
          {bootstrap.product !== "el" && page === "device" && <DevicePage bootstrap={bootstrap} port={port} normal={normalColor} pressed={pressedColor} setNormal={setNormalColor} setPressed={setPressedColor} appendLog={appendLog} showError={showError} />}
          {page === "firmware" && <FirmwarePage port={port} firmwares={firmwares} firmware={firmware} setFirmware={setFirmware} refreshFirmware={refreshFirmware} run={startOperation} busy={busy || isRunning} showPowerWarning={bootstrap.product === "snailheater"} />}
          {page === "media" && <MediaPage bootstrap={bootstrap} port={port} run={startOperation} busy={busy || isRunning} />}
          {page === "logs" && <LogsPage logs={logs} operation={operation} cancel={cancelOperation} />}
          {page === "settings" && <SettingsPage theme={theme} setTheme={saveTheme} bootstrap={bootstrap} versions={versions} versionLoading={versionLoading} checkVersions={checkVersions} />}
        </section>

        {progress && <section className={`global-progress ${progress.status}`} role="progressbar" aria-label={progressDisplayLabel(progress.kind, progress.status)} aria-valuemin={0} aria-valuemax={100} aria-valuenow={progress.value} aria-valuetext={`${progressDisplayLabel(progress.kind, progress.status)}，${progress.value}%`}>
          <div className="global-progress-header"><span>{progressDisplayLabel(progress.kind, progress.status)}</span><strong>{progress.value}%</strong></div>
          <div className="global-progress-track"><span style={{ width: `${progress.value}%` }} /></div>
        </section>}
        <footer className="statusbar"><span><Activity size={14} /> {operation?.status === "running" ? `正在执行：${operation.kind}` : "就绪"}</span><span><HardDrive size={14} /> 本地模式</span></footer>
      </main>
      {coredumpSavedPath && <div className="confirmation-backdrop" onMouseDown={() => setCoredumpSavedPath(null)}>
        <section className="confirmation-dialog" role="dialog" aria-modal="true" aria-labelledby="coredump-saved-title" onMouseDown={(event) => event.stopPropagation()} onKeyDown={(event) => { if (event.key === "Escape") setCoredumpSavedPath(null); }}>
          <div className="confirmation-icon"><CheckCircle2 size={22} /></div>
          <div className="confirmation-copy"><h2 id="coredump-saved-title">Coredump 已保存</h2><p>{coredumpSavedPath}</p></div>
          <div className="confirmation-actions"><button className="primary-button" autoFocus onClick={() => setCoredumpSavedPath(null)}>知道了</button></div>
        </section>
      </div>}
      {confirmationMessage && <div className="confirmation-backdrop" onMouseDown={() => resolveConfirmation(false)}>
        <section className="confirmation-dialog" role="alertdialog" aria-modal="true" aria-labelledby="confirmation-title" aria-describedby="confirmation-message" onMouseDown={(event) => event.stopPropagation()} onKeyDown={(event) => { if (event.key === "Escape") resolveConfirmation(false); }}>
          <div className="confirmation-icon"><CircleAlert size={22} /></div>
          <div className="confirmation-copy"><h2 id="confirmation-title">确认操作</h2><p id="confirmation-message">{confirmationMessage}</p></div>
          <div className="confirmation-actions"><button className="secondary-button" autoFocus onClick={() => resolveConfirmation(false)}>取消</button><button className="primary-button" onClick={() => resolveConfirmation(true)}>确认</button></div>
        </section>
      </div>}
    </div>
  );
}

function NavItem({ icon, label, active, onClick }: { icon: ReactNode; label: string; active: boolean; onClick: () => void }): ReactElement {
  return <button className={`nav-item ${active ? "active" : ""}`} title={label} aria-label={label} onClick={onClick}>{icon}<span>{label}</span></button>;
}

function Card({ title, subtitle, children, action }: { title: string; subtitle?: string; children: ReactNode; action?: ReactNode }): ReactElement {
  return <section className="card"><header className="card-header"><div><h2>{title}</h2>{subtitle && <p>{subtitle}</p>}</div>{action}</header>{children}</section>;
}

function DevicePage({ bootstrap, port, normal, pressed, setNormal, setPressed, appendLog, showError }: { bootstrap: Bootstrap; port: string; normal: string; pressed: string; setNormal: (value: string) => void; setPressed: (value: string) => void; appendLog: (text: string) => void; showError: (error: unknown) => void }): ReactElement {
  const [machineCode, setMachineCode] = useState("");
  const [sn, setSn] = useState("");
  const [registrant, setRegistrant] = useState("");
  const [queryLoading, setQueryLoading] = useState(false);
  const [activationLoading, setActivationLoading] = useState(false);

  useEffect(() => {
    setMachineCode("");
    setSn("");
    setRegistrant("");
  }, [port]);

  const queryActivation = async (): Promise<void> => {
    try {
      setQueryLoading(true);
      setMachineCode("");
      setSn("");
      setRegistrant("");
      appendLog("正在读取机器码并查询激活码……");
      const data = await api<{
        machineCode: string;
        activation: { sn: string; registrant: string } | null;
        activationError?: string | null;
      }>("/api/device/query", { method: "POST", body: JSON.stringify({ port }) });
      setMachineCode(data.machineCode);
      setSn(data.activation?.sn || "");
      setRegistrant(data.activation?.registrant || "");
      appendLog(`机器码查询成功：${data.machineCode}`);
      if (data.activationError) {
        showError(new Error(`激活码联网查询失败：${data.activationError}`));
      } else if (data.activation?.sn) {
        appendLog("激活码查询成功，已自动填入。");
      } else {
        appendLog("未查询到该机器码对应的激活码。");
      }
    } catch (error) {
      showError(error);
    } finally {
      setQueryLoading(false);
    }
  };

  const activate = async (): Promise<void> => {
    try {
      setActivationLoading(true);
      const result = await api<{ success: boolean; message: string }>("/api/device/activate", { method: "POST", body: JSON.stringify({ port, sn }) });
      appendLog(result.message);
    } catch (error) {
      showError(error);
    } finally {
      setActivationLoading(false);
    }
  };
  const readColors = async (): Promise<void> => {
    try {
      const colors = await api<{ normal: string; pressed: string }>(`/api/device/colors?port=${encodeURIComponent(port)}`);
      setNormal(colors.normal); setPressed(colors.pressed); appendLog("已读取设备文本颜色。");
    } catch (error) { showError(error); }
  };
  const writeColors = async (): Promise<void> => {
    try {
      await api("/api/device/colors", { method: "PUT", body: JSON.stringify({ port, normal, pressed }) });
      appendLog("已写入设备文本颜色。");
    } catch (error) { showError(error); }
  };

  return <div className="device-tools-grid">
    {bootstrap.supportsActivation && <Card title="设备激活" subtitle="查询机器码对应的激活码，并写入当前设备">
      <div className="activation-fields">
        <label><span>机器码</span><div className="inline-form"><input value={machineCode} readOnly placeholder="请先查询机器码" /><button className="secondary-button" disabled={!port || queryLoading || activationLoading} onClick={() => void queryActivation()}>{queryLoading ? <LoaderCircle className="spin" size={16} /> : <RefreshCw size={16} />} 查询</button></div></label>
        <label><span>激活码</span><div className="inline-form"><input value={sn} onChange={(event) => setSn(event.target.value)} placeholder="输入或查询 SN 激活码" /><button className="primary-button" disabled={!port || !sn.trim() || queryLoading || activationLoading} onClick={() => void activate()}>{activationLoading ? <LoaderCircle className="spin" size={16} /> : <Zap size={16} />} 写入激活</button></div></label>
      </div>
      <p className="activation-source">{registrant ? `机器来源：${registrant}` : "查询成功后将在此显示机器来源"}</p>
    </Card>}
    <Card title="文本颜色" subtitle="可直接输入十六进制颜色，也可以点击色块打开系统调色盘">
      <div className="color-fields">
        <ColorField label="普通色" value={normal} onChange={setNormal} />
        <ColorField label="可点按钮" value={pressed} onChange={setPressed} />
      </div>
      <div className="button-row"><button className="secondary-button" disabled={!port} onClick={() => void readColors()}><Palette size={16} /> 读取</button><button className="primary-button" disabled={!port} onClick={() => void writeColors()}><CheckCircle2 size={16} /> 写入</button></div>
    </Card>
  </div>;
}

function FirmwarePage({ port, firmwares, firmware, setFirmware, refreshFirmware, run, busy, showPowerWarning }: { port: string; firmwares: string[]; firmware: string; setFirmware: (value: string) => void; refreshFirmware: () => Promise<void>; run: (kind: string, payload: Record<string, unknown>, confirmation: string) => Promise<void>; busy: boolean; showPowerWarning: boolean }): ReactElement {
  const [mode, setMode] = useState("更新式");
  return <div className="grid firmware-layout">
    <Card title="固件刷写" subtitle="刷写期间请保持数据线稳定，避免设备断电">
      <div className="form-grid"><label>目标串口<input value={port || "未选择"} disabled /></label><label>固件文件<select value={firmware} onPointerDown={() => void refreshFirmware()} onKeyDown={(event) => { if (["ArrowDown", "Enter", " "].includes(event.key)) void refreshFirmware(); }} onChange={(event) => setFirmware(event.target.value)}><option value="">请选择固件</option>{firmwares.map((item) => <option key={item} value={item}>{item}</option>)}</select></label></div>
      <div className="mode-selector"><button className={mode === "更新式" ? "selected" : ""} aria-pressed={mode === "更新式"} onClick={() => setMode("更新式")}><UploadCloud size={18} /><span>更新式</span><small>保留用户设置，仅升级固件</small></button><button className={mode === "清空式" ? "selected danger" : ""} aria-pressed={mode === "清空式"} onClick={() => setMode("清空式")}><CircleAlert size={18} /><span>清空式</span><small>擦除可清空数据后重新刷写</small></button></div>
      <button className="primary-button wide" disabled={!port || !firmware || busy} onClick={() => void run("flash", { port, firmware, mode }, `确认开始固件刷写？该操作将修改设备闪存。${showPowerWarning ? "\n开始前一定要拔掉220V电源线！" : ""}`)}><UploadCloud size={17} /> 开始刷写</button>
    </Card>
    <Card title="诊断工具" subtitle="读取异常信息需要设备处于可通信状态"><div className="diagnostic"><Cpu size={28} /><div><strong>读取 Coredump</strong><p>将设备异常信息保存到本地 Generate/Coredump 目录。</p></div><button className="secondary-button" disabled={!port || busy} onClick={() => void run("coredump", { port }, "确认读取设备 Coredump？")}><FileDown size={16} /> 读取</button></div><div className="firmware-tip"><Info size={16} /> 若刷写被取消，当前 esptool 步骤会完成后再安全停止。</div></Card>
  </div>;
}

function MediaPage({ bootstrap, port, run, busy }: { bootstrap: Bootstrap; port: string; run: (kind: string, payload: Record<string, unknown>, confirmation: string) => Promise<void>; busy: boolean }): ReactElement {
  const [files, setFiles] = useState<string[]>([]);
  const [backgroundFile, setBackgroundFile] = useState("");
  const [resolution, setResolution] = useState(bootstrap.resolutions[0]);
  const [fps, setFps] = useState("20");
  const [quality, setQuality] = useState("5");
  const [startTime, setStartTime] = useState("0");
  const [endTime, setEndTime] = useState("0");
  const [cropToFill, setCropToFill] = useState(true);

  const chooseFiles = async (): Promise<void> => {
    const selected = window.pywebview?.api?.select_files
      ? await window.pywebview.api.select_files(true)
      : (window.prompt("开发模式：输入一个或多个文件路径，以分号分隔") || "").split(";").filter(Boolean);
    if (selected.length) setFiles(selected);
  };
  const chooseBackground = async (): Promise<void> => {
    const selected = window.pywebview?.api?.select_files
      ? await window.pywebview.api.select_files(false, true)
      : (window.prompt("开发模式：输入一个背景图片或 BIN 文件路径") || "").split(";").filter(Boolean);
    if (selected[0]) setBackgroundFile(selected[0]);
  };
  const payload = useMemo(() => ({ port, files, resolution, fps, quality, startTime: Number(startTime || 0), endTime: Number(endTime || 0), cropToFill }), [port, files, resolution, fps, quality, startTime, endTime, cropToFill]);
  const backgroundPayload = useMemo(() => ({ ...payload, files: backgroundFile ? [backgroundFile] : [] }), [payload, backgroundFile]);
  const convertMp4ToRtttl = async (): Promise<void> => {
    const selected = window.pywebview?.api?.select_files
      ? await window.pywebview.api.select_files(false)
      : (window.prompt("开发模式：输入一个 MP4 文件路径") || "").split(";").filter(Boolean);
    if (selected.length) await run("convert-rtttl", { source: selected[0] }, "确认按旧版算法将该 MP4 转换为 RTTTL？");
  };

  return <div className="grid media-layout">
    <Card title="壁纸与背景图通用设置" subtitle="分辨率和图片适配模式会同时应用于壁纸图片与背景图">
      <div className="shared-media-options"><label>分辨率<select value={resolution} onChange={(event) => setResolution(event.target.value)}>{bootstrap.resolutions.map((item) => <option key={item}>{item}</option>)}</select></label><fieldset className="fit-mode-field"><legend>图片适配模式</legend><div className="fit-mode-options"><label className={cropToFill ? "selected" : ""} title="保持指定的分辨率比例，在中心区域最大化裁剪"><input type="radio" name="media-fit-mode" checked={cropToFill} onChange={() => setCropToFill(true)} /><span>保持比例裁剪</span></label><label className={!cropToFill ? "selected" : ""} title="全图范围内适配最佳比例缩放，保留素材的每个区域"><input type="radio" name="media-fit-mode" checked={!cropToFill} onChange={() => setCropToFill(false)} /><span>全尺寸缩放</span></label></div></fieldset></div>
    </Card>
    {bootstrap.product !== "el" && <Card title="壁纸素材" subtitle="支持图片、视频、RTTTL、BIN 与已打包的 LSW 文件">
      <div className="file-list">{files.length ? files.map((file) => <div key={file}><Image size={16} /><span>{file}</span><button title="移除素材" onClick={() => setFiles((current) => current.filter((item) => item !== file))}><X size={15} /></button></div>) : <div className="empty-file"><Image size={28} /><span>尚未选择壁纸素材文件</span><button className="secondary-button" onClick={() => void chooseFiles()}>选择素材</button></div>}</div>
      <div className="media-options"><label title="性能一定，帧率越大越卡顿">帧率<input value={fps} onChange={(event) => setFps(event.target.value.replace(/\D/g, ""))} /></label><label title="数字越小，质量越高">质量<select value={quality} onChange={(event) => setQuality(event.target.value)}>{["1", "2", "3", "4", "5", "6", "7", "8", "9", "10"].map((item) => <option key={item}>{item}</option>)}</select></label><label title="如果不需要指定时间段请填0">开始秒<input value={startTime} onChange={(event) => setStartTime(event.target.value.replace(/\D/g, ""))} /></label><label title="如果不需要指定时间段请填0">结束秒<input value={endTime} onChange={(event) => setEndTime(event.target.value.replace(/\D/g, ""))} /></label></div>
      <section className="card-subsection"><header><h2>壁纸资源操作</h2><p>转换、打包与烧录操作均会显示在实时日志中</p></header><div className="action-grid">{bootstrap.supportsRtttl && <ActionButton icon={<Music2 size={19} />} title="MP4 转 RTTTL" text="按旧版音高识别算法生成蜂鸣器旋律" disabled onClick={() => void convertMp4ToRtttl()} />}<ActionButton icon={<ClipboardList size={19} />} title="生成壁纸包" text="将转换结果打包为 LSW 文件" disabled={!files.length || busy} onClick={() => void run("pack-wallpaper", payload, "确认生成壁纸包？")} /><ActionButton icon={<UploadCloud size={19} />} title="写入壁纸" text="转换、打包并烧录到设备" disabled={!files.length || !port || busy} onClick={() => void run("wallpaper", payload, `确认写入壁纸？该操作将修改设备闪存。${bootstrap.product === "snailheater" ? "\n开始前一定要拔掉220V电源线！" : ""}`)} /><ActionButton icon={<CircleStop size={19} />} title="清空壁纸" text="写入默认空壁纸文件" disabled={!port || busy} onClick={() => void run("clean-wallpaper", { port }, "确认清空设备中的壁纸？")} /></div></section>
    </Card>}
    <Card title="背景图" subtitle="背景素材独立于壁纸素材；仅支持 JPG、JPEG、PNG 或已生成的 RGB565 BIN 文件">
      <div className="file-list">{backgroundFile ? <div><Monitor size={16} /><span>{backgroundFile}</span><button title="清除背景素材" onClick={() => setBackgroundFile("")}><X size={15} /></button></div> : <div className="empty-file"><Monitor size={28} /><span>尚未选择背景图素材</span><button className="secondary-button" onClick={() => void chooseBackground()}>选择背景</button></div>}</div>
      <div className="button-row"><button className="primary-button" disabled={!backgroundFile || !port || busy} onClick={() => void run("background", backgroundPayload, `确认写入背景图？该操作将修改设备闪存，完成后设备会自动复位。${bootstrap.product === "snailheater" ? "\n开始前一定要拔掉220V电源线！" : ""}`)}><Monitor size={16} /> 写入背景</button></div>
    </Card>
  </div>;
}

function LogsPage({ logs, operation, cancel }: { logs: string[]; operation: Operation | null; cancel: () => Promise<void> }): ReactElement {
  const [copied, setCopied] = useState(false);
  const copyLogs = async (): Promise<void> => {
    const content = logs.join("\n");
    try {
      if (navigator.clipboard?.writeText) {
        await navigator.clipboard.writeText(content);
      } else {
        const textarea = document.createElement("textarea");
        textarea.value = content;
        textarea.style.position = "fixed";
        textarea.style.opacity = "0";
        document.body.append(textarea);
        textarea.select();
        const copiedWithFallback = document.execCommand("copy");
        textarea.remove();
        if (!copiedWithFallback) throw new Error("复制日志失败");
      }
      setCopied(true);
      window.setTimeout(() => setCopied(false), 1600);
    } catch {
      setCopied(false);
    }
  };
  return <div className="logs-layout"><Card title="实时操作日志" subtitle="日志会保留当前会话最近 1000 条记录" action={<div className="log-actions"><button className="icon-button" title={copied ? "已复制" : "复制日志"} aria-label="复制日志" onClick={() => void copyLogs()}><Copy size={16} /></button>{operation?.cancellable && <button className="danger-button" onClick={() => void cancel()}><CircleStop size={16} /> 取消任务</button>}</div>}><pre className="log-view">{logs.join("\n")}</pre></Card><Card title="任务状态"><div className="operation-status"><span className={`operation-dot ${operation?.status || "idle"}`} /><div><strong>{operation ? operation.kind : "暂无运行任务"}</strong><p>{operation ? `状态：${operation.status}` : "可以从固件刷写或媒体资源页面启动操作。"}</p></div></div></Card></div>;
}

function SettingsPage({ theme, setTheme, bootstrap, versions, versionLoading, checkVersions }: { theme: Theme; setTheme: (theme: Theme) => Promise<void>; bootstrap: Bootstrap; versions: VersionInfo | null; versionLoading: boolean; checkVersions: () => Promise<void> }): ReactElement {
  return <div className="settings-layout">
    <Card title="外观主题" subtitle="主题设置会立即应用并保存到本机用户配置"><div className="theme-options">{(["system", "light", "dark"] as Theme[]).map((item) => <button key={item} className={theme === item ? "selected" : ""} onClick={() => void setTheme(item)}>{item === "system" ? <Monitor size={19} /> : item === "light" ? <Sun size={19} /> : <Moon size={19} />}<span>{{ system: "跟随系统", light: "浅色主题", dark: "深色主题" }[item]}</span></button>)}</div></Card>
    <Card title="工具版本" subtitle="当前工具版本直接来自本地程序；在线检查不会阻塞界面" action={<button className="text-button" disabled={versionLoading} onClick={() => void checkVersions()}>{versionLoading ? <LoaderCircle className="spin" size={15} /> : <RefreshCw size={15} />} 检查更新</button>}><dl className="info-list"><div><dt>当前工具版本</dt><dd>{bootstrap.toolVersion}</dd></div><div><dt>在线工具版本</dt><dd>{versions?.tool || "尚未检查"}</dd></div><div><dt>最新固件版本</dt><dd>{versions?.firmware || "尚未检查"}</dd></div></dl></Card>
    <Card title="工具信息"><dl className="info-list"><div><dt>产品配置</dt><dd>{bootstrap.toolName}</dd></div><div><dt>支持分辨率</dt><dd>{bootstrap.resolutions.join(" / ")}</dd></div><div><dt>{bootstrap.links.guide.name || "使用教程"}</dt><dd><ExternalValue value={bootstrap.links.guide} /></dd></div><div><dt>{bootstrap.links.updates.name || "更新日志"}</dt><dd><ExternalValue value={bootstrap.links.updates} /></dd></div><div><dt>联系信息</dt><dd>{bootstrap.qq.filter(Boolean).length ? <span className="contact-list">{bootstrap.qq.filter(Boolean).map((item) => <span key={item}>{item}</span>)}</span> : "未配置"}</dd></div></dl></Card>
  </div>;
}

function ExternalValue({ value }: { value: InfoLink }): ReactElement {
  const target = value.url.trim();
  const url = extractExternalUrl(target);
  if (url) return <a className="external-link" href={url} target="_blank" rel="noreferrer"><span>{target}</span><ExternalLink size={13} /></a>;
  return <span>{target || "未配置"}</span>;
}

function ColorField({ label, value, onChange }: { label: string; value: string; onChange: (value: string) => void }): ReactElement {
  return <label className="color-field"><span>{label}</span><div className="color-input"><input className="color-hex" value={value} onChange={(event) => onChange(event.target.value.toUpperCase().replace(/[^0-9A-F]/g, ""))} maxLength={6} /><input className="color-picker" type="color" value={toColorInputValue(value)} title={`打开${label}调色盘`} onChange={(event) => onChange(fromColorInputValue(event.target.value))} /></div></label>;
}
function ActionButton({ icon, title, text, disabled, onClick }: { icon: ReactNode; title: string; text: string; disabled: boolean; onClick: () => void }): ReactElement { return <button className="action-button" disabled={disabled} onClick={onClick}><span>{icon}</span><strong>{title}</strong><small>{text}</small></button>; }

export default App;







