#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int sudoku[9][9];   // matriz global

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

    return 0;
}