param(
    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Release',
    [switch]$RunTests,
    [switch]$SkipTests
)

$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot

if ($RunTests -and $SkipTests)
{
    throw 'Use either -RunTests or -SkipTests, not both.'
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

# ── Qt kit detection ────────────────────────────────────────────────────
if (-not $env:QT_ROOT_DIR)
{
    $kit = Get-ChildItem -LiteralPath 'C:\Qt' -Directory -ErrorAction SilentlyContinue |
        ForEach-Object {
            $p = Join-Path $_.FullName 'msvc2022_64'
            if (Test-Path "$p\bin\windeployqt.exe") { Get-Item $p }
        } |
        Sort-Object { [version]$_.Parent.Name } -Descending |
        Select-Object -First 1
    if ($kit)
    {
        $env:QT_ROOT_DIR = $kit.FullName
        Write-Host "Using Qt kit: $($env:QT_ROOT_DIR)"
    }
}
if (-not $env:QT_ROOT_DIR -or
    -not (Test-Path -LiteralPath (Join-Path $env:QT_ROOT_DIR 'bin\windeployqt.exe')))
{
    throw 'QT_ROOT_DIR must point to a Qt 6.8+ msvc2022_64 kit.'
}

$preset    = $Config.ToLowerInvariant()
$buildDir  = Join-Path $PSScriptRoot "build/$preset"
$deployDir = Join-Path $buildDir 'bin'
$exe       = Join-Path $deployDir 'gsx-integrator-installer.exe'
$cacheFile = Join-Path $buildDir 'CMakeCache.txt'

# ── Drop incompatible cache (wrong generator) ───────────────────────────
if (Test-Path -LiteralPath $cacheFile)
{
    $cacheLines = Get-Content -LiteralPath $cacheFile
    $okGen = $cacheLines -contains 'CMAKE_GENERATOR:INTERNAL=Visual Studio 17 2022'
    $okX64 = $cacheLines -contains 'CMAKE_GENERATOR_PLATFORM:INTERNAL=x64'
    if (-not $okGen -or -not $okX64)
    {
        Write-Host 'Removing incompatible CMake cache before configuring MSVC x64.'
        Remove-Item -LiteralPath $cacheFile -Force
        $cmakeFiles = Join-Path $buildDir 'CMakeFiles'
        if (Test-Path -LiteralPath $cmakeFiles) { Remove-Item -LiteralPath $cmakeFiles -Recurse -Force }
    }
}

# ── Configure ───────────────────────────────────────────────────────────
$configureArgs = @('--preset', $preset)
if ($SkipTests) { $configureArgs += '-DBUILD_TESTING=OFF' }
& $cmake @configureArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# ── Build (windeployqt runs as POST_BUILD via cmake/Deployment.cmake) ────
& $cmake --build --preset $preset --parallel
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# ── Optional tests ──────────────────────────────────────────────────────
if ($RunTests)
{
    $ctest = Join-Path (Split-Path -Parent $cmake) 'ctest.exe'
    if (-not (Test-Path -LiteralPath $ctest))
    {
        throw 'ctest.exe not found next to cmake.'
    }
    & $ctest --test-dir $buildDir -C $Config --output-on-failure
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

# ── Deployment sanity check ─────────────────────────────────────────────
$suffix = if ($Config -eq 'Debug') { 'd' } else { '' }
$required = @(
    'gsx-integrator-installer.exe',
    "Qt6Core$suffix.dll",
    "Qt6Quick$suffix.dll",
    "Qt6Qml$suffix.dll",
    "platforms/qwindows$suffix.dll"
)
$missing = $required | Where-Object { -not (Test-Path -LiteralPath (Join-Path $deployDir $_)) }
if ($missing)
{
    throw "Deployment incomplete, missing:`n$($missing -join "`n")"
}

$files = @(Get-ChildItem -LiteralPath $deployDir -File -Recurse)
$totalMiB = [math]::Round((($files | Measure-Object Length -Sum).Sum) / 1MB, 2)

Write-Host ''
Write-Host "Deployment verified: $($files.Count) files, $totalMiB MiB."
Write-Host "Executable ready: $exe"
Write-Host "Distribute the whole folder (or zipped): $deployDir"
