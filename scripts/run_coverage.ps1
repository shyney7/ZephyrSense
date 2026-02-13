<#
.SYNOPSIS
    Collects code coverage for ZephyrSense test suites using LLVM source-based coverage.
.DESCRIPTION
    Discovers all tst_*.exe in build/, runs each with profiling enabled,
    merges profiles with llvm-profdata, and generates reports with llvm-cov.
.PARAMETER Open
    Opens an HTML coverage report after generation.
#>

param(
    [switch]$Open
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"
$CoverageDir = Join-Path $BuildDir "coverage"
$SrcDir = Join-Path $ProjectRoot "src"
$SummaryScript = Join-Path (Join-Path $ProjectRoot "scripts") "coverage_summary.py"

# Verify LLVM tools are available
foreach ($Tool in @("llvm-profdata", "llvm-cov")) {
    if (-not (Get-Command $Tool -ErrorAction SilentlyContinue)) {
        Write-Error "$Tool not found in PATH. Install LLVM 18+ (21+ recommended for MC/DC)."
        exit 1
    }
}

Write-Host "Using: $(Get-Command llvm-cov | Select-Object -ExpandProperty Source)" -ForegroundColor Cyan

# Prepare coverage output directory
if (Test-Path $CoverageDir) {
    Remove-Item -Recurse -Force $CoverageDir
}
New-Item -ItemType Directory -Path $CoverageDir -Force | Out-Null

# Discover test executables (Ninja puts them in build root, multi-config generators use subdirs)
$TestExes = Get-ChildItem -Path $BuildDir -Filter "tst_*.exe" -Recurse -ErrorAction SilentlyContinue
if (-not $TestExes -or $TestExes.Count -eq 0) {
    Write-Error "No test executables found in $BuildDir. Build with -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON first."
    exit 1
}

Write-Host "`nFound $($TestExes.Count) test executable(s):" -ForegroundColor Green
$TestExes | ForEach-Object { Write-Host "  - $($_.Name)" }

# Run each test exe with its own profile output
foreach ($Exe in $TestExes) {
    $Name = $Exe.BaseName
    $env:LLVM_PROFILE_FILE = Join-Path $CoverageDir "$Name.profraw"
    Write-Host "`nRunning $Name..." -ForegroundColor Yellow

    & $Exe.FullName
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "$Name exited with code $LASTEXITCODE"
    }

    Remove-Item Env:LLVM_PROFILE_FILE
}

# Merge all .profraw files into a single .profdata
$ProfrawFiles = Get-ChildItem -Path $CoverageDir -Filter "*.profraw" -ErrorAction SilentlyContinue
if (-not $ProfrawFiles -or $ProfrawFiles.Count -eq 0) {
    Write-Error "No .profraw files generated. Were test exes built with -DENABLE_COVERAGE=ON?"
    exit 1
}

$MergedProfile = Join-Path $CoverageDir "merged.profdata"
Write-Host "`nMerging $($ProfrawFiles.Count) profile(s)..." -ForegroundColor Yellow

& llvm-profdata merge -sparse ($ProfrawFiles | ForEach-Object { $_.FullName }) -o $MergedProfile
if ($LASTEXITCODE -ne 0) {
    Write-Error "llvm-profdata merge failed."
    exit 1
}

# Build llvm-cov object list: first exe is positional, rest use -object=
$FirstExe = $TestExes[0].FullName
$ObjectArgs = @()
if ($TestExes.Count -gt 1) {
    $ObjectArgs = $TestExes[1..($TestExes.Count - 1)] | ForEach-Object { "-object=$($_.FullName)" }
}

$CommonArgs = @($FirstExe) + $ObjectArgs + @(
    "-instr-profile=$MergedProfile",
    "-ignore-filename-regex=(libs|build|tests|moc_|qrc_)/",
    "-sources", $SrcDir
)

# Export JSON for summary script
$ExportJson = Join-Path $CoverageDir "coverage.json"
Write-Host "`nExporting coverage data..." -ForegroundColor Yellow

$ExportArgs = $CommonArgs + @("-format=text")
$JsonOutput = & llvm-cov export @ExportArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "llvm-cov export failed."
    exit 1
}
[System.IO.File]::WriteAllText($ExportJson, ($JsonOutput -join "`n"), [System.Text.UTF8Encoding]::new($false))

# Generate JSON summary
if (Test-Path $SummaryScript) {
    $SummaryJson = Join-Path $CoverageDir "coverage-summary.json"
    python $SummaryScript $ExportJson $SummaryJson
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Summary: $SummaryJson" -ForegroundColor Green
    }
} else {
    Write-Warning "Summary script not found at $SummaryScript"
}

# Terminal report
Write-Host "`n" -NoNewline
& llvm-cov report @CommonArgs

# HTML report if -Open requested
if ($Open) {
    $HtmlDir = Join-Path $CoverageDir "html"
    Write-Host "`nGenerating HTML report..." -ForegroundColor Yellow

    $ShowArgs = $CommonArgs + @("-format=html", "-output-dir=$HtmlDir")
    & llvm-cov show @ShowArgs

    if ($LASTEXITCODE -eq 0) {
        $IndexHtml = Join-Path $HtmlDir "index.html"
        if (Test-Path $IndexHtml) {
            Write-Host "Opening $IndexHtml" -ForegroundColor Cyan
            Start-Process $IndexHtml
        }
    }
}

Write-Host "`nDone. Coverage output: $CoverageDir" -ForegroundColor Green
