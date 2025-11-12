/**
 * @file matrix.hpp
 * @brief Generic template matrix class for numerical computations
 * 
 * This module provides a flexible 2D matrix implementation with 1-based indexing,
 * supporting dynamic resizing, I/O operations, and efficient memory management.
 */

#pragma once

#include <iostream>
#include <iomanip>
#include <cassert>

/// Default column width for matrix output formatting
#define WIDE 6
/// Default precision for floating-point matrix output
#define PRECISION 1

using namespace std;

namespace GOMA
{
    /**
     * @class matrix
     * @brief Generic 2D matrix template with 1-based indexing
     * 
     * This class provides a flexible matrix implementation suitable for mathematical
     * and optimization applications. Key features:
     * 
     * - **1-based indexing**: Uses mathematical notation (1,1) for first element
     * - **0-based raw access**: Also supports operator[] for raw memory access
     * - **Dynamic resizing**: Can resize with or without preserving existing data
     * - **Memory management**: RAII-compliant with proper copy semantics
     * - **I/O support**: Read/write operations with formatting
     * - **Matrix operations**: Transpose, copy, fill
     * 
     * @tparam T Element type (typically int, double, or float)
     * 
     * @note This class uses row-major storage order
     * @warning Assertion failures occur on out-of-bounds access in debug mode
     * 
     * @example
     * ```cpp
     * // Create a 3x4 matrix initialized to 0.0
     * GOMA::matrix<double> M(3, 4, 0.0);
     * 
     * // Access elements using 1-based indexing
     * M(1, 1) = 5.5;  // First element
     * M(2, 3) = 7.2;  // Second row, third column
     * 
     * // Read from stream
     * std::cin >> M;
     * 
     * // Write to stream (formatted output)
     * std::cout << M;
     * ```
     */
    template <class T>
    class matrix
    {
    private:
        size_t m_; ///< Number of rows
        size_t n_; ///< Number of columns

        T *v_; ///< Data storage (row-major order)

    public:
        /**
         * @brief Default constructor - creates an empty matrix
         */
        matrix(void) : m_(0),
                       n_(0),
                       v_(NULL) {}

        /**
         * @brief Construct a matrix with specified dimensions (uninitialized)
         * @param m Number of rows
         * @param n Number of columns
         */
        matrix(int m, int n) : m_(m),
                               n_(n),
                               v_(NULL)
        {
            build();
        }

        /**
         * @brief Construct a matrix with specified dimensions and initial value
         * @param m Number of rows
         * @param n Number of columns
         * @param data Value to initialize all elements
         */
        matrix(int m, int n, T data) : m_(m),
                                       n_(n),
                                       v_(NULL)
        {
            build();

            fill(data);
        }

        /**
         * @brief Copy constructor
         * @param M Matrix to copy
         */
        matrix(const matrix &M) : m_(M.m_),
                                  n_(M.n_),
                                  v_(NULL)
        {
            build();

            copy(M.v_, M.m_, M.n_, v_, m_, n_);
        }

        /**
         * @brief Destructor - frees allocated memory
         */
        virtual ~matrix(void)
        {
            if (v_)
            {
                delete[] v_;
            }
        }

        /**
         * @brief Resize matrix to new dimensions (data is reinitialized to 0)
         * @param m New number of rows
         * @param n New number of columns
         * @note Previous data is lost
         */
        void resize(size_t m, size_t n)
        {
            if (m == m_ && n == n_)
                return;

            m_ = m;
            n_ = n;

            build();

            init(0);
        }

        /**
         * @brief Resize matrix while preserving existing data
         * @param m New number of rows
         * @param n New number of columns
         * @note Preserves elements up to min(old_size, new_size) in each dimension
         */
        void resize_and_keep(size_t m, size_t n)
        {
            if (m == m_ && n == n_)
                return;

            T *v;

            build(v, m, n);

            copy(v_, m_, n_, v, m, n);

            v_ = v;

            m_ = m;
            n_ = n;
        }

        /**
         * @brief Copy data from one matrix buffer to another
         * @param v Source buffer
         * @param m Source rows
         * @param n Source columns
         * @param w Destination buffer (output)
         * @param p Destination rows
         * @param q Destination columns
         * @note Copies min(m,p) x min(n,q) elements
         */
        void copy(T *v, size_t m, size_t n, T *&w, size_t p, size_t q)
        {
            const size_t l_m = min(m, p);
            const size_t l_n = min(n, q);

            for (size_t i = 1; i <= l_m; i++)
                for (size_t j = 1; j <= l_n; j++)
                    w[pos(q, i, j)] = v[pos(i, j)];
        }

        /**
         * @brief Initialize all elements to a given value
         * @param data Initialization value
         */
        void init(T data)
        {

            fill(data);
        }

        /**
         * @brief Fill all elements with a given value
         * @param data Fill value
         */
        void fill(T data)
        {
            const size_t sz = m_ * n_;

            T *v_ptr{v_};

            for (size_t i{0}; i < sz; i++)
            {
                *v_ptr = data;
                v_ptr++;
            }
        }

        /**
         * @brief 0-based raw array access (mutable)
         * @param i Linear index into raw storage
         * @return Reference to element at position i
         * @warning Asserts i < m*n in debug mode
         */
        T &operator[](size_t i)
        {
            assert(i < m_ * n_);

            return v_[i];
        }

        /**
         * @brief 0-based raw array access (const)
         * @param i Linear index into raw storage
         * @return Const reference to element at position i
         * @warning Asserts i < m*n in debug mode
         */
        const T &operator[](size_t i) const
        {
            assert(i < m_ * n_);

            return v_[i];
        }

        /**
         * @brief 1-based matrix element access (mutable)
         * @param i Row index (1-based)
         * @param j Column index (1-based)
         * @return Reference to element at (i, j)
         * @warning Asserts 1 ≤ i ≤ m and 1 ≤ j ≤ n in debug mode
         */
        T &operator()(size_t i, size_t j)
        {
            assert(i >= 1);
            assert(i <= m_);

            assert(j >= 1);
            assert(j <= n_);

            return v_[pos(i, j)];
        }

        /**
         * @brief 1-based matrix element access with bounds checking (mutable)
         * @param i Row index (1-based)
         * @param j Column index (1-based)
         * @return Reference to element at (i, j)
         * @warning Asserts 1 ≤ i ≤ m and 1 ≤ j ≤ n in debug mode
         */
        T &at(size_t i, size_t j)
        {
            assert(i >= 1);
            assert(i <= m_);

            assert(j >= 1);
            assert(j <= n_);

            return v_[pos(i, j)];
        }

        /**
         * @brief 1-based matrix element access (const)
         * @param i Row index (1-based)
         * @param j Column index (1-based)
         * @return Const reference to element at (i, j)
         * @warning Asserts 1 ≤ i ≤ m and 1 ≤ j ≤ n in debug mode
         */
        const T &operator()(size_t i, size_t j) const
        {
            assert(i >= 1);
            assert(i <= m_);

            assert(j >= 1);
            assert(j <= n_);

            return v_[pos(i, j)];
        }

        /**
         * @brief 1-based matrix element access with bounds checking (const)
         * @param i Row index (1-based)
         * @param j Column index (1-based)
         * @return Const reference to element at (i, j)
         * @warning Asserts 1 ≤ i ≤ m and 1 ≤ j ≤ n in debug mode
         */
        const T &at(size_t i, size_t j) const
        {
            assert(i >= 1);
            assert(i <= m_);

            assert(j >= 1);
            assert(j <= n_);

            return v_[pos(i, j)];
        }

        /**
         * @brief Assignment operator (deep copy)
         * @param M Matrix to copy
         * @return Reference to this matrix
         * @note Resizes if dimensions don't match
         */
        matrix &operator=(const matrix &M)
        {
            if (this == &M)
                return *this;

            if (m_ != M.m_ || n_ != M.n_)
            {
                resize(M.m_, M.n_);
            }

            copy(M.v_, M.m_, M.n_, v_, m_, n_);

            return *this;
        }

        /**
         * @brief Get number of rows
         * @return Number of rows
         */
        inline size_t get_m(void) const
        {
            return m_;
        }

        /**
         * @brief Get number of columns
         * @return Number of columns
         */
        inline size_t get_n(void) const
        {
            return n_;
        }

        /**
         * @brief Get number of rows (alias for get_m)
         * @return Number of rows
         */
        inline size_t get_n_rows(void) const
        {
            return m_;
        }

        /**
         * @brief Get number of columns (alias for get_n)
         * @return Number of columns
         */
        inline size_t get_n_cols(void) const
        {
            return n_;
        }

        /**
         * @brief Compute transpose of this matrix
         * @param M [output] Transposed matrix (will be resized to n x m)
         */
        void transpose(matrix &M) const
        {
            M.resize(n_, m_);

            for (size_t i = 1; i <= m_; i++)
                for (size_t j = 1; j <= n_; j++)
                    M(j, i) = (*this)(i, j);
        }

        /**
         * @brief Read matrix from input stream (raw format)
         * @param is Input stream
         * @return Input stream reference
         * @note Reads m*n values in row-major order
         */
        istream &read_raw(istream &is)
        {

            for (size_t i = 1; i <= m_; i++)
                for (size_t j = 1; j <= n_; j++)
                    is >> v_[pos(i, j)];

            return is;
        }

        /**
         * @brief Write matrix to output stream (formatted)
         * @param os Output stream
         * @return Output stream reference
         * @note Values > 1e6 are displayed as "inf"
         * @note Uses WIDE and PRECISION macros for formatting
         */
        ostream &write_raw(ostream &os) const
        {

            for (size_t i = 1; i <= m_; i++)
            {
                for (size_t j = 1; j <= n_; j++)
                {
                    const double val = v_[pos(i, j)];

                    if (val > 1E+6)
                        os << setw(WIDE) << "   inf ";
                    else
                        os << setw(WIDE) << fixed << setprecision(PRECISION) << val << " ";
                }
                os << endl;
            }

            return os;
        }

        /**
         * @brief Convert 1-based (i,j) indices to 0-based linear index
         * @param i Row index (1-based)
         * @param j Column index (1-based)
         * @return Linear index in row-major storage
         */
        inline int pos(size_t i, size_t j) const
        {
            return (i - 1) * n_ + j - 1;
        }

        /**
         * @brief Convert 1-based (i,j) indices to 0-based linear index (static version)
         * @param n Number of columns
         * @param i Row index (1-based)
         * @param j Column index (1-based)
         * @return Linear index in row-major storage
         */
        inline int pos(size_t n, size_t i, size_t j)
        {
            return (i - 1) * n + j - 1;
        }

    private:
        /**
         * @brief Allocate memory for matrix data
         * @note Deletes existing memory before allocating
         */
        void build(void)
        {
            if (v_)
            {
                delete[] v_;
            }

            if (m_ * n_ > 0)
            {
                v_ = new T[m_ * n_];
            }
            else
            {
                v_ = NULL;
            }
        }

        /**
         * @brief Allocate memory for external buffer
         * @param v Buffer pointer (will be deleted and reallocated)
         * @param m Number of rows
         * @param n Number of columns
         */
        void
        build(T *v, size_t m, size_t n)
        {
            if (v)
            {
                delete[] v;
            }

            if (m * n > 0)
            {
                v = new T[m * n];
            }
            else
            {
                v = NULL;
            }
        }
    };
}

/**
 * @brief Stream input operator for matrix
 * @tparam T Element type
 * @param is Input stream
 * @param M Matrix to read into
 * @return Input stream reference
 */
template <class T>
istream &operator>>(istream &is, GOMA::matrix<T> &M)
{
    M.read_raw(is);
    return is;
}

/**
 * @brief Stream output operator for matrix
 * @tparam T Element type
 * @param os Output stream
 * @param M Matrix to write
 * @return Output stream reference
 */
template <class T>
ostream &operator<<(ostream &os, const GOMA::matrix<T> &M)
{
    M.write_raw(os);
    return os;
}
