#!/usr/bin/env bash
# Compila OSInspector-DualSO.c en Linux.
# Uso:
#   ./os_inspector_linux.sh          -> compila
#   ./os_inspector_linux.sh run      -> compila y ejecuta
#   ./os_inspector_linux.sh clean    -> borra binario y archivos generados

set -euo pipefail

cd "$(dirname "$0")"

FUENTE="OSInspector-DualSO.c"
SALIDA="osinspector"

if [[ "${1:-}" == "clean" ]]; then
    rm -f "$SALIDA" reporte_fs.txt prueba.bin archivo.txt archivo_hardlink.txt archivo_symlink.txt
    echo "Limpieza completada."
    exit 0
fi

if [[ ! -f "$FUENTE" ]]; then
    echo "Error: no se encontró $FUENTE en $(pwd)" >&2
    exit 1
fi

# Buscar un compilador disponible
CC_BIN=""
for c in gcc cc clang; do
    if command -v "$c" >/dev/null 2>&1; then
        CC_BIN="$c"
        break
    fi
done

if [[ -z "$CC_BIN" ]]; then
    echo "Error: no hay compilador de C instalado." >&2
    echo "  Debian/Ubuntu: sudo apt install build-essential" >&2
    echo "  Fedora/RHEL:   sudo dnf install gcc" >&2
    echo "  Arch:          sudo pacman -S base-devel" >&2
    exit 1
fi

echo "Compilando con $CC_BIN..."
# gnu11 expone symlink/readlink/getmntent/clock_gettime (POSIX/GNU)
"$CC_BIN" -std=gnu11 -O2 -Wall -Wextra -o "$SALIDA" "$FUENTE"

echo "Listo: ./$SALIDA"

if [[ "${1:-}" == "run" ]]; then
    echo "-----------------------------------------"
    ./"$SALIDA"
fi
