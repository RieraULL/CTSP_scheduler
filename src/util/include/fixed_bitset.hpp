/**
 * @file fixed_bitset.hpp
 * @brief Template-based fixed-size bitset with compile-time capacity
 * 
 * Implements a bitset with fixed capacity determined at compile time via template parameter.
 * Uses array of type T (typically unsigned integers) as storage blocks.
 * More efficient than dynamic bitset when capacity is known at compile time.
 * Elements are 1-indexed (range: 1 to N).
 * 
 * Template parameters:
 * - T: Storage block type (e.g., uint64_t, unsigned long)
 * - N: Maximum capacity (number of elements)
 * 
 * Key features:
 * - Compile-time fixed size (no dynamic resizing)
 * - Template parameterized storage type and capacity
 * - Zero overhead abstraction (inline operations)
 * - Set operations using bitwise operations on blocks
 * - GCC intrinsics for efficient bit manipulation
 * 
 * Example:
 * @code
 * fixed_bitset<uint64_t, 100> bs;  // Bitset for 100 elements
 * bs.insert(5);    // Add element 5
 * bs.insert(42);   // Add element 42
 * if (bs.contains(5)) { ... }
 * @endcode
 */

#pragma once

#include <cstring>
#include <cassert>
#include <iostream>
#include <cmath>

using namespace std;

/** @brief Number of bits per storage block of type T */
#define N_BITS_W (8 * sizeof(T))

namespace GOMA
{

	/**
	 * @class fixed_bitset
	 * @brief Fixed-size bitset with compile-time capacity
	 * 
	 * Template-based bitset where capacity N is fixed at compile time.
	 * Storage type T can be customized (typically unsigned int or uint64_t).
	 * Elements are 1-indexed: valid range is [1, N].
	 * 
	 * @tparam T Storage block type (must be unsigned integer type)
	 * @tparam N Maximum number of elements (compile-time constant)
	 * 
	 * Storage:
	 * - block_[]: Array of T blocks (size = ceil(N / N_BITS_W) + 1)
	 * - sz_: Number of blocks allocated
	 * 
	 * Advantages over dynamic bitset:
	 * - No runtime memory allocation overhead
	 * - Compiler can optimize for known size
	 * - Type-safe capacity (checked at compile time)
	 * - Better cache locality for small sets
	 * 
	 * Example:
	 * @code
	 * fixed_bitset<uint64_t, 256> visited;  // Track visited vertices
	 * visited.insert(vertex_id);
	 * if (!visited.contains(next_vertex)) { ... }
	 * @endcode
	 */
	template <class T, size_t N>
	class fixed_bitset
	{
	public:
		T *block_;    ///< Array of bit storage blocks
		int sz_;      ///< Number of blocks (ceil(N/N_BITS_W) + 1)

	public:
		/**
		 * @brief Default constructor: Create empty bitset
		 * 
		 * Allocates ceil(N/N_BITS_W)+1 blocks and clears all bits.
		 * Capacity N is compile-time constant from template parameter.
		 */
		fixed_bitset(void) : block_(NULL),
				   sz_(ceil((double)N / (double)N_BITS_W))
		{

			if ((N % N_BITS_W) != 0)
				sz_++;

			block_ = new T[sz_];

			clear();
		};

		/**
		 * @brief Constructor: Create bitset with single element
		 * @param i Initial element to insert (1-indexed)
		 * 
		 * Allocates blocks, clears all bits, then inserts element i.
		 */
		fixed_bitset(const int i) : block_(NULL),
				   sz_(ceil((double)N / (double)N_BITS_W))
		{

			if ((N % N_BITS_W) != 0)
				sz_++;

			block_ = new T[sz_];

			clear();

			insert(i);
		};

		/**
		 * @brief Copy constructor: Deep copy of bitset
		 * @param bs Source bitset to copy
		 * 
		 * Allocates new block array and copies all bits.
		 */
		fixed_bitset(const fixed_bitset &bs) : block_(NULL),
							   sz_(bs.sz_)
		{
			block_ = new T[bs.sz_];

			for (int i{0}; i < sz_; i++)
				block_[i] = bs.block_[i];
		}		/**
		 * @brief Destructor: Free allocated blocks
		 */
		~fixed_bitset(void)
		{
			if (block_)
				delete[] block_;
		}

		/**
		 * @brief Assignment operator: Copy bits from source
		 * @param bs Source bitset
		 * @return Reference to source bitset
		 * 
		 * Copies all blocks. Both bitsets must have same template parameters (T, N).
		 */
		const fixed_bitset &operator=(const fixed_bitset &bs)
		{
			for (int i{0}; i < sz_; i++)
				block_[i] = bs.block_[i];

			return bs;
		}

		/**
		 * @brief Clear all bits (set to empty)
		 */
		void clear(void)
		{

			for (int i{0}; i < sz_; i++)
				block_[i] = 0;
		}

		/**
		 * @brief Check if bitset is empty
		 * @return true if no bits set, false otherwise
		 * 
		 * Scans all blocks until finding non-zero block or end.
		 */
		bool empty(void) const
		{
			bool empty_set = true;

			int i = 0;

			while ((i < sz_) && (empty_set))
			{
				empty_set = empty_set && (block_[i] == 0);
				i++;
			}

			return empty_set;
		}

		/**
		 * @brief Insert element into bitset
		 * @param i Element to insert (1-indexed, range: 1 to N)
		 * 
		 * Sets bit at position (i-1) using bitwise OR.
		 * Computes block index: pos = (i-1) / N_BITS_W
		 * Computes bit offset: offset = (i-1) % N_BITS_W
		 * Sets bit: block_[pos] |= (1 << offset)
		 */
		void insert(unsigned int i)
		{

			assert(i <= N);
			assert(i >= 1);

			i--;

			const int pos = i / N_BITS_W;
			const int offset = i % N_BITS_W;

			block_[pos] |= (T(0x1) << offset);
		}

		/**
		 * @brief Remove element from bitset
		 * @param i Element to remove (1-indexed)
		 * 
		 * Clears bit at position (i-1) using bitwise AND with inverted mask.
		 * Clears bit: block_[pos] &= ~(1 << offset)
		 */
		void remove(unsigned int i)
		{

			assert(i <= N);
			assert(i >= 1);

			i--;

			const int pos = i / N_BITS_W;
			const int offset = i % N_BITS_W;

			block_[pos] &= ~(T(0x1) << offset);
		}

		/**
		 * @brief Check if element is in bitset
		 * @param i Element to test (1-indexed)
		 * @return true if bit set, false otherwise
		 * 
		 * Tests bit: (block_[pos] & (1 << offset)) != 0
		 */
		bool contains(unsigned int i) const
		{

			assert(i <= N);
			assert(i >= 1);

			i--;

			const int pos = i / N_BITS_W;
			const int offset = i % N_BITS_W;

			return (block_[pos] & (T(0x1) << offset)) != T(0x0);
		}

		/**
		 * @brief Union with another bitset (this |= bs)
		 * @param bs Bitset to add
		 * 
		 * Performs bitwise OR on all blocks: this = this ∪ bs
		 */
		void insert(const fixed_bitset &bs)
		{
			for (int i{0}; i < sz_; i++)
				block_[i] |= bs.block_[i];
		}

		/**
		 * @brief Difference with another bitset (this -= bs)
		 * @param bs Bitset to remove
		 * 
		 * Performs bitwise AND-NOT on all blocks: this = this \ bs
		 */
		void remove(const fixed_bitset &bs)
		{
			for (int i{0}; i < sz_; i++)
				block_[i] &= ~bs.block_[i];
		}

		/**
		 * @brief Check if this is superset of bs (bs ⊆ this)
		 * @param bs Bitset to test
		 * @return true if all elements of bs are in this
		 * 
		 * Tests: (this & bs) == bs for all blocks
		 */
		bool contains(const fixed_bitset &bs) const
		{
			bool contains_set = true;

			int i = 0;

			while ((i < sz_) && (contains_set))
			{
				contains_set = contains_set && ((block_[i] & bs.block_[i]) == bs.block_[i]);
				i++;
			}

			return contains_set;
		}

		/**
		 * @brief Check if bitsets are disjoint (this ∩ bs = ∅)
		 * @param bs Bitset to test
		 * @return true if no common elements
		 * 
		 * Tests: (this & bs) == 0 for all blocks
		 */
		bool disjoint(const fixed_bitset &bs) const
		{
			bool disjoint_set = true;

			int i = 0;

			while ((i < sz_) && (disjoint_set))
			{
				disjoint_set = disjoint_set && ((block_[i] & bs.block_[i]) == T(0x0));
				i++;
			}

			return disjoint_set;
		}

		/**
		 * @brief Find smallest element in bitset
		 * @return Index of first set bit (1-indexed), or -1 if empty
		 * 
		 * Uses __builtin_ffsl (find first set bit in long).
		 * Scans blocks from lowest to highest.
		 */
		int first_item(void) const
		{
			int sm = -1;

			for (int i{0}; (i < sz_) && (sm == -1); i++)
			{
				if (block_[i] != 0)
					sm = first_item(block_[i]) + N_BITS_W * i;
			}

			return sm;
		}

		/**
		 * @brief Find largest element in bitset
		 * @return Index of last set bit (1-indexed), or -1 if empty
		 * 
		 * Uses N_BITS_W - __builtin_clzl (count leading zeros).
		 * Scans blocks from highest to lowest.
		 */
		int last_item(void) const
		{
			int bg = -1;

			for (int i = sz_ - 1; (i >= 0) && (bg == -1); i--)
			{
				if (block_[i] != 0)
					bg = last_item(block_[i]) + N_BITS_W * i;
			}

			return bg;
		}

		/**
		 * @brief Count number of elements in bitset
		 * @return Number of set bits (cardinality)
		 * 
		 * Uses __builtin_popcount (population count - count set bits).
		 * Sums bit counts across all blocks.
		 */
		int cardinality(void) const
		{
			int card = 0;

			for (int i{0}; i < sz_; i++)
				card += cardinality(block_[i]);

			return card;
		}

		/**
		 * @brief Write bitset to output stream as binary string
		 * @param os Output stream
		 * @return Reference to output stream
		 */
		ostream &write(ostream &os) const
		{

			string s;
			to_string(s);
			os << s;

			return os;
		}

		/**
		 * @brief Compute union of two bitsets (C = this ∪ B)
		 * @param B Second operand
		 * @param C Output bitset (modified)
		 * 
		 * Performs bitwise OR: C[i] = this[i] | B[i] for all blocks
		 */
		void union_set(const fixed_bitset &B, fixed_bitset &C) const
		{
			for (int j{0}; j < sz_; j++)
				C.block_[j] = block_[j] | B.block_[j];
		}

		/**
		 * @brief Compute intersection of two bitsets (C = this ∩ B)
		 * @param B Second operand
		 * @param C Output bitset (modified)
		 * 
		 * Performs bitwise AND: C[i] = this[i] & B[i] for all blocks
		 */
		void intersec_set(const fixed_bitset &B, fixed_bitset &C) const
		{
			for (int j{0}; j < sz_; j++)
				C.block_[j] = block_[j] & B.block_[j];
		}

		/**
		 * @brief Compute set difference (C = this \ B)
		 * @param B Second operand (elements to remove)
		 * @param C Output bitset (modified)
		 * 
		 * Performs bitwise AND-NOT: C[i] = this[i] & ~B[i] for all blocks
		 */
		void diff_set(const fixed_bitset &B, fixed_bitset &C) const
		{
			for (int j{0}; j < sz_; j++)
				C.block_[j] = block_[j] & ~B.block_[j];
		}

	private:
		/**
		 * @brief Find first set bit in block (GCC intrinsic)
		 * @param block Block of type T to scan
		 * @return Position of first set bit (1-indexed from LSB)
		 */
		int first_item(T block) const
		{
			return __builtin_ffsl(block);
		}

		/**
		 * @brief Find last set bit in block
		 * @param block Block of type T to scan
		 * @return Position of last set bit (1-indexed from LSB)
		 * 
		 * Uses __builtin_clzl (count leading zeros from MSB)
		 */
		int last_item(T block) const
		{
			return N_BITS_W - __builtin_clzl(block);
		}

		/**
		 * @brief Count set bits in block (GCC intrinsic)
		 * @param block Block of type T to count
		 * @return Number of 1-bits (population count)
		 */
		int cardinality(T block) const
		{
			return __builtin_popcount(block);
		}

		/**
		 * @brief Convert bitset to binary string representation
		 * @param s Output string (modified)
		 * 
		 * Builds string from LSB to MSB, inserting '0' or '1' for each bit.
		 * Respects template parameter N for maximum capacity.
		 */
		void to_string(string &s) const
		{
			for (int j{0}; j < sz_; j++)
			{

				const int sz = min(N_BITS_W, N - j * N_BITS_W);

				T block = block_[j];

				for (int i{0}; i < sz; i++)
				{
					const char c = '0' + (block & T(0x1));
					s.insert(s.begin(), c);
					block >>= 1;
				}
			}
		}
	};

}