#define _WIN32_WINNT 0x0601   /* Windows 7 o superior */
#define WINVER       0x0601

#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include <stdlib.h>
#include <io.h>
#include <stdarg.h>
#include <time.h>

#ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 0x2
#endif

#ifndef FILE_NAME_NORMALIZED
#define FILE_NAME_NORMALIZED 0x0
#endif

#define ARCHIVO_REPORTE "reporte_fs.txt"

static FILE *log_f = NULL;

#pragma comment(lib, "psapi.lib")

/* Imprime en pantalla y en el archivo de reporte */
static void out(const char *fmt, ...)
{
    va_list a;
    va_start(a, fmt);
    vprintf(fmt, a);
    va_end(a);
    if (log_f)
    {
        va_start(a, fmt);
        vfprintf(log_f, fmt, a);
        va_end(a);
    }
}

static double ahora(void)
{
    LARGE_INTEGER f, c;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart / f.QuadPart;
}

static void mostrar_fecha(const char *etiqueta, FILETIME ft)
{
    FILETIME local;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&ft, &local);
    FileTimeToSystemTime(&local, &st);
    out("%-22s %04d-%02d-%02d %02d:%02d:%02d\n", etiqueta,
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

static void mostrar_contenido(const char *ruta) {
    FILE *f = fopen(ruta, "r");
    if (!f) { perror("fopen"); return; }

    printf("Contenido leído desde '%s':\n", ruta);
    char linea[256];
    while (fgets(linea, sizeof(linea), f))
        printf("  | %s", linea);
    fclose(f);
    printf("\n");
}

static void info(const char *ruta)
{
    HANDLE h = CreateFileA(ruta, 0,
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "CreateFile fallo: %lu\n", GetLastError());
        return;
    }

    BY_HANDLE_FILE_INFORMATION bi;
    if (!GetFileInformationByHandle(h, &bi))
    {
        fprintf(stderr, "GetFileInformationByHandle fallo: %lu\n", GetLastError());
        CloseHandle(h);
    }
    else
    {
        CloseHandle(h);

        unsigned long long indice = ((unsigned long long)bi.nFileIndexHigh << 32) | bi.nFileIndexLow;
        unsigned long long tam = ((unsigned long long)bi.nFileSizeHigh << 32) | bi.nFileSizeLow;
        DWORD a = bi.dwFileAttributes;

        out("Archivo: %s\n", ruta);
        out("  Tipo: %s\n", (a & FILE_ATTRIBUTE_DIRECTORY) ? "directorio" : "archivo regular");
        out("  Tamaño: %llu bytes\n", tam);
        out("  Atributos: %s%s%s%s\n",
            (a & FILE_ATTRIBUTE_READONLY) ? "[solo lectura] " : "",
            (a & FILE_ATTRIBUTE_HIDDEN) ? "[oculto] " : "",
            (a & FILE_ATTRIBUTE_SYSTEM) ? "[sistema] " : "",
            (a & FILE_ATTRIBUTE_ARCHIVE) ? "[archivo] " : "");
        out("  Índice de archivo: %llu\n", indice);
        out("  Enlaces: %lu\n", bi.nNumberOfLinks);
        mostrar_fecha("Creación:", bi.ftCreationTime);
        mostrar_fecha("Último acceso:", bi.ftLastAccessTime);
        mostrar_fecha("Última modificación:", bi.ftLastWriteTime);
        out("\n");
    }
}

int main(void)
{
    SetConsoleOutputCP(CP_UTF8); /* para que los acentos se vean bien en consola */

    log_f = fopen(ARCHIVO_REPORTE, "w"); /* usa "a" para ir acumulando corridas */
    if (!log_f)
        fprintf(stderr, "Aviso: no se pudo crear %s, solo se imprimirá en pantalla\n",
                ARCHIVO_REPORTE);

    const char *original = "archivo.txt";
    const char *enlace = "archivo_hardlink.txt";
    const char *simbolico = "archivo_symlink.txt";

    // Declaración de variables info memoria
    unsigned long page_size;
    unsigned long long total_bytes, avail_bytes;
    unsigned long long swap_total, swap_free;

    // Obtener el tamaño de página del sistema
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    page_size = si.dwPageSize;
    out("Tamaño de página: %lu bytes\n", page_size);

    // Obtener información de memoria usando GlobalMemoryStatusEx
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    if (!GlobalMemoryStatusEx(&ms))
    {
        fprintf(stderr, "GlobalMemoryStatusEx fallo: %lu\n", GetLastError());
    }
    else
    {

        total_bytes = ms.ullTotalPhys;
        avail_bytes = ms.ullAvailPhys;

        /* ullTotalPageFile = RAM + page file (commit limit) */
        swap_total = (ms.ullTotalPageFile > ms.ullTotalPhys)
                         ? ms.ullTotalPageFile - ms.ullTotalPhys
                         : 0;
        /* Estimación del page file sin usar */
        swap_free = (ms.ullAvailPageFile > ms.ullAvailPhys)
                        ? ms.ullAvailPageFile - ms.ullAvailPhys
                        : 0;
        if (swap_free > swap_total)
            swap_free = swap_total;

        unsigned long long total_pages = total_bytes / page_size;
        unsigned long long swap_used = swap_total - swap_free;

        // Imprimir la información del sistema
        out("INFO DE MEMORIA DEL SISTEMA\n");
        out("Tamaño de memoria: %.2f MB\n", total_bytes / (1024.0 * 1024.0));
        out("Memoria libre/disponible: %.2f MB\n", avail_bytes / (1024.0 * 1024.0));
        out("Total de páginas: %llu\n", total_pages);
        out("Page file total (aprox.): %.2f MB\n", swap_total / (1024.0 * 1024.0));
        out("Page file libre (aprox.): %.2f MB\n", swap_free / (1024.0 * 1024.0));
        out("Page file en uso (aprox.): %.2f MB\n", swap_used / (1024.0 * 1024.0));
    }
    // Obtener información de fallos de página del proceso actual
    PROCESS_MEMORY_COUNTERS pmc;
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        out("Fallos de página: no disponible\n");
    }
    else
    {
        out("Fallos de página totales: ND\n");
        out("Fallos mayores (lectura de disco): ND\n");
        out("KB leídos desde disco (pgpgin): ND\n");
        out("Páginas leídas desde swap (pswpin): ND\n");
        out("Fallos de página del proceso: %lu\n", pmc.PageFaultCount);
    }
        out("\n");

    char win[MAX_PATH];
    if (!GetWindowsDirectoryA(win, MAX_PATH))
    {
        fprintf(stderr, "GetWindowsDirectory fallo: %lu\n", GetLastError());
    }
    else
    {

        char root[4] = {win[0], ':', '\\', 0};
        char drive[3] = {win[0], ':', 0};

        char label[MAX_PATH] = "", fs[MAX_PATH] = "", dev[MAX_PATH] = "";
        DWORD serial = 0;

        if (!GetVolumeInformationA(root, label, sizeof(label), &serial,
                                   NULL, NULL, fs, sizeof(fs)))
        {
            fprintf(stderr, "GetVolumeInformation fallo: %lu\n", GetLastError());
        }
        else
        {
            QueryDosDeviceA(drive, dev, sizeof(dev));

            ULARGE_INTEGER avail, total, freeb;
            if (!GetDiskFreeSpaceExA(root, &avail, &total, &freeb))
            {
                fprintf(stderr, "GetDiskFreeSpaceEx fallo: %lu\n", GetLastError());
            }
            else
            {

                double MB = 1024.0 * 1024.0;
                out("INFORMACIÓN DEL VOLUMEN PRINCIPAL\n");
                out("Volumen principal: %s (etiqueta: %s)\n", drive, label[0] ? label : "sin etiqueta");
                out("Dispositivo: %s\n", dev);
                out("Sistema de archivos: %s\n", fs);
                out("Tamaño del volumen: %.2f MB\n", total.QuadPart / MB);
            }
        }
    }

    DWORD mascara = GetLogicalDrives();
    if (mascara == 0)
    {
        fprintf(stderr, "GetLogicalDrives fallo: %lu\n", GetLastError());
    }
    else
    {

        out("INFORMACIÓN DE UNIDADES DE ALMACENAMIENTO\n");
        out("%-8s %-22s %s\n", "Unidad", "Tipo", "Tamaño (MB)");

        for (int i = 0; i < 26; i++)
        {
            if (!(mascara & (1u << i)))
                continue;

            char raiz[4] = {'A' + i, ':', '\\', 0};
            const char *tipo;
            switch (GetDriveTypeA(raiz))
            {
            case DRIVE_FIXED:
                tipo = "Disco fijo";
                break;
            case DRIVE_REMOVABLE:
                tipo = "Extraíble (USB)";
                break;
            case DRIVE_CDROM:
                tipo = "CD/DVD";
                break;
            case DRIVE_REMOTE:
                tipo = "Unidad de red";
                break;
            case DRIVE_RAMDISK:
                tipo = "Disco RAM";
                break;
            default:
                tipo = "Desconocido";
                break;
            }

            ULARGE_INTEGER disp, total, libre;
            double mb = 0;
            if (GetDiskFreeSpaceExA(raiz, &disp, &total, &libre))
                mb = total.QuadPart / (1024.0 * 1024.0);

            out("%-8s %-22s %.2f\n", raiz, tipo, mb);
        }
    }
    char *buf = calloc(1024 * 1024, 1);
    if (!buf)
    {
        out("Error al asignar memoria para el buffer de prueba\n");
    }
    else
    {
        double t0 = ahora();
        FILE *f = fopen("prueba.bin", "wb");
        if (!f)
        {
            perror("fopen");
        }
        else
        {
            for (int i = 0; i < 256; i++)
                fwrite(buf, 1, 1024 * 1024, f);
            fflush(f);
            _commit(_fileno(f)); /* forzar a disco */
            fclose(f);
            double t_esc = ahora() - t0;

            t0 = ahora();
            f = fopen("prueba.bin", "rb");
            if (!f)
            {
                perror("fopen");
            }
            else
            {
                while (fread(buf, 1, 1024 * 1024, f) > 0)
                {
                }
                fclose(f);
                double t_lec = ahora() - t0;

                out("== Prueba de velocidad de lectura y escritura ==\n");
                out("Escritura: %.3f s (%.1f MiB/s)\n", t_esc, 256 / t_esc);
                out("Lectura:   %.3f s (%.1f MiB/s)\n", t_lec, 256 / t_lec);
                out("\n");

                remove("prueba.bin");
            }
        }
        free(buf);
    }


    DeleteFileA(enlace);
    DeleteFileA(simbolico); /* por si quedó de una corrida anterior */

    /* Crear el archivo regular */
    FILE *f = fopen(original, "w");
    if (!f)
    {
        perror("fopen");
    }
    else
    {
        fputs("Contenido de prueba\n", f);
        fclose(f);

        out("CONSULTAS SISTEMA DE ARCHIVOS\n");
        out("== Antes del hard link ==\n");
        info(original);

        /* Crear el hard link (nuevo nombre, existente) */
        if (!CreateHardLinkA(enlace, original, NULL))
        {
            fprintf(stderr, "CreateHardLink fallo: %lu\n", GetLastError());
        }
        else
        {

            out("== Después del hard link ==\n");
            info(original);
            info(enlace);

            f = fopen(enlace, "a");
            if (!f)
            {
                perror("fopen (enlace)");
            }
            else
            {
                fputs("Linea 2: agregada desde el hard link\n", f);
                fclose(f);

                out("== Después de modificar por '%s' ==\n", enlace);
                mostrar_contenido(original);
                info(original);
                info(enlace);
                out("\n");

                f = fopen(original, "a");
                if (!f)
                {
                    perror("fopen (original)");
                }
                else
                {
                }
                fputs("Linea 3: agregada desde el original\n", f);
                fclose(f);

                out("== Después de modificar por '%s' ==\n", original);
                mostrar_contenido(enlace);

                DeleteFileA(enlace);
                DeleteFileA(original);
            }
        }
    }
    f = fopen(original, "w");
    if (!f)
    {
       perror("fopen");
    }
    else
    {
        fputs("Contenido de prueba\n", f);
        fclose(f);


    /* Intentar el enlace simbólico (primero sin privilegios, útil con Modo desarrollador) */
    BOOLEAN ok = CreateSymbolicLinkA(simbolico, original,
                                     SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE);
    if (!ok && GetLastError() == ERROR_INVALID_PARAMETER)
    {
        /* Windows antiguo que no reconoce el flag: intento clásico */
        ok = CreateSymbolicLinkA(simbolico, original, 0);
    }

    if (!ok)
    {
        DWORD err = GetLastError();
        out("Enlace simbólico: N/D (error %lu%s)\n", err,
            err == 1314 ? ": falta el privilegio, ejecuta como administrador o activa el Modo desarrollador" : "");
        DeleteFileA(original);
    }
    else
    {

        DWORD attr = GetFileAttributesA(simbolico);
        out("Enlace simbólico: %s -> %s\n", simbolico, original);
        out("  Es enlace simbólico (reparse point): %s\n",
            (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_REPARSE_POINT)) ? "sí" : "no");

        /* Ruta real a la que resuelve el enlace */
        HANDLE h = CreateFileA(simbolico, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE)
        {
            char ruta[MAX_PATH];
            if (GetFinalPathNameByHandleA(h, ruta, MAX_PATH, FILE_NAME_NORMALIZED))
                out("  Resuelve a: %s\n", ruta);
            else
                out("  Resuelve a: N/D\n");
            CloseHandle(h);
        }

        /* Leer el contenido a través del enlace */
        f = fopen(simbolico, "r");
        if (f)
        {
            char linea[256];
            out("  Contenido leído por el enlace: ");
            if (fgets(linea, sizeof(linea), f))
                printf("%s", linea);
            fclose(f);
        }

        /* Romper el enlace: borrar el destino */
        DeleteFileA(original);
        out("\nDespués de borrar el original:\n");
        f = fopen(simbolico, "r");
        if (!f)
            out("  El enlace quedó roto (abrirlo falla)\n");
        else
            fclose(f);
        attr = GetFileAttributesA(simbolico);
        out("  El enlace como entrada sigue existiendo: %s\n",
            (attr != INVALID_FILE_ATTRIBUTES) ? "sí" : "no (N/D)");

        DeleteFileA(simbolico);
     }
    }


    if (!GetWindowsDirectoryA(win, MAX_PATH))
    {
    fprintf(stderr, "GetWindowsDirectory fallo: %lu\n", GetLastError());
    }
    else
    {

    char root[4] = {win[0], ':', '\\', 0};
    char drive[3] = {win[0], ':', 0};

   char  label[MAX_PATH] = "";
char  fs[MAX_PATH]    = "";
char  dev[MAX_PATH]   = "";
DWORD serial          = 0;
DWORD maxComp         = 0;   // <- antes era char max_nombre[MAX_PATH]

if (!GetVolumeInformationA(root, label, sizeof(label), &serial,
                           &maxComp, NULL, fs, sizeof(fs)))
{
    fprintf(stderr, "GetVolumeInformation fallo: %lu\n", GetLastError());
}
else
{
    // drive debe ser del estilo "C:" (sin barra final)
    if (QueryDosDeviceA(drive, dev, sizeof(dev)) == 0)
    {
        fprintf(stderr, "QueryDosDevice fallo: %lu\n", GetLastError());
        dev[0] = '\0';
    }

    out("\nINFORMACION DE LA UNIDAD\n");
    out("Etiqueta: %s\n", label);
    out("Dispositivo: %s\n", dev);
    out("Sistema de archivos: %s\n", fs);
    out("Numero de serie: %04lX-%04lX\n", serial >> 16, serial & 0xFFFF);
    out("Longitud maxima de nombre: %lu\n", maxComp);
}
    DWORD sec_por_cluster, bytes_por_sector, libres, total;
    if (GetDiskFreeSpaceA(root, &sec_por_cluster, &bytes_por_sector,
                          &libres, &total))
    {
        out("Bytes por sector: %lu\n", bytes_por_sector);
        out("Sectores por clúster: %lu\n", sec_por_cluster);
        out("Unidad de asignación (clúster): %lu bytes\n",
            sec_por_cluster * bytes_por_sector);
    }

    ULARGE_INTEGER disponible, capacidad, libre_total;
    if (GetDiskFreeSpaceExA(root, &disponible, &capacidad, &libre_total))
    {
        double MB = 1024.0 * 1024.0;
        double usado = (double)(capacidad.QuadPart - libre_total.QuadPart);
        double pct = capacidad.QuadPart ? 100.0 * usado / (double)capacidad.QuadPart : 0;

        out("Capacidad total: %.2f MB\n", capacidad.QuadPart / MB);
        out("Espacio usado: %.2f MB (%.1f%%)\n", usado / MB, pct);
        out("Espacio libre: %.2f MB\n", disponible.QuadPart / MB);
    }
}

    if (log_f)
    {
    fclose(log_f);
    printf("\nReporte guardado en %s\n", ARCHIVO_REPORTE);
    }
    return 0;
}