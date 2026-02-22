<#
.SYNOPSIS
    Runs qmllint in CI and filters known KDDockWidgets false positives.
.DESCRIPTION
    Captures raw qmllint output, removes warnings caused by unresolved
    "com.kdab.dockwidgets" imports, and prints a cleaner warning stream.
#>

param(
    [string]$BuildDir = "build",
    [string]$Target = "appZephyrSense_qmllint"
)

$ErrorActionPreference = "Stop"

# Keep native command failures non-terminating so this script stays warn-only.
if (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) {
    $PSNativeCommandUseErrorActionPreference = $false
}

function Test-IsKddwFalsePositive {
    param(
        [string[]]$WarningBlock
    )

    $text = $WarningBlock -join "`n"

    if ($text -match "com\.kdab\.dockwidgets") {
        return $true
    }

    if ($text -match "KDDW\.") {
        return $true
    }

    $isGraphsViewBlock = $text -match "qml[/\\]views[/\\]GraphsView\.qml"
    if (-not $isGraphsViewBlock) {
        return $false
    }

    if ($text -match 'Could not find property "uniqueName"\.') {
        return $true
    }

    if ($text -match 'Could not find property "title"\.') {
        return $true
    }

    if ($text -match "Cannot assign to non-existent default property") {
        return $true
    }

    return $false
}

if (-not (Test-Path $BuildDir)) {
    Write-Warning "Build directory '$BuildDir' does not exist. Skipping filtered qmllint."
    exit 0
}

$rawPath = Join-Path $BuildDir "qmllint_raw.txt"
$filteredPath = Join-Path $BuildDir "qmllint_filtered.txt"

Write-Host "Running qmllint target '$Target' in '$BuildDir'..." -ForegroundColor Cyan

$previousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = "Continue"
$rawOutput = & cmake --build $BuildDir --target $Target 2>&1
$cmakeExitCode = $LASTEXITCODE
$ErrorActionPreference = $previousErrorActionPreference

$rawLines = @($rawOutput | ForEach-Object { $_.ToString() })
[System.IO.File]::WriteAllLines($rawPath, $rawLines, [System.Text.UTF8Encoding]::new($false))

if (-not (Test-Path $rawPath)) {
    Write-Warning "Raw qmllint output file was not created. Skipping filter stage."
    exit 0
}

$records = New-Object System.Collections.Generic.List[object]
$currentType = "normal"
$currentLines = New-Object System.Collections.Generic.List[string]

foreach ($line in $rawLines) {
    if ($line.StartsWith("Warning:")) {
        if ($currentLines.Count -gt 0) {
            $records.Add([pscustomobject]@{
                Type = $currentType
                Lines = @($currentLines)
            })
            $currentLines = New-Object System.Collections.Generic.List[string]
        }

        $currentType = "warning"
        $currentLines.Add($line)
        continue
    }

    $currentLines.Add($line)
}

if ($currentLines.Count -gt 0) {
    $records.Add([pscustomobject]@{
        Type = $currentType
        Lines = @($currentLines)
    })
}

$filteredLines = New-Object System.Collections.Generic.List[string]
$totalWarnings = 0
$filteredWarnings = 0

foreach ($record in $records) {
    if ($record.Type -eq "warning") {
        $totalWarnings++
        if (Test-IsKddwFalsePositive -WarningBlock $record.Lines) {
            $filteredWarnings++
            continue
        }
    }

    foreach ($line in $record.Lines) {
        $filteredLines.Add($line)
    }
}

[System.IO.File]::WriteAllLines($filteredPath, @($filteredLines), [System.Text.UTF8Encoding]::new($false))

foreach ($line in $filteredLines) {
    Write-Host $line
}

$remainingWarnings = $totalWarnings - $filteredWarnings
Write-Host ""
Write-Host "qmllint warning summary:" -ForegroundColor Cyan
Write-Host "  Total warnings:     $totalWarnings"
Write-Host "  Filtered warnings:  $filteredWarnings (KDDW false positives)"
Write-Host "  Remaining warnings: $remainingWarnings"
Write-Host "  Raw output:         $rawPath"
Write-Host "  Filtered output:    $filteredPath"

if ($cmakeExitCode -ne 0) {
    Write-Warning "qmllint target exited with code $cmakeExitCode (job remains warn-only)."
}

exit 0
