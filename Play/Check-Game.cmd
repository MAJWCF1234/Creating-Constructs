@echo off
start "" /wait "%~dp0CreatingConstructs.exe" --smoke-test --validate --verify-blur
if errorlevel 1 exit /b 1
py -3 "%~dp0..\tools\audit_catalog.py"
