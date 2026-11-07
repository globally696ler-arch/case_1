#include <iostream>
#include <string>
#include <locale.h>
#include <fstream>
#include <thread>
#include <chrono>
#include <cctype>

using namespace std;

// структура запроса на оплату
struct PaymentRequest
{
    string recipient;  // получатель
    string purpose;    // назначение платежа
    double amount;     // сумма
    string currency;   // валюта
    string orderId;    // идентификатор заказа
    string cardNumber; // номер карты
    string expiryDate; // срок действия
    string cvv;        // cvv-код
    bool isConfirmed;  // флаг подтверждения
};

// структура ответа от банка
struct PaymentResponse
{
    bool success;     // результат операции
    string errorCode; // код ошибки
    string message;   // текст сообщения
};
// проверка карты по алгоритму Луна
bool checkLuhn(const string &cardNumber)
{
    int sum = 0;
    bool doubleDigit = false;
    // проходим по цифрам справа налево
    for (int i = cardNumber.length() - 1; i >= 0; --i)
    {
        int digit = cardNumber[i] - '0';
        if (doubleDigit)
        {
            digit = digit * 2;
            if (digit > 9)
                digit = digit - 9;
        }
        sum += digit;
        doubleDigit = !doubleDigit;
    }
    // карта валидна, если сумма кратна 10
    return (sum % 10 == 0);
}