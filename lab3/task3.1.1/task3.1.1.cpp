/*
НГУ ММФ МКН 3 семестр
Лаборатораня работа по программированию (ООП)
Задание 3, Задача "3.1.1 (усложнённый) Хеш-таблица, цепочки + Доп. задание"
*/

#include <fstream>
#include <iostream>
#include <list>
#include <set>
#include <vector>

class Matrix
{
    size_t size_;
    int **matrix_data_;

public:
    Matrix();

    Matrix(size_t size);

    Matrix(size_t size, const int *diag);

    size_t get_size() const;

    Matrix operator+(const Matrix &other_matrix) const;

    Matrix operator-(const Matrix &other_matrix) const;

    Matrix operator*(const Matrix &other_matrix) const;

    bool operator==(const Matrix &other_matrix) const;

    bool operator!=(const Matrix &other_matrix) const;

    Matrix operator~() const;

    Matrix operator()(size_t row, size_t column) const;

    int *operator[](size_t index) const;

    class Vector
    {
    private:
        const Matrix *matrix_;
        size_t index_;

    public:
        Vector(const Matrix *matrix, size_t index);

        int operator[](size_t index) const;

        int &operator[](size_t);
    };

    Vector operator()(size_t index) const;

    friend std::ostream &operator<<(std::ostream &os, const Matrix &matrix);

    friend std::istream &operator>>(std::istream &is, Matrix &matrix);

    ~Matrix();
};

Matrix::Matrix() : size_(0), matrix_data_(nullptr) {}

Matrix::Matrix(const size_t size) : size_(size)
{
    matrix_data_ = new int *[size];
    for (size_t i = 0; i < size; ++i)
    {
        matrix_data_[i] = new int[size]{};
        matrix_data_[i][i] = 1;
    }
}

Matrix::Matrix(const size_t size, const int *diag) : size_(size)
{
    matrix_data_ = new int *[size];
    for (size_t i = 0; i < size; ++i)
    {
        matrix_data_[i] = new int[size]{};
        matrix_data_[i][i] = diag[i];
    }
}

size_t Matrix::get_size() const { return size_; }

Matrix::~Matrix()
{
    for (size_t i = 0; i < size_; ++i)
        delete[] this->matrix_data_[i];
    delete[] this->matrix_data_;
}

Matrix Matrix::operator+(const Matrix &other_matrix) const
{
    if (this->size_ != other_matrix.size_)
        throw std::exception();
    const size_t size = this->size_;
    Matrix new_matrix(size);
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            new_matrix.matrix_data_[i][j] =
                this->matrix_data_[i][j] +
                other_matrix.matrix_data_[i][j];
        }
    }
    return new_matrix;
}

Matrix Matrix::operator-(const Matrix &other_matrix) const
{
    if (this->size_ != other_matrix.size_)
        throw std::exception();
    const size_t size = this->size_;
    Matrix new_matrix(size);
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            new_matrix.matrix_data_[i][j] =
                this->matrix_data_[i][j] -
                other_matrix.matrix_data_[i][j];
        }
    }
    return new_matrix;
}

Matrix Matrix::operator*(const Matrix &other_matrix) const
{
    if (this->size_ != other_matrix.size_)
        throw std::exception();
    const size_t size = this->size_;
    int *diag = new int[size]{};
    Matrix new_matrix(size, diag);
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            for (size_t k = 0; k < size; ++k)
            {
                new_matrix.matrix_data_[i][j] +=
                    this->matrix_data_[i][k] *
                    other_matrix.matrix_data_[k][j];
            }
        }
    }
    delete[] diag;
    diag = nullptr;
    return new_matrix;
}

bool Matrix::operator==(const Matrix &other_matrix) const
{
    if (this->size_ != other_matrix.size_)
        return false;
    const size_t size = this->size_;
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            if (this->matrix_data_[i][j] != other_matrix.matrix_data_[i][j])
                return false;
        }
    }
    return true;
}

bool Matrix::operator!=(const Matrix &other_matrix) const
{
    return !(*this == other_matrix);
}

Matrix Matrix::operator~() const
{
    const size_t size = this->size_;
    Matrix new_matrix(size);
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            new_matrix.matrix_data_[i][j] = this->matrix_data_[j][i];
        }
    }
    return new_matrix;
}

Matrix Matrix::operator()(const size_t row, const size_t column) const
{
    Matrix minor_matrix(size_ - 1);
    for (size_t i = 0; i < size_ - 1; ++i)
    {
        for (size_t j = 0; j < size_ - 1; ++j)
        {
            if (i < row && j < column)
                minor_matrix.matrix_data_[i][j] = this->matrix_data_[i][j];
            if (i < row && j > column)
                minor_matrix.matrix_data_[i][j] = this->matrix_data_[i][j + 1];
            if (i > row && j < column)
                minor_matrix.matrix_data_[i][j] = this->matrix_data_[i + 1][j];
            if (i > row && j > column)
                minor_matrix.matrix_data_[i][j] = this->matrix_data_[i + 1][j + 1];
        }
    }
    return minor_matrix;
}

int *Matrix::operator[](const size_t index) const
{
    if (index < 0 || index > this->size_)
        throw std::exception();
    return this->matrix_data_[index];
}

Matrix::Vector Matrix::operator()(const size_t index) const
{
    return Vector(this, index);
}

Matrix::Vector::Vector(const Matrix *matrix, size_t index) : matrix_(matrix), index_(index) {}

int Matrix::Vector::operator[](const size_t index) const
{
    if (index < 0 || index > this->matrix_->size_)
        throw std::exception();
    return this->matrix_->matrix_data_[index][this->index_];
}

int &Matrix::Vector::operator[](const size_t index)
{
    if (index < 0 || index > this->matrix_->size_)
        throw std::exception();
    return this->matrix_->matrix_data_[index][this->index_];
}

std::ostream &operator<<(std::ostream &os, const Matrix &matrix)
{
    for (size_t i = 0; i < matrix.size_; ++i)
    {
        for (size_t j = 0; j < matrix.size_; ++j)
        {
            os << matrix.matrix_data_[i][j] << ' ';
        }
        os << '\n';
    }
    return os;
}

std::istream &operator>>(std::istream &is, Matrix &matrix)
{
    for (size_t i = 0; i < matrix.size_; ++i)
    {
        for (size_t j = 0; j < matrix.size_; ++j)
        {
            is >> matrix.matrix_data_[i][j];
        }
    }
    return is;
}

template <>
struct std::hash<Matrix>
{
    size_t operator()(const Matrix &matrix) const noexcept
    {
        int hash = 0;
        for (int i = 0; i < matrix.get_size(); ++i)
        {
            for (int j = 0; j < matrix.get_size(); ++j)
            {
                hash ^= std::hash<int>()(matrix[i][j]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
        }
        return hash;
    }
};

template <typename K, typename V>
class HashMap
{
    struct HashMapNode
    {
        K key;
        V value;
        HashMapNode *next;

        HashMapNode(K key, V value) : key(key), value(value), next(nullptr) {}
    };

    HashMapNode **table;
    int size;
    int capacity;
    float loadFactor;

    int hash(const K &key)
    {
        return std::hash<K>()(key) % capacity;
    }

    void rehash()
    {
        int oldCapacity = capacity;
        capacity *= 2;
        HashMapNode **newTable = new HashMapNode *[capacity]();
        for (int i = 0; i < oldCapacity; i++)
        {
            HashMapNode *current = table[i];
            while (current)
            {
                HashMapNode *next = current->next;
                int index = hash(current->key);
                current->next = newTable[index];
                newTable[index] = current;
                current = next;
            }
        }
        delete[] table;
        table = newTable;
    }

public:
    HashMap(float loadFactor = 0.70, int capacity = 4) : loadFactor(loadFactor), capacity(capacity), size(0), table(new HashMapNode *[capacity]()) {}

    ~HashMap()
    {
        for (int i = 0; i < size; i++)
        {
            HashMapNode *current = table[i];
            while (current)
            {
                HashMapNode *next = current->next;
                delete current;
                current = next;
            }
        }
        delete[] table;
    }

    V get(const K &key)
    {
        int index = hash(key);
        HashMapNode *current = table[index];
        while (current)
        {
            if (current->key == key)
                return current->value;
            current = current->next;
        }
        throw std::out_of_range("Key not found");
    }

    void add(const K &key, const V &value)
    {
        if (static_cast<float>(size) / capacity >= loadFactor)
            rehash();
        int index = hash(key);
        HashMapNode *current = table[index];
        while (current)
        {
            if (current->key == key)
            {
                current->value = value;
                return;
            }
            current = current->next;
        }
        auto *node = new HashMapNode(key, value);
        node->next = table[index];
        table[index] = node;
        size++;
    }

    void remove(const K &key)
    {
        int index = hash(key);
        HashMapNode *current = table[index];
        HashMapNode *previous = nullptr;
        while (current)
        {
            if (current->key == key)
            {
                if (previous)
                {
                    previous->next = current->next;
                }
                else
                {
                    table[index] = current->next;
                }
                delete current;
                size--;
                return;
            }
            previous = current;
            current = current->next;
        }
    }

    class Iterator
    {
        HashMap &map;
        int index;
        HashMapNode *current;

    public:
        Iterator(HashMap &map, int index, HashMapNode *node) : map(map), index(index), current(node) {}

        std::pair<K, V> operator*() const
        {
            return {current->key, current->value};
        }

        Iterator &operator++()
        {
            if (current)
                current = current->next;
            while (!current && index < map.capacity - 1)
            {
                ++index;
                current = map.table[index];
            }
            return *this;
        }

        bool operator!=(const Iterator &other) const
        {
            return current != other.current || index != other.index;
        }
    };

    Iterator begin()
    {
        int index = 0;
        while (index < capacity && !table[index])
            ++index;
        return Iterator(*this, index, index < capacity ? table[index] : nullptr);
    }

    Iterator end()
    {
        return Iterator(*this, capacity - 1, nullptr);
    }

    int getSize() const
    {
        int size = 0;
        for (int i = 0; i < capacity; i++)
        {
            HashMapNode *current = table[i];
            while (current)
            {
                ++size;
                current = current->next;
            }
        }
        return size;
    }

    int getUniqueValues() const
    {
        std::set<V> uniqueValues;
        for (int i = 0; i < capacity; i++)
        {
            HashMapNode *current = table[i];
            while (current)
            {
                uniqueValues.insert(current->value);
                current = current->next;
            }
        }
        return uniqueValues.size();
    }
};

template <typename K, typename V>
class MultiHashMap
{
    struct HashMapNode
    {
        K key;
        V value;
        HashMapNode *next;

        HashMapNode(K key, V value) : key(key), value(value), next(nullptr) {}
    };

    HashMapNode **table;
    int size;
    int capacity;
    float loadFactor;

    int hash(const K &key)
    {
        return std::hash<K>()(key) % capacity;
    }

    void rehash()
    {
        int oldCapacity = capacity;
        capacity *= 2;
        HashMapNode **newTable = new HashMapNode *[capacity]();
        for (int i = 0; i < oldCapacity; i++)
        {
            HashMapNode *current = table[i];
            while (current)
            {
                HashMapNode *next = current->next;
                int index = hash(current->key);
                current->next = newTable[index];
                newTable[index] = current;
                current = next;
            }
        }
        delete[] table;
        table = newTable;
    }

public:
    MultiHashMap(float loadFactor = 0.70, int capacity = 4) : loadFactor(loadFactor), capacity(capacity), size(0), table(new HashMapNode *[capacity]()) {}

    ~MultiHashMap()
    {
        for (int i = 0; i < size; i++)
        {
            HashMapNode *current = table[i];
            while (current)
            {
                HashMapNode *next = current->next;
                delete current;
                current = next;
            }
        }
        delete[] table;
    }

    V get(const K &key)
    {
        int index = hash(key);
        HashMapNode *current = table[index];
        while (current)
        {
            if (current->key == key)
                return current->value;
            current = current->next;
        }
        throw std::out_of_range("Key not found");
    }

    std::vector<V> getAll(const K &key)
    {
        std::vector<V> result;
        int index = hash(key);
        HashMapNode *current = table[index];
        while (current)
        {
            if (current->key == key)
                result.emplace_back(current->value);
            current = current->next;
        }
        return result;
    }

    int count(const K &key)
    {
        int count = 0;
        int index = hash(key);
        HashMapNode *current = table[index];
        while (current)
        {
            if (current->key == key)
                count++;
            current = current->next;
        }
        return count;
    }

    void add(const K &key, const V &value)
    {
        if (static_cast<float>(size) / capacity >= loadFactor)
            rehash();
        int index = hash(key);
        HashMapNode *current = table[index];
        auto *node = new HashMapNode(key, value);
        node->next = table[index];
        table[index] = node;
        ++size;
    }

    void remove(const K &key)
    {
        int index = hash(key);
        HashMapNode *current = table[index];
        HashMapNode *previous = nullptr;
        while (current)
        {
            if (current->key == key)
            {
                if (previous)
                {
                    previous->next = current->next;
                }
                else
                {
                    table[index] = current->next;
                }
                delete current;
                size--;
                return;
            }
            previous = current;
            current = current->next;
        }
    }

    void removeAll(const K &key)
    {
        int index = hash(key);
        HashMapNode *current = table[index];
        while (current && current->key == key)
        {
            HashMapNode *temp = current;
            current = current->next;
            delete temp;
            size--;
        }
        while (current && current->next)
        {
            if (current->next->key == key)
            {
                HashMapNode *temp = current->next;
                current->next = current->next->next;
                delete temp;
                size--;
            }
            else
            {
                current = current->next;
            }
        }
    }

    class Iterator
    {
        MultiHashMap &map;
        int index;
        HashMapNode *current;

    public:
        Iterator(MultiHashMap &map, int index, HashMapNode *node) : map(map), index(index), current(node) {}

        std::pair<K, V> operator*() const
        {
            return {current->key, current->value};
        }

        Iterator &operator++()
        {
            if (current)
                current = current->next;
            while (!current && index < map.capacity - 1)
            {
                ++index;
                current = map.table[index];
            }
            return *this;
        }

        bool operator!=(const Iterator &other) const
        {
            return current != other.current || index != other.index;
        }
    };

    Iterator begin()
    {
        int index = 0;
        while (index < capacity && !table[index])
            ++index;
        return Iterator(*this, index, index < capacity ? table[index] : nullptr);
    }

    Iterator end()
    {
        return Iterator(*this, capacity - 1, nullptr);
    }
};

template <typename K, typename V>
std::pair<int, int> makeHashMap(
    std::ifstream &file = dynamic_cast<std::ifstream &>(std::cin))
{
    HashMap<K, V> map;
    int N;
    file >> N;
    for (int i = 0; i < N; i++)
    {
        char command;
        file >> command;
        if (command == 'A')
        {
            K key;
            V value;
            file >> key >> value;
            map.add(key, value);
        }
        else if (command == 'R')
        {
            K key;
            file >> key;
            map.remove(key);
        }
        else
            throw std::invalid_argument("Invalid command");
    }
    return std::make_pair(map.getSize(), map.getUniqueValues());
}

std::pair<int, int> executeProgram(char keyType, char valueType, std::ifstream &file)
{
    switch (keyType)
    {
    case 'I':
        switch (valueType)
        {
        case 'I':
            return makeHashMap<int, int>(file);
        case 'D':
            return makeHashMap<int, double>(file);
        case 'S':
            return makeHashMap<int, std::string>(file);
        default:
            throw std::out_of_range("Wrong value type");
        }
    case 'D':
        switch (valueType)
        {
        case 'I':
            return makeHashMap<double, int>(file);
        case 'D':
            return makeHashMap<double, double>(file);
        case 'S':
            return makeHashMap<double, std::string>(file);
        default:
            throw std::out_of_range("Wrong value type");
        }
    case 'S':
        switch (valueType)
        {
        case 'I':
            return makeHashMap<std::string, int>(file);
        case 'D':
            return makeHashMap<std::string, double>(file);
        case 'S':
            return makeHashMap<std::string, std::string>(file);
        default:
            throw std::out_of_range("Wrong value type");
        }
    case 'M':
        switch (valueType)
        {
        case 'I':
            return makeHashMap<Matrix, int>(file);
        case 'D':
            return makeHashMap<Matrix, double>(file);
        case 'S':
            return makeHashMap<Matrix, std::string>(file);
        default:
            throw std::out_of_range("Wrong value type");
        }
    default:
        throw std::out_of_range("Wrong key type");
    }
}

#if 0
int main()
{
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
#endif

#if 0
int main() {
    HashMap<std::string, int> map;
    map.add("A", 1);
    map.add("A", 2);
    map.add("A", 3);
    map.add("B", 4);
    map.add("B", 5);
    map.add("F", 6);

    for (auto it = map.begin(); it != map.end(); ++it) {
        auto [key, value] = *it;
        std::cout << key << ' ' << value << std::endl;
    }
    map.remove("A");
    std::cout << std::endl;
    for (auto it = map.begin(); it != map.end(); ++it) {
        auto [key, value] = *it;
        std::cout << key << ' ' << value << std::endl;
    }
    return 0;
}
#endif

#if 0
int main() {
    int m1_diag[2] = {1, 2};
    const Matrix m1(2, m1_diag);

    int m2_diag[3] = {-2, -4, 10};
    const Matrix m2(3, m2_diag);

    std::cout << std::hash<Matrix>()(m1) << std::endl;
    std::cout << std::hash<Matrix>()(m2) << std::endl;
}

#endif

#if 1
int main()
{
    MultiHashMap<std::string, int> map;
    map.add("A", 1);
    map.add("A", 2);
    map.add("A", 3);
    map.add("B", 4);
    map.add("B", 5);
    map.add("F", 6);
    map.add("G", 7);

    for (auto it = map.begin(); it != map.end(); ++it)
    {
        auto [key, value] = *it;
        std::cout << key << ' ' << value << std::endl;
    }

    std::cout << std::endl
              << map.get("A") << std::endl;

    std::cout << std::endl;
    for (const auto elem : map.getAll("A"))
    {
        std::cout << elem << ' ';
    }
    std::cout << "\n\n";

    map.removeAll("F");

    for (auto it = map.begin(); it != map.end(); ++it)
    {
        auto [key, value] = *it;
        std::cout << key << ' ' << value << std::endl;
    }
    return 0;
}
#endif
