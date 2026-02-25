#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

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

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Uso: %s <archivo_sudoku>\n", argv[0]);
        return 1;
    }

    // Abrir archivo
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Error al abrir archivo");
        return 1;
    }

    // Mapear archivo en memoria
    char *map = mmap(NULL, 81, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error en mmap");
        return 1;
    }

    // Copiar datos a matriz
    int index = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            sudoku[i][j] = map[index++] - '0';
        }
    }

    // Mostrar matriz para verificar
    printf("Sudoku cargado:\n");
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            printf("%d ", sudoku[i][j]);
        }
        printf("\n");
    }

    munmap(map, 81);
    close(fd);

    int valido = 1;

    // Validar filas
    for (int i = 0; i < 9; i++) {
        if (!validar_fila(i)) {
            valido = 0;
        }
    }

    // Validar columnas
    for (int i = 0; i < 9; i++) {
        if (!validar_columna(i)) {
            valido = 0;
        }
    }

    // Validar subcuadros 3x3
    for (int i = 0; i < 9; i += 3) {
        for (int j = 0; j < 9; j += 3) {
            if (!validar_subcuadro(i, j)) {
                valido = 0;
            }
        }
    }

    if (valido)
        printf("El Sudoku es correcto.\n");
    else
        printf("El Sudoku NO es correcto.\n");

    return 0;
}