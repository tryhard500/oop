/*
НГУ ММФ МКН 3 семестр
Лаборатораня работа по программированию (ООП)
Задание 4, Задача "4.1.2 (усложнённый) ДКА"
*/

#include <fstream>
#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>


struct SetHash {
    template<typename T>
    std::size_t operator()(const std::unordered_set<T>& set) const {
        std::size_t hash = 0;
        for (const auto& elem : set) {
            hash ^= std::hash<T>()(elem) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

class FiniteAutomaton {
    int numStates;
    int startState;
    std::unordered_set<int> finalStates;
    std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions;

public:
    FiniteAutomaton() : numStates(0), startState(0) {}

    FiniteAutomaton(const int numStates,
        const int startState,
        std::unordered_set<int> finalStates,
        std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions)
        : numStates(numStates),
        startState(startState),
        finalStates(std::move(finalStates)),
        transitions(std::move(transitions)) {
    }

    FiniteAutomaton determinize() const {
        // Создаем новый объект DFA
        FiniteAutomaton dfa;
        dfa.startState = 0; // Начальное состояние DFA - 0

        // Сопоставление множества состояний NFA с состояниями DFA
        std::unordered_map<std::unordered_set<int>, int, SetHash> stateMapping;

        // Очередь для обработки новых множеств состояний
        std::queue<std::unordered_set<int>> queue;

        // Хранение уже обработанных множеств состояний
        std::unordered_set<std::unordered_set<int>, SetHash> processedStates;

        // Функция для вычисления ε-замыкания множества состояний
        auto epsilonClosure = [&](const std::unordered_set<int>& states) {
            std::unordered_set<int> closure = states;
            std::queue<int> stateQueue;
            for (int state : states) {
                stateQueue.push(state);
            }
            while (!stateQueue.empty()) {
                int currentState = stateQueue.front();
                stateQueue.pop();
                if (transitions.count(currentState) && transitions.at(currentState).count('\0')) {
                    for (int nextState : transitions.at(currentState).at('\0')) {
                        if (closure.insert(nextState).second) {
                            stateQueue.push(nextState);
                        }
                    }
                }
            }
            return closure;
            };

        // Начальное множество состояний NFA, включая ε-замыкание начального состояния
        std::unordered_set<int> startSet = epsilonClosure({ startState });
        stateMapping[startSet] = dfa.startState; // Сопоставляем начальное множество состоянию DFA
        queue.push(startSet); // Добавляем начальное множество в очередь

        while (!queue.empty()) {
            // Берем текущее множество состояний NFA из очереди
            auto currentSet = queue.front();
            queue.pop();

            // Отмечаем это множество как обработанное
            processedStates.insert(currentSet);

            // Проверяем, содержит ли текущее множество хотя бы одно финальное состояние NFA
            for (int state : currentSet) {
                if (finalStates.count(state)) {
                    // Если содержит, соответствующее состояние DFA становится финальным
                    dfa.finalStates.insert(stateMapping[currentSet]);
                    break;
                }
            }

            // Построение переходов для текущего множества состояний
            std::unordered_map<char, std::unordered_set<int>> newTransitions;
            for (int state : currentSet) {
                if (transitions.count(state)) {
                    for (const auto& [symbol, targets] : transitions.at(state)) {
                        if (symbol != '\0') {
                            newTransitions[symbol].insert(targets.begin(), targets.end());
                        }
                    }
                }
            }

            // Обработка переходов для нового множества состояний
            for (const auto& [symbol, targetSet] : newTransitions) {
                // Вычисляем ε-замыкание для targetSet
                std::unordered_set<int> epsilonClosureSet = epsilonClosure(targetSet);

                // Если epsilonClosureSet еще не было добавлено в stateMapping
                if (!stateMapping.count(epsilonClosureSet)) {
                    // Создаем новое состояние DFA
                    int newState = static_cast<int>(stateMapping.size());
                    stateMapping[epsilonClosureSet] = newState; // Сопоставляем множество новому состоянию DFA
                    queue.push(epsilonClosureSet); // Добавляем новое множество в очередь
                }

                // Добавляем переход из текущего состояния DFA в новое по символу symbol
                dfa.transitions[stateMapping[currentSet]][symbol] = { stateMapping[epsilonClosureSet] };
            }
        }

        dfa.numStates = static_cast<int>(stateMapping.size());
        return dfa;
    }


    bool isRecognized(const std::string& automationWord) {
        int currentState = startState;
        for (char ch : automationWord) {
            if (!transitions[currentState].count(ch))
                return false;
            currentState = transitions[currentState][ch][0];
        }
        return finalStates.count(currentState) > 0;
    }
};

class BuildAutomatonByRegex {
    struct NFAFragment {
        int startState;
        int endState;
        std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions;
    };

    static void mergeTransitions(std::unordered_map<int, std::unordered_map<char, std::vector<int>>>& dest,
        const std::unordered_map<int, std::unordered_map<char, std::vector<int>>>& src) {
        for (const auto& [state, transitionMap] : src) {
            for (const auto& [symbol, targets] : transitionMap) {
                dest[state][symbol].insert(dest[state][symbol].end(), targets.begin(), targets.end());
            }
        }
    }

    static void processOperator(std::stack<NFAFragment>& nfaStack, char op, int& stateCounter) {
        if (op == '|') {
            auto frag2 = nfaStack.top();
            nfaStack.pop();
            auto frag1 = nfaStack.top();
            nfaStack.pop();

            int start = stateCounter++;
            int end = stateCounter++;
            std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions = frag1.transitions;

            mergeTransitions(transitions, frag2.transitions);

            transitions[start]['\0'].push_back(frag1.startState);
            transitions[start]['\0'].push_back(frag2.startState);
            transitions[frag1.endState]['\0'].push_back(end);
            transitions[frag2.endState]['\0'].push_back(end);

            nfaStack.push({ start, end, transitions });
        } else if (op == '.') {
            auto frag2 = nfaStack.top();
            nfaStack.pop();
            auto frag1 = nfaStack.top();
            nfaStack.pop();

            std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions = frag1.transitions;
            mergeTransitions(transitions, frag2.transitions);
            transitions[frag1.endState]['\0'].push_back(frag2.startState);

            nfaStack.push({ frag1.startState, frag2.endState, transitions });
        } else if (op == '*') {
            auto frag = nfaStack.top();
            nfaStack.pop();

            int start = stateCounter++;
            int end = stateCounter++;
            std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions = frag.transitions;

            transitions[start]['\0'].push_back(frag.startState);
            transitions[start]['\0'].push_back(end);
            transitions[frag.endState]['\0'].push_back(frag.startState);
            transitions[frag.endState]['\0'].push_back(end);

            nfaStack.push({ start, end, transitions });
        }
    }
public:
    static FiniteAutomaton buildAutomaton(const std::string& regex) {
        std::stack<NFAFragment> nfaStack;
        std::stack<char> operatorStack;
        int stateCounter = 0;

        for (const char ch : regex) {
            if (isalpha(ch) || isdigit(ch)) {
                int start = stateCounter++;
                int end = stateCounter++;
                std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions;
                transitions[start][ch].push_back(end);
                nfaStack.push({ start, end, transitions });
            } else if (ch == '(') {
                operatorStack.push(ch);
            } else if (ch == ')') {
                while (!operatorStack.empty() && operatorStack.top() != '(') {
                    processOperator(nfaStack, operatorStack.top(), stateCounter);
                    operatorStack.pop();
                }
                operatorStack.pop(); // Убираем '('
            } else if (ch == '|' || ch == '.' || ch == '*') {
                while (!operatorStack.empty() && operatorStack.top() != '(' &&
                    (ch != '*' || operatorStack.top() == '*')) {
                    processOperator(nfaStack, operatorStack.top(), stateCounter);
                    operatorStack.pop();
                }
                operatorStack.push(ch);
            }
        }

        while (!operatorStack.empty()) {
            processOperator(nfaStack, operatorStack.top(), stateCounter);
            operatorStack.pop();
        }

        auto finalFragments = nfaStack.top();
        nfaStack.pop();

        return FiniteAutomaton(
            stateCounter,
            finalFragments.startState,
            { finalFragments.endState },
            finalFragments.transitions
        );
    }
};

FiniteAutomaton loadFromFile(std::ifstream& inputFile) {
    int numStates, startState, numFinalStates, numTransitions;
    std::unordered_set<int> finalStates;
    std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions;

    inputFile >> numStates >> startState >> numFinalStates;
    for (int i = 0; i < numFinalStates; i++) {
        int state;
        inputFile >> state;
        finalStates.insert(state);
    }

    inputFile >> numTransitions;
    for (int i = 0; i < numTransitions; i++) {
        int fromState, toState;
        char value;
        inputFile >> fromState >> toState >> value;
        transitions[fromState][value].push_back(toState);
    }

    return { numStates, startState, finalStates, transitions };
}

#if 1
int main() {
    std::ifstream inputFile("input.txt");
    std::ofstream outputFile("output.txt");

    FiniteAutomaton automaton = loadFromFile(inputFile);

    FiniteAutomaton DFA = automaton.determinize();

    int numWords;
    inputFile >> numWords;
    for (int i = 0; i < numWords; i++) {
        std::string word;
        inputFile >> word;
        outputFile << (DFA.isRecognized(word) ? "YES" : "NO") << std::endl;
    }

    inputFile.close();
    outputFile.close();
    return 0;
}
#endif

#if 0
int main() {
    const std::string regex = "(a.b)*";

    const FiniteAutomaton nfa = BuildAutomatonByRegex::buildAutomaton(regex);

    FiniteAutomaton dfa = nfa.determinize();

    for (const auto rg : { "ab", "abab", "", "abababababab", "a", "b", "aaaaa" }) {
        std::cout << rg << " : " << (dfa.isRecognized(rg) ? "YES" : "NO") << std::endl;
    }
}
#endif
