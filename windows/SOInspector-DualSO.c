#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include <stdlib.h>
#include <io.h>

static double ahora(void)
{
    LARGE_INTEGER f, c;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart / f.QuadPart;
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
            return 1;
        }
        QueryDosDeviceA(drive, dev, sizeof(dev));

        ULARGE_INTEGER avail, total, freeb;
        if (!GetDiskFreeSpaceExA(root, &avail, &total, &freeb))
        {
            fprintf(stderr, "GetDiskFreeSpaceEx fallo: %lu\n", GetLastError());
            return 1;
        }

        double MB = 1024.0 * 1024.0;
        printf("INFORMACIÓN DEL VOLUMEN PRINCIPAL\n");
        printf("Volumen principal: %s (etiqueta: %s)\n", drive, label[0] ? label : "sin etiqueta");
        printf("Dispositivo: %s\n", dev);
        printf("Sistema de archivos: %s\n", fs);
        printf("Tamaño del volumen: %.2f MB\n", total.QuadPart / MB);
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

                printf("Escritura: %.3f s (%.1f MiB/s)\n", t_esc, 256 / t_esc);
                printf("Lectura:   %.3f s (%.1f MiB/s)\n", t_lec, 256 / t_lec);

                remove("prueba.bin");
            }
        }
        free(buf);
    }
    return 0;
}
