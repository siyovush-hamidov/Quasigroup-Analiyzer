#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <fstream>
#include <stdexcept>

class Quasigroup {
private:
    std::vector<std::vector<int>> table;  // Таблица Кэли
    int size;  // Размер квазигруппы

public:
    Quasigroup(const std::vector<std::vector<int>>& cayleyTable) {
        table = cayleyTable;
        size = table.size();
    }

    int getSize() const {
        return size;
    }

    // Умножение элементов квазигруппы
    int multiply(int x, int y) const {
        if (x >= 0 && x < size && y >= 0 && y < size) {
            return table[x][y];
        }
        throw std::out_of_range("Индекс вне диапазона таблицы Кэли");
    }

    bool isSimple() const {
        std::set<int> checked;
        for (int x = 0; x < size; x++) {
            if (checked.count(x) == 0) {
                std::unordered_set<int> cycle;
                int y = x;
                while (cycle.insert(y).second) {
                    y = multiply(y, y);
                }
                checked.insert(cycle.begin(), cycle.end());
                if (hasProperSubquasigroup(x)) {
                    return false;
                }
            }
        }
        return true;
    }

private:
    bool hasProperSubquasigroup(int x) const {
        std::unordered_set<int> subquasigroup;
        subquasigroup.insert(x);
        int fx = multiply(x, x);
        subquasigroup.insert(fx);
        bool changed = true;

        while (changed) {
            changed = false;
            std::vector<int> elements(subquasigroup.begin(), subquasigroup.end());
            for (int i = 0; i < elements.size(); i++) {
                for (int j = 0; j < elements.size(); j++) {
                    int product = multiply(elements[i], elements[j]);
                    if (subquasigroup.find(product) == subquasigroup.end()) {
                        subquasigroup.insert(product);
                        changed = true;
                        if (subquasigroup.size() > size / 2) {
                            return false;
                        }
                    }
                }
            }
        }
        return (subquasigroup.size() > 1 && subquasigroup.size() < size);
    }
};

// Чтение таблицы Кэли из файла
std::vector<std::vector<int>> readCayleyTableFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Не удалось открыть файл");
    }

    int n;
    file >> n;
    std::vector<std::vector<int>> table(n, std::vector<int>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file >> table[i][j];
        }
    }
    return table;
}

// Чтение таблицы Кэли вручную
std::vector<std::vector<int>> readCayleyTable() {
    int n;
    std::cout << "\nВведите порядок квазигруппы: ";
    std::cin >> n;

    std::vector<std::vector<int>> table(n, std::vector<int>(n));  
    std::cout << "\nВведите таблицу Кэли (" << n << "x" << n << " элементы):" << std::endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            std::cout << "Элемент (" << i << ", " << j << "): ";
            std::cin >> table[i][j];
        }
    }
    
    return table; 
}

// Создание таблицы Кэли для циклической группы
std::vector<std::vector<int>> createCyclicGroup(int n) {
    std::vector<std::vector<int>> table(n, std::vector<int>(n));  
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            table[i][j] = (i + j) % n;  // Операция сложения по модулю n
        }
    }
    
    return table;
}

void printCayleyTable(const std::vector<std::vector<int>>& table) {
    int n = table.size();
    std::cout << "\nТаблица Кэли (" << n << "x" << n << "):\n";
    
    // Вывод заголовка
    std::cout << "  | ";
    for (int j = 0; j < n; j++) {
        std::cout << j << " ";
    }
    std::cout << "\n--+-";
    for (int j = 0; j < n; j++) {
        std::cout << "--";
    }
    std::cout << std::endl;
    
    // Вывод таблицы
    for (int i = 0; i < n; i++) {
        std::cout << i << " | ";
        for (int j = 0; j < n; j++) {
            std::cout << table[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "Выберите способ ввода: (1 - файл, 2 - вручную, 3 - генерация циклической группы): ";
    int choice;
    std::cin >> choice;

    std::vector<std::vector<int>> cayleyTable;

    try {
        if (choice == 1) {
            std::string filename;
            std::cout << "Введите имя файла с таблицей Кэли: ";
            std::cin >> filename;
            cayleyTable = readCayleyTableFromFile(filename);
        } else if (choice == 2) {
            cayleyTable = readCayleyTable();
        } else if (choice == 3) {
            int n;
            std::cout << "Введите порядок циклической группы: ";
            std::cin >> n;
            cayleyTable = createCyclicGroup(n);
        } else {
            std::cout << "Неверный выбор" << std::endl;
            return 1;
        }
        
        Quasigroup q(cayleyTable);
        printCayleyTable(cayleyTable);

        if (q.isSimple()) {
            std::cout << "Квазигруппа не содержит нетривиальных подквазигрупп." << std::endl;
        } else {
            std::cout << "Квазигруппа содержит нетривиальные подквазигруппы." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    return 0;
}
