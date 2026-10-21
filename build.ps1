param([string]$OutputName='CreatingConstructs.exe')
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
& py -3 (Join-Path $PSScriptRoot 'tools\sync_catalog.py')
if ($LASTEXITCODE -ne 0) { throw 'HTML catalog sync failed' }
$sdk = 'C:\VulkanSDK\1.4.341.1'
$vs = & 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe' -latest -property installationPath
New-Item -ItemType Directory -Force -Path build,Play | Out-Null
$header = @()
foreach ($shader in @('scene.vert','scene.frag','water.frag','sky.vert','sky.frag','post.vert','post.frag')) {
    & "$sdk\Bin\glslc.exe" -O "src\$shader" -o "build\$shader.spv"
    if ($LASTEXITCODE -ne 0) { throw "Shader failed: $shader" }
    $bytes = [IO.File]::ReadAllBytes((Join-Path $PSScriptRoot "build\$shader.spv"))
    $name = $shader.Replace('.','_')
    $header += "alignas(4) const unsigned char $name[] = {" + (($bytes | ForEach-Object { $_.ToString() }) -join ',') + '};'
}
[IO.File]::WriteAllLines((Join-Path $PSScriptRoot 'src\shaders.h'),$header)
if (!(Test-Path 'assets\audio\soundbank.bin')) {
    & py -3 tools\prepare_audio.py
    if ($LASTEXITCODE -ne 0) { throw 'Audio preparation failed' }
}
$audioBank = (Join-Path $PSScriptRoot 'assets\audio\soundbank.bin').Replace('\','\\')
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'build\audio.rc'), '201 RCDATA "' + $audioBank + '"')
$bat = @"
@echo off
call "$vs\VC\Auxiliary\Build\vcvars64.bat" >nul
rc /nologo /fo build\audio.res build\audio.rc
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /O2 /Ot /fp:precise /GL /MT /EHsc /W3 /DUNICODE /D_UNICODE /I"$sdk\Include" src\main.cpp build\audio.res /Fo:build\main.obj /Fe:"Play\$OutputName" /link /LTCG /OPT:REF /OPT:ICF /SUBSYSTEM:WINDOWS /STACK:4194304 /DYNAMICBASE /NXCOMPAT /INCREMENTAL:NO /LIBPATH:"$sdk\Lib" vulkan-1.lib user32.lib gdi32.lib msimg32.lib winmm.lib
"@
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'build\compile.cmd'),$bat)
& cmd.exe /c build\compile.cmd
if ($LASTEXITCODE -ne 0) { throw 'C++ build failed' }
$size = (Get-Item (Join-Path Play $OutputName)).Length
Write-Output "Executable: $size bytes"
if ($size -gt 32000000) { throw 'Executable exceeds 32 MB budget' }
