/*
НГУ ММФ МКН 3 семестр
Лаборатораня работа по программированию (ООП)
Задание 1, Задача "1.1.1 (усложнённый) Матрицы" + Доп. задание
*/

#include <iostream>
#include <fstream>

class Matrix
{
private:
    size_t size_;
    int **matrix_data_;

public:
    Matrix();
    Matrix(size_t size);
    Matrix(size_t size, int *diag);

    Matrix operator+(const Matrix &other_matrix) const;
    Matrix operator-(const Matrix &other_matrix) const;
    Matrix operator*(const Matrix &other_matrix) const;
    bool operator==(const Matrix &other_matrix) const;
    bool operator!=(const Matrix &other_matrix) const;
    Matrix operator~() const;
    Matrix operator()(const size_t row, const size_t column) const;

    int *operator[](const size_t index) const;

    class Vector
    {
    private:
        const Matrix *matrix_;
        size_t index_;

    public:
        Vector(const Matrix *matrix, size_t index);

        int operator[](const size_t index) const;
        int &operator[](const size_t);
    };

    Vector operator()(const size_t index) const;

    friend std::ostream &operator<<(std::ostream &os, const Matrix &matrix);
    friend std::istream &operator>>(std::istream &is, Matrix &matrix);

    ~Matrix();
};

Matrix::Matrix() : size_(0), matrix_data_(nullptr) {}

Matrix::Matrix(size_t size) : size_(size)
{
    matrix_data_ = new int *[size];
    for (size_t i = 0; i < size; ++i)
    {
        matrix_data_[i] = new int[size]{};
        matrix_data_[i][i] = 1;
    }
}

Matrix::Matrix(size_t size, int *diag) : size_(size)
{
    matrix_data_ = new int *[size];
    for (size_t i = 0; i < size; ++i)
    {
        matrix_data_[i] = new int[size]{};
        matrix_data_[i][i] = diag[i];
    }
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

Matrix::~Matrix()
{
    for (size_t i = 0; i < size_; ++i)
        delete[] this->matrix_data_[i];
    delete[] this->matrix_data_;
}

int main()
{
    std::ifstream in("input.txt");

    int N, k;
    in >> N >> k;

    int *diag = new int[N];
    for (int i = 0; i < N; ++i)
        diag[i] = k;
    Matrix A(N), B(N), C(N), D(N), K(N, diag);
    in >> A >> B >> C >> D;
    delete[] diag;
    diag = nullptr;
    in.close();

    std::ofstream out("output.txt");
    out << (A + B * ~C + K) * ~D;
    out.close();

    std::cout << "123" << std::endl;

#if 0
    for (size_t i = 0; i < 2; i++)
        std::cout << C(0)[i] << '\n';
    std::cout << std::endl;

    C(0)[0] = 1000;

    for (size_t i = 0; i < 2; i++)
        std::cout << C(0)[i] << '\n';
    std::cout << std::endl;
#endif

    return 0;
}