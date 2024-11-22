/*
НГУ ММФ МКН 3 семестр
Лаборатораня работа по программированию (ООП)
Задание 4, Задача "4.1.2 (усложнённый) ДКА"
*/

#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// Обобщенная хэш-функция для множества произвольного типа
struct SetHash
{
    template <typename T>
    std::size_t operator()(const std::unordered_set<T> &set) const
    {
        std::size_t hash = 0;
        // Комбинируем хэши каждого элемента множества
        for (const auto &elem : set)
        {
            hash ^= std::hash<T>()(elem) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

// Класс для представления конечного автомата (КA)
class FiniteAutomaton
{
    int numStates;                                                                   // Количество состояний
    int startState;                                                                  // Начальное состояние
    std::unordered_set<int> finalStates;                                             // Множество финальных состояний
    std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions; // Переходы

public:
    // Конструктор по умолчанию
    FiniteAutomaton() : numStates(0), startState(0) {}

    // Конструктор с параметрами
    FiniteAutomaton(const int numStates,
                    const int startState,
                    std::unordered_set<int> finalStates,
                    std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions)
        : numStates(numStates),
          startState(startState),
          finalStates(std::move(finalStates)),
          transitions(std::move(transitions)) {}

    // Получить количество состояний
    int getNumStates() const { return numStates; }

    // Получить начальное состояние
    int getStartState() const { return startState; }

    // Получить финальные состояния
    auto getFinalStates() { return finalStates; }

    // Преобразование NFA в DFA (детерминизация)
    FiniteAutomaton determinize() const
    {
        // Создаем новый объект DFA
        FiniteAutomaton dfa;
        dfa.startState = 0; // Начальное состояние DFA - 0

        // Сопоставление множества состояний NFA с состояниями DFA
        std::unordered_map<std::unordered_set<int>, int, SetHash> stateMapping;

        // Очередь для обработки новых множеств состояний
        std::queue<std::unordered_set<int>> queue;

        // Хранение уже обработанных множеств состояний
        std::unordered_set<std::unordered_set<int>, SetHash> processedStates;

        // Начальное множество состояний NFA, содержащее только начальное состояние
        std::unordered_set<int> startSet = {startState};
        stateMapping[startSet] = dfa.startState; // Сопоставляем начальное множество состоянию DFA
        queue.push(startSet);                    // Добавляем начальное множество в очередь

        // Основной цикл обработки очереди
        while (!queue.empty())
        {
            // Берем текущее множество состояний NFA из очереди
            auto currentSet = queue.front();
            queue.pop();

            // Отмечаем это множество как обработанное
            processedStates.insert(currentSet);

            // Проверяем, содержит ли текущее множество хотя бы одно финальное состояние NFA
            for (int state : currentSet)
            {
                if (finalStates.count(state))
                {
                    // Если содержит, соответствующее состояние DFA становится финальным
                    dfa.finalStates.insert(stateMapping[currentSet]);
                    break;
                }
            }

            // Построение переходов для текущего множества состояний
            std::unordered_map<char, std::unordered_set<int>> newTransitions;
            for (int state : currentSet)
            {
                if (transitions.count(state))
                {
                    // Для каждого символа добавляем все целевые состояния
                    for (const auto &[symbol, targets] : transitions.at(state))
                    {
                        newTransitions[symbol].insert(targets.begin(), targets.end());
                    }
                }
            }

            // Создание новых состояний DFA и добавление их переходов
            for (const auto &[symbol, targetSet] : newTransitions)
            {
                // Если targetSet еще не было добавлено в stateMapping
                if (!stateMapping.count(targetSet))
                {
                    // Создаем новое состояние DFA
                    int newState = static_cast<int>(stateMapping.size());
                    stateMapping[targetSet] = newState; // Сопоставляем множество новому состоянию DFA
                    queue.push(targetSet);              // Добавляем новое множество в очередь
                }
                // Добавляем переход из текущего состояния DFA в новое по символу symbol
                dfa.transitions[stateMapping[currentSet]][symbol] = {stateMapping[targetSet]};
            }
        }

        // Устанавливаем количество состояний DFA
        dfa.numStates = static_cast<int>(stateMapping.size());
        return dfa; // Возвращаем детерминированный автомат
    }

    // Проверяет, распознает ли автомат заданное слово
    bool isRecognized(const std::string &automationWord)
    {
        int currentState = startState; // Начинаем с начального состояния
        for (char ch : automationWord)
        {
            // Если нет перехода по символу, слово не распознано
            if (!transitions[currentState].count(ch))
                return false;
            // Переход в следующее состояние
            currentState = transitions[currentState][ch][0];
        }
        // Проверяем, является ли текущее состояние финальным
        return finalStates.count(currentState);
    }
};

// Функция загрузки автомата из файла
FiniteAutomaton loadFromFile(std::ifstream &inputFile)
{
    int numStates, startState, numFinalStates, numTransitions;
    std::unordered_set<int> finalStates;
    std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions;

    // Читаем количество состояний, начальное состояние и количество финальных состояний
    inputFile >> numStates >> startState >> numFinalStates;
    for (int i = 0; i < numFinalStates; i++)
    {
        int state;
        inputFile >> state; // Читаем каждое финальное состояние
        finalStates.insert(state);
    }
    // Читаем количество переходов
    inputFile >> numTransitions;
    for (int i = 0; i < numTransitions; i++)
    {
        int fromState, toState;
        char value;
        inputFile >> fromState >> toState >> value; // Читаем переход
        transitions[fromState][value].push_back(toState);
    }
    // Создаем и возвращаем автомат
    FiniteAutomaton automaton(numStates, startState, finalStates, transitions);
    return automaton;
}

int main()
{
    // Открываем входной и выходной файлы
    std::ifstream inputFile("input.txt");
    std::ofstream outputFile("output.txt");

    // Загружаем NFA из файла
    FiniteAutomaton automaton = loadFromFile(inputFile);

    // Преобразуем NFA в DFA
    FiniteAutomaton DFA = automaton.determinize();

    // Читаем количество слов для проверки
    int numWords;
    inputFile >> numWords;
    for (int i = 0; i < numWords; i++)
    {
        std::string word;
        inputFile >> word; // Читаем слово
        // Проверяем, распознает ли его DFA
        if (DFA.isRecognized(word))
            outputFile << "YES\n";
        else
            outputFile << "NO\n";
    }

    // Закрываем файлы
    inputFile.close();
    outputFile.close();
    return 0;
}
