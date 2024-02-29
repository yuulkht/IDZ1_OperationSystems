#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
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
    const char *channel1_fifo = "channel1.fifo";
    const char *channel2_fifo = "channel2.fifo";

    // Создаем именованные каналы
    mkfifo(channel1_fifo, 0666);
    mkfifo(channel2_fifo, 0666);

    // Процесс 1: Чтение из входного файла и запись в channel1_fifo
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("Ошибка при создании процесса 1");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        int input_fd = open(input_file, O_RDONLY);
        if (input_fd == -1) {
            perror("Ошибка при открытии входного файла");
            exit(EXIT_FAILURE);
        }

        int channel_fd = open(channel1_fifo, O_WRONLY);
        if (channel_fd == -1) {
            perror("Ошибка при открытии именованного канала для записи");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        // Читаем данные из входного файла
        bytes_read = read(input_fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Ошибка при чтении данных из файла");
            exit(EXIT_FAILURE);
        }

        // Записываем данные в канал
        write(channel_fd, buffer, bytes_read);

        close(input_fd);
        close(channel_fd);

        exit(EXIT_SUCCESS);
    }

    // Процесс 2: Чтение из channel1_fifo, обработка данных и запись в channel2_fifo
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("Ошибка при создании процесса 2");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
        int channel1_fd = open(channel1_fifo, O_RDONLY);
        if (channel1_fd == -1) {
            perror("Ошибка при открытии именованного канала для чтения");
            exit(EXIT_FAILURE);
        }

        int channel2_fd = open(channel2_fifo, O_WRONLY);
        if (channel2_fd == -1) {
            perror("Ошибка при открытии именованного канала для записи");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        // Читаем данные из канала и записываем их в другой канал
        bytes_read = read(channel1_fd, buffer, BUFFER_SIZE);
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

        // Записываем обработанные данные в другой канал
        write(channel2_fd, buffer, bytes_read);

        close(channel1_fd);
        close(channel2_fd);

        exit(EXIT_SUCCESS);
    }

    // Процесс 3: Чтение из channel2_fifo и запись в выходной файл
    pid_t pid3 = fork();
    if (pid3 == -1) {
        perror("Ошибка при создании процесса 3");
        exit(EXIT_FAILURE);
    } else if (pid3 == 0) {
        int channel2_fd = open(channel2_fifo, O_RDONLY);
        if (channel2_fd == -1) {
            perror("Ошибка при открытии именованного канала для чтения");
            exit(EXIT_FAILURE);
        }

        int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (output_fd == -1) {
            perror("Ошибка при открытии выходного файла");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        // Читаем данные из канала и записываем их в выходной файл
        bytes_read = read(channel2_fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Ошибка при чтении данных из канала");
            exit(EXIT_FAILURE);
        }

        write(output_fd, buffer, bytes_read);

        close(channel2_fd);
        close(output_fd);

        exit(EXIT_SUCCESS);
    }

    // Родительский процесс ждет завершения всех дочерних процессов
    while (wait(NULL) != -1);

    // Удаляем именованные каналы
    unlink(channel1_fifo);
    unlink(channel2_fifo);

    return 0;
}
