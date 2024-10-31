#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <getopt.h>
#include <stdbool.h>  

// Библиотека для подсчета суммы
#include "sum_lib.h"

// Функция для генерации массива
void GenerateArray(int *array, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100;
    }
}

struct SumArgs {
    int *array;
    int begin;
    int end;
};

// Поточная функция для подсчета суммы
void *ThreadSum(void *args) {
    struct SumArgs *sum_args = (struct SumArgs *)args;
    return (void *)(size_t)Sum(sum_args->array, sum_args->begin, sum_args->end);
}

int main(int argc, char **argv) {
    uint32_t threads_num = 0;
    uint32_t array_size = 0;
    uint32_t seed = 0;
    struct timeval start_time, end_time;

    // Парсинг аргументов командной строки
    while (true) {
        static struct option options[] = {
            {"threads_num", required_argument, 0, 't'},
            {"seed", required_argument, 0, 's'},
            {"array_size", required_argument, 0, 'a'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "t:s:a:", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 't': threads_num = atoi(optarg); break;
            case 's': seed = atoi(optarg); break;
            case 'a': array_size = atoi(optarg); break;
            case '?':
                printf("Неверный аргумент командной строки.\n");
                return 1;
        }
    }

    if (threads_num == 0 || array_size == 0 || seed == 0) {
        printf("Usage: ./psum --threads_num <num> --seed <num> --array_size <num>\n");
        return 1;
    }

    // Создание массива
    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    // Замер времени начала подсчета суммы
    gettimeofday(&start_time, NULL);

    // Создание и запуск потоков
    pthread_t threads[threads_num];
    struct SumArgs args[threads_num];
    int chunk_size = array_size / threads_num;

    for (uint32_t i = 0; i < threads_num; i++) {
        args[i].array = array;
        args[i].begin = i * chunk_size;
        args[i].end = (i == threads_num - 1) ? array_size : args[i].begin + chunk_size;
        if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
            printf("Error: pthread_create failed!\n");
            return 1;
        }
    }

    // Объединение результатов потоков
    int total_sum = 0;
    for (uint32_t i = 0; i < threads_num; i++) {
        int sum = 0;
        pthread_join(threads[i], (void **)&sum);
        total_sum += sum;
    }

    // Замер времени завершения подсчета суммы
    gettimeofday(&end_time, NULL);

    // Вычисление времени выполнения
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
                         (end_time.tv_usec - start_time.tv_usec) / 1000.0;

    // Освобождение памяти
    free(array);

    // Вывод результатов
    printf("Total sum: %d\n", total_sum);
    printf("Elapsed time: %.3f ms\n", elapsed_time);

    return 0;
}