#!/bin/bash

# Проверка на наличие аргументов
if [ "$#" -eq 0 ]; then
    echo "Нет входных аргументов."
    exit 1
fi

# Подсчет количества аргументов
count=$#
# Подсчет суммы аргументов
sum=0

for num in "$@"; do
    sum=$((sum + num))
done

# Вычисление среднего арифметического
average=$(echo "scale=2; $sum / $count" | bc)

# Вывод результата
echo "Количество: $count"
echo "Среднее арифметическое: $average"
