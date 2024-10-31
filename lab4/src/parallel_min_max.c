#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include "find_min_max.h"
#include "utils.h"
#include "utils.c"

static int timeout = 0; // Переменная для хранения таймаута

void handle_timeout(int sig) {
    // Обработчик сигнала для таймаута
    printf("Таймаут истек! Завершение дочерних процессов.\n");
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;

    // Обработка аргументов командной строки
    while (true) {
        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {"timeout", required_argument, 0, 0}, // Новый параметр для таймаута
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0: seed = atoi(optarg); break;
                    case 1: array_size = atoi(optarg); break;
                    case 2: pnum = atoi(optarg); break;
                    case 3: with_files = true; break;
                    case 4: timeout = atoi(optarg); break; // Получение таймаута
                }
                break;
            case 'f': with_files = true; break;
            case '?': break;
            default: printf("getopt returned character code 0%o\n", c);
        }
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed <num> --array_size <num> --pnum <num> [--timeout <num>]\n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);
    int active_child_processes = 0;

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int pipefd[2];
    if (!with_files) {
        pipe(pipefd); // Создание пайпа
    }

    // Установка обработчика сигнала для таймаута
    signal(SIGALRM, handle_timeout);

    // Создание дочерних процессов
    pid_t child_pids[pnum];
    for (int i = 0; i < pnum; i++) {
        child_pids[i] = fork();
    if (child_pids[i] >= 0) {
            active_child_processes += 1;
            if (child_pids[i] == 0) { // Дочерний процесс
                int chunk_size = array_size / pnum;
                int start_index = i * chunk_size;
                int end_index = (i == pnum - 1) ? array_size : start_index + chunk_size;

                struct MinMax min_max;
                min_max.min = INT_MAX;
                min_max.max = INT_MIN;

                for (int j = start_index; j < end_index; j++) {
                    if (array[j] < min_max.min) min_max.min = array[j];
                    if (array[j] > min_max.max) min_max.max = array[j];
                }

                if (with_files) {
                    char filename[20];
                    sprintf(filename, "result_%d.txt", i);
                    FILE *file = fopen(filename, "w");
                    fprintf(file, "%d %dn", min_max.min, min_max.max);
                    fclose(file);
                } else {
                    write(pipefd[1], &min_max, sizeof(min_max));
                }
                return 0;
            }
        } else {
            printf("Fork failed!\n");
            return 1;
        }
    }

    // Установка таймера
    if (timeout > 0) {
        alarm(timeout); // Установка сигнала SIGALRM через timeout секунд
    }

    // Ожидание завершения дочерних процессов
    while (active_child_processes > 0) {
        pid_t result = waitpid(-1, NULL, WNOHANG); // Неблокирующий wait
        if (result > 0) {
            active_child_processes -= 1; // Один из дочерних процессов завершился
        } else if (result == -1 && errno != ECHILD) {
            perror("waitpid failed");
        }

        if (timeout > 0 && active_child_processes > 0 && result == 0) {
            // Если истек таймаут и есть еще активные дочерние процессы
            for (int i = 0; i < pnum; i++) {
                kill(child_pids[i], SIGKILL); // Убить дочерние процессы
            }
            active_child_processes = 0; // Сбросить счетчик активных процессов
            break; // Выход из цикла ожидания
        }

        usleep(100000); // Небольшая пауза для снижения нагрузки на CPU
    }

    struct MinMax final_min_max;
    final_min_max.min = INT_MAX;
    final_min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        if (with_files) {
            char filename[20];
            sprintf(filename, "result_%d.txt", i);
            FILE *file = fopen(filename, "r");
            struct MinMax min_max;
            fscanf(file, "%d %d", &min_max.min, &min_max.max);
            fclose(file);

            if (min_max.min < final_min_max.min) final_min_max.min = min_max.min;
            if (min_max.max > final_min_max.max) final_min_max.max = min_max.max;
        } else {
            struct MinMax min_max;
            read(pipefd[0], &min_max, sizeof(min_max));

            if (min_max.min < final_min_max.min) final_min_max.min = min_max.min;
            if (min_max.max > final_min_max.max) final_min_max.max = min_max.max;
        }
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0 +
                          (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);

    printf("Min: %d\n", final_min_max.min);
    printf("Max: %d\n", final_min_max.max);
    printf("Elapsed time: %f ms\n", elapsed_time);

    return 0;
}