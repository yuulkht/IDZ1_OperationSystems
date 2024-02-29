#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 128

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    const char *fifo_name = "channel.fifo";
    const char *result_fifo_name = "result.fifo";

    // Открываем файл для чтения
    int input_fd = open(input_file, O_RDONLY);
    if (input_fd == -1) {
        perror("Ошибка при открытии входного файла");
        exit(EXIT_FAILURE);
    }

    // Создаем и открываем именованный канал для записи
    mkfifo(fifo_name, 0666);
    int fifo_fd = open(fifo_name, O_WRONLY);
    if (fifo_fd == -1) {
        perror("Ошибка при открытии именованного канала для записи");
        exit(EXIT_FAILURE);
    }

    // Читаем данные из файла и записываем в именованный канал
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(input_fd, buffer, BUFFER_SIZE)) > 0) {
        write(fifo_fd, buffer, bytes_read);
    }

    // Закрываем файл и канал
    close(input_fd);
    close(fifo_fd);

    printf("Данные переданы через именованный канал %s\n", fifo_name);

    // Открываем файл для записи данных из канала
    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("Ошибка при открытии выходного файла");
        exit(EXIT_FAILURE);
    }

    // Открываем именованный канал для чтения
    fifo_fd = open(result_fifo_name, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Ошибка при открытии именованного канала для чтения");
        exit(EXIT_FAILURE);
    }

    // Читаем данные из именованного канала и записываем их в файл
    while ((bytes_read = read(fifo_fd, buffer, BUFFER_SIZE)) > 0) {
        write(output_fd, buffer, bytes_read);
    }

    // Закрываем файл и канал
    close(output_fd);
    close(fifo_fd);

    printf("Данные записаны в файл %s\n", output_file);

    // Удаляем именованные каналы
    unlink(fifo_name);
    unlink(result_fifo_name);

    return 0;
}
