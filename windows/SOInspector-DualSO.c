#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include <stdlib.h>
#include <io.h>

#ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 0x2
#endif

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
    printf("%-22s %04d-%02d-%02d %02d:%02d:%02d\n", etiqueta,
           st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
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

        printf("Archivo: %s\n", ruta);
        printf("  Tipo: %s\n", (a & FILE_ATTRIBUTE_DIRECTORY) ? "directorio" : "archivo regular");
        printf("  Tamaño: %llu bytes\n", tam);
        printf("  Atributos: %s%s%s%s\n",
               (a & FILE_ATTRIBUTE_READONLY) ? "[solo lectura] " : "",
               (a & FILE_ATTRIBUTE_HIDDEN) ? "[oculto] " : "",
               (a & FILE_ATTRIBUTE_SYSTEM) ? "[sistema] " : "",
               (a & FILE_ATTRIBUTE_ARCHIVE) ? "[archivo] " : "");
        printf("  Índice de archivo: %llu\n", indice);
        printf("  Enlaces: %lu\n", bi.nNumberOfLinks);
        mostrar_fecha("Creación:", bi.ftCreationTime);
        mostrar_fecha("Último acceso:", bi.ftLastAccessTime);
        mostrar_fecha("Última modificación:", bi.ftLastWriteTime);
        printf("\n");
    }
}

int main(void)
{
    // Declaración de variables info memoria
    unsigned long page_size;
    unsigned long long total_bytes, avail_bytes;

    // Obtener el tamaño de página del sistema
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    page_size = si.dwPageSize;
    printf("Tamaño de página: %lu bytes\n", page_size);

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

        // Imprimir la información del sistema
        printf("INFO DE MEMORIA DEL SISTEMA\n");
        printf("Tamaño de memoria: %.2f MB\n", total_bytes / (1024.0 * 1024.0));
        printf("Memoria libre/disponible: %.2f MB\n", avail_bytes / (1024.0 * 1024.0));
        printf("Total de páginas: %llu\n", total_pages);
        printf("Page file total (aprox.): %.2f MB\n", swap_total / (1024.0 * 1024.0));
        printf("Page file libre (aprox.): %.2f MB\n", swap_free / (1024.0 * 1024.0));
        printf("Page file en uso (aprox.): %.2f MB\n", swap_used / (1024.0 * 1024.0));
    }
    // Obtener información de fallos de página del proceso actual
    PROCESS_MEMORY_COUNTERS pmc;
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        printf("Fallos de página: no disponible\n");
    }
    else
    {
        printf("Fallos de página totales: ND\n");
        printf("Fallos mayores (lectura de disco): ND\n");
        printf("KB leídos desde disco (pgpgin): ND\n");
        printf("Páginas leídas desde swap (pswpin): ND\n");
        printf("Fallos de página del proceso: %lu\n", pmc.PageFaultCount);
    }
    printf("\n");

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
                printf("INFORMACIÓN DEL VOLUMEN PRINCIPAL\n");
                printf("Volumen principal: %s (etiqueta: %s)\n", drive, label[0] ? label : "sin etiqueta");
                printf("Dispositivo: %s\n", dev);
                printf("Sistema de archivos: %s\n", fs);
                printf("Tamaño del volumen: %.2f MB\n", total.QuadPart / MB);
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

        printf("INFORMACIÓN DE UNIDADES DE ALMACENAMIENTO\n");
        printf("%-8s %-22s %s\n", "Unidad", "Tipo", "Tamaño (MB)");

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

            printf("%-8s %-22s %.2f\n", raiz, tipo, mb);
        }
    }
    char *buf = calloc(1024 * 1024, 1);
    if (!buf)
    {
        printf("Error al asignar memoria para el buffer de prueba\n");
    }
    else
    {
        double t0 = ahora();
        f = fopen("prueba.bin", "wb");
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

                printf("== Prueba de velocidad de lectura y escritura ==\n");
                printf("Escritura: %.3f s (%.1f MiB/s)\n", t_esc, 256 / t_esc);
                printf("Lectura:   %.3f s (%.1f MiB/s)\n", t_lec, 256 / t_lec);
                printf("\n");

                remove("prueba.bin");
            }
        }
        free(buf);
    }

    const char *original = "archivo.txt";
    const char *enlace = "archivo_hardlink.txt";
    const char *simbolico = "archivo_symlink.txt";

    DeleteFileA(enlace);
    DeleteFileA(simbolico); /* por si quedó de una corrida anterior */

    /* Crear el archivo regular */
    f = fopen(original, "w");
    if (!f)
    {
        perror("fopen");
    }
    else
    {
        fputs("Contenido de prueba\n", f);
        fclose(f);

        printf("CONSULTAS SISTEMA DE ARCHIVOS\n");
        printf("== Antes del hard link ==\n");
        info(original);

        /* Crear el hard link (nuevo nombre, existente) */
        if (!CreateHardLinkA(enlace, original, NULL))
        {
            fprintf(stderr, "CreateHardLink fallo: %lu\n", GetLastError());
        }
        else
        {

            printf("== Después del hard link ==\n");
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

                printf("== Después de modificar por '%s' ==\n", enlace);
                mostrar_contenido(original);
                mostrar_info(original);
                mostrar_info(enlace);
                printf("\n");

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

                printf("== Después de modificar por '%s' ==\n", original);
                mostrar_contenido(enlace);

                DeleteFileA(enlace);
                DeleteFileA(original);
            }
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
        printf("Enlace simbólico: N/D (error %lu%s)\n", err,
               err == 1314 ? ": falta el privilegio, ejecuta como administrador o activa el Modo desarrollador" : "");
        DeleteFileA(original);
    }
    else
    {

        DWORD attr = GetFileAttributesA(simbolico);
        printf("Enlace simbólico: %s -> %s\n", simbolico, original);
        printf("  Es enlace simbólico (reparse point): %s\n",
               (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_REPARSE_POINT)) ? "sí" : "no");

        /* Ruta real a la que resuelve el enlace */
        HANDLE h = CreateFileA(simbolico, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE)
        {
            char ruta[MAX_PATH];
            if (GetFinalPathNameByHandleA(h, ruta, MAX_PATH, FILE_NAME_NORMALIZED))
                printf("  Resuelve a: %s\n", ruta);
            else
                printf("  Resuelve a: N/D\n");
            CloseHandle(h);
        }

        /* Leer el contenido a través del enlace */
        f = fopen(simbolico, "r");
        if (f)
        {
            char linea[256];
            printf("  Contenido leído por el enlace: ");
            if (fgets(linea, sizeof(linea), f))
                printf("%s", linea);
            fclose(f);
        }

        /* Romper el enlace: borrar el destino */
        DeleteFileA(original);
        printf("\nDespués de borrar el original:\n");
        f = fopen(simbolico, "r");
        if (!f)
            printf("  El enlace quedó roto (abrirlo falla)\n");
        else
            fclose(f);
        attr = GetFileAttributesA(simbolico);
        printf("  El enlace como entrada sigue existiendo: %s\n",
               (attr != INVALID_FILE_ATTRIBUTES) ? "sí" : "no (N/D)");

        DeleteFileA(simbolico);
    }
}

char win[MAX_PATH];
if (!GetWindowsDirectoryA(win, MAX_PATH))
{
    fprintf(stderr, "GetWindowsDirectory fallo: %lu\n", GetLastError());
}
else
{

    char root[4] = {win[0], ':', '\\', 0};
    char drive[3] = {win[0], ':', 0};

    char label[MAX_PATH] = "", fs[MAX_PATH] = "", dev[MAX_PATH] = "", max_nombre[MAX_PATH] = "";
    DWORD serial = 0;

    if (!GetVolumeInformationA(root, label, sizeof(label), &serial,
                               max_nombre, &flags, fs, sizeof(fs)))
    {
        fprintf(stderr, "GetVolumeInformation fallo: %lu\n", GetLastError());
    }
    else
    {
        QueryDosDeviceA(drive, dev, sizeof(dev));

        double MB = 1024.0 * 1024.0;
        printf("INFORMACIÓN DE LA UNIDAD\n");
        printf("Dispositivo: %s\n", dev);
        printf("Sistema de archivos: %s\n", fs);
        printf("Longitud máxima de nombre: %lu\n", max_nombre);
    }
    DWORD sec_por_cluster, bytes_por_sector, libres, total;
    if (GetDiskFreeSpaceA(raiz, &sec_por_cluster, &bytes_por_sector,
                          &libres, &total))
    {
        printf("Bytes por sector: %lu\n", bytes_por_sector);
        printf("Sectores por clúster: %lu\n", sec_por_cluster);
        printf("Unidad de asignación (clúster): %lu bytes\n",
               sec_por_cluster * bytes_por_sector);
    }
}
return 0;
}
