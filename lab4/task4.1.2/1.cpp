#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>
#include <stack>
#include <algorithm>

// Структура для описания перехода автомата
struct Transition {
    int from; // Начальное состояние
    int to; // Конечное состояние
    char symbol; // Символ перехода (или '\0' для ε-переходов)
};


class FiniteAutomaton {
public:
    // Чтение данных об автомате из файла
    void readFromFile(const std::string &filename) {
        std::ifstream file(filename);
        file >> stateCount >> startState >> finalStateCount;

        // Считываем конечные состояния
        for (int i = 0; i < finalStateCount; ++i) {
            int state;
            file >> state;
            finalStates.insert(state);
        }

        // Считываем переходы автомата
        file >> transitionCount;
        transitions.resize(transitionCount);
        for (Transition &t: transitions) {
            file >> t.from >> t.to >> t.symbol;
        }

        // Считываем строки для проверки
        file >> stringCount;
        stringsToCheck.resize(stringCount);
        for (std::string &s: stringsToCheck) {
            file >> s;
        }

        file.close(); // Закрываем файл
    }

    // Запись результатов проверки строк в файл
    void writeToFile(const std::string &filename, const std::vector<std::string> &results) {
        std::ofstream file(filename);
        for (const auto &result: results) {
            file << result << "\n";
        }
        file.close();
    }

    // Основной процесс проверки строк
    void processStrings() {
        std::vector<std::string> results;
        for (const auto &str: stringsToCheck) {
            results.push_back(matches(str) ? "YES" : "NO");
        }
        writeToFile("output.txt", results);
    }

    // Построение автомата на основе регулярного выражения
    void createFromRegex(const std::string &regexStr) {
        // Строим НКА с использованием алгоритма Томпсона
        buildNFAFromRegex(regexStr);

        // Преобразуем НКА в ДКА
        determinize();
    }
private:
    int stateCount = 0, startState = 0, finalStateCount = 0, transitionCount = 0, stringCount = 0;
    std::unordered_set<int> finalStates;
    std::vector<Transition> transitions;
    std::vector<std::string> stringsToCheck;

    // Проверка строки на распознаваемость автоматом
    bool matches(const std::string &str) {
        std::unordered_set<int> currentStates = {startState};
        for (char c: str) {
            // Проходим по каждому символу строки
            std::unordered_set<int> nextStates;
            for (int state: currentStates) {
                for (const auto &t: transitions) {
                    if (t.from == state && t.symbol == c) {
                        nextStates.insert(t.to); // Добавляем новое состояние
                    }
                }
            }
            if (nextStates.empty()) return false; // Нет доступных переходов
            currentStates = nextStates;
        }
        // Проверяем, достиг ли автомат конечного состояния
        for (int state: currentStates) {
            if (finalStates.find(state) != finalStates.end()) {
                return true; // Найдено конечное состояние
            }
        }
        return false;
        // Строка не распознана
    }


    // Построение НКА из регулярного выражения (алгоритм Томпсона)
    void buildNFAFromRegex(const std::string &regexStr) {
        /*
        Алгоритм Томпсона:
        1. Преобразует регулярное выражение в недетерминированный конечный автомат (НКА).
        2. Использует стек для работы с фрагментами автомата, создаваемыми по мере разбора выражения.
        3. Основные операции:
           - **Символ (a)**: Создаёт простейший автомат с одним переходом (от начального к конечному состоянию).
           - **Объединение (|)**: Создаёт новый автомат, который может перейти в два подавтомата (A и B).
           - **Конкатенация (.)**: Последовательно соединяет два подавтомата.
           - **Замыкание (*)**: Создаёт автомат, который может повторять подавтомат любое число раз.
        4. По завершении разбора стек содержит единственный фрагмент, представляющий весь НКА.
        */

        struct NFAFragment {
            int startState; // Начальное состояние фрагмента
            int endState; // Конечное состояние фрагмента
        };

        std::stack<NFAFragment> stack; // Стек для хранения промежуточных фрагментов
        stateCount = 0; // Счётчик состояний

        for (char c: regexStr) {
            if (std::isalpha(c)) {
                // Обычный символ
                int start = stateCount++;
                int end = stateCount++;
                transitions.push_back({start, end, c}); // Создаём переход
                stack.push({start, end});
            } else if (c == '|') {
                // Оператор объединения
                NFAFragment b = stack.top();
                stack.pop();
                NFAFragment a = stack.top();
                stack.pop();

                int start = stateCount++;
                int end = stateCount++;
                transitions.push_back({start, a.startState, '\0'}); // ε-переход
                transitions.push_back({start, b.startState, '\0'});
                transitions.push_back({a.endState, end, '\0'});
                transitions.push_back({b.endState, end, '\0'});
                stack.push({start, end});
            } else if (c == '*') {
                // Замыкание Клини
                NFAFragment a = stack.top();
                stack.pop();

                int start = stateCount++;
                int end = stateCount++;
                transitions.push_back({start, a.startState, '\0'}); // ε-переход
                transitions.push_back({a.endState, end, '\0'});
                transitions.push_back({start, end, '\0'});
                transitions.push_back({a.endState, a.startState, '\0'});
                stack.push({start, end});
            } else if (c == '.') {
                // Конкатенация
                NFAFragment b = stack.top();
                stack.pop();
                NFAFragment a = stack.top();
                stack.pop();

                transitions.push_back({a.endState, b.startState, '\0'}); // Соединяем конец A с началом B
                stack.push({a.startState, b.endState});
            }
        }

        // Итоговый НКА
        NFAFragment result = stack.top();
        stack.pop();

        startState = result.startState;
        finalStates = {result.endState};
    }

    // Детерминация НКА для получения ДКА
    void determinize() {
        // Хэш и сравнение для std::unordered_set
        struct HashSet {
            std::size_t operator()(const std::unordered_set<int> &set) const {
                std::size_t hash = 0;
                for (int elem: set) {
                    hash ^= std::hash<int>()(elem) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                }
                return hash;
            }
        };

        struct EqualSet {
            bool operator()(const std::unordered_set<int> &lhs, const std::unordered_set<int> &rhs) const {
                return lhs == rhs;
            }
        };

        // Множество состояний ДКА
        std::unordered_map<std::unordered_set<int>, int, HashSet, EqualSet> stateMap;
        std::queue<std::unordered_set<int>> stateQueue;


        // Инициализация
        stateCount = 0;
        transitions.clear();
        finalStates.clear();

        // Функция для получения ID состояния
        auto getStateId = [&stateMap, &stateQueue, this](const std::unordered_set<int> &states) {
            if (stateMap.find(states) == stateMap.end()) {
                stateMap[states] = stateCount++;
                stateQueue.push(states);
            }
            return stateMap[states];
        };

        // Начальное состояние ДКА
        std::unordered_set<int> startSet = {startState};
        getStateId(startSet);

        // Обработка очереди состояний
        while (!stateQueue.empty()) {
            auto currentSet = stateQueue.front();
            stateQueue.pop();

            int currentStateId = stateMap[currentSet];

            // Проверяем, является ли текущее множество состояний конечным
            for (int state: currentSet) {
                if (std::find(finalStates.begin(), finalStates.end(), state) != finalStates.end()) {
                    finalStates.insert(currentStateId);
                    break;
                }
            }

            // Собираем переходы для текущего множества состояний
            std::unordered_map<char, std::unordered_set<int>> combinedTransitions;
            for (int state: currentSet) {
                for (const auto &t: transitions) {
                    if (t.from == state) {
                        combinedTransitions[t.symbol].insert(t.to);
                    }
                }
            }

            // Обрабатываем переходы
            for (const auto &entry: combinedTransitions) {
                char symbol = entry.first;
                const std::unordered_set<int> &targetStates = entry.second;

                int newStateId = getStateId(targetStates);
                transitions.push_back({currentStateId, newStateId, symbol});
            }
        }

        // Устанавливаем начальное состояние ДКА
        startState = stateMap[startSet];
    }
};


int main() {
    FiniteAutomaton automaton;

    // Проверка строк из файла
    automaton.readFromFile("input.txt");
    automaton.processStrings();

    return 0;
}
