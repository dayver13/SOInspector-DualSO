#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <mntent.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>

static int leer_ull(const char *ruta, unsigned long long *v)
{
    FILE *f = fopen(ruta, "r");
    if (!f)
        return -1;
    int ok = fscanf(f, "%llu", v);
    fclose(f);
    return ok == 1 ? 0 : -1;
}

static double ahora(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(void)
{
    // Declaración de variables info memoria
    unsigned long page_size;
    char line[256];
    unsigned long total_kb = 0, free_kb = 0, avail_kb = 0;
    unsigned long swap_total_kb = 0, swap_free_kb = 0;
    // Declaración de variables info fallos de página
    char key[64];
    unsigned long long val;
    unsigned long long pgfault = 0, pgmajfault = 0, pgpgin = 0, pswpin = 0;
    int found = 0;

    // Obtener el tamaño de página del sistema
    page_size = getpagesize();
    printf("MEMORIA DEL SISTEMA\n");
    printf("Tamaño de página: %lu bytes\n", page_size);

    // Abrir el archivo /proc/meminfo para leer la información de memoria
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f)
    {
        perror("fopen");
    }
    else
    {

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
        printf("Memoria total: %lu MB\n", total_kb / 1024);
        printf("Memoria libre: %lu MB\n", free_kb / 1024);
        printf("Memoria disponible: %lu MB\n", avail_kb / 1024);
        printf("Número total de páginas: %lu\n", total_pages);
        printf("Swap total: %.2f MB\n", swap_total_kb / 1024.0);
        printf("Swap libre: %.2f MB\n", swap_free_kb / 1024.0);
        printf("Swap en uso: %.2f MB\n", (swap_total_kb - swap_free_kb) / 1024.0);
        printf("\n");
    }

    // Abrir el archivo /proc/vmstat para leer la información de fallos de página
    f = fopen("/proc/vmstat", "r");
    if (!f)
    {
        printf("Fallos de página: no disponible\n");
    }
    else
    {

        while (fscanf(f, "%63s %llu", key, &val) == 2)
        {
            if (strcmp(key, "pgfault") == 0)
            {
                pgfault = val;
                found++;
            }
            else if (strcmp(key, "pgmajfault") == 0)
            {
                pgmajfault = val;
                found++;
            }
            else if (strcmp(key, "pgpgin") == 0)
            {
                pgpgin = val;
                found++;
            }
            else if (strcmp(key, "pswpin") == 0)
            {
                pswpin = val;
                found++;
            }
        }
        fclose(f);

        if (found == 0)
        {
            printf("Fallos de página: no disponible\n");
        }
        else
        {
            // Imprimir la información de fallos de página
            printf("Fallos de página totales: %llu\n", pgfault);
            printf("Fallos mayores (lectura de disco): %llu\n", pgmajfault);
            printf("KB leídos desde disco (pgpgin): %llu\n", pgpgin);
            printf("Páginas leídas desde swap (pswpin): %llu\n", pswpin);
        }
    }

    f = setmntent("/proc/mounts", "r");
    if (!f)
    {
        perror("setmntent");
    }
    else
    {
        const char *ruta = "/";

        struct mntent *m;
        char dev[256] = "desconocido", fs[64] = "desconocido", mnt[256] = "desconocido";

        /* Si la ruta aparece varias veces, la última es la vigente */
        while ((m = getmntent(f)) != NULL)
        {
            if (strcmp(m->mnt_dir, ruta) == 0)
            {
                snprintf(dev, sizeof(dev), "%s", m->mnt_fsname);
                snprintf(fs, sizeof(fs), "%s", m->mnt_type);
                snprintf(mnt, sizeof(mnt), "%s", m->mnt_dir);
            }
        }
        endmntent(f);

        struct statvfs sv;
        if (statvfs(ruta, &sv) != 0)
        {
            perror("statvfs");
            return 1;
        }

        double MB = 1024.0 * 1024.0;
        double total = (double)sv.f_blocks * sv.f_frsize / MB;
        printf("\nINFORMACIÓN DEL VOLUMEN PRINCIPAL\n");
        printf("Dispositivo principal: %s\n", dev);
        printf("Punto de montaje: %s\n", mnt);
        printf("Sistema de archivos: %s\n", fs);
        printf("Tamaño del volumen: %.2f MB\n", total);
    }

    DIR *d = opendir("/sys/block");
    if (!d)
    {
        perror("opendir /sys/block");
    }
    else
    {

        printf("== Dispositivos de bloque ==\n");
        printf("%-14s %12s  %s\n", "Dispositivo", "Tamaño (MB)", "Tipo");

        struct dirent *e;
        while ((e = readdir(d)) != NULL)
        {
            if (e->d_name[0] == '.')
                continue;
            if (strncmp(e->d_name, "loop", 4) == 0 ||
                strncmp(e->d_name, "ram", 3) == 0)
                continue;

            char ruta[512];
            unsigned long long sectores = 0, rot = 2;

            snprintf(ruta, sizeof(ruta), "/sys/block/%s/size", e->d_name);
            leer_ull(ruta, &sectores);

            snprintf(ruta, sizeof(ruta), "/sys/block/%s/queue/rotational", e->d_name);
            leer_ull(ruta, &rot);

            const char *tipo = (rot == 1)   ? "HDD (rotacional)"
                               : (rot == 0) ? "SSD/NVMe/virtual"
                                            : "desconocido";

            /* los sectores de /sys/block siempre son de 512 bytes */
            printf("%-14s %12.2f  %s\n", e->d_name,
                   sectores * 512.0 / (1024.0 * 1024.0), tipo);
        }
        closedir(d);

        printf("== Contadores de E/S (desde el arranque) ==\n");
        printf("%-14s %14s %14s\n", "Dispositivo", "Lecturas", "Escrituras");
    }

    f = fopen("/proc/diskstats", "r");
    if (!f)
    {
        perror("fopen /proc/diskstats");
    }
    else
    {

        char linea[512], nombre[64];
        unsigned int mayor, menor;
        unsigned long long lect, lect_m, sect_l, ms_l, escr;

        while (fgets(linea, sizeof(linea), f))
        {
            if (sscanf(linea, "%u %u %63s %llu %llu %llu %llu %llu",
                       &mayor, &menor, nombre, &lect, &lect_m, &sect_l, &ms_l, &escr) == 8)
            {
                if (strncmp(nombre, "loop", 4) == 0 ||
                    strncmp(nombre, "ram", 3) == 0)
                    continue;
                printf("%-14s %14llu %14llu\n", nombre, lect, escr);
            }
        }
        fclose(f);
        printf("\n");
    }

    char *buf = calloc(1024 * 1024, 1);
    if (!buf)
        return 1;

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
        fsync(fileno(f)); /* forzar a disco */
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
            free(buf);
        }
    }

    return 0;
}
