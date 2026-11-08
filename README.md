# SOInspector-DualSO

Herramienta en C que consulta información del sistema operativo y la muestra en pantalla y en el archivo `reporte_fs.txt`. Hay una versión para cada sistema, con su propio script de compilación.

## Contenido del proyecto

| Sistema | Código fuente | Script de compilación | Ejecutable generado |
| --- | --- | --- | --- |
| Linux | `OSInspector-DualSO.c` | `os_inspector_linux.sh` | `osinspector` |
| Windows | `SOInspector-DualSO.c` | `so_inspector_windows.ps1` | `so_inspector.exe` |

Cada script debe estar en la misma carpeta que su archivo `.c`.

## Qué hace el programa

- **Memoria:** tamaño de página, memoria total, libre y disponible, y swap o page file.
- **Fallos de página:** contadores del sistema en Linux; del proceso actual en Windows.
- **Almacenamiento:** volumen principal, sistema de archivos, tamaño, unidades o dispositivos de bloque, y espacio usado y libre.
- **Prueba de velocidad:** escribe y lee un archivo temporal de 256 MiB y muestra los MiB/s.
- **Enlaces:** crea un hard link y un enlace simbólico de prueba, compara sus datos y muestra qué pasa al borrar el original.

Los archivos temporales (`prueba.bin`, `archivo.txt`, `archivo_hardlink.txt`, `archivo_symlink.txt`) se borran solos al terminar. El único archivo que queda es ` reporte_<plataforma>_<hostname>_<fecha>.txt `.

---

## Linux

### Requisitos

- Linux (el programa lee `/proc` y `/sys`, así que no funciona en macOS ni en WSL1).
- Un compilador de C: `gcc`, `cc` o `clang`.

Si no tienes compilador:

```bash
sudo apt install build-essential     # Debian / Ubuntu
sudo dnf install gcc                 # Fedora / RHEL
sudo pacman -S base-devel            # Arch
```

### Compilar y ejecutar

```bash
chmod +x os_inspector_linux.sh
./os_inspector_linux.sh          # compila y genera ./osinspector
./os_inspector_linux.sh run      # compila y ejecuta
./os_inspector_linux.sh clean    # borra el binario y los archivos generados
```

Para ejecutar el programa ya compilado:

```bash
./osinspector
```

El script compila con `gcc -std=gnu11 -O2 -Wall -Wextra`. No necesita librerías adicionales.

### Compilación manual (sin el script)

```bash
gcc -std=gnu11 -O2 -Wall -Wextra -o osinspector OSInspector-DualSO.c
```

---

## Windows

### Requisitos

- Windows 10 o superior con PowerShell.
- Un compilador de C en el `PATH`, cualquiera de estos:
  - **MinGW-w64 / MSYS2** (`gcc`).
  - **clang**.
  - **Visual Studio Build Tools** (`cl`): abre *Developer PowerShell for VS* para que el compilador esté en el `PATH`.

### Compilar y ejecutar

```powershell
.\so_inspector_windows.ps1          # compila y genera so_inspector.exe
.\so_inspector_windows.ps1 -Run     # compila y ejecuta
.\so_inspector_windows.ps1 -Clean   # borra el .exe y los archivos generados
```

Para ejecutar el programa ya compilado:

```powershell
.\so_inspector.exe
```

Si PowerShell bloquea el script por la política de ejecución:

```powershell
powershell -ExecutionPolicy Bypass -File .\so_inspector_windows.ps1
```

### Compilación manual (sin el script)

```powershell
gcc -O2 -Wall -Wextra -o so_inspector.exe SOInspector-DualSO.c -lpsapi
```

Con Visual Studio:

```powershell
cl /nologo /O2 /utf-8 SOInspector-DualSO.c /Fe:so_inspector.exe /link psapi.lib
```

### Enlace simbólico en Windows

Crear enlaces simbólicos requiere ejecutar la terminal como administrador o activar el **Modo desarrollador** (Configuración > Privacidad y seguridad > Para desarrolladores). Si no se cumple, el programa muestra "Enlace simbólico: N/D" y continúa con el resto.

> **Importante:** en su estado actual, `SOInspector-DualSO.c` no compila: usa variables y funciones sin declarar y tiene llaves desbalanceadas. El script mostrará los errores del compilador hasta que se corrija el código fuente.

---

## Uso y resultados

1. Ejecuta el programa desde una carpeta donde tengas permiso de escritura.
2. La información aparece en pantalla y se guarda en `reporte_fs.txt` en la misma carpeta.
3. Cada ejecución sobrescribe `reporte_fs.txt`. Para acumular corridas, cambia `"w"` por `"a"` en la línea donde se abre `ARCHIVO_REPORTE`.
4. La prueba de velocidad escribe 256 MiB en disco, así que necesitas ese espacio libre y puede tardar unos segundos.

## Solución de problemas

| Problema | Causa probable | Solución |
| --- | --- | --- |
| `no hay compilador de C instalado` | Falta gcc, clang o cl | Instala uno según la sección de requisitos |
| `Permission denied` al ejecutar el `.sh` | Falta el permiso de ejecución | `chmod +x os_inspector_linux.sh` |
| El `.ps1` no se ejecuta | Política de ejecución de PowerShell | Usa `-ExecutionPolicy Bypass` como se indicó arriba |
| `no se pudo crear reporte_fs.txt` | Carpeta sin permiso de escritura | Ejecuta desde otra carpeta |
| Acentos mal mostrados en Windows | Consola sin UTF-8 | Usa Windows Terminal; el programa ya activa UTF-8 con `SetConsoleOutputCP` |
