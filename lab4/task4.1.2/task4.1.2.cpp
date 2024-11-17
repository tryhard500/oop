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

struct HashSetHash
{
    std::size_t operator()(const std::unordered_set<int> &set) const
    {
        std::size_t hash = 0;
        for (const int elem : set)
        {
            hash ^= std::hash<int>()(elem) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

struct SetHash
{
    template <typename T>
    std::size_t operator()(const std::unordered_set<T> &set) const
    {
        std::size_t hash = 0;
        for (const auto &elem : set)
        {
            hash ^= std::hash<T>()(elem) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

class FiniteAutomaton
{
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
          transitions(std::move(transitions)) {}

    FiniteAutomaton determinize() const
    {
        FiniteAutomaton dfa;
        dfa.startState = 0;
        std::unordered_map<std::unordered_set<int>, int, SetHash> stateMapping;
        std::queue<std::unordered_set<int>> queue;
        std::unordered_set<std::unordered_set<int>, SetHash> processedStates;

        std::unordered_set<int> startSet = {startState};
        stateMapping[startSet] = dfa.startState;
        queue.push(startSet);

        while (!queue.empty())
        {
            auto currentSet = queue.front();
            queue.pop();
            processedStates.insert(currentSet);

            for (int state : currentSet)
            {
                if (finalStates.count(state))
                {
                    dfa.finalStates.insert(stateMapping[currentSet]);
                    break;
                }
            }

            std::unordered_map<char, std::unordered_set<int>> newTransitions;
            for (int state : currentSet)
            {
                if (transitions.count(state))
                {
                    for (const auto &[symbol, targets] : transitions.at(state))
                    {
                        newTransitions[symbol].insert(targets.begin(), targets.end());
                    }
                }
            }

            for (const auto &[symbol, targetSet] : newTransitions)
            {
                if (!stateMapping.count(targetSet))
                {
                    int newState = stateMapping.size();
                    stateMapping[targetSet] = newState;
                    queue.push(targetSet);
                }
                dfa.transitions[stateMapping[currentSet]][symbol] = {stateMapping[targetSet]};
            }
        }

        dfa.numStates = stateMapping.size();
        return dfa;
    }

    bool isRecognized(const std::string &automationWord)
    {
        int currentState = startState;
        for (char ch : automationWord)
        {
            if (transitions[currentState].count(ch) == 0)
                return false;
            currentState = transitions[currentState][ch][0];
        }
        return finalStates.count(currentState) > 0;
    }
};

FiniteAutomaton loadFromFile(std::ifstream &inputFile)
{
    int numStates, startState, numFinalStates, numTransitions;
    std::unordered_set<int> finalStates;
    std::unordered_map<int, std::unordered_map<char, std::vector<int>>> transitions;

    inputFile >> numStates >> startState >> numFinalStates;
    for (int i = 0; i < numFinalStates; i++)
    {
        int state;
        inputFile >> state;
        finalStates.insert(state);
    }
    inputFile >> numTransitions;
    for (int i = 0; i < numTransitions; i++)
    {
        int fromState, toState;
        char value;
        inputFile >> fromState >> toState >> value;
        transitions[fromState][value].push_back(toState);
    }
    FiniteAutomaton automaton(numStates, startState, finalStates, transitions);
    return automaton;
}

int main()
{
    std::ifstream inputFile("input.txt");
    std::ofstream outputFile("output.txt");

    FiniteAutomaton automaton = loadFromFile(inputFile);

    FiniteAutomaton DFA = automaton.determinize();

    int numWords;
    inputFile >> numWords;
    for (int i = 0; i < numWords; i++)
    {
        std::string word;
        inputFile >> word;
        if (DFA.isRecognized(word))
            outputFile << "YES\n";
        else
            outputFile << "NO\n";
    }

    inputFile.close();
    outputFile.close();
    return 0;
}
