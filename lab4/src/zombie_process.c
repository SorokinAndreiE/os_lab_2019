#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    // Создаем дочерний процесс
    pid = fork();

    if (pid < 0) {
        // Ошибка при создании процесса
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Дочерний процесс
        printf("Дочерний процесс (PID: %d) завершает работу.n", getpid());
        exit(0); // Завершение дочернего процесса
    } else {
        // Родительский процесс
        printf("Родительский процесс (PID: %d) спит 10 секунд.n", getpid());
        sleep(10); // Спим, не вызывая wait()
        printf("Родительский процесс завершает работу.n");
    }

    return 0;
}
   