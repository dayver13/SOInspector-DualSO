#include <stdio.h>
#include <unistd.h>

int main(void)
{
    unsigned long page_size;
    char line[256];
    unsigned long total_kb = 0, free_kb = 0, avail_kb = 0;
    unsigned long swap_total_kb = 0, swap_free_kb = 0;

    // Obtener el tamaño de página del sistema
    page_size = getpagesize();

    // Abrir el archivo /proc/meminfo para leer la información de memoria
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f)
    {
        perror("fopen");
        return 1;
    }

    while (fgets(line, sizeof(line), f))
    {
        sscanf(line, "MemTotal: %lu kB", &total_kb);
        sscanf(line, "MemFree: %lu kB", &free_kb);
        sscanf(line, "MemAvailable: %lu kB", &avail_kb);
        sscanf(line, "SwapTotal: %lu kB", &swap_total_kb);
        sscanf(line, "SwapFree: %lu kB", &swap_free_kb);
    }
    fclose(f);

    unsigned long total_pages = (total_kb * 1024UL) / page_size;

    // Imprimir la información del sistema
    printf("Tamaño de página: %lu bytes\n", page_size);
    printf("Memoria total: %lu MB\n", total_kb / 1024);
    printf("Memoria libre: %lu MB\n", free_kb / 1024);
    printf("Memoria disponible: %lu MB\n", avail_kb / 1024);
    printf("Número total de páginas: %lu\n", total_pages);
    printf("Swap total: %.2f MB\n", swap_total_kb / 1024.0);
    printf("Swap libre: %.2f MB\n", swap_free_kb / 1024.0);
    printf("Swap en uso: %.2f MB\n", (swap_total_kb - swap_free_kb) / 1024.0);
    return 0;
}
