#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 5000

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];

    int pipe1[2]; // Канал между процессом 1 и процессом 2
    int pipe2[2]; // Канал между процессом 2 и процессом 3

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Ошибка при создании каналов");
        exit(EXIT_FAILURE);
    }

    pid_t pid;

    // Процесс 1: Чтение из входного файла и запись в pipe1
    pid = fork();
    if (pid == -1) {
        perror("Ошибка при создании процесса 1");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(pipe1[0]); // Закрыть чтение из pipe1

        // Открыть входной файл
        int input_fd = open(input_file, O_RDONLY);
        if (input_fd == -1) {
            perror("Ошибка при открытии входного файла");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        // Чтение из входного файла и запись в pipe1
        bytes_read = read(input_fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Ошибка при чтении данных из файла");
            exit(EXIT_FAILURE);
        }
        write(pipe1[1], buffer, bytes_read);

        close(pipe1[1]); // Закрыть запись в pipe1
        close(input_fd); // Закрыть входной файл

        exit(EXIT_SUCCESS);
    }

    // Процесс 2: Чтение из pipe1, обработка данных и запись в pipe2
    pid = fork();
    if (pid == -1) {
        perror("Ошибка при создании процесса 2");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(pipe1[1]); // Закрыть запись в pipe1
        close(pipe2[0]); // Закрыть чтение из pipe2

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        // Чтение из pipe1, обработка данных и запись в pipe2
        bytes_read = read(pipe1[0], buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Ошибка при чтении данных из канала");
            exit(EXIT_FAILURE);
        }

        // Обработка данных - преобразование согласных букв в верхний регистр
        for (int i = 0; i < bytes_read; i++) {
            if (isalpha(buffer[i]) && !strchr("AEIOUaeiou", buffer[i])) {
                buffer[i] = toupper(buffer[i]);
            }
        }

        write(pipe2[1], buffer, bytes_read);

        close(pipe1[0]); // Закрыть чтение из pipe1
        close(pipe2[1]); // Закрыть запись в pipe2

        exit(EXIT_SUCCESS);
    }

    // Процесс 3: Чтение из pipe2 и запись в выходной файл
    pid = fork();
    if (pid == -1) {
        perror("Ошибка при создании процесса 3");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(pipe1[0]); // Закрыть чтение из pipe1
        close(pipe1[1]); // Закрыть запись в pipe1
        close(pipe2[1]); // Закрыть запись в pipe2

        // Открыть выходной файл
        int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (output_fd == -1) {
            perror("Ошибка при открытии выходного файла");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        // Чтение из pipe2 и запись в выходной файл
        bytes_read = read(pipe2[0], buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Ошибка при чтении данных из канала");
            exit(EXIT_FAILURE);
        }

        write(output_fd, buffer, bytes_read);

        close(pipe2[0]); // Закрыть чтение из pipe2
        close(output_fd); // Закрыть выходной файл

        exit(EXIT_SUCCESS);
    }

    // Родительский процесс
    close(pipe1[0]); // Закрыть чтение из pipe1
    close(pipe1[1]); // Закрыть запись в pipe1
    close(pipe2[0]); // Закрыть чтение из pipe2
    close(pipe2[1]); // Закрыть запись в pipe2

    // Ожидание завершения всех дочерних процессов
    while (wait(NULL) != -1);

    return 0;
}
