<#
    Keil 编译脚本 —— 由 VS Code 的 .vscode\tasks.json 调用，不依赖任何插件。
    手动运行方式：
        powershell -NoProfile -ExecutionPolicy Bypass -File .vscode\keil-build.ps1 -Target build

    -Target  build   增量编译（等价于 Keil 里按 F7）
             rebuild 全部重新编译（等价于 Keil 里 Rebuild）
             flash   把固件下载到芯片（用 Keil 里配置好的 ST-Link）
#>
[CmdletBinding()]
param(
    [ValidateSet('build', 'rebuild', 'flash')]
    [string]$Target = 'build'
)

$ErrorActionPreference = 'Stop'

$vscodeDir   = $PSScriptRoot
$projectDir  = Split-Path -Parent $vscodeDir
$projectFile = Join-Path $projectDir 'Project.uvprojx'
$uv4Path     = 'E:\1Ruanjian\ARM_keil\UV4\UV4.exe'
$logFile     = Join-Path $vscodeDir 'uv4.log'

function Stop-WithError([string]$Message, [int]$Code = 3) {
    Write-Host $Message -ForegroundColor Red
    exit $Code
}

if (-not (Test-Path -LiteralPath $uv4Path)) {
    Stop-WithError ("找不到 Keil 的 UV4.exe：`n  {0}`n请打开 .vscode\keil-build.ps1 把 `$uv4Path 改成正确路径。" -f $uv4Path)
}
if (-not (Test-Path -LiteralPath $projectFile)) {
    Stop-WithError ("找不到工程文件：`n  {0}" -f $projectFile)
}

# 刚点掉 Keil 时它可能还在退出，先等一会儿再动手
for ($i = 0; $i -lt 30; $i++) {
    if (-not (Get-Process -Name 'UV4' -ErrorAction SilentlyContinue)) { break }
    if ($i -eq 0) { Write-Host '检测到 Keil 还在运行，等待它退出…' -ForegroundColor Yellow }
    Start-Sleep -Milliseconds 500
}
if (Get-Process -Name 'UV4' -ErrorAction SilentlyContinue) {
    Write-Host '提示：Keil 好像还开着。Keil 不允许同时批处理编译同一个工程，如果下面失败，请先关掉 Keil 窗口。' -ForegroundColor Yellow
}

switch ($Target) {
    'build'   { $flag = '-b'; $title = '增量编译' }
    'rebuild' { $flag = '-r'; $title = '全部重新编译' }
    'flash'   { $flag = '-f'; $title = '下载到芯片' }
}

Write-Host ("===== Keil {0} : {1} =====" -f $title, (Split-Path -Leaf $projectFile)) -ForegroundColor Cyan

foreach ($f in @($logFile, ($logFile + '.lock'))) {
    if (Test-Path -LiteralPath $f) {
        Remove-Item -LiteralPath $f -Force -ErrorAction SilentlyContinue
    }
}

$sw = [System.Diagnostics.Stopwatch]::StartNew()
$proc = Start-Process -FilePath $uv4Path `
    -ArgumentList @($flag, "`"$projectFile`"", '-j0', '-o', "`"$logFile`"") `
    -WorkingDirectory $projectDir `
    -Wait -PassThru -NoNewWindow
$sw.Stop()

# UV4 退出后稍等日志落盘，避免读到不完整内容
for ($i = 0; $i -lt 20 -and -not (Test-Path -LiteralPath $logFile); $i++) {
    Start-Sleep -Milliseconds 100
}

if (Test-Path -LiteralPath $logFile) {
    Get-Content -LiteralPath $logFile -Encoding Default | ForEach-Object { Write-Host $_ }
}
else {
    Write-Host '(没有生成编译日志，UV4 可能没有真正启动)' -ForegroundColor Yellow
}

Write-Host ''
Write-Host ("耗时 {0:N1} 秒" -f $sw.Elapsed.TotalSeconds) -ForegroundColor DarkGray

$code = $proc.ExitCode
switch ($code) {
    0       { Write-Host '成功：0 个错误，0 个警告'  -ForegroundColor Green }
    1       { Write-Host '完成：有警告，没有错误'    -ForegroundColor Yellow }
    2       { Write-Host '失败：有错误，请看上面的列表（按 F8 逐个跳转）' -ForegroundColor Red }
    3       { Write-Host '失败：致命错误（常见原因：许可证无效 / 编译器缺失）' -ForegroundColor Red }
    11      { Write-Host '失败：无法打开工程文件（路径不对，或工程正被 Keil 占用）' -ForegroundColor Red }
    12      { Write-Host '失败：设备无法访问（没插调试器 / 驱动或供电问题）' -ForegroundColor Red }
    15      { Write-Host '失败：调试器报错（检查 ST-Link 连接与接线）' -ForegroundColor Red }
    default { Write-Host ("失败：UV4 返回码 {0}" -f $code) -ForegroundColor Red }
}

if ($Target -ne 'flash' -and $code -le 1) {
    $axf = Join-Path $projectDir 'Objects\Project.axf'
    $hex = Join-Path $projectDir 'Objects\Project.hex'
    if (Test-Path -LiteralPath $axf) { Write-Host ("固件：{0}" -f $axf) -ForegroundColor DarkGray }
    if (Test-Path -LiteralPath $hex) { Write-Host ("十六进制：{0}" -f $hex) -ForegroundColor DarkGray }
}

exit $code
