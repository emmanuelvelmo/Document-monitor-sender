@echo off
setlocal

REM VARIABLES
set "nombre_usuario=%USERNAME%"
set "carpeta_startup=C:\Users\%nombre_usuario%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"
set "carpeta_roaming=C:\Users\%nombre_usuario%\AppData\Roaming"
set "carpeta_actual=%~dp0"
set "carpeta_origen=%~dp0Startup"

REM Copiar carpeta Startup al directorio Roaming
xcopy "%carpeta_origen%" "%carpeta_roaming%\Startup\" /E /I /Y /H /K >nul

REM Buscar el primer acceso directo en el directorio actual
for %%a in ("%carpeta_actual%*.lnk") do (
    if not defined acceso_directo (
        set "acceso_directo=%%a"
    )
)

REM Copiar acceso directo a la carpeta Startup del menu inicio
copy "%acceso_directo%" "%carpeta_startup%\" /Y >nul

endlocal