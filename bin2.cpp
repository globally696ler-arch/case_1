#include <fstream>      //
#include <iostream>     //
#include <stdio.h>      // Для printf и scanf (ввод/вывод)
#include <string.h>     // Для strcmp (сравнение строк)
#include <locale.h>

// Вспомогательная функция: находит позицию n-й запятой в строке (n начинается с 1).
// Если запятых меньше n — возвращает std::string::npos.
size_t findNthComma(const std::string& s, int n) {
    size_t pos = 0;                  // текущая позиция поиска
    for (int i = 1; i <= n; ++i) {   // ищем запятые по порядку
        pos = s.find(',', pos);      // ищем ближайшую запятую начиная с pos
        if (pos == std::string::npos) return std::string::npos; // не нашли — выходим
        if (i < n) ++pos;            // сдвиг, чтобы не найти ту же запятую снова
    }
    return pos;                      // позиция n-й запятой
}

int main() {
    setlocale(LC_ALL,"Russian");
    std::ifstream file("binbank.txt"); 
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть binbank.txt\n";
        return 1;
    }  // открыть на чтение
    char digits[19];//
    // Приглашение ко вводу
    printf("Введите цифры: ");
    // Читаем максимум 6 символов (первые 6 цифр номера карты — это БИН банка).
    // %6s защищает от переполнения: больше 6 символов не запишется.
    scanf("%6s", digits);
    std::string line;
    while (std::getline(file, line)) {
        
         if (line.compare(0, 6, digits) != 0) continue; //если не совпадает с 6ю цифрами введенными пользователем пропускаем строку
            // Ищем пятую и шестую запятые
            size_t fifth = findNthComma(line, 4);
            size_t sixth = findNthComma(line, 5);
            // Если обе найдены — выводим символы строго между 5-й и 6-й запятыми
            if (fifth != std::string::npos && sixth != std::string::npos) {
                std::string between = line.substr(fifth + 1, sixth - fifth - 1);
                std::cout << between << '\n';
            }
        }
    return 0;
    }
    