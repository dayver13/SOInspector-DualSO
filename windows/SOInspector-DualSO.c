#include <stdio.h>
#include <windows.h>

int main(void)
{
    unsigned long page_size;
    unsigned long long total_bytes, avail_bytes;

    // Obtener el tamaño de página del sistema
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    page_size = si.dwPageSize;

    // Obtener información de memoria usando GlobalMemoryStatusEx
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    if (!GlobalMemoryStatusEx(&ms))
    {
        fprintf(stderr, "GlobalMemoryStatusEx fallo: %lu\n", GetLastError());
        return 1;
    }

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
    printf("Tamaño de página: %lu bytes\n", page_size);
    printf("Tamaño de memoria: %.2f MB\n", total_bytes / (1024.0 * 1024.0));
    printf("Memoria libre/disponible: %.2f MB\n", avail_bytes / (1024.0 * 1024.0));
    printf("Total de páginas: %llu\n", total_pages);
    printf("Page file total (aprox.): %.2f MB\n", swap_total / (1024.0 * 1024.0));
    printf("Page file libre (aprox.): %.2f MB\n", swap_free / (1024.0 * 1024.0));
    printf("Page file en uso (aprox.): %.2f MB\n", swap_used / (1024.0 * 1024.0));
    return 0;
}
