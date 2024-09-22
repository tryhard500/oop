#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>

class Complex
{
private:
    double real_;
    double imaginary_;

public:
    Complex();
    Complex(double real, double imaginary);

    friend const Complex operator+(const Complex &left_complex_number, const Complex &right_complex_number);
    friend const Complex operator+(const double &left_double_number, const Complex &right_complex_number);
    friend const Complex operator+(const Complex &left_complex_number, const double &right_double_number);

    friend const Complex operator-(const Complex &left_complex_number, const Complex &right_complex_number);
    friend const Complex operator-(const double &left_double_number, const Complex &right_complex_number);
    friend const Complex operator-(const Complex &left_complex_number, const double &right_double_number);

    friend const Complex operator*(const Complex &left_complex_number, const Complex &right_complex_number);
    friend const Complex operator*(const double &left_double_number, const Complex &right_complex_number);
    friend const Complex operator*(const Complex &left_complex_number, const double &right_double_number);

    friend const bool operator==(const Complex &left_complex_number, const Complex &right_complex_number);
    friend const bool operator==(const double &left_double_number, const Complex &right_complex_number);
    friend const bool operator==(const Complex &left_complex_number, const double &right_double_number);

    friend const bool operator!=(const Complex &left_complex_number, const Complex &right_complex_number);
    friend const bool operator!=(const double &left_double_number, const Complex &right_complex_number);
    friend const bool operator!=(const Complex &left_complex_number, const double &right_double_number);

    friend const double operator~(const Complex &comlex_number);

    friend std::ostream &operator<<(std::ostream &os, const Complex &number);
    friend std::istream &operator>>(std::istream &is, Complex &number);
};

Complex::Complex() : real_(1), imaginary_(0) {}

Complex::Complex(double real, double imaginary) : real_(real), imaginary_(imaginary) {}

const Complex operator+(const Complex &left_number, const Complex &right_number)
{
    return Complex(left_number.real_ + right_number.real_,
                   left_number.imaginary_ + right_number.imaginary_);
}

const Complex operator+(const double &left_double_number, const Complex &right_complex_number)
{
    return Complex(left_double_number + right_complex_number.real_,
                   right_complex_number.imaginary_);
}

const Complex operator+(const Complex &left_complex_number, const double &right_double_number)
{
    return Complex(left_complex_number.real_ + right_double_number,
                   left_complex_number.imaginary_);
}

const Complex operator-(const Complex &left_complex_number, const Complex &right_complex_number)
{
    return Complex(left_complex_number.real_ - right_complex_number.real_,
                   left_complex_number.imaginary_ - right_complex_number.imaginary_);
}

const Complex operator-(const double &left_double_number, const Complex &right_complex_number)
{
    return Complex(left_double_number - right_complex_number.real_,
                   -right_complex_number.imaginary_);
}

const Complex operator-(const Complex &left_complex_number, const double &right_double_number)
{
    return Complex(left_complex_number.real_ - right_double_number,
                   left_complex_number.imaginary_);
}

const Complex operator*(const Complex &left_complex_number, const Complex &right_complex_number)
{
    return Complex(left_complex_number.real_ * right_complex_number.real_ -
                       left_complex_number.imaginary_ * right_complex_number.imaginary_,
                   left_complex_number.real_ * right_complex_number.imaginary_ +
                       left_complex_number.imaginary_ * right_complex_number.real_);
}

const Complex operator*(const double &left_double_number, const Complex &right_complex_number)
{
    return Complex(left_double_number * right_complex_number.real_,
                   left_double_number * right_complex_number.imaginary_);
}

const Complex operator*(const Complex &left_complex_number, const double &right_double_number)
{
    return Complex(left_complex_number.real_ * right_double_number,
                   left_complex_number.imaginary_ * right_double_number);
}

const bool operator==(const Complex &left_complex_number, const Complex &right_complex_number)
{
    if (left_complex_number.real_ == right_complex_number.real_ &&
        left_complex_number.imaginary_ == right_complex_number.imaginary_)
        return true;
    return false;
}

const bool operator==(const double &left_double_number, const Complex &right_complex_number)
{
    if (left_double_number == right_complex_number.real_ &&
        right_complex_number.imaginary_ == 0)
        return true;
    return false;
}

const bool operator==(const Complex &left_complex_number, const double &right_double_number)
{
    if (left_complex_number.real_ == right_double_number &&
        left_complex_number.imaginary_ == 0)
        return true;
    return false;
}

const bool operator!=(const Complex &left_complex_number, const Complex &right_complex_number)
{
    return !(left_complex_number == right_complex_number);
}

const bool operator!=(const double &left_double_number, const Complex &right_complex_number)
{
    return !(left_double_number == right_complex_number);
}

const bool operator!=(const Complex &left_complex_number, const double &right_double_number)
{
    return !(left_complex_number == right_double_number);
}

const double operator~(const Complex &comlex_number)
{
    return sqrt(comlex_number.real_ * comlex_number.real_ +
                comlex_number.imaginary_ * comlex_number.imaginary_);
}

std::ostream &operator<<(std::ostream &os, const Complex &number)
{
    if (number.imaginary_ > 0)
    {
        os << number.real_ << "+i*" << number.imaginary_ << std::endl;
    }
    else
    {
        os << number.real_ << "-i*" << -number.imaginary_ << std::endl;
    }
    return os;
}

std::istream &operator>>(std::istream &is, Complex &number)
{
    is >> number.real_ >> number.imaginary_;
    return is;
}

#define MAIN_TASK 1

int main()
{
    #if MAIN_TASK
    std::ifstream in("input.txt");
    Complex a, b, c, d;
    in >> a >> b >> c >> d;
    in.close();

    std::ofstream out("output.txt");
    Complex i(0, 1);
    out << std::fixed << std::setprecision(2) << (a * b - c * d) + (~a * ~a + i * ~d * ~d) + c * (~b * ~b + i);
    out.close();
    return 0;
    #else


    #endif
}