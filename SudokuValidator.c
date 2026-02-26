#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <pthread.h>
#include <sys/syscall.h>

#include <omp.h>

int sudoku[9][9];   // matriz global

int validar_fila(int fila) {

    int check[10] = {0};

    for (int j = 0; j < 9; j++) {
        int num = sudoku[fila][j];

        if (num < 1 || num > 9)
            return 0;

        if (check[num] == 1)
            return 0;

        check[num] = 1;
    }

    return 1;
}

int validar_columna(int col) {

    int check[10] = {0};

    for (int i = 0; i < 9; i++) {
        int num = sudoku[i][col];

        if (num < 1 || num > 9)
            return 0;

        if (check[num] == 1)
            return 0;

        check[num] = 1;
    }

    return 1;
}

int validar_subcuadro(int fila_inicio, int col_inicio) {

    int check[10] = {0};

    for (int i = fila_inicio; i < fila_inicio + 3; i++) {
        for (int j = col_inicio; j < col_inicio + 3; j++) {

            int num = sudoku[i][j];

            if (num < 1 || num > 9)
                return 0;

            if (check[num] == 1)
                return 0;

            check[num] = 1;
        }
    }

    return 1;
}

/* ----------- THREAD CON OPENMP ----------- */

void* revisar_columnas_thread(void* arg) {

    printf("El thread que ejecuta el metodo de revision de columnas es: %ld\n",
           syscall(SYS_gettid));

    // Paralelización con OpenMP
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < 9; i++) {

        printf("En la revision de columnas el siguiente es un thread en ejecucion: %ld\n",
               syscall(SYS_gettid));

        sleep(2);

        validar_columna(i);
    }

    pthread_exit(0);
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Uso: %s <archivo_sudoku>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Error al abrir archivo");
        return 1;
    }

    char *map = mmap(NULL, 81, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error en mmap");
        return 1;
    }

    int index = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            sudoku[i][j] = map[index++] - '0';
        }
    }

    printf("Sudoku cargado:\n");
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            printf("%d ", sudoku[i][j]);
        }
        printf("\n");
    }

    munmap(map, 81);
    close(fd);

    pid_t pid = getpid();
    printf("PID del proceso padre: %d\n", pid);

    /* ----------- CREAR PTHREAD ----------- */

    pthread_t thread_col;

    if (pthread_create(&thread_col, NULL,
                       revisar_columnas_thread, NULL) != 0) {
        perror("Error creando pthread");
        return 1;
    }

    /* ----------- PS MIENTRAS EL THREAD EXISTE ----------- */

    pid_t child_ps = fork();

    if (child_ps == 0) {

        char pid_str[20];
        sprintf(pid_str, "%d", pid);

        execlp("ps", "ps", "-p", pid_str, "-lLf", NULL);

        perror("Error en execlp");
        exit(1);
    }
    else {
        wait(NULL);
    }

    pthread_join(thread_col, NULL);

    printf("El thread en el que se ejecuta main es: %ld\n",
           syscall(SYS_gettid));

    int valido = 1;

    /* ----------- VALIDAR FILAS ----------- */

    for (int i = 0; i < 9; i++) {
        if (!validar_fila(i)) {
            valido = 0;
        }
    }

    /* ----------- VALIDAR SUBCUADROS ----------- */

    for (int i = 0; i < 9; i += 3) {
        for (int j = 0; j < 9; j += 3) {
            if (!validar_subcuadro(i, j)) {
                valido = 0;
            }
        }
    }

    if (valido)
        printf("Sudoku resuelto!\n");
    else
        printf("Sudoku incorrecto.\n");

    printf("Antes de terminar el estado de este proceso y sus threads es:\n");

    /* ----------- SEGUNDO PS ----------- */

    pid_t child2 = fork();

    if (child2 == 0) {

        char pid_str[20];
        sprintf(pid_str, "%d", pid);

        execlp("ps", "ps", "-p", pid_str, "-lLf", NULL);

        perror("Error en execlp");
        exit(1);
    }
    else {
        wait(NULL);
    }

    return 0;
}