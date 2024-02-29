#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 128

int main() {
    const char *fifo_name = "channel.fifo";
    const char *result_fifo_name = "result.fifo";

    // Открываем именованный канал для чтения
    int fifo_fd = open(fifo_name, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Ошибка при открытии именованного канала для чтения");
        exit(EXIT_FAILURE);
    }

    // Создаем и открываем именованный канал для записи
    mkfifo(result_fifo_name, 0666);
    int result_fifo_fd = open(result_fifo_name, O_WRONLY);
    if (result_fifo_fd == -1) {
        perror("Ошибка при открытии именованного канала для записи");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Читаем данные из именованного канала, обрабатываем и передаем обратно
    while ((bytes_read = read(fifo_fd, buffer, BUFFER_SIZE)) > 0) {
        // Обработка данных - преобразование согласных букв в верхний регистр
        for (int i = 0; i < bytes_read; i++) {
            if (isalpha(buffer[i]) && !strchr("AEIOUaeiou", buffer[i])) {
                buffer[i] = toupper(buffer[i]);
            }
        }

        // Запись обработанных данных обратно в именованный канал
        ssize_t bytes_written = write(result_fifo_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Ошибка при записи в именованный канал");
            exit(EXIT_FAILURE);
        }
    }

    printf("Результат записан в именованный канал %s\n", result_fifo_name);

    // Закрываем именованные каналы
    close(fifo_fd);
    close(result_fifo_fd);

    return 0;
}
