/*
НГУ ММФ МКН 3 семестр
Лаборатораня работа по программированию (ООП)
Задание 3, Задача "3.1.2 (усложнённый) Хеш-таблица, открытая адресация"
*/

#include <fstream>
#include <iostream>
#include <set>

template<typename K, typename V> class HashMap {
    struct HashMapNode {
        K key;
        V value;
        bool isOccupied;

        HashMapNode() : isOccupied(false) {}
        HashMapNode(K key, V value) : key(key), value(value), isOccupied(true) {}
    };

    HashMapNode* table;
    int size;
    int capacity;
    float loadFactor;


    int hash(const K& key) {
        return std::hash<K>()(key) % capacity;
    }

    void rehash() {
        int oldCapacity = capacity;
        capacity *= 2;
        HashMapNode* oldTable = table;
        table = new HashMapNode[capacity]();
        size = 0;
        for (int i = 0; i < oldCapacity; i++) {
            if (oldTable[i].isOccupied)
                add(oldTable[i].key, oldTable[i].value);
        }
        delete[] oldTable;
    }

public:
    HashMap(float loadFactor = 0.70, int capacity = 4) :
        loadFactor(loadFactor), capacity(capacity), size(0), table(new HashMapNode[capacity]()) {
    }

    ~HashMap() {
        delete[] table;
    }

    V get(const K& key) {
        int index = hash(key);
        while (table[index].isOccupied) {
            if (table[index].key == key)
                return table[index].value;
            index = (index + 1) % capacity;
        }
        throw std::out_of_range("Key not found");
    }

    void add(const K& key, const V& value) {
        if (static_cast<float>(size) / capacity >= loadFactor) rehash();
        int index = hash(key);
        while (table[index].isOccupied) {
            if (table[index].key == key) {
                table[index].value = value;
                return;
            }
            index = (index + 1) % capacity;
        }
        table[index] = HashMapNode(key, value);
        size++;
    }

    void remove(const K& key) {
        int index = hash(key);
        while (table[index].isOccupied) {
            if (table[index].key == key) {
                table[index].isOccupied = false;
                size--;
                return;
            }
            index = (index + 1) % capacity;
        }
    }

    class Iterator {
        HashMapNode* current;
        HashMapNode* end;

    public:
        Iterator(HashMapNode* current, HashMapNode* end) : current(current), end(end) {
            while (current <= end && !current->isOccupied) ++current;
        }

        std::pair<K, V> operator*() const {
            return std::make_pair(current->key, current->value);
        }

        Iterator& operator++() {
            ++current;
            while (current < end && !current->isOccupied) ++current;
            return *this;
        }

        bool operator!=(const Iterator& other) const { return current != other.current; }
    };

    Iterator begin() const {
        return Iterator(table, table + capacity);
    }

    Iterator end() const {
        return Iterator(table + capacity, table + capacity);
    }

    int getSize() const { return size; }

    int getUniqueValues() const {
        std::set<V> uniqueValues;
        for (int i = 0; i < capacity; ++i) {
            if (table[i].isOccupied) {
                uniqueValues.insert(table[i].value);
            }
        }
        return uniqueValues.size();
    }
};

template<typename K, typename V> std::pair<int, int> makeHashMap(
    std::ifstream& file = dynamic_cast<std::ifstream&>(std::cin)) {
    HashMap<K, V> map;
    int N;
    file >> N;
    for (int i = 0; i < N; i++) {
        char command;
        file >> command;
        if (command == 'A') {
            K key;
            V value;
            file >> key >> value;
            map.add(key, value);
        } else if (command == 'R') {
            K key;
            file >> key;
            map.remove(key);
        } else {
            throw std::invalid_argument("Invalid command");
        }
    }
    return std::make_pair(map.getSize(), map.getUniqueValues());
}

std::pair<int, int> executeProgram(char keyType, char valueType, std::ifstream& file) {
    switch (keyType) {
    case 'I':
        switch (valueType) {
        case 'I': return makeHashMap<int, int>(file);
        case 'D': return makeHashMap<int, double>(file);
        case 'S': return makeHashMap<int, std::string>(file);
        default: throw std::out_of_range("Wrong value type");
        }
    case 'D':
        switch (valueType) {
        case 'I': return makeHashMap<double, int>(file);
        case 'D': return makeHashMap<double, double>(file);
        case 'S': return makeHashMap<double, std::string>(file);
        default: throw std::out_of_range("Wrong value type");
        }
    case 'S':
        switch (valueType) {
        case 'I': return makeHashMap<std::string, int>(file);
        case 'D': return makeHashMap<std::string, double>(file);
        case 'S': return makeHashMap<std::string, std::string>(file);
        default: throw std::out_of_range("Wrong value type");
        }
    default: throw std::out_of_range("Wrong key type");
    }
}

int main() {
    char keyType, valueType;

    std::ifstream inputfile("input.txt");
    inputfile >> keyType >> valueType;
    auto [size, uniqueValues] = executeProgram(keyType, valueType, inputfile);
    inputfile.close();

    std::ofstream outputfile("output.txt");
    outputfile << size << ' ' << uniqueValues;
    outputfile.close();


    return 0;
}
