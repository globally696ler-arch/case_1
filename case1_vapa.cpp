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

// эмулятор банка
PaymentResponse processPayment(const PaymentRequest& req) {
    PaymentResponse resp;

    // считаем "хэш" от orderId (сумма кодов символов)
    int hash = 0;
    for (char c : req.orderId) {
        hash += (int)c;
    }
    int scenario = hash % 10; // 0..9

    // проверка на таймаут
    if (scenario == 9) {
        resp.success = false;
        resp.errorCode = "TIMEOUT";
        resp.message = "Превышено время ожидания ответа от банка.";
        return resp;
    }
// рассмотрение ситуации таймаута

// проверка подтверждения (если сумма > 10000)
    if (req.amount > 10000.0 && !req.isConfirmed) {
        resp.success = false;
        resp.errorCode = "NOT_CONFIRMED";
        resp.message = "Операция не подтверждена.";
        return resp;
    }

    // проверка на недостаток средств
    double sum1 = 15400.0;
    if (req.amount > sum1 || scenario == 7) {
        resp.success = false;
        resp.errorCode = "INSUFFICIENT_FUNDS";
        resp.message = "Недостаточно средств.";
        return resp;
    }
// рассмотрение других сценариев
// сценарий отклонения карты 
    if (scenario == 8) {
        resp.success = false;
        resp.errorCode = "CARD_DECLINED";
        resp.message = "Карта отклонена банком.";
        return resp;
    }

    // сценарий успеха
    resp.success = true;
    resp.message = "Банк одобрил операцию.";
    return resp;
}
// защита от повторов
bool isDuplicateOrder(const string& orderId) {
    ifstream db("orders_db.txt");
    if (!db.is_open()) return false;
    string line;
    // ищем orderId среди сохраненных
    while (getline(db, line)) {
        if (line == orderId) {
            db.close();
            return true;
        }
    }
    db.close();
    return false;
}
