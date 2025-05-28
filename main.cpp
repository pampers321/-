#include <iostream>
#include <string>
#include <cctype>
#include <windows.h> 

/*───────────────────────────────────────────────────────────────*/
/*   СТРУКТУРЫ ДАННЫХ                                            */
/*───────────────────────────────────────────────────────────────*/

// Узел списка: одна цифра и указатель на следующий узел
struct DigitNode {
    int digit;              // 0...9
    DigitNode* next = nullptr;
    explicit DigitNode(int d) : digit(d) {}
};

// Свой список цифр (LSB-порядок)
class DigitList {
public:
    DigitList() = default;
    ~DigitList() { clear(); }

    // добавить цифру в КОНЕЦ (то есть к самым старшим разрядам)
    void push_back(int d) {
        DigitNode* node = new DigitNode(d);
        if (!head) head = tail = node;
        else { tail->next = node; tail = node; }
        _size++;
    }

    // последняя (самая старшая) цифра
    int back() const { return tail ? tail->digit : 0; }

    // удалить последнюю цифру
    void pop_back() {
        if (!_size) return;
        if (head == tail) { delete head; head = tail = nullptr; }
        else {
            DigitNode* cur = head;
            while (cur->next != tail) cur = cur->next;
            delete tail;
            tail = cur;
            tail->next = nullptr;
        }
        --_size;
    }

    std::size_t size() const { return _size; }

    DigitNode* begin() const { return head; }

    void clear() {
        while (head) { DigitNode* tmp = head; head = head->next; delete tmp; }
        tail = nullptr; _size = 0;
    }

private:
    DigitNode* head = nullptr;        // младший разряд
    DigitNode* tail = nullptr;        // старший разряд
    std::size_t _size = 0;
};

/*───────────────────────────────────────────────────────────────*/
/*   ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ                                     */
/*───────────────────────────────────────────────────────────────*/

// Удалить ведущие нули (они в КОНЦЕ списка)
void trimZeros(DigitList& num) {
    while (num.size() > 1 && num.back() == 0)
        num.pop_back();
}

// Строка -> DigitList   (игнорируем не-цифры)
DigitList toDigits(const std::string& s) {
    DigitList num;
    for (auto it = s.rbegin(); it != s.rend(); ++it)
        if (std::isdigit(*it)) num.push_back(*it - '0');
    if (num.size() == 0) num.push_back(0);
    trimZeros(num);
    return num;
}

// DigitList -> строка
std::string toString(const DigitList& num) {
    std::string out;
    char buf[2]{};
    // проходим задом наперёд: сначала нужен tail
    std::string tmp;
    for (DigitNode* p = num.begin(); p; p = p->next)
        tmp.push_back(char('0' + p->digit));
    for (auto it = tmp.rbegin(); it != tmp.rend(); ++it) out.push_back(*it);
    return out;
}

// |a| ? |b|   (-1,0,+1)
int absCompare(const DigitList& a, const DigitList& b) {
    if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;

    /* чтобы сравнить слева направо, перегоняем цифры во временный буфер */
    std::string sa = toString(a), sb = toString(b);
    if (sa == sb) return 0;
    return sa < sb ? -1 : 1;
}

/*───────────────────────────────────────────────────────────────*/
/*   АРИФМЕТИКА                                                 */
/*───────────────────────────────────────────────────────────────*/

// a = a + b   (оба неотрицательны)
void add(DigitList& a, const DigitList& b) {
    DigitNode* pa = a.begin();
    DigitNode* pb = b.begin();
    int carry = 0;
    DigitList result;

    while (pa || pb || carry) {
        int sum = carry;
        if (pa) { sum += pa->digit; pa = pa->next; }
        if (pb) { sum += pb->digit; pb = pb->next; }
        result.push_back(sum % 10);
        carry = sum / 10;
    }
    a = std::move(result); 
    trimZeros(a);         // заменить результатом
}

// a = a - b   (|a| >= |b|, оба >= 0)
void sub(DigitList& a, const DigitList& b) {
    DigitNode* pa = a.begin();
    DigitNode* pb = b.begin();
    int borrow = 0;
    DigitList result;

    while (pa) {
        int diff = pa->digit - borrow - (pb ? pb->digit : 0);
        if (diff < 0) { diff += 10; borrow = 1; } else borrow = 0;
        result.push_back(diff);
        pa = pa->next;
        if (pb) pb = pb->next;
    }
    trimZeros(result);
    a = std::move(result);
}

/*───────────────────────────────────────────────────────────────*/
/*   ДЕМО                                                       */
/*───────────────────────────────────────────────────────────────*/
int main() {
    SetConsoleOutputCP(65001); 
    std::cout << "Введите выражение (A + B | A - B):\n";
    std::string s1, s2;
    char op;
    if (!(std::cin >> s1 >> op >> s2)) return 0;

    // вычленяем знаки
    bool neg1 = false, neg2 = false;
    if (s1[0]=='+'||s1[0]=='-') { neg1 = (s1[0]=='-'); s1.erase(0,1); }
    if (s2[0]=='+'||s2[0]=='-') { neg2 = (s2[0]=='-'); s2.erase(0,1); }

    // конвертируем в списки цифр
    DigitList a = toDigits(s1);
    DigitList b = toDigits(s2);

    // для операции '-' инвертируем знак второго
    if (op == '-') neg2 = !neg2;
    if (op!='+' && op!='-') {
        std::cerr << "Поддерживаются только + и -\n";
        return 1;
    }

    // общий префикс вывода
    const char* label = (op=='+' ? "Сумма = " : "Разность = ");

    if (neg1 == neg2) {
        // одинаковые знаки -> сложение модулей
        add(a, b);
        std::cout << label;
        if (neg1) std::cout << '-';
        std::cout << toString(a) << '\n';
    } else {
        // разные знаки -> вычитание модулей
        int cmp = absCompare(a, b);
        if (cmp == 0) {
            std::cout << label << "0\n";
        } else if (cmp > 0) {
            sub(a, b);
            std::cout << label;
            if (neg1) std::cout << '-';
            std::cout << toString(a) << '\n';
        } else {
            sub(b, a);
            std::cout << label;
            if (neg2) std::cout << '-';
            std::cout << toString(b) << '\n';
        }
    }
}
