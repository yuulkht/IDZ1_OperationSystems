#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> // Добавленное объявление

#define BUFFER_SIZE 5000

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    const char *fifo_name = "channel.fifo";


    mkfifo(fifo_name, 0666);
    int fifo_fd = open(fifo_name, O_RDWR);
    if (fifo_fd == -1) {
        perror("Ошибка при открытии именованного канала для записи");
        exit(EXIT_FAILURE);
    }

    // Открываем входной файл
    int input_fd = open(input_file, O_RDONLY);
    if (input_fd == -1) {
        perror("Ошибка при открытии входного файла");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Читаем данные из файла
    bytes_read = read(input_fd, buffer, BUFFER_SIZE);
    if (bytes_read == -1) {
        perror("Ошибка при чтении данных из файла");
        exit(EXIT_FAILURE);
    }

    write(fifo_fd, buffer, bytes_read);

    close(input_fd);  // Закрываем входной файл

    // Процесс 2: Получение данных из канала, обработка и запись обратно в канал
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("Ошибка при создании процесса 2");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {

        // Читаем данные из канала
        bytes_read = read(fifo_fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Ошибка при чтении данных из канала");
            exit(EXIT_FAILURE);
        }

        // Обрабатываем данные - пример: преобразование согласных букв в верхний регистр
        for (int i = 0; i < bytes_read; i++) {
            if (isalpha(buffer[i]) && !strchr("AEIOUaeiou", buffer[i])) {
                buffer[i] = toupper(buffer[i]);

            }
        }

        // Записываем обработанные данные обратно в канал
        write(fifo_fd, buffer, bytes_read);

        exit(EXIT_SUCCESS);
    }
     
    wait(NULL);
    
    char buffer_result[BUFFER_SIZE];
    ssize_t bytes_read_result;

    // Открываем выходной файл
    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("Ошибка при открытии выходного файла");
        exit(EXIT_FAILURE);
    }

    // Читаем данные из канала и записываем в выходной файл
    bytes_read_result = read(fifo_fd, buffer_result, BUFFER_SIZE);
    if (bytes_read_result == -1) {
        perror("Ошибка при чтении данных из канала");
        exit(EXIT_FAILURE);
    }
    write(output_fd, buffer_result, bytes_read_result);

    // Закрываем выходной файл
    close(output_fd);
    unlink(fifo_name);

    return 0;
}
