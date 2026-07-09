param(
    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Debug',

    [string]$Filter = '',

    [switch]$Reconfigure
)

$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot

# ── Map a test source path to its CTest name (tst_foo_bar.cpp -> foo-bar) ──
if ($Filter -and ($Filter -like '*\*' -or $Filter -like '*/*' -or $Filter -like '*.cpp'))
{
    $fileName = Split-Path -Leaf $Filter
    Write-Host "==> Detected a file path in the filter: $fileName"

    if ($fileName -like 'tst_*.cpp')
    {
        $Filter = $fileName -replace '^tst_', '' -replace '\.cpp$', '' -replace '_', '-'
        Write-Host "==> Mapped to CTest test: $Filter"
    }
    else
    {
        Write-Host "==> Not a recognized C++ test file. Running all tests."
        $Filter = ''
    }
}

# ── Find cmake ──────────────────────────────────────────────────────────
$cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmakeCmd)
{
    $cmake = $cmakeCmd.Source
}
else
{
    $candidates = @(
        "$env:ProgramFiles\CMake\bin\cmake.exe",
        "$env:LOCALAPPDATA\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe"
    )
    $cmake = $candidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
}
if (-not $cmake)
{
    throw 'cmake.exe not found. Install CMake or CLion.'
}

$ctest = Join-Path (Split-Path -Parent $cmake) 'ctest.exe'
if (-not (Test-Path -LiteralPath $ctest))
{
    throw 'ctest.exe not found next to cmake.'
}

# ── Qt ──────────────────────────────────────────────────────────────────
if (-not $env:QT_ROOT_DIR)
{
    $kit = Get-ChildItem -LiteralPath 'C:\Qt' -Directory -ErrorAction SilentlyContinue |
        ForEach-Object {
            $p = Join-Path $_.FullName 'msvc2022_64'
            if (Test-Path "$p\bin\windeployqt.exe") { Get-Item $p }
        } |
        Sort-Object { [version]$_.Parent.Name } -Descending |
        Select-Object -First 1
    if ($kit) { $env:QT_ROOT_DIR = $kit.FullName }
}
if (-not $env:QT_ROOT_DIR)
{
    throw 'QT_ROOT_DIR not set and no Qt msvc2022_64 kit found in C:\Qt.'
}

$preset = $Config.ToLowerInvariant()
$buildDir = Join-Path $PSScriptRoot "build/$preset"

# ── Configure ───────────────────────────────────────────────────────────
$cacheFile = Join-Path $buildDir 'CMakeCache.txt'
if ($Reconfigure -or -not (Test-Path -LiteralPath $cacheFile))
{
    Write-Host "==> Configuring preset '$preset'..."
    & $cmake --preset $preset
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

# ── Compile ─────────────────────────────────────────────────────────────
if ($Filter)
{
    $target = "gsxi-$Filter-tests"
    Write-Host "==> Building only the target: $target ($Config)..."
    & $cmake --build --preset $preset --target $target --parallel
}
else
{
    Write-Host "==> Building all targets ($Config)..."
    & $cmake --build --preset $preset --parallel
}
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# ── Run CTest ───────────────────────────────────────────────────────────
Write-Host "`n==> Running tests..."
$ctestArgs = @(
    '--test-dir', $buildDir
    '-C', $Config
    '--output-on-failure'
    '-j', [Environment]::ProcessorCount
)
if ($Filter) { $ctestArgs += '-R', $Filter }

& $ctest @ctestArgs
exit $LASTEXITCODE
