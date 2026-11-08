{\rtf1\ansi\ansicpg1252\cocoartf2870
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fnil\fcharset0 .AppleSystemUIFontMonospaced-Regular;}
{\colortbl;\red255\green255\blue255;\red0\green0\blue0;\red255\green255\blue255;\red0\green0\blue0;
}
{\*\expandedcolortbl;;\cssrgb\c0\c0\c0\c90196;\cssrgb\c100000\c100000\c100000;\cssrgb\c0\c0\c0\c60000;
}
\margl1440\margr1440\vieww32040\viewh18700\viewkind0
\deftab720
\pard\pardeftab720\partightenfactor0

\f0\fs24 \cf2 \cb3 \expnd0\expndtw0\kerning0
\outl0\strokewidth0 \strokec2 # Compila SOInspector-DualSO.c en Windows.\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 # Uso:\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 #   .\\so_inspector_windows.ps1          -> compila\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 #   .\\so_inspector_windows.ps1 -Run     -> compila y ejecuta\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 #   .\\so_inspector_windows.ps1 -Clean   -> borra el .exe y archivos generados\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 #\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 # Si PowerShell bloquea el script:\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 #   powershell -ExecutionPolicy Bypass -File .\\so_inspector_windows.ps1\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 param(\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     [switch]$Run,\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     [switch]$Clean\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 )\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 $ErrorActionPreference = "Stop"\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 Set-Location -Path $PSScriptRoot\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 $Fuente = "SOInspector-DualSO.c"\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 $Salida = "so_inspector.exe"\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 if ($Clean) \{\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     Remove-Item -Force -ErrorAction SilentlyContinue `\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2         $Salida, "so_inspector.obj", "reporte_fs.txt", "prueba.bin", `\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2         "archivo.txt", "archivo_hardlink.txt", "archivo_symlink.txt"\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     Write-Host "Limpieza completada."\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     exit 0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \}\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 if (-not (Test-Path $Fuente)) \{\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     Write-Error "No se encontro $Fuente en $PSScriptRoot"\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     exit 1\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \}\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 # Buscar un compilador disponible: gcc (MinGW/MSYS2), clang o cl (Visual Studio)\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 $compilador = $null\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 foreach ($c in @("gcc", "clang", "cl")) \{\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     if (Get-Command $c -ErrorAction SilentlyContinue) \{\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2         $compilador = $c\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2         break\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     \}\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \}\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 if (-not $compilador) \{\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     Write-Host "Error: no se encontro ningun compilador de C en el PATH." -ForegroundColor Red\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     Write-Host "Opciones:"\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     Write-Host "  - MinGW-w64 / MSYS2:  winget install MSYS2.MSYS2   (luego instalar mingw-w64-ucrt-x86_64-gcc)"\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     Write-Host "  - Visual Studio Build Tools y abrir 'Developer PowerShell for VS'"\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     exit 1\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \}\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 Write-Host "Compilando con $compilador..."\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 switch ($compilador) \{\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     "cl" \{\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2         # /utf-8 para que los acentos del codigo fuente se lean bien\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2         & cl /nologo /O2 /W3 /utf-8 $Fuente /Fe:$Salida /link psapi.lib\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     \}\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     default \{\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2         # -lpsapi: GetProcessMemoryInfo\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2         & $compilador -O2 -Wall -Wextra -o $Salida $Fuente -lpsapi\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     \}\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \}\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 if ($LASTEXITCODE -ne 0) \{\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     Write-Host "La compilacion fallo (codigo $LASTEXITCODE)." -ForegroundColor Red\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     exit $LASTEXITCODE\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \}\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 # Limpiar objeto intermedio de MSVC\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 Remove-Item -Force -ErrorAction SilentlyContinue "SOInspector-DualSO.obj"\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 Write-Host "Listo: .\\$Salida" -ForegroundColor Green\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 if ($Run) \{\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     Write-Host "-----------------------------------------"\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2     & ".\\$Salida"\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \}\cb1 \
\pard\pardeftab720\partightenfactor0
\cf4 \strokec4 \
\pard\pardeftab720\partightenfactor0
\cf2 \cb3 \strokec2 \'a0\cb1 \
}