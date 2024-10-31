#include "sum_lib.h"
int Sum(int *array, int begin, int end) {
    int sum = 0;
    for (int i = begin; i < end; i++) {
        sum += array[i];
    }
    return sum;
}