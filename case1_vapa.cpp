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
// сохраняем id заказа в файл
void saveOrderId(const string& orderId) {
    ofstream db("orders_db.txt", ios::app);
    if (db.is_open()) db << orderId << endl;
}

// запись в csv
void logToCSV(const PaymentRequest& req, const PaymentResponse& resp) {
    ofstream csv("payments_out.csv", ios::app);
    if (csv.is_open()) {
        csv << req.orderId << "," << req.amount << "," << req.currency << "," 
            << (resp.success ? "SUCCESS" : "FAIL") << "," << resp.errorCode << "\n";
    }
}

int main() {
    // устанавливаем русскую локаль для корректного вывода
    setlocale(LC_ALL, "Russian");
    srand(time(NULL));
    double sum1 = 15400.0;
    
    PaymentRequest req;
    req.recipient = "MED_SERVICE";
    req.currency = "RUB";
    req.isConfirmed = false;

    cout << "=== Перевод с карты ===\n\n";
    double sum2 = 0.0;
    string input;

    // генерируем id заказа из времени
    req.orderId = "order_" + to_string(time(nullptr)); 
    
    // проверка на дубль
    if (isDuplicateOrder(req.orderId)) {
        cout << "Ошибка: Дубликат заказа (HTTP 409 Conflict)\n";
        return 1;
    }

    cout<< "Выберите услугу!";
    cout<< "1)Анализы";
    cout<<" 2500.0 RUB\n";
    cout<< "2)МРТ";
    cout<<" 8000.0 RUB\n";
    cout<< "3)Консультация";
    cout<<" 1000.0 RUB\n";
    cout<< "4)Другое\n";
    cout<<"Ваш выбор :";


    int choice;
    cin>>choice;
    cin.ignore();
    switch(choice){
        case 1:
            req.purpose = "Анализы";
            req.amount = 2500.0;
            sum2=2500.0;
            break;
        case 2:
            req.purpose = "МРТ";
            req.amount = 8000.0;
            sum2=8000.0;
            break;
        case 3:
            req.purpose = "Консультация";
            req.amount = 1000.0;
            sum2=1000.0;
            break;
        case 4:
            req.purpose = "Другое";
            break;
            default:
        cout<<"Неверный ввод. Будет выбрано 'Другое' ";
        req.purpose="Другое";
    }



    // ввод данных карты
    cout << "\nВведите данные карты (3 элемента):\n";
    while(true){
        cout << "1) Номер карты: ";
        getline(cin, req.cardNumber);
        // проверяем длину номера карты
        if (req.cardNumber.length() >= 13 && req.cardNumber.length() <= 19){
            bool onlydigits = true;
            // проверяем, что все символы - цифры
            for (char c: req.cardNumber){
                if (!isdigit(c)){
                    onlydigits = false;
                    break;
                }
            }
            if (!onlydigits){
                cout<< "Ошибка.Номер карты должен содержать только цифры.\n" ;
                continue;
            }    
            // проверяем карту по алгоритму Луна
            if (!checkLuhn(req.cardNumber)){
                cout<<"Ошибка.Карта не действительна.\n" ;
                continue;
            }
            break;
        } else {
            cout<<"Ошибка при вводе данных.\n";
        }
    }

    // срок действия
    while(true){
    cout << "2) Срок действия (ММ/ГГ).\n Два числа через пробел: ";
    getline(cin, req.expiryDate);
    // ожидаем формат из 5 символов
    if (req.expiryDate.length() == 5) {
        int year = stoi(req.expiryDate.substr(3,2));
        int month = stoi(req.expiryDate.substr(0,2));
        if (req.expiryDate.find_first_not_of("0123456789 ") != string::npos) {
            cout << "Ошибка!Cрок действия должен содержать только цифры." << endl;
            continue;
        }
        // проверяем корректность месяца
        if (month < 1 || month > 12){
            cout << "Некорректные данные.\n";
            continue; // ошибка валидации
        } else if (year < 26 || (year == 26 && month <= 11)|| (year>36)){
            // проверяем, что срок еще не истек
            cout << "Срок действия карты недействителен.\n";
            return 2;
        }else{
            cout << "Срок действия корректен.\n";
            break;
        }
    }else{
        cout<<"Ошибка при вводе данных.\n";
        return 2;
    }
    }
    // cvv
    while (true){
        cout << "3) CVV-код: ";
        getline(cin, req.cvv);
        // проверяем, что cvv состоит только из цифр
        if (req.cvv.find_first_not_of("0123456789") != string::npos) {
            cout << "Ошибка!CVV должен содержать только цифры." << endl;
            continue;
        }
        // cvv должен быть ровно 3 цифры
        if (req.cvv.length() != 3){
            cout << "Ошибка при вводе данных.\n";
            continue;
        }
        // имитация проверки карты
        cout << "Проверка карты. Подождите пару секунд";
        cout.flush();
        this_thread::sleep_for(chrono::seconds(1));
        cout << " Готово.\n";
        break;
    }
    // ввод суммы
    if ((choice==4)||choice<1 || choice>4){
        while(true){
            cout<<"Введите название услуги : ";
            getline(cin , req.purpose);
            if (req.purpose.empty()){
                cout<< "Ошибка! Название не может быть пустым";
                continue;
            }
            break;
        }
    
        while(true){
        cout << "\nВведите сумму перевода: ";
        getline(cin, input);
        // допускаем только цифры, запятую и точку
        if (input.find_first_not_of("0123456789,.") != string::npos) {
            cout << "Ошибка! Сумма должна содержать только цифры." << endl;
            continue;
        }
        // заменяем запятую на точку для корректного парсинга
        for (char& c : input) { if (c == ',') c = '.'; }
         size_t dotPos = input.find('.');
        if (dotPos != string::npos) {
            // Считаем, сколько символов находится после точки
            int decimals = input.length() - dotPos - 1;
            // Если знаков больше 2 — выдаем ошибку
            if (decimals > 2) {
                cout << "Ошибка! Нельзя вводить больше 2 знаков после запятой (копеек)." << endl;
                continue; // Просим ввести заново
            
            }
        }
        try {
            sum2 = stod(input); 
            // проверяем допустимый диапазон суммы
            if (sum2 > 0 && sum2 <= 1000000) {
                req.amount = sum2;
             break; 
            } else {
                cout << "Ошибка! Сумма должна быть от 0 до 1 000 000.\n";
            } 
        } catch (...) {
            cout << "Ошибка! Некорректный ввод.\n";
        }
    }
    }else{
        cout<<"К оплате : "<<req.amount<<"RUB("<<req.purpose<<")\n";
    }

    

    // код подтверждения
    bool confirmed = true;
    if (req.amount > 10000.0) {
        req.isConfirmed = false; // по умолчанию не подтверждено
        int randomCode = rand() % 9000 + 1000;
        string confirmCode = to_string(randomCode);
        cout << "\nСумма перевода превышает 10000 RUB. Требуется подтверждение.\n";
        cout << "[код из смс]: " << confirmCode << "\n";
        cout << "Введите код из смс: "; 
        string userCode;
        getline(cin, userCode);
        // сравниваем введенный код с сгенерированным
        if (userCode != confirmCode) {
            confirmed = false;
            cout << "\nНеверный код подтверждения.\n";
        } else {
            confirmed = true;
            cout << "\nКод подтверждения принят.\n";
        }
    }
    req.isConfirmed = confirmed;

    // отправка в банк
    cout << "\nОтправка запроса в банк. Подождите пару секунд...\n";
    this_thread::sleep_for(chrono::seconds(1));
    
    PaymentResponse resp = processPayment(req);
    // сохраняем id заказа (если успех)
    if (resp.success) saveOrderId(req.orderId);
    // пишем в csv аудит
    logToCSV(req, resp);
    // вывод результата и коды выхода
    // берем последние 4 цифры номера карты для маскирования
    string last_four = req.cardNumber.substr(req.cardNumber.length() - 4);
    
    if (resp.success) {
        cout << "\n" << resp.message << "\n";
        cout << "Вывод: " << req.purpose << ", карта ****"<< last_four << ". " << req.amount << " " << req.currency << ".\n";
        cout << req.recipient << ".\n";
        cout << "Остаток: " << sum1 - req.amount << " RUB.\n";
        cout << "Ваш идентификатор заказа: " << req.orderId << "\n";
        return 0; // успех
    } else {
        cout << "\nОтказ.\n";
        cout << "Код ошибки: " << resp.errorCode << "\n";
        cout << "Причина: " << resp.message << "\n";
        
        // таймаут возвращает отдельный код выхода
        if (resp.errorCode == "TIMEOUT") {
            return 3; // код выхода 3 - таймаут
        } else {
            return 1; // код выхода 1 - отказ банка
        }
    }
}

