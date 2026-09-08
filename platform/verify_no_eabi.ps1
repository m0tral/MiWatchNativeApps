# verify_no_eabi.ps1 -- fail the build if any __aeabi_* helper leaked in.
#
# After the link step runs, every .o and .elf in the workspace folder is
# scanned with `arm-none-eabi-nm -u`, which prints undefined symbols (i.e.
# imports). The bare-metal runtime does not ship __aeabi_* helpers, so any
# such import here means the C source has a float/64-bit operation that
# the linker would otherwise have to resolve against libgcc -- which is
# not linked. Exit code: 0 clean, 1 if any import found, 2 if nm missing.
#
# Invoke from VSCode / Zed / .vscode/tasks.json as a `powershell -File`
# shell task right after the `link` step and before `deploy`. Works for
# both mb10pro and watchs3 build pipelines (the scanned artifacts are
# model-agnostic -- only the address bindings differ).

$ErrorActionPreference = 'Continue'

if (-not (Get-Command arm-none-eabi-nm -ErrorAction SilentlyContinue)) {
    Write-Host 'verify: arm-none-eabi-nm not in PATH -- skip'
    exit 0
}

$root   = if ($env:WORKSPACE_FOLDER) { $env:WORKSPACE_FOLDER } else { (Get-Location).Path }
$artDir = Join-Path $root 'build'
if (-not (Test-Path -LiteralPath $artDir)) {
    $artDir = $root
}

$artifacts = @(Get-ChildItem -Path (Join-Path $artDir '*.o')   -ErrorAction SilentlyContinue) +
             @(Get-ChildItem -Path (Join-Path $artDir '*.elf') -ErrorAction SilentlyContinue)

if ($artifacts.Count -eq 0) {
    Write-Host 'verify: no .o or .elf artifacts, nothing to check'
    exit 0
}

$bad = $false
foreach ($f in $artifacts) {
    $out = & arm-none-eabi-nm -u $f.FullName 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "verify: nm failed on $($f.Name) (exit $LASTEXITCODE)"
        $bad = $true
        continue
    }
    $hits = @($out | Select-String -SimpleMatch '__aeabi')
    if ($hits.Count -gt 0) {
        Write-Host ""
        Write-Host "verify: __aeabi imports in $($f.Name):"
        foreach ($h in $hits) { Write-Host ("  " + $h.ToString().Trim()) }
        $bad = $true
    }
}

if ($bad) {
    Write-Host ""
    Write-Host 'verify: FAILED -- fix the source that triggered __aeabi_* helpers'
    exit 1
}
Write-Host 'verify: no __aeabi_* imports'
exit 0
