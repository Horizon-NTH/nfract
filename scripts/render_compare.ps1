#!/usr/bin/env pwsh
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Show-Usage {
    @'
Usage: scripts\render_compare.ps1 [-o OUTPUT_DIR] [-n DEGREE]

Builds the Newton fractal renderer twice (ISPC + CPU), renders every color
palette with both binaries using heavier settings, and prints precise elapsed
seconds so you can compare performance. By default images are written to
.\renders (and that folder is removed afterwards); pass -o to keep them
somewhere else.
'@ | Write-Host
}

$DefaultOutputDir = 'renders'
$OutputDir = $null
$OutputDirSpecified = $false
$RenderDegree = 5

function ConvertTo-Int {
    param(
        [string]$Value,
        [string]$Name
    )

    [int]$parsed = 0
    if (-not [int]::TryParse($Value, [ref]$parsed)) {
        throw "$Name must be a positive integer (received '$Value')."
    }

    return $parsed
}

$argQueue = [System.Collections.Generic.Queue[string]]::new()
foreach ($arg in $args) { $argQueue.Enqueue($arg) }

while ($argQueue.Count -gt 0) {
    $current = $argQueue.Dequeue()
    switch ($current) {
        '-o' { 
            if ($argQueue.Count -eq 0) { Write-Error "Missing argument for $current"; exit 1 }
            $OutputDir = $argQueue.Dequeue()
            $OutputDirSpecified = $true
        }
        '--output-dir' {
            if ($argQueue.Count -eq 0) { Write-Error "Missing argument for $current"; exit 1 }
            $OutputDir = $argQueue.Dequeue()
            $OutputDirSpecified = $true
        }
        '-n' {
            if ($argQueue.Count -eq 0) { Write-Error "Missing argument for $current"; exit 1 }
            try {
                $RenderDegree = ConvertTo-Int -Value $argQueue.Dequeue() -Name 'Degree'
            } catch {
                Write-Error $_.Exception.Message
                exit 1
            }
        }
        '--degree' {
            if ($argQueue.Count -eq 0) { Write-Error "Missing argument for $current"; exit 1 }
            try {
                $RenderDegree = ConvertTo-Int -Value $argQueue.Dequeue() -Name 'Degree'
            } catch {
                Write-Error $_.Exception.Message
                exit 1
            }
        }
        '-h' { Show-Usage; exit 0 }
        '--help' { Show-Usage; exit 0 }
        default {
            Write-Error "Unknown argument: $current"
            Show-Usage
            exit 1
        }
    }
}

if (-not $OutputDir) { $OutputDir = $DefaultOutputDir }

if ($RenderDegree -lt 2 -or $RenderDegree -gt 64) {
    Write-Error "Degree must be between 2 and 64 (received '$RenderDegree')."
    exit 1
}

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$BuildIspc = Join-Path $RepoRoot 'build_ispc'
$BuildCpu = Join-Path $RepoRoot 'build_cpu'

if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}
$OutputDir = (Resolve-Path $OutputDir).Path

$ColorNames = @('classic', 'neon', 'jewelry')
$IspcTimes = @{}
$CpuTimes = @{}

$RenderWidth = 1536
$RenderHeight = 1536
$RenderMaxIter = 400
$RenderArgs = @(
    '--degree', "$RenderDegree",
    '--width', "$RenderWidth",
    '--height', "$RenderHeight",
    '--max-iter', "$RenderMaxIter"
)

function Get-ColorFlag {
    param([string]$Color)
    switch ($Color) {
        'classic' { @() }
        'neon' { @('--neon') }
        'jewelry' { @('--jewelry') }
        default { throw "Unknown color $Color" }
    }
}

function Invoke-ConfigureBuild {
    param(
        [string]$BuildDir,
        [string]$RunOnCpu,
        [string]$Label
    )

    Write-Host ">>> Configuring $Label build ($BuildDir)"
    & cmake -S $RepoRoot -B $BuildDir -DRUN_ON_CPU=$RunOnCpu -DCMAKE_BUILD_TYPE=Release
    if ($LASTEXITCODE -ne 0) { throw "cmake configure failed for $Label build." }

    & cmake --build $BuildDir --target nfract --config Release --parallel
    if ($LASTEXITCODE -ne 0) { throw "cmake build failed for $Label build." }
}

function Get-NfractBinary {
    param([string]$BuildDir)

    $candidates = @(
        Join-Path $BuildDir 'nfract.exe',
        Join-Path $BuildDir 'nfract',
        Join-Path (Join-Path $BuildDir 'Release') 'nfract.exe',
        Join-Path (Join-Path $BuildDir 'Release') 'nfract',
        Join-Path (Join-Path $BuildDir 'Debug') 'nfract.exe',
        Join-Path (Join-Path $BuildDir 'Debug') 'nfract'
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return (Resolve-Path $candidate).Path
        }
    }

    throw "Could not find nfract binary in $BuildDir"
}

function Measure-CommandSeconds {
    param([string[]]$Command)

    if ($Command.Count -eq 0) { throw "No command specified for timing." }

    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    $arguments = @()
    if ($Command.Count -gt 1) {
        $arguments = $Command[1..($Command.Count - 1)]
    }

    & $Command[0] @arguments
    $exitCode = $LASTEXITCODE
    $stopwatch.Stop()

    if ($exitCode -ne 0) {
        throw "Command failed with exit code $exitCode: $($Command -join ' ')"
    }

    return ('{0:F3}' -f $stopwatch.Elapsed.TotalSeconds)
}

$cleanup = {
    foreach ($dir in @($BuildIspc, $BuildCpu)) {
        if (Test-Path $dir) {
            Remove-Item -Path $dir -Recurse -Force -ErrorAction SilentlyContinue
        }
    }

    if (-not $OutputDirSpecified -and (Test-Path $OutputDir)) {
        Remove-Item -Path $OutputDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}

try {
    Invoke-ConfigureBuild -BuildDir $BuildIspc -RunOnCpu 'OFF' -Label 'ISPC'
    Invoke-ConfigureBuild -BuildDir $BuildCpu -RunOnCpu 'ON' -Label 'CPU'

    $IspcBin = Get-NfractBinary -BuildDir $BuildIspc
    $CpuBin = Get-NfractBinary -BuildDir $BuildCpu

    foreach ($color in $ColorNames) {
        $flag = Get-ColorFlag -Color $color

        $ispFile = Join-Path $OutputDir ("nfract_{0}_ispc.png" -f $color)
        $ispCommand = @($IspcBin) + $RenderArgs + @('--out', $ispFile) + $flag
        $ispSeconds = Measure-CommandSeconds -Command $ispCommand
        if ($OutputDirSpecified) {
            Write-Host ("Rendered {0,-7} via {1,-4} in {2,5}s -> {3}" -f $color, 'ispc', $ispSeconds, $ispFile)
        } else {
            Write-Host ("Rendered {0,-7} via {1,-4} in {2,5}s" -f $color, 'ispc', $ispSeconds)
        }
        $IspcTimes[$color] = $ispSeconds

        $cpuFile = Join-Path $OutputDir ("nfract_{0}_cpu.png" -f $color)
        $cpuCommand = @($CpuBin) + $RenderArgs + @('--out', $cpuFile) + $flag
        $cpuSeconds = Measure-CommandSeconds -Command $cpuCommand
        if ($OutputDirSpecified) {
            Write-Host ("Rendered {0,-7} via {1,-4} in {2,5}s -> {3}" -f $color, 'cpu', $cpuSeconds, $cpuFile)
        } else {
            Write-Host ("Rendered {0,-7} via {1,-4} in {2,5}s" -f $color, 'cpu', $cpuSeconds)
        }
        $CpuTimes[$color] = $cpuSeconds
    }

    Write-Host ''
    Write-Host ("{0,-8} {1,-6} {2}" -f 'Color', 'Mode', 'Seconds')
    Write-Host ("{0,-8} {1,-6} {2}" -f '------', '------', '-------')
    foreach ($color in $ColorNames) {
        Write-Host ("{0,-8} {1,-6} {2}" -f $color, 'ispc', $IspcTimes[$color])
        Write-Host ("{0,-8} {1,-6} {2}" -f $color, 'cpu', $CpuTimes[$color])
    }

    Write-Host ''
    if ($OutputDirSpecified) {
        Write-Host "Images saved under: $OutputDir"
    } else {
        Write-Host 'Rendered images were removed during cleanup (no output directory specified).'
    }
}
finally {
    & $cleanup
}
