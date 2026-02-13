<#
.SYNOPSIS
    Collects code coverage for ZephyrSense test suites using Microsoft.CodeCoverage.Console.
.DESCRIPTION
    Discovers all tst_*.exe in build/ (or build/tests/), runs each under the coverage collector,
    merges results into Cobertura XML, and generates a JSON summary.
.PARAMETER Open
    Opens the coverage report after generation.
#>

param(
    [switch]$Open
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"
$TestsDir = Join-Path $BuildDir "tests"
$CoverageDir = Join-Path $BuildDir "coverage"
$ConfigFile = Join-Path (Join-Path $ProjectRoot "scripts") "coverage.config"
$SummaryScript = Join-Path (Join-Path $ProjectRoot "scripts") "coverage_summary.py"

# Find Microsoft.CodeCoverage.Console
$CoverageExe = $null
$VsWherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $VsWherePath) {
    $VsInstallPath = & $VsWherePath -latest -property installationPath 2>$null
    if ($VsInstallPath) {
        # Search in both known locations
        $SearchPaths = @(
            (Join-Path $VsInstallPath "Common7\IDE\Extensions\Microsoft\CodeCoverage.Console"),
            (Join-Path $VsInstallPath "Team Tools\Dynamic Code Coverage Tools")
        )
        foreach ($SearchPath in $SearchPaths) {
            if (Test-Path $SearchPath) {
                $Found = Get-ChildItem -Path $SearchPath -Filter "Microsoft.CodeCoverage.Console.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
                if ($Found) { $CoverageExe = $Found.FullName; break }
            }
        }
    }
}

# Fallback: search in common VS paths
if (-not $CoverageExe) {
    $CommonPaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\*\*\Common7\IDE\Extensions\Microsoft\CodeCoverage.Console\Microsoft.CodeCoverage.Console.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\*\*\Team Tools\Dynamic Code Coverage Tools\Microsoft.CodeCoverage.Console.exe",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\*\*\Team Tools\Dynamic Code Coverage Tools\Microsoft.CodeCoverage.Console.exe"
    )
    foreach ($Pattern in $CommonPaths) {
        $Found = Get-Item -Path $Pattern -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($Found) { $CoverageExe = $Found.FullName; break }
    }
}

if (-not $CoverageExe) {
    Write-Error "Microsoft.CodeCoverage.Console.exe not found. Ensure Visual Studio is installed with code coverage tools."
    exit 1
}

Write-Host "Using coverage tool: $CoverageExe" -ForegroundColor Cyan

# Prepare coverage output directory
if (Test-Path $CoverageDir) {
    Remove-Item -Recurse -Force $CoverageDir
}
New-Item -ItemType Directory -Path $CoverageDir -Force | Out-Null

# Discover test executables (Ninja puts them in build root, MSVC puts them in build/tests/)
$TestExes = Get-ChildItem -Path $BuildDir -Filter "tst_*.exe" -ErrorAction SilentlyContinue
if (-not $TestExes -or $TestExes.Count -eq 0) {
    $TestExes = Get-ChildItem -Path $TestsDir -Filter "tst_*.exe" -ErrorAction SilentlyContinue
}
if (-not $TestExes -or $TestExes.Count -eq 0) {
    Write-Error "No test executables found in $TestsDir. Build with -DBUILD_TESTS=ON first."
    exit 1
}

Write-Host "`nFound $($TestExes.Count) test executable(s):" -ForegroundColor Green
$TestExes | ForEach-Object { Write-Host "  - $($_.Name)" }

# Collect coverage for each test
$CoverageFiles = @()
foreach ($Exe in $TestExes) {
    $Name = $Exe.BaseName
    $OutputFile = Join-Path $CoverageDir "$Name.coverage"
    Write-Host "`nCollecting coverage for $Name..." -ForegroundColor Yellow

    & $CoverageExe collect `
        --settings $ConfigFile `
        --output $OutputFile `
        $Exe.FullName

    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Coverage collection failed for $Name (exit code: $LASTEXITCODE)"
        continue
    }

    if (Test-Path $OutputFile) {
        $CoverageFiles += $OutputFile
        Write-Host "  -> $OutputFile" -ForegroundColor DarkGreen
    }
}

if ($CoverageFiles.Count -eq 0) {
    Write-Error "No coverage files were generated."
    exit 1
}

# Merge into Cobertura XML
$MergedOutput = Join-Path $CoverageDir "coverage.cobertura.xml"
Write-Host "`nMerging $($CoverageFiles.Count) coverage file(s)..." -ForegroundColor Yellow

$MergeArgs = @("merge", "-f", "cobertura", "-o", $MergedOutput) + $CoverageFiles
& $CoverageExe @MergeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Warning "Coverage merge returned exit code: $LASTEXITCODE"
}

if (Test-Path $MergedOutput) {
    Write-Host "Merged coverage report: $MergedOutput" -ForegroundColor Green
} else {
    Write-Error "Failed to generate merged coverage report."
    exit 1
}

# Generate JSON summary
if (Test-Path $SummaryScript) {
    Write-Host "`nGenerating coverage summary..." -ForegroundColor Yellow
    python $SummaryScript $MergedOutput (Join-Path $CoverageDir "coverage-summary.json")
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Summary: $(Join-Path $CoverageDir 'coverage-summary.json')" -ForegroundColor Green
    }
} else {
    Write-Warning "Summary script not found at $SummaryScript"
}

# Open report if requested
if ($Open -and (Test-Path $MergedOutput)) {
    Write-Host "`nOpening coverage report..." -ForegroundColor Cyan
    Start-Process $MergedOutput
}

Write-Host "`nDone." -ForegroundColor Green
