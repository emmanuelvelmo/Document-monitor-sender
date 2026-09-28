setlocal

REM VARIABLES
set "carpeta_origen=%APPDATA%\Runtime"
set "carpeta_destino=%~dp0Runtime"

REM Copiar carpeta Runtime al directorio actual
xcopy "%carpeta_origen%" "%carpeta_destino%\" /E /I /Y /H /K

endlocal