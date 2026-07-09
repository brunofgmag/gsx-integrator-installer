param(
    [Parameter(Mandatory)]
    [string]$Lupdate,

    [Parameter(Mandatory)]
    [string]$SourceDir,

    [Parameter(Mandatory)]
    [string]$I18nDir
)

$ErrorActionPreference = 'Stop'

function Get-SourceInventory([string]$path)
{
    $raw = Get-Content -LiteralPath $path -Raw
    $raw = $raw -replace '<!DOCTYPE[^>]*>', ''
    $doc = [xml]$raw
    $items = New-Object System.Collections.Generic.List[string]
    foreach ($ctx in $doc.TS.context)
    {
        $name = [string]$ctx.name
        foreach ($msg in $ctx.message)
        {
            $items.Add($name + "`t" + [string]$msg.source)
        }
    }
    , ($items | Sort-Object)
}

$tsFiles = @(Get-ChildItem -LiteralPath $I18nDir -Filter '*.ts' -File |
    Select-Object -ExpandProperty FullName)
if (-not $tsFiles)
{
    throw "No .ts file found in $I18nDir"
}

$tempDir = Join-Path ([IO.Path]::GetTempPath()) ('ts-check-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $tempDir | Out-Null
try
{
    $copies = @()
    foreach ($ts in $tsFiles)
    {
        $dest = Join-Path $tempDir (Split-Path -Leaf $ts)
        Copy-Item -LiteralPath $ts -Destination $dest -Force
        $copies += $dest
    }

    $lupdateArgs = @($SourceDir, '-ts') + $copies + @('-no-obsolete', '-locations', 'none')
    & $Lupdate @lupdateArgs 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "lupdate failed (exit $LASTEXITCODE)." }

    $problems = @()
    for ($i = 0; $i -lt $tsFiles.Count; $i++)
    {
        $committed = Get-SourceInventory $tsFiles[$i]
        $regenerated = Get-SourceInventory $copies[$i]
        $diff = Compare-Object -ReferenceObject @($committed) -DifferenceObject @($regenerated)
        if ($diff)
        {
            $problems += "==> $($tsFiles[$i]) is out of date:"
            foreach ($d in $diff)
            {
                if ($d.SideIndicator -eq '=>')
                {
                    $problems += "    [MISSING from .ts — string in code, not extracted] $($d.InputObject)"
                }
                else
                {
                    $problems += "    [STALE in .ts — removed from code] $($d.InputObject)"
                }
            }
        }
    }

    if ($problems)
    {
        $msg = @(
            'Translations are out of date with the source.'
            'Run lupdate and translate in the .ts (target: update_translations).'
            ''
        ) + $problems
        Write-Error ($msg -join "`n")
        exit 1
    }

    Write-Host "Translations are in sync with the source ($($tsFiles.Count) file(s) checked)."
}
finally
{
    Remove-Item -LiteralPath $tempDir -Recurse -Force -ErrorAction SilentlyContinue
}
