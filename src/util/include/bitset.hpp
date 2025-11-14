/**
 * @file bitset.hpp
 * @brief Dynamic bitset implementation with block-based storage
 * 
 * Implements a resizable bitset using array of long integers as storage blocks.
 * Provides efficient set operations using bitwise operations on 64-bit words.
 * Elements are 1-indexed (range: 1 to max_inx).
 * 
 * Key features:
 * - Dynamic sizing (can resize at runtime)
 * - Block-based storage (long integers as bit containers)
 * - Set operations: union, intersection, difference
 * - Optimized membership tests using bit manipulation
 * - Built-in GCC intrinsics for bit counting and scanning
 */

#pragma once

#include <cstring>
#include <cassert>
#include <iostream>
#include <cmath>

using namespace std;

/** @brief Number of bits per storage block (typically 64 on 64-bit systems) */
#define N_BITS_WORD (8 * sizeof(long))

namespace GOMA
{

	/**
	 * @class bitset
	 * @brief Dynamic bitset with resizable capacity
	 * 
	 * Stores bits in array of long integers for efficient operations.
	 * Elements are 1-indexed: valid range is [1, max_inx].
	 * 
	 * Storage:
	 * - block_[]: Array of long integers (each holds N_BITS_WORD bits)
	 * - sz_: Number of blocks allocated
	 * - max_inx_: Maximum element index (capacity)
	 * 
	 * Example:
	 * @code
	 * bitset bs(100);      // Create bitset for elements 1-100
	 * bs.insert(5);        // Add element 5
	 * bs.insert(42);       // Add element 42
	 * bool has = bs.contains(5);  // true
	 * int count = bs.cardinality(); // 2
	 * @endcode
	 */
	class bitset
	{
	public:
		long *block_;      ///< Array of bit storage blocks
		size_t sz_;        ///< Number of blocks (ceil(max_inx/N_BITS_WORD))
		size_t max_inx_;   ///< Maximum element index (1-based)

	public:
		/**
		 * @brief Construct bitset with specified capacity
		 * @param max_inx Maximum element index (1-based)
		 * 
		 * Allocates ceil((max_inx+1)/N_BITS_WORD) blocks.
		 * All bits initialized to 0 (empty set).
		 */
		bitset(size_t max_inx) : block_(NULL),
				   sz_(ceil((double)(max_inx + 1) / (double)N_BITS_WORD)),
				   max_inx_(max_inx)
		{

			block_ = new long [sz_];
			clear();
		};

		/**
		 * @brief Default constructor: Create bitset with capacity 65536
		 * 
		 * Provides reasonable default size for general use.
		 */
		bitset(void) : block_(NULL),
				   sz_(ceil((double)(65536 + 1) / (double)N_BITS_WORD)),
				   max_inx_(65536)
		{

			block_ = new long [sz_];
			clear();
		};

		/**
		 * @brief Copy constructor: Deep copy of bitset
		 * @param bs Source bitset to copy
		 * 
		 * Allocates new block array and copies all bits.
		 */
		bitset(const bitset &bs) : block_(NULL),
							   sz_(bs.sz_),
							   max_inx_(bs.max_inx_)
		{
			block_ = new long[sz_];

			for (size_t i{0}; i < sz_; i++)
				block_[i] = bs.block_[i];
		}		/**
		 * @brief Destructor: Free allocated blocks
		 */
		~bitset(void)
		{
			if (block_)
				delete[] block_;
		}

		/**
		 * @brief Get maximum element index (capacity)
		 * @return Maximum index (1-based)
		 */
		size_t get_size(void) const
		{
			return max_inx_;
		}

		/**
		 * @brief Resize bitset to new capacity
		 * @param max_inx New maximum element index
		 * 
		 * Deallocates old storage, allocates new blocks, clears all bits.
		 * WARNING: All existing data is lost.
		 */
		void set_size(size_t max_inx)
		{
			if (block_)
				delete[] block_;

			max_inx_ = max_inx;
			sz_ = ceil((double)(max_inx + 1) / (double)N_BITS_WORD);

			block_ = new long[sz_];
			clear();
		}

		/**
		 * @brief Assignment operator: Copy bits from source
		 * @param bs Source bitset
		 * @return Reference to source bitset
		 * 
		 * Copies all blocks. Requires same size (sz_).
		 */
		const bitset &operator=(const bitset &bs)
		{
			for (size_t i{0}; i < sz_; i++)
				block_[i] = bs.block_[i];

			return bs;
		}

		/**
		 * @brief Clear all bits (set to empty)
		 */
		void clear(void)
		{

			for (size_t i{0}; i < sz_; i++)
				block_[i] = 0;
		}

		/**
		 * @brief Alias for clear()
		 */
		void reset(void)
		{
			clear();
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

			size_t i{0};

			while ((i < sz_) && (empty_set))
			{
				empty_set = empty_set && (block_[i] == 0);
				i++;
			}

			return empty_set;
		}

		/**
		 * @brief Insert element into bitset
		 * @param i Element to insert (1-indexed, range: 1 to max_inx)
		 * 
		 * Sets bit at position (i-1) using bitwise OR.
		 * Computes block index: pos = (i-1) / N_BITS_WORD
		 * Computes bit offset: offset = (i-1) % N_BITS_WORD
		 * Sets bit: block_[pos] |= (1 << offset)
		 */
		void insert(unsigned int i)
		{

			assert(i <= max_inx_);
			assert(i >= 1);

			i--;

			const size_t pos = i / N_BITS_WORD;
			const size_t offset = i % N_BITS_WORD;

			block_[pos] |= (long(0x1) << offset);
		}

		/**
		 * @brief Alias for insert()
		 * @param i Element to set
		 */
		void set(unsigned int i)
		{
			insert(i);
		}

		/**
		 * @brief Alias for remove()
		 * @param i Element to reset
		 */
		void reset(unsigned int i)
		{
			remove(i);
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

			assert(i <= max_inx_);
			assert(i >= 1);

			i--;

			const size_t pos = i / N_BITS_WORD;
			const size_t offset = i % N_BITS_WORD;

			block_[pos] &= ~(long(0x1) << offset);
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

			assert(i <= max_inx_);
			assert(i >= 1);

			i--;

			const size_t pos = i / N_BITS_WORD;
			const size_t offset = i % N_BITS_WORD;

			return (block_[pos] & (long(0x1) << offset)) != long(0x0);
		}

		/**
		 * @brief Alias for contains()
		 * @param i Element to test
		 * @return true if element present
		 */
		bool test(unsigned int i) const
		{
			return contains(i);
		}

		/**
		 * @brief Alias for contains()
		 * @param i Element to find
		 * @return true if element present
		 */
		bool find(unsigned int i) const
		{
			return contains(i);
		}

		/**
		 * @brief Union with another bitset (this |= bs)
		 * @param bs Bitset to add
		 * 
		 * Performs bitwise OR on all blocks: this = this ∪ bs
		 */
		void insert(const bitset &bs)
		{
			for (size_t i{0}; i < sz_; i++)
				block_[i] |= bs.block_[i];
		}

		/**
		 * @brief Difference with another bitset (this -= bs)
		 * @param bs Bitset to remove
		 * 
		 * Performs bitwise AND-NOT on all blocks: this = this \ bs
		 */
		void remove(const bitset &bs)
		{
			for (size_t i{0}; i < sz_; i++)
				block_[i] &= ~bs.block_[i];
		}

		/**
		 * @brief Check if this is superset of bs (bs ⊆ this)
		 * @param bs Bitset to test
		 * @return true if all elements of bs are in this
		 * 
		 * Tests: (this & bs) == bs for all blocks
		 */
		bool contains(const bitset &bs) const
		{
			bool contains_set = true;

			size_t i{0};

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
		bool disjoint(const bitset &bs) const
		{
			bool disjoint_set = true;

			size_t i{0};

			while ((i < sz_) && (disjoint_set))
			{
				disjoint_set = disjoint_set && ((block_[i] & bs.block_[i]) == long(0x0));
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

			for (size_t i{0}; (i < sz_) && (sm == -1); i++)
			{
				if (block_[i] != 0)
					sm = first_item(block_[i]) + N_BITS_WORD * i;
			}

			return sm;
		}

		/**
		 * @brief Find largest element in bitset
		 * @return Index of last set bit (1-indexed), or -1 if empty
		 * 
		 * Uses N_BITS_WORD - __builtin_clzl (count leading zeros).
		 * Scans blocks from highest to lowest.
		 */
		int last_item(void) const
		{
			int bg = -1;

			for (int i = sz_ - 1; (i >= 0) && (bg == -1); i--)
			{
				if (block_[i] != 0)
					bg = last_item(block_[i]) + N_BITS_WORD * i;
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

			for (size_t i{0}; i < sz_; i++)
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
		void union_set(const bitset &B, bitset &C) const
		{
			for (size_t j{0}; j < sz_; j++)
				C.block_[j] = block_[j] | B.block_[j];
		}

		/**
		 * @brief Compute intersection of two bitsets (C = this ∩ B)
		 * @param B Second operand
		 * @param C Output bitset (modified)
		 * 
		 * Performs bitwise AND: C[i] = this[i] & B[i] for all blocks
		 */
		void intersec_set(const bitset &B, bitset &C) const
		{
			for (size_t j{0}; j < sz_; j++)
				C.block_[j] = block_[j] & B.block_[j];
		}

		/**
		 * @brief Compute set difference (C = this \ B)
		 * @param B Second operand (elements to remove)
		 * @param C Output bitset (modified)
		 * 
		 * Performs bitwise AND-NOT: C[i] = this[i] & ~B[i] for all blocks
		 */
		void diff_set(const bitset &B, bitset &C) const
		{
			for (size_t j{0}; j < sz_; j++)
				C.block_[j] = block_[j] & ~B.block_[j];
		}

	private:
		/**
		 * @brief Find first set bit in block (GCC intrinsic)
		 * @param block Long integer to scan
		 * @return Position of first set bit (1-indexed from LSB)
		 */
		int first_item(long block) const
		{
			return __builtin_ffsl(block);
		}

		/**
		 * @brief Find last set bit in block
		 * @param block Long integer to scan
		 * @return Position of last set bit (1-indexed from LSB)
		 * 
		 * Uses __builtin_clzl (count leading zeros from MSB)
		 */
		int last_item(long block) const
		{
			return N_BITS_WORD - __builtin_clzl(block);
		}

		/**
		 * @brief Count set bits in block (GCC intrinsic)
		 * @param block Long integer to count
		 * @return Number of 1-bits (population count)
		 */
		int cardinality(long block) const
		{
			return __builtin_popcount(block);
		}

		/**
		 * @brief Convert bitset to binary string representation
		 * @param s Output string (modified)
		 * 
		 * Builds string from LSB to MSB, inserting '0' or '1' for each bit.
		 */
		void to_string(string &s) const
		{
			for (size_t j{0}; j < sz_; j++)
			{

				const size_t sz = min(N_BITS_WORD,max_inx_ - j * N_BITS_WORD);

				long block = block_[j];

				for (size_t i{0}; i < sz; i++)
				{
					const char c = '0' + (block & long(0x1));
					s.insert(s.begin(), c);
					block >>= 1;
				}
			}
		}
	};

}