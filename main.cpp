#include <iostream>
#include <vector>
#include <unordered_set>
#include <set>
#include <fstream>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <tuple>
#include <unordered_map>

// Предоставляет глобальный генератор случайных чисел для единообразной рандомизации.
// Обеспечивает потокобезопасную инициализацию с использованием статического экземпляра.
// Возвращает: Ссылку на генератор Mersenne Twister.
static std::mt19937 &getRandomNumberGenerator()
{
    static std::mt19937 generator(std::random_device{}());
    return generator;
}

// Выбирает случайный элемент из множества, используется в алгоритмах генерации.
// Шаблонный параметр Set: Контейнер с методами begin(), end(), size() (например, std::unordered_set).
// Параметр set: Множество для выбора.
// Возвращает: Случайно выбранный элемент.
// Выбрасывает: Неопределенное поведение, если множество пустое.
template <class Set>
auto selectRandomElement(const Set &set)
{
    auto iterator = set.begin();
    std::uniform_int_distribution<std::size_t> distribution(0, set.size() - 1);
    std::advance(iterator, distribution(getRandomNumberGenerator()));
    return *iterator;
}

// Вычисляет наибольший общий делитель (НОД) двух чисел с помощью алгоритма Евклида.
// Используется для проверки взаимной простоты коэффициентов аффинной квазигруппы с модулем.
// Параметры:
//   firstNumber, secondNumber: Числа для вычисления НОД.
// Возвращает: НОД двух чисел.
int computeGreatestCommonDivisor(int firstNumber, int secondNumber)
{
    while (secondNumber)
    {
        firstNumber %= secondNumber;
        std::swap(firstNumber, secondNumber);
    }
    return firstNumber;
}

// Представляет конечную квазигруппу — алгебраическую структуру с бинарной операцией, образующей латинский квадрат.
// Хранит таблицу Кэли и предоставляет методы для проверки собственных и нетривиальных подквазигрупп.
class Quasigroup
{
    std::vector<std::vector<int>> cayleyTable; // Таблица Кэли, хранящая операцию квазигруппы.
    int order;                                 // Порядок (размер) квазигруппы.

public:
    // Конструирует квазигруппу из заданной таблицы Кэли.
    // Параметр cayleyTable: Квадратная матрица, задающая операцию квазигруппы.
    // Инициализирует порядок на основе размера таблицы.
    explicit Quasigroup(const std::vector<std::vector<int>> &cayleyTable)
        : cayleyTable(cayleyTable), order(static_cast<int>(cayleyTable.size())) {}

    // Вычисляет результат операции квазигруппы для двух элементов.
    // Параметры:
    //   firstElement, secondElement: Индексы элементов (от 0 до order-1).
    // Возвращает: Результат операции (firstElement * secondElement) из таблицы Кэли.
    // Выбрасывает: std::out_of_range при некорректных индексах.
    int applyOperation(int firstElement, int secondElement) const
    {
        if (firstElement >= 0 && firstElement < order && secondElement >= 0 && secondElement < order)
        {
            return cayleyTable[firstElement][secondElement];
        }
        throw std::out_of_range("Индекс вне диапазона таблицы Кэли");
    }

    // Проверяет наличие подквазигрупп (собственных или нетривиальных).
    // Параметр checkForProperSubquasigroups:
    //   - true: Проверяет собственные подквазигруппы (размер < порядок).
    //   - false: Проверяет нетривиальные подквазигруппы (размер > 1).
    // Возвращает: true, если подквазигруппа указанного типа существует, false — иначе.
    // Использует циклический подход для генерации начальных множеств подквазигрупп.
    bool hasSubquasigroups(bool checkForProperSubquasigroups) const
    {
        std::vector<bool> visitedElements(order, false);
        for (int startElement = 0; startElement < order; ++startElement)
        {
            if (!visitedElements[startElement])
            {
                std::unordered_set<int> cycleElements;
                int currentElement = startElement;
                while (cycleElements.insert(currentElement).second)
                {
                    visitedElements[currentElement] = true;
                    currentElement = applyOperation(currentElement, currentElement);
                }
                if (checkForProperSubquasigroups)
                {
                    if (verifyProperSubquasigroup(cycleElements))
                    {
                        return true;
                    }
                }
                else
                {
                    if (verifyNonTrivialSubquasigroup(cycleElements))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

private:
    // Проверяет, порождает ли начальное множество собственную подквазигруппу (размер < порядок).
    // Параметр seedSet: Начальное множество элементов для проверки замыкания.
    // Возвращает: true, если порожденное множество — собственная подквазигруппа, false — иначе.
    // Итеративно замыкает множество под операцией, останавливаясь при размере > порядок/2.
    bool verifyProperSubquasigroup(const std::unordered_set<int> &seedSet) const
    {
        auto closedSet = seedSet;
        bool setChanged = true;
        while (setChanged)
        {
            setChanged = false;
            std::vector<int> currentElements(closedSet.begin(), closedSet.end());
            for (int firstElement : currentElements)
            {
                for (int secondElement : currentElements)
                {
                    int result = applyOperation(firstElement, secondElement);
                    if (closedSet.insert(result).second)
                    {
                        setChanged = true;
                        if (static_cast<int>(closedSet.size()) > order / 2)
                        {
                            return false;
                        }
                    }
                }
            }
        }
        return static_cast<int>(closedSet.size()) < order;
    }

    // Проверяет, порождает ли начальное множество нетривиальную подквазигруппу (размер > 1).
    // Параметр seedSet: Начальное множество элементов для проверки замыкания.
    // Возвращает: true, если порожденное множество нетривиально, false — иначе.
    // Итеративно замыкает множество под операцией квазигруппы.
    bool verifyNonTrivialSubquasigroup(const std::unordered_set<int> &seedSet) const
    {
        auto closedSet = seedSet;
        if (closedSet.size() == 1)
        {
            return false;
        }
        bool setChanged = true;
        while (setChanged)
        {
            setChanged = false;
            std::vector<int> currentElements(closedSet.begin(), closedSet.end());
            for (int firstElement : currentElements)
            {
                for (int secondElement : currentElements)
                {
                    int result = applyOperation(firstElement, secondElement);
                    if (closedSet.insert(result).second)
                    {
                        setChanged = true;
                    }
                }
            }
        }
        return static_cast<int>(closedSet.size()) > 1;
    }
};

// Читает таблицу Кэли из файла для создания квазигруппы.
// Параметр fileName: Путь к входному файлу.
// Формат: Первая строка — порядок n, затем n x n целых чисел.
// Возвращает: Вектор векторов, представляющий таблицу Кэли.
// Выбрасывает: std::runtime_error, если файл не удалось открыть.
std::vector<std::vector<int>> readCayleyTableFromFile(const std::string &fileName)
{
    std::ifstream file(fileName);
    if (!file)
    {
        throw std::runtime_error("Не удалось открыть файл");
    }
    int order;
    file >> order;
    std::vector<std::vector<int>> cayleyTable(order, std::vector<int>(order));
    for (int row = 0; row < order; ++row)
    {
        for (int column = 0; column < order; ++column)
        {
            file >> cayleyTable[row][column];
        }
    }
    return cayleyTable;
}

// Читает таблицу Кэли из стандартного ввода, запрашивая значения у пользователя.
// Запрашивает порядок и каждый элемент таблицы, проверяя корректность ввода.
// Возвращает: Вектор векторов, представляющий таблицу Кэли.
std::vector<std::vector<int>> readCayleyTableFromStandardInput()
{
    int order;
    std::cout << "\nВведите порядок квазигруппы: ";
    std::cin >> order;
    std::vector<std::vector<int>> cayleyTable(order, std::vector<int>(order));
    std::cout << "\nВведите таблицу Кэли (" << order << "x" << order << "):\n";
    for (int row = 0; row < order; ++row)
    {
        for (int column = 0; column < order; ++column)
        {
            std::cout << "(" << row << "," << column << "): ";
            std::cin >> cayleyTable[row][column];
            while (cayleyTable[row][column] > order - 1)
            {
                std::cout << "Элементы должны быть меньше " << order << "\nВведите снова: \n";
                std::cout << "(" << row << "," << column << "): ";
                std::cin >> cayleyTable[row][column];
            }
        }
    }
    return cayleyTable;
}

// Генерирует таблицу Кэли для циклической группы порядка n.
// Операция: x * y = (x + y) mod n.
// Параметр order: Размер группы.
// Возвращает: Вектор векторов, представляющий таблицу Кэли.
std::vector<std::vector<int>> generateCyclicGroupCayleyTable(int order)
{
    std::vector<std::vector<int>> cayleyTable(order, std::vector<int>(order));
    for (int row = 0; row < order; ++row)
    {
        for (int column = 0; column < order; ++column)
        {
            cayleyTable[row][column] = (row + column) % order;
        }
    }
    return cayleyTable;
}

// Генерирует случайную перестановку чисел от 0 до n-1.
// Используется в генерации аффинной квазигруппы для функции f.
// Параметр order: Размер перестановки.
// Возвращает: Вектор, представляющий перестановку.
std::vector<int> generateRandomPermutation(int order)
{
    std::vector<int> permutation(order);
    std::iota(permutation.begin(), permutation.end(), 0);
    std::shuffle(permutation.begin(), permutation.end(), getRandomNumberGenerator());
    return permutation;
}

// Генерирует таблицу Кэли для аффинной квазигруппы.
// Операция: x * y = (alpha * x + beta * f(y) + c) mod n, где f — перестановка.
// Параметр order: Размер квазигруппы.
// Запрашивает alpha, beta (должны быть взаимно простыми с n) и c (0 <= c < n).
// Возвращает: Вектор векторов, представляющий таблицу Кэли.
std::vector<std::vector<int>> generateAffineQuasigroupCayleyTable(int order)
{
    int coefficientAlpha, coefficientBeta, constantC;
    std::cout << "Введите коэффициент alpha (должен быть взаимно простым с " << order << "): ";
    std::cin >> coefficientAlpha;
    while (computeGreatestCommonDivisor(coefficientAlpha, order) != 1)
    {
        std::cout << "alpha должен быть взаимно простым с " << order << ". Введите снова: ";
        std::cin >> coefficientAlpha;
    }
    std::cout << "Введите коэффициент beta (должен быть взаимно простым с " << order << "): ";
    std::cin >> coefficientBeta;
    while (computeGreatestCommonDivisor(coefficientBeta, order) != 1)
    {
        std::cout << "beta должен быть взаимно простым с " << order << ". Введите снова: ";
        std::cin >> coefficientBeta;
    }
    std::cout << "Введите константу c (0 <= c < " << order << "): ";
    std::cin >> constantC;
    while (constantC < 0 || constantC >= order)
    {
        std::cout << "c должно быть в диапазоне [0, " << order - 1 << "]. Введите снова: ";
        std::cin >> constantC;
    }

    auto permutationFunction = generateRandomPermutation(order);
    std::cout << "Сгенерированная перестановка f: ";
    for (int value : permutationFunction)
    {
        std::cout << value << ' ';
    }
    std::cout << "\n";

    std::vector<std::vector<int>> cayleyTable(order, std::vector<int>(order));
    for (int row = 0; row < order; ++row)
    {
        for (int column = 0; column < order; ++column)
        {
            cayleyTable[row][column] = (coefficientAlpha * row + coefficientBeta * permutationFunction[column] + constantC) % order;
        }
    }
    return cayleyTable;
}

// Генерирует таблицу Кэли с помощью метода последовательного графа замен.
// Создает латинский квадрат, последовательно заполняя строки с учетом доступных символов.
// Параметр order: Размер квазигруппы.
// Возвращает: Вектор векторов, представляющий таблицу Кэли.
class SequentialReplacementGraphGenerator
{
    int order;                                               // Порядок квазигруппы.
    std::vector<std::vector<int>> cayleyTable;               // Таблица Кэли, изначально заполнена -1.
    std::vector<std::unordered_set<int>> availableInColumns; // Доступные символы для каждого столбца.
    std::unordered_set<int> availableSymbols;                // Все возможные символы (0 до order-1).

public:
    // Инициализирует генератор для квазигруппы заданного порядка.
    // Параметр order: Размер квазигруппы.
    // Заполняет множество символов и доступные символы для столбцов.
    explicit SequentialReplacementGraphGenerator(int order)
        : order(order), cayleyTable(order, std::vector<int>(order, -1)), availableInColumns(order), availableSymbols()
    {
        for (int symbol = 0; symbol < order; ++symbol)
        {
            availableSymbols.insert(symbol);
            for (int column = 0; column < order; ++column)
            {
                availableInColumns[column].insert(symbol);
            }
        }
    }

    // Генерирует таблицу Кэли, заполняя строки с помощью метода графа замен.
    // Возвращает: Полностью сформированную таблицу Кэли.
    std::vector<std::vector<int>> generate()
    {
        for (int row = 0; row < order; ++row)
        {
            cayleyTable[row] = generateRow();
        }
        return cayleyTable;
    }

private:
    // Создает снимок текущих доступных символов в столбцах для использования в графе замен.
    // Возвращает: Вектор векторов, представляющий доступные символы для каждого столбца.
    std::vector<std::vector<int>> snapshotColumns() const
    {
        std::vector<std::vector<int>> snapshot(order);
        for (int column = 0; column < order; ++column)
        {
            snapshot[column].assign(availableInColumns[column].begin(), availableInColumns[column].end());
        }
        return snapshot;
    }

    // Генерирует одну строку таблицы Кэли, выбирая символы, чтобы сохранить свойства латинского квадрата.
    // Возвращает: Вектор, представляющий строку таблицы.
    std::vector<int> generateRow()
    {
        std::unordered_set<int> availableInRow = availableSymbols;
        std::vector<std::unordered_set<int>> initialAvailable = availableInColumns;
        std::vector<int> row;
        row.reserve(order);
        int currentColumn = 0;
        while (currentColumn < order)
        {
            std::unordered_set<int> available = availableInColumns[currentColumn];
            std::unordered_set<int> validSymbols;
            for (int symbol : available)
            {
                if (availableInRow.count(symbol))
                {
                    validSymbols.insert(symbol);
                }
            }
            if (!validSymbols.empty())
            {
                int selectedSymbol = selectRandomElement(validSymbols);
                availableInColumns[currentColumn].erase(selectedSymbol);
                availableInRow.erase(selectedSymbol);
                row.push_back(selectedSymbol);
                ++currentColumn;
            }
            else
            {
                auto replacementGraph = constructReplacementGraph(row, currentColumn, initialAvailable);
                int selectedElement = selectRandomElement(availableInColumns[currentColumn]);
                makeElementAvailable(selectedElement, replacementGraph, row, currentColumn, availableInRow);
            }
        }
        return row;
    }

    // Тип данных для представления графа замен, используемого при генерации строки.
    using ReplacementGraph = std::unordered_map<int, std::unordered_set<int>>;

    // Строит граф замен для текущей строки и столбца.
    // Параметры:
    //   row: Текущая частично заполненная строка.
    //   currentColumn: Текущий столбец для заполнения.
    //   initialAvailable: Начальные доступные символы для столбцов.
    // Возвращает: Граф замен, где ключи — индексы столбцов, значения — доступные символы.
    ReplacementGraph constructReplacementGraph(const std::vector<int> &row, int currentColumn,
                                               const std::vector<std::unordered_set<int>> &initialAvailable)
    {
        ReplacementGraph graph;
        for (int column = currentColumn; column >= 0; --column)
        {
            if (!initialAvailable[column].empty())
            {
                graph[column] = initialAvailable[column];
            }
        }
        return graph;
    }

    // Делает элемент доступным в текущем столбце, обновляя строку и граф замен.
    // Параметры:
    //   oldElement: Элемент, который нужно сделать доступным.
    //   graph: Граф замен для текущей строки.
    //   row: Текущая строка.
    //   currentColumn: Текущий столбец.
    //   availableInRow: Доступные символы для строки.
    void makeElementAvailable(int oldElement, ReplacementGraph &graph, std::vector<int> &row,
                              int currentColumn, std::unordered_set<int> &availableInRow)
    {
        int initialElement = oldElement;
        eraseInitialElementFromGraph(graph, initialElement);
        int oldIndex = std::find(row.begin(), row.end(), oldElement) - row.begin();
        int newIndex;
        std::unordered_set<int> visitedPath;
        while (true)
        {
            auto &choices = graph[oldIndex];
            std::unordered_set<int> availableChoices = choices;
            for (int pathElement : visitedPath)
            {
                availableChoices.erase(pathElement);
            }
            if (availableChoices.empty())
            {
                visitedPath.clear();
                availableChoices = choices;
            }
            int newElement = selectRandomElement(availableChoices);
            newIndex = std::find(row.begin(), row.end(), newElement) - row.begin();
            row[oldIndex] = newElement;
            visitedPath.insert(newElement);
            if (std::find(row.begin(), row.end(), oldElement) == row.end())
            {
                availableInRow.insert(oldElement);
            }
            availableInRow.erase(newElement);
            availableInColumns[oldIndex].insert(oldElement);
            availableInColumns[oldIndex].erase(newElement);
            if (availableInRow.count(initialElement) && (newIndex >= static_cast<int>(row.size())))
            {
                break;
            }
            if (newIndex >= static_cast<int>(row.size()))
            {
                break;
            }
            oldIndex = newIndex;
            oldElement = newElement;
        }
    }

    // Удаляет начальный элемент из графа замен, чтобы избежать его повторного использования.
    // Параметры:
    //   graph: Граф замен.
    //   initialElement: Элемент для удаления.
    static void eraseInitialElementFromGraph(ReplacementGraph &graph, int initialElement)
    {
        for (auto &[column, symbols] : graph)
        {
            symbols.erase(initialElement);
        }
    }
};

// Создает таблицу Кэли с помощью метода последовательного графа замен.
// Параметр order: Размер квазигруппы.
// Возвращает: Вектор векторов, представляющий таблицу Кэли.
std::vector<std::vector<int>> generateSequentialReplacementGraphCayleyTable(int order)
{
    SequentialReplacementGraphGenerator generator(order);
    return generator.generate();
}

// Проверяет, является ли таблица латинским квадратом.
// Проверяет, что каждый элемент встречается ровно один раз в каждой строке и столбце.
// Параметр table: Таблица для проверки.
// Возвращает: true, если таблица — латинский квадрат, false — иначе.
bool isLatinSquare(const std::vector<std::vector<int>> &table)
{
    int order = table.size();
    for (int row = 0; row < order; ++row)
    {
        std::vector<bool> rowUsed(order, false), columnUsed(order, false);
        for (int column = 0; column < order; ++column)
        {
            int rowValue = table[row][column], columnValue = table[column][row];
            if (rowValue < 0 || rowValue >= order || columnValue < 0 || columnValue >= order ||
                rowUsed[rowValue] || columnUsed[columnValue])
            {
                return false;
            }
            rowUsed[rowValue] = columnUsed[columnValue] = true;
        }
    }
    return true;
}

// Выводит таблицу Кэли в консоль в читаемом формате.
// Параметр table: Таблица Кэли для вывода.
// Форматирует таблицу с заголовками строк и столбцов.
void printCayleyTable(const std::vector<std::vector<int>> &table)
{
    int order = table.size();
    std::cout << "\n  | ";
    for (int column = 0; column < order; ++column)
    {
        std::cout << column << ' ';
    }
    std::cout << "\n--+-";
    for (int column = 0; column < order; ++column)
    {
        std::cout << "--";
    }
    std::cout << "\n";
    for (int row = 0; row < order; ++row)
    {
        std::cout << row << " | ";
        for (int column = 0; column < order; ++column)
        {
            std::cout << table[row][column] << ' ';
        }
        std::cout << "\n";
    }
}

// Сохраняет таблицу Кэли и результаты проверки подквазигрупп в файл.
// Параметры:
//   table: Таблица Кэли.
//   quasigroup: Квазигруппа для проверки подквазигрупп.
//   fileName: Имя файла для записи.
// Записывает порядок, таблицу и результаты проверки.
void writeResultsToFile(const std::vector<std::vector<int>> &table, const Quasigroup &quasigroup,
                        const std::string &fileName)
{
    std::ofstream file(fileName);
    if (!file)
    {
        throw std::runtime_error("Не удалось открыть файл для записи");
    }
    int order = table.size();
    file << order << "\n";
    for (const auto &row : table)
    {
        for (int value : row)
        {
            file << value << ' ';
        }
        file << "\n";
    }
    bool hasProperSubquasigroups = quasigroup.hasSubquasigroups(true);
    bool hasNonTrivialSubquasigroups = quasigroup.hasSubquasigroups(false);
    file << "\nРезультаты проверки:\n";
    file << "- Собственные подквазигруппы: " << (hasProperSubquasigroups ? "присутствуют" : "отсутствуют") << "\n";
    file << "- Нетривиальные подквазигруппы: " << (hasNonTrivialSubquasigroups ? "присутствуют" : "отсутствуют") << "\n";
    if (hasProperSubquasigroups && hasNonTrivialSubquasigroups)
    {
        file << "Квазигруппа содержит собственные нетривиальные подквазигруппы.\n";
    }
    else
    {
        file << "Квазигруппа не содержит одновременно собственных и нетривиальных подквазигрупп.\n";
    }
    std::cout << "Результаты сохранены в " << fileName << "\n";
}

// Основная функция программы, предоставляет интерактивный интерфейс для работы с квазигруппами.
// Позволяет пользователю выбирать способы ввода таблицы Кэли, выполнять проверки подквазигрупп и сохранять результаты.
// Управляет основным циклом программы с обработкой ошибок.
int main()
{
    bool exitProgram = false;
    while (!exitProgram)
    {
        std::cout << "Выберите способ ввода:\n"
                  << "1 - Чтение из файла\n"
                  << "2 - Ввод вручную\n"
                  << "3 - Генерация циклической группы\n"
                  << "4 - Генерация аффинной квазигруппы (a * b = (alpha * a + beta * f(b) + c) mod n)\n"
                  << "5 - Генерация методом последовательного графа замен\n"
                  << "6 - Завершить программу\n"
                  << "Выбор: ";
        int choice;
        if (!(std::cin >> choice))
        {
            return 0;
        }
        std::vector<std::vector<int>> cayleyTable;
        try
        {
            if (choice == 1)
            {
                std::string fileName;
                std::cout << "Введите имя файла: ";
                std::cin >> fileName;
                cayleyTable = readCayleyTableFromFile(fileName);
            }
            else if (choice == 2)
            {
                cayleyTable = readCayleyTableFromStandardInput();
            }
            else if (choice == 3)
            {
                int order;
                std::cout << "Введите порядок квазигруппы: ";
                std::cin >> order;
                cayleyTable = generateCyclicGroupCayleyTable(order);
            }
            else if (choice == 4)
            {
                int order;
                std::cout << "Введите порядок квазигруппы: ";
                std::cin >> order;
                cayleyTable = generateAffineQuasigroupCayleyTable(order);
            }
            else if (choice == 5)
            {
                int order;
                std::cout << "Введите порядок квазигруппы: ";
                std::cin >> order;
                cayleyTable = generateSequentialReplacementGraphCayleyTable(order);
            }
            else if (choice == 6)
            {
                exitProgram = true;
                continue;
            }
            else
            {
                continue;
            }
        }
        catch (const std::exception &error)
        {
            std::cerr << "Ошибка: " << error.what() << "\n";
            continue;
        }
        Quasigroup quasigroup(cayleyTable);
        printCayleyTable(cayleyTable);
        bool returnToMainMenu = false;
        bool hasProperSubquasigroups = false, hasNonTrivialSubquasigroups = false;
        std::string outputFileName;
        while (!returnToMainMenu && !exitProgram)
        {
            std::cout << "\nДействия:\n"
                      << "1 - Проверка собственных подквазигрупп\n"
                      << "2 - Проверка нетривиальных подквазигрупп\n"
                      << "3 - Обе проверки\n"
                      << "4 - Сохранение результатов в файл\n"
                      << "5 - Вернуться в главное меню\n"
                      << "6 - Выход\n"
                      << "Выбор: ";
            int action;
            std::cin >> action;
            switch (action)
            {
            case 1:
                hasProperSubquasigroups = quasigroup.hasSubquasigroups(true);
                std::cout << (hasProperSubquasigroups ? "Обнаружена собственная подквазигруппа\n"
                                                      : "Собственные подквазигруппы отсутствуют\n");
                break;
            case 2:
                hasNonTrivialSubquasigroups = quasigroup.hasSubquasigroups(false);
                std::cout << (hasNonTrivialSubquasigroups ? "Обнаружена нетривиальная подквазигруппа\n"
                                                          : "Нетривиальные подквазигруппы отсутствуют\n");
                break;
            case 3:
                hasProperSubquasigroups = quasigroup.hasSubquasigroups(true);
                hasNonTrivialSubquasigroups = quasigroup.hasSubquasigroups(false);
                std::cout << "Собственная подквазигруппа: " << (hasProperSubquasigroups ? "есть\n" : "нет\n")
                          << "Нетривиальная подквазигруппа: " << (hasNonTrivialSubquasigroups ? "есть\n" : "нет\n");
                break;
            case 4:
                std::cout << "Введите имя файла для записи: ";
                std::cin >> outputFileName;
                writeResultsToFile(cayleyTable, quasigroup, outputFileName);
                break;
            case 5:
                returnToMainMenu = true;
                break;
            case 6:
                exitProgram = true;
                break;
            default:
                break;
            }
        }
    }
    return 0;
}