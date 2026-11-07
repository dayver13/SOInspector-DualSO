#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <mntent.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

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

static void mostrar_fecha(const char *etiqueta, time_t t)
{
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    printf("%-22s %s\n", etiqueta, buf);
}

static void info(const char *ruta)
{
    struct stat st;
    if (stat(ruta, &st) != 0)
    {
        perror("stat");
        return;
    }

    char p[10];
    snprintf(p, sizeof(p), "%c%c%c%c%c%c%c%c%c",
             (st.st_mode & S_IRUSR) ? 'r' : '-',
             (st.st_mode & S_IWUSR) ? 'w' : '-',
             (st.st_mode & S_IXUSR) ? 'x' : '-',
             (st.st_mode & S_IRGRP) ? 'r' : '-',
             (st.st_mode & S_IWGRP) ? 'w' : '-',
             (st.st_mode & S_IXGRP) ? 'x' : '-',
             (st.st_mode & S_IROTH) ? 'r' : '-',
             (st.st_mode & S_IWOTH) ? 'w' : '-',
             (st.st_mode & S_IXOTH) ? 'x' : '-');

    printf("Archivo: %s\n", ruta);
    printf("Tipo: %s\n", S_ISREG(st.st_mode) ? "archivo regular" : "otro");
    printf("Tamaño: %lld bytes\n", (long long)st.st_size);
    printf("Permisos: %s (%04o)\n", p, st.st_mode & 07777);
    mostrar_fecha("Último acceso:", st.st_atime);
    mostrar_fecha("Última modificación:", st.st_mtime);
    mostrar_fecha("Último cambio (inodo):", st.st_ctime);
    printf("\n");
}

static void comparar_inodos(const char *a, const char *b)
{
    struct stat sa, sb;
    if (stat(a, &sa) != 0)
    {
        perror(a);
        return;
    }
    if (stat(b, &sb) != 0)
    {
        perror(b);
        return;
    }

    printf("== Comparación de inodos ==\n");
    printf("  %-22s inodo: %llu  dispositivo: %llu\n", a,
           (unsigned long long)sa.st_ino, (unsigned long long)sa.st_dev);
    printf("  %-22s inodo: %llu  dispositivo: %llu\n", b,
           (unsigned long long)sb.st_ino, (unsigned long long)sb.st_dev);

    if (sa.st_ino == sb.st_ino && sa.st_dev == sb.st_dev)
        printf("  Resultado: IGUALES, ambos nombres apuntan al mismo archivo\n\n");
    else
        printf("  Resultado: DISTINTOS, son archivos diferentes\n\n");
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
    printf("INFO MEMORIA DEL SISTEMA\n");
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

            printf("== Prueba de velocidad de lectura y escritura ==\n");
            printf("Escritura: %.3f s (%.1f MiB/s)\n", t_esc, 256 / t_esc);
            printf("Lectura:   %.3f s (%.1f MiB/s)\n", t_lec, 256 / t_lec);
            printf("\n");

            remove("prueba.bin");
            free(buf);
        }
    }

    // Sistema de archivos
    const char *original = "archivo.txt";
    const char *enlace = "archivo_hardlink.txt";
    const char *simbolico = "archivo_symlink.txt";

    unlink(simbolico);

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

        /* Crear el hard link */
        if (link(original, enlace) != 0)
        {
            fprintf(stderr, "link fallo: %s\n", strerror(errno));
        }
        else
        {

            printf("== Después del hard link ==\n");
            info(original);
            info(enlace);

            comparar_inodos(original, enlace);

            /* Borrar el original: el contenido sigue accesible por el enlace */
            unlink(original);
            printf("== Después de borrar el original ==\n");
            info(enlace);

            unlink(enlace);
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

        if (symlink(original, simbolico) != 0)
        {
            printf("Enlace simbólico: N/D (%s)\n", strerror(errno));
            unlink(original);
        }
        else
        {

            struct stat st_link, st_dest;
            char destino[256];
            ssize_t n = readlink(simbolico, destino, sizeof(destino) - 1);
            if (n >= 0)
                destino[n] = '\0';
            else
                strcpy(destino, "N/D");

            if (lstat(simbolico, &st_link) != 0 || stat(simbolico, &st_dest) != 0)
            {
                printf("Enlace simbólico creado, pero no se pudo consultar: N/D\n");
                unlink(simbolico);
                unlink(original);
            }
            else
            {

                printf("Enlace simbólico: %s -> %s\n", simbolico, destino);
                printf("  Es enlace simbólico (lstat): %s\n", S_ISLNK(st_link.st_mode) ? "sí" : "no");
                printf("  Inodo del enlace:  %llu  tamaño: %lld bytes (largo de la ruta)\n",
                       (unsigned long long)st_link.st_ino, (long long)st_link.st_size);
                printf("  Inodo del destino: %llu  tamaño: %lld bytes\n",
                       (unsigned long long)st_dest.st_ino, (long long)st_dest.st_size);
                printf("  Enlaces del destino: %lu\n", (unsigned long)st_dest.st_nlink);

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
                unlink(original);
                printf("\nDespués de borrar el original:\n");
                if (fopen(simbolico, "r") == NULL)
                    printf("  El enlace quedó roto (abrirlo falla: %s)\n", strerror(errno));
                printf("  lstat sigue funcionando: %s\n",
                       lstat(simbolico, &st_link) == 0 ? "sí, el enlace existe" : "no");

                unlink(simbolico);
            }
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

        printf("\nINFORMACIÓN DE LA UNIDAD\n");
        printf("Sistema de archivos: %s\n", fs);
        printf("Tamaño de bloque fundamental (f_frsize): %lu bytes\n", sv.f_frsize);
        printf("Tamaño de bloque preferido de E/S (f_bsize): %lu bytes\n", sv.f_bsize);
        printf("Longitud máxima de nombre: %lu\n", sv.f_namemax);
    }

    return 0;
}
