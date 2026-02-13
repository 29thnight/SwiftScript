import * as vscode from "vscode";
import * as path from "path";
import * as fs from "fs";


import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  TransportKind,
} from "vscode-languageclient/node";

/* =========================================================
   Types
========================================================= */

type SSProjectInfo = {
  projectFile: string;   // absolute path to .ssproject
  projectDir: string;    // folder containing .ssproject
  entryFile: string;     // absolute path resolved from <Entry>
  importRoots: string[]; // absolute paths resolved from <ImportRoots><Root>
};

/* =========================================================
   FS helpers
========================================================= */

function existsFile(p: string): boolean {
  try { return fs.statSync(p).isFile(); } catch { return false; }
}
function existsDir(p: string): boolean {
  try { return fs.statSync(p).isDirectory(); } catch { return false; }
}
function readAllText(p: string): string {
  return fs.readFileSync(p, "utf8");
}

function ensureDir(p: string): void {
  if (!existsDir(p)) fs.mkdirSync(p, { recursive: true });
}

function writeAllText(p: string, content: string): void {
  ensureDir(path.dirname(p));
  fs.writeFileSync(p, content, { encoding: "utf8" });
}

/* =========================================================
   Naive XML parsing (C++ style)
========================================================= */

function extractTag(xml: string, tag: string): string | null {
  const open = `<${tag}>`;
  const close = `</${tag}>`;
  const a = xml.indexOf(open);
  if (a < 0) return null;
  const start = a + open.length;
  const b = xml.indexOf(close, start);
  if (b < 0) return null;
  return xml.substring(start, b).trim();
}

function extractRepeatedTags(xml: string, tag: string): string[] {
  const open = `<${tag}>`;
  const close = `</${tag}>`;
  const out: string[] = [];
  let pos = 0;

  for (;;) {
    const a = xml.indexOf(open, pos);
    if (a < 0) break;
    const start = a + open.length;
    const b = xml.indexOf(close, start);
    if (b < 0) break;
    out.push(xml.substring(start, b).trim());
    pos = b + close.length;
  }
  return out;
}

function loadSSProjectInfo(ssprojectAbs: string): SSProjectInfo {
  const xml = readAllText(ssprojectAbs);
  const projectDir = path.dirname(ssprojectAbs);

  const entryRel = extractTag(xml, "Entry");
  if (!entryRel) throw new Error(`missing <Entry>...</Entry> in: ${ssprojectAbs}`);

  const entryFile = path.normalize(path.join(projectDir, entryRel));

  const rootsBlock = extractTag(xml, "ImportRoots");
  let importRoots: string[] = [];

  if (rootsBlock) {
    const roots = extractRepeatedTags(rootsBlock, "Root");
    importRoots = roots.map(r => path.normalize(path.join(projectDir, r)));
  } else {
    importRoots = [projectDir];
  }

  return { projectFile: ssprojectAbs, projectDir, entryFile, importRoots };
}

/* =========================================================
   .ssproject discovery
========================================================= */

function findNearestSSProject(startFileOrDirAbs: string): string | null {
  let dir = existsDir(startFileOrDirAbs) ? startFileOrDirAbs : path.dirname(startFileOrDirAbs);

  for (;;) {
    try {
      const items = fs.readdirSync(dir);
      for (const it of items) {
        if (it.toLowerCase().endsWith(".ssproject")) return path.join(dir, it);
      }
    } catch {}

    const parent = path.dirname(dir);
    if (parent === dir) break;
    dir = parent;
  }
  return null;
}

async function findFirstSSProjectInWorkspace(): Promise<string | null> {
  const uris = await vscode.workspace.findFiles("**/*.ssproject", "**/node_modules/**", 1);
  return uris.length > 0 ? uris[0].fsPath : null;
}

async function getProjectPath(): Promise<string> {
  const editor = vscode.window.activeTextEditor;
  const activePath = editor?.document?.uri?.fsPath;

  let ssproject: string | null = null;
  if (activePath) ssproject = findNearestSSProject(activePath);
  if (!ssproject) ssproject = await findFirstSSProjectInWorkspace();
  if (!ssproject) throw new Error("No .ssproject found in workspace.");
  return ssproject;
}

/* =========================================================
   Tool path resolution
========================================================= */

function resolveToolPath(
  toolPathSetting: string,
  workspaceFolder: vscode.WorkspaceFolder | undefined,
  bundledFallbackAbs: string
): string {
  const trimmed = (toolPathSetting ?? "").trim();

  // empty => bundled
  if (!trimmed) return bundledFallbackAbs;

  // absolute => 그대로
  if (path.isAbsolute(trimmed)) return trimmed;

  // workspace-relative
  if (workspaceFolder) return path.join(workspaceFolder.uri.fsPath, trimmed);

  return trimmed;
}

/* =========================================================
   Output convention: <projectDir>/bin/<BuildType>/<projectName>.ssasm
========================================================= */

function computeAsmPath(projectFileAbs: string, buildType: string): string {
  const projectDir = path.dirname(projectFileAbs);
  const projectName = path.parse(projectFileAbs).name;
  return path.join(projectDir, "bin", buildType, `${projectName}.ssasm`);
}

/* =========================================================
   Terminal runner
========================================================= */

let term: vscode.Terminal | null = null;
let client: LanguageClient | null = null;
let busy = false;

function ensureTerminal(cwd: string): vscode.Terminal {
  if (term) return term;

  term = vscode.window.createTerminal({
    name: "swive",
    cwd,
  });

  // 사용자가 터미널을 수동으로 닫으면 참조 제거
  vscode.window.onDidCloseTerminal((t) => {
    if (term && t === term) term = null;
  });

  return term;
}

function quoteArgWindows(s: string): string {
  // PowerShell/cmd에서 안전하게 쓰려면 큰따옴표로 감싸고 내부 "를 \"로
  const escaped = s.replace(/"/g, '\\"');
  return `"${escaped}"`;
}

/* =========================================================
   LSP startup
========================================================= */

function startLanguageServer(context: vscode.ExtensionContext): void {
  const outputChannel = vscode.window.createOutputChannel("swive LSP Debug");

  const cfg = vscode.workspace.getConfiguration("swive");

  const serverPathSetting = cfg.get<string>("serverPath", "");
  const wsFolder = vscode.workspace.workspaceFolders?.[0];

  // ✅ server/ 안에 있다고 했으니 여기로 고정
  const bundledServer = context.asAbsolutePath(path.join("server", "SwiveServer.exe"));
  const serverExe = resolveToolPath(serverPathSetting, wsFolder, bundledServer);

  outputChannel.appendLine(`[LSP] extensionPath: ${context.extensionPath}`);
  outputChannel.appendLine(`[LSP] bundledServer: ${bundledServer}`);
  outputChannel.appendLine(`[LSP] serverExe: ${serverExe}`);
  outputChannel.appendLine(`[LSP] existsFile: ${existsFile(serverExe)}`);
  outputChannel.show(true);

  if (!existsFile(serverExe)) {
    vscode.window.showWarningMessage(
      `swive LSP server not found:\n${serverExe}`
    );
    return;
  }

  outputChannel.appendLine(`[LSP] Starting server...`);

  const serverOptions: ServerOptions = {
    command: serverExe,
    args: [],
    transport: TransportKind.stdio,
    options: { cwd: path.dirname(serverExe) },
  };

  const clientOptions: LanguageClientOptions = {
    documentSelector: [
      { scheme: "file", language: "swive" },
      { scheme: "file", language: "swive-project" },
    ],
    initializationOptions: {},
  };

  client = new LanguageClient(
    "swiveLsp",
    "swive Language Server",
    serverOptions,
    clientOptions
  );

  client.start().then(() => {
    outputChannel.appendLine(`[LSP] Server started successfully!`);
  }).catch((err: Error) => {
    outputChannel.appendLine(`[LSP] Server start FAILED: ${err.message}`);
  });
}

/* =========================================================
   Command: compile & run (IN TERMINAL)
========================================================= */

async function compileAndRunProject(context: vscode.ExtensionContext): Promise<void> {
  if (busy) return;
  busy = true;

  try {
    const ssproject = await getProjectPath();
    const wsFolder = vscode.workspace.getWorkspaceFolder(vscode.Uri.file(ssproject));

    const cfg = vscode.workspace.getConfiguration("swive");
    const buildType = cfg.get<string>("buildType", "Debug");

    const swivePathSetting = cfg.get<string>("swivePath", "");

    // ✅ 기본은 extension/server/swive.exe (Unified CLI)
    const bundledSwive = context.asAbsolutePath(path.join("server", "swive.exe"));
    const swiveExe = resolveToolPath(swivePathSetting, wsFolder, bundledSwive);

    if (!existsFile(swiveExe)) {
      throw new Error(
        `Swive CLI not found:\n${swiveExe}\n` +
        `Fix: set "swive.swivePath" to absolute path, or bundle it at extension/server/.`
      );
    }

    // 터미널은 exe가 있는 폴더에서 실행
    const terminal = ensureTerminal(path.dirname(swiveExe));
    terminal.show(true);

    // swive exec <project.ssproject> -c <Debug|Release>
    const execCmd =
      `& ${quoteArgWindows(swiveExe)} exec ${quoteArgWindows(ssproject)} -c ${buildType}`;
    terminal.sendText(execCmd);

  } finally {
    busy = false;
  }
}


/* =========================================================
   Command: add project (template)
========================================================= */

function resolveTemplateZipPath(context: vscode.ExtensionContext): string {
  const candidates: string[] = [
    context.asAbsolutePath("ScriptProjTest.zip"),
    context.asAbsolutePath(path.join("templates", "ScriptProjTest.zip")),
    context.asAbsolutePath(path.join("resources", "ScriptProjTest.zip")),
    context.asAbsolutePath(path.join("server", "ScriptProjTest.zip")),
  ];

  for (const c of candidates) {
    if (existsFile(c)) return c;
  }

  throw new Error(
    "Template zip not found. Put ScriptProjTest.zip into extension root (or templates/resources/server)."
  );
}

function buildSSProjectXml(projectName: string): string {
  // Extra tags are allowed; loader only requires <Entry> and optionally <ImportRoots>.
  return (
    "<Project>\n" +
    `    <Name>${projectName}</Name>\n` +
    "    <Entry>Scripts/main.ss</Entry>\n" +
    "    <ImportRoots>\n" +
    "        <Root>Libs</Root>\n" +
    "        <Root>Scripts</Root>\n" +
    "    </ImportRoots>\n" +
    "</Project>\n"
  );
}

function safeJoin(rootAbs: string, relPath: string): string {
  // Prevent ZipSlip: ensure resolved path stays within rootAbs.
  const resolved = path.resolve(rootAbs, relPath);
  const rootResolved = path.resolve(rootAbs);
  if (!resolved.startsWith(rootResolved + path.sep) && resolved !== rootResolved) {
    throw new Error(`Unsafe path in zip entry: ${relPath}`);
  }
  return resolved;
}

async function addProject(context: vscode.ExtensionContext) {
  // 1. 프로젝트 이름 입력
  const projectName = await vscode.window.showInputBox({
    title: "Create Swive Project",
    prompt: "Enter project name",
    validateInput: (v) =>
      !v || !v.trim() ? "Project name is required." : null,
  });

  if (!projectName) return;

  // 2. 부모 폴더 GUI 선택 (항상 표시)
  const picked = await vscode.window.showOpenDialog({
    canSelectFiles: false,
    canSelectFolders: true,
    canSelectMany: false,
    openLabel: "Create project here",
    title: "Select parent folder for the project",
  });

  if (!picked || picked.length === 0) return;

  const parentDir = picked[0].fsPath;
  const projectRoot = path.join(parentDir, projectName);

  // 3. 프로젝트 루트 및 기본 구조 생성
  await fs.promises.mkdir(path.join(projectRoot, "bin", "Debug"), {
    recursive: true,
  });
  await fs.promises.mkdir(path.join(projectRoot, "Libs"), {
    recursive: true,
  });
  await fs.promises.mkdir(path.join(projectRoot, "Scripts"), {
    recursive: true,
  });

  // 4. .ssproject 생성 (프로젝트 이름 기반)
  const ssprojectPath = path.join(
    projectRoot,
    `${projectName}.ssproject`
  );

  const ssprojectContent =
    `<Project>\n` +
    `  <Name>${escapeXml(projectName)}</Name>\n` +
    `  <Entry>Scripts/main.ss</Entry>\n` +
    `  <ImportRoots>\n` +
    `    <ImportRoot>Libs</ImportRoot>\n` +
    `  </ImportRoots>\n` +
    `</Project>\n`;

  await fs.promises.writeFile(
    ssprojectPath,
    ssprojectContent,
    "utf8"
  );

  // 5. main.ss 생성
  const mainScriptPath = path.join(
    projectRoot,
    "Scripts",
    "main.ss"
  );

  await fs.promises.writeFile(
    mainScriptPath,
    `print("hello world")\n`,
    "utf8"
  );

  vscode.window.showInformationMessage(
    `swive project created: ${projectRoot}`
  );
}

function escapeXml(s: string): string {
  return s
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&apos;");
}

/* =========================================================
   Command: stop
========================================================= */

function stop(): void {
  if (!term) {
    vscode.window.showInformationMessage("swive: terminal not running.");
    return;
  }

  // VSCode API로 “프로세스만” 정확히 kill은 불가.
  // 현실적인 stop:
  // 1) SIGINT (Ctrl+C) 시도
  // 2) 안 되면 터미널 dispose(강제 닫기)
  try {
    // vscode 1.85+에 sendText로 ^C는 보통 안 먹힘. 그래도 시도는 가능.
    term.sendText("\x03");
  } catch {}

  // fallback: 그냥 터미널 닫기
  try {
    term.dispose();
  } catch {}

  term = null;
}

/* =========================================================
   activate / deactivate
========================================================= */


/* =========================================================
   Debug Adapter
========================================================= */

class SwiveDebugAdapterFactory
  implements vscode.DebugAdapterDescriptorFactory
{
  constructor(private context: vscode.ExtensionContext) {}

  async createDebugAdapterDescriptor(
    _session: vscode.DebugSession
  ): Promise<vscode.DebugAdapterDescriptor> {
    const cfg = vscode.workspace.getConfiguration("swive");
    let adapterPath = cfg.get<string>("debugAdapterPath", "");
    if (!adapterPath) {
      adapterPath = this.context.asAbsolutePath(
        path.join("server", "SwiveDebugAdapter.exe")
      );
    }

    // Launch adapter in integrated terminal with TCP mode
    // so stdin/stdout are free for VM I/O (print/readLine)
    return new Promise<vscode.DebugAdapterDescriptor>((resolve, reject) => {
      const net = require("net") as typeof import("net");
      const cp = require("child_process") as typeof import("child_process");

      // Start adapter with --dap-port 0 (auto-assign port)
      const child = cp.spawn(adapterPath, ["--dap-port", "0"], {
        stdio: ["pipe", "pipe", "pipe"],
      });

      let portResolved = false;

      // The adapter prints the port number on its first stdout line
      child.stdout!.once("data", (data: Buffer) => {
        const portStr = data.toString().trim();
        const port = parseInt(portStr, 10);
        if (isNaN(port) || port <= 0) {
          reject(new Error("Invalid DAP port: " + portStr));
          return;
        }
        portResolved = true;

        // Now create a terminal for VM I/O and pipe stdin/stdout
        const writeEmitter = new vscode.EventEmitter<string>();
        const pty: vscode.Pseudoterminal = {
          onDidWrite: writeEmitter.event,
          open: () => {},
          close: () => {
            child.kill();
          },
          handleInput: (input: string) => {
            // Forward terminal input to adapter's stdin (for readLine)
            child.stdin!.write(input);
          },
        };

        const terminal = vscode.window.createTerminal({
          name: "Swive Debug",
          pty,
        });
        terminal.show();

        // Forward adapter's stdout to terminal (VM print output)
        child.stdout!.on("data", (chunk: Buffer) => {
          writeEmitter.fire(chunk.toString().replace(/\n/g, "\r\n"));
        });
        child.stderr!.on("data", (chunk: Buffer) => {
          writeEmitter.fire(chunk.toString().replace(/\n/g, "\r\n"));
        });
        child.on("exit", () => {
          writeEmitter.fire("\r\n[Debug session ended]\r\n");
        });

        resolve(new vscode.DebugAdapterServer(port));
      });

      child.on("error", (err: Error) => {
        if (!portResolved) reject(err);
      });
      child.on("exit", (code: number) => {
        if (!portResolved) reject(new Error("Adapter exited with code " + code));
      });
    });
  }
}

export function activate(context: vscode.ExtensionContext) {
  startLanguageServer(context);

  context.subscriptions.push(
    vscode.commands.registerCommand("swive.compileAndRunProject", async () => {
      try {
        await compileAndRunProject(context);
      } catch (e: any) {
        vscode.window.showErrorMessage(String(e?.message ?? e));
      }
    })
  );

  context.subscriptions.push(
    vscode.commands.registerCommand("swive.addProject", async () => {
      try {
        await addProject(context);
      } catch (e: any) {
        vscode.window.showErrorMessage(String(e?.message ?? e));
      }
    })
  );

  context.subscriptions.push(
    vscode.commands.registerCommand("swive.stop", () => {
      stop();
    })
  );

  // Register DAP debug adapter
  context.subscriptions.push(
    vscode.debug.registerDebugAdapterDescriptorFactory(
      "swive",
      new SwiveDebugAdapterFactory(context)
    )
  );
}


export async function deactivate(): Promise<void> {
  try {
    if (term) {
      try { term.dispose(); } catch {}
      term = null;
    }
  } finally {
    if (client) {
      await client.stop();
      client = null;
    }
  }
}
