#include <stdio.h>
#include <windows.h>
#include <psapi.h>

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
        printf("Fallos de página del proceso: %lu\n", pmc.PageFaultCount);
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
        printf("Volumen principal: %s (etiqueta: %s)\n", drive, label[0] ? label : "sin etiqueta");
        printf("Dispositivo: %s\n", dev);
        printf("Sistema de archivos: %s\n", fs);
        printf("Tamaño del volumen: %.2f MB\n", total.QuadPart / MB);
    }
    return 0;
}
