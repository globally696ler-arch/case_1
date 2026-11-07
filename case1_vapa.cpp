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
