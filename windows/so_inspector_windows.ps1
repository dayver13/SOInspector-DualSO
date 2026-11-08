# Compila SOInspector-DualSO.c en Windows.
# Uso:
#   .\so_inspector_windows.ps1          -> compila
#   .\so_inspector_windows.ps1 -Run     -> compila y ejecuta
#   .\so_inspector_windows.ps1 -Clean   -> borra el .exe y archivos generados
#
# Si PowerShell bloquea el script:
#   powershell -ExecutionPolicy Bypass -File .\so_inspector_windows.ps1

param(
    [switch]$Run,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
Set-Location -Path $PSScriptRoot

$Fuente = "SOInspector-DualSO.c"
$Salida = "so_inspector.exe"

if ($Clean) {
    Remove-Item -Force -ErrorAction SilentlyContinue `
        $Salida, "so_inspector.obj", "reporte_fs.txt", "prueba.bin", `
        "archivo.txt", "archivo_hardlink.txt", "archivo_symlink.txt"
    Write-Host "Limpieza completada."
    exit 0
}

if (-not (Test-Path $Fuente)) {
    Write-Error "No se encontro $Fuente en $PSScriptRoot"
    exit 1
}

# Buscar un compilador disponible: gcc (MinGW/MSYS2), clang o cl (Visual Studio)
$compilador = $null
foreach ($c in @("gcc", "clang", "cl")) {
    if (Get-Command $c -ErrorAction SilentlyContinue) {
        $compilador = $c
        break
    }
}

if (-not $compilador) {
    Write-Host "Error: no se encontro ningun compilador de C en el PATH." -ForegroundColor Red
    Write-Host "Opciones:"
    Write-Host "  - MinGW-w64 / MSYS2:  winget install MSYS2.MSYS2   (luego instalar mingw-w64-ucrt-x86_64-gcc)"
    Write-Host "  - Visual Studio Build Tools y abrir 'Developer PowerShell for VS'"
    exit 1
}

Write-Host "Compilando con $compilador..."

switch ($compilador) {
    "cl" {
        # /utf-8 para que los acentos del codigo fuente se lean bien
        & cl /nologo /O2 /W3 /utf-8 $Fuente /Fe:$Salida /link psapi.lib
    }
    default {
        # -lpsapi: GetProcessMemoryInfo
        & $compilador -O2 -Wall -Wextra -o $Salida $Fuente -lpsapi
    }
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "La compilacion fallo (codigo $LASTEXITCODE)." -ForegroundColor Red
    exit $LASTEXITCODE
}

# Limpiar objeto intermedio de MSVC
Remove-Item -Force -ErrorAction SilentlyContinue "SOInspector-DualSO.obj"

Write-Host "Listo: .\$Salida" -ForegroundColor Green

if ($Run) {
    Write-Host "-----------------------------------------"
    & ".\$Salida"
}