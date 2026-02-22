<#
.SYNOPSIS
    Collects QML line coverage for ZephyrSense QML test suite using qoverage.
.DESCRIPTION
    Instruments QML files with qoverage tracking code, runs QML tests,
    collects coverage data from stdout, and generates Cobertura XML report.
.PARAMETER Open
    Opens the coverage XML in the default browser after generation.
#>

param(
    [switch]$Open
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"
$CoverageDir = Join-Path $BuildDir "coverage"
$QmlDir = Join-Path $ProjectRoot "qml"
$VenvPython = Join-Path $ProjectRoot ".venv\qoverage\Scripts\python.exe"
$QmlDom = "C:\Qt\6.10.1\msvc2022_64\bin\qmldom.exe"
$TestExe = Join-Path $BuildDir "tst_qml.exe"

# Verify prerequisites
if (-not (Test-Path $VenvPython)) {
    Write-Error "qoverage venv not found at $VenvPython. Set up with: python -m venv .venv\qoverage && .venv\qoverage\Scripts\pip install qoverage"
    exit 1
}

if (-not (Test-Path $TestExe)) {
    Write-Error "tst_qml.exe not found. Build with -DBUILD_TESTS=ON first."
    exit 1
}

if (-not (Test-Path $QmlDom)) {
    Write-Error "qmldom not found at $QmlDom"
    exit 1
}

# Ensure coverage output directory exists
if (-not (Test-Path $CoverageDir)) {
    New-Item -ItemType Directory -Path $CoverageDir -Force | Out-Null
}

# Step 1: Instrument QML files in-place (creates .qoverage.bkp backups)
Write-Host "`nInstrumenting QML files..." -ForegroundColor Yellow
& $VenvPython -m qoverage instrument `
    --in-place `
    --path $QmlDir `
    --qmldom $QmlDom
if ($LASTEXITCODE -ne 0) {
    Write-Warning "qoverage instrument exited with code $LASTEXITCODE (some files may have been skipped)"
}

# Step 2: Get qoverage QML import path
$ImportPath = & $VenvPython -m qoverage --importpath
Write-Host "qoverage import path: $ImportPath" -ForegroundColor Cyan

# Step 3: Run QML tests with qoverage import path, capture output
Write-Host "`nRunning QML tests with coverage instrumentation..." -ForegroundColor Yellow
$RunLog = Join-Path $CoverageDir "qml-run.log"
$QmlProfraw = Join-Path $CoverageDir "tst_qml.profraw"

$env:QML_IMPORT_PATH = $ImportPath
$env:LLVM_PROFILE_FILE = $QmlProfraw
$env:QT_QPA_PLATFORM = "offscreen"
$env:QT_LOGGING_RULES = "*.debug=true; qt.*.debug=false"
$env:QT_ASSUME_STDERR_HAS_CONSOLE = "1"

# Capture test output then write as UTF-8 (Tee-Object writes UTF-16 on PS 5.1,
# which breaks qoverage collect's plain-text parsing)
$output = & $TestExe 2>&1
$output | ForEach-Object { $_.ToString() }
$output | ForEach-Object { $_.ToString() } | Out-File -FilePath $RunLog -Encoding utf8

Remove-Item Env:QML_IMPORT_PATH -ErrorAction SilentlyContinue
Remove-Item Env:LLVM_PROFILE_FILE -ErrorAction SilentlyContinue
Remove-Item Env:QT_LOGGING_RULES -ErrorAction SilentlyContinue
Remove-Item Env:QT_ASSUME_STDERR_HAS_CONSOLE -ErrorAction SilentlyContinue

# Step 4: Collect coverage into Cobertura XML (before restore, which deletes .qoverage.js files)
Write-Host "`nCollecting coverage data..." -ForegroundColor Yellow
$CoverageXml = Join-Path $CoverageDir "qml-coverage.xml"

& $VenvPython -m qoverage collect `
    --files-path $QmlDir `
    --input $RunLog `
    --report $CoverageXml

if ($LASTEXITCODE -ne 0) {
    Write-Warning "qoverage collect exited with code $LASTEXITCODE"
}

# Step 5: Restore original QML files from backups
Write-Host "`nRestoring original QML files..." -ForegroundColor Yellow
& $VenvPython -m qoverage restore --path $QmlDir
if ($LASTEXITCODE -ne 0) {
    Write-Error "qoverage restore failed! Check .qoverage.bkp files in $QmlDir"
    exit 1
}

if (Test-Path $CoverageXml) {
    Write-Host "`nQML coverage report: $CoverageXml" -ForegroundColor Green
    if ($Open) {
        Start-Process $CoverageXml
    }
} else {
    Write-Warning "No coverage report generated. Check $RunLog for details."
}

Write-Host "`nDone." -ForegroundColor Green
