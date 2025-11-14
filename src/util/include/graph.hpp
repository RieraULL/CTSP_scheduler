/**
 * @file graph.hpp
 * @brief Graph data structures for DFS path finding
 * 
 * This module provides a specialized graph infrastructure for computing
 * Depth-First Search (DFS) to find all paths between two vertices.
 * It includes efficient adjacency list representation and stack-based
 * traversal optimized for path enumeration in routing problems.
 */

#pragma once

#define DIJKSTRA_INF 1E+20

#include "fixed_bitset.hpp"
#include "matrix.hpp"
#include "bitset.hpp"
#include <vector>
#include <set>
#include <stack>

namespace GOMA
{

    /**
     * @typedef search_fixed_bitset
     * @brief Fixed-size bitset for marking visited vertices during search
     * 
     * Uses 640-bit capacity, supporting graphs up to 640 vertices.
     * Provides O(1) membership testing for visited vertex tracking.
     */
    typedef fixed_bitset<long, 640> search_fixed_bitset;

    /**
     * @class node_info
     * @brief Information about a node during graph traversal
     * 
     * Stores the state of a vertex during DFS traversal, including:
     * - Current vertex ID
     * - Path from source to this vertex
     * - Set of visited vertices (to avoid cycles)
     * - Accumulated cost along the path
     * 
     * Used internally by search_graph for path enumeration.
     */
    class node_info
    {
    private:
        int id_;                      ///< Current vertex identifier
        std::vector<int> path_;       ///< Path from source to current vertex
        search_fixed_bitset visited_; ///< Set of visited vertices
        double cost_;                 ///< Accumulated cost along path

    public:
        /**
         * @brief Construct node info with full state
         * @param i Vertex ID
         * @param path Path from source to this vertex
         * @param visited Set of visited vertices
         * @param cost Accumulated path cost
         */
        node_info(const int i, const std::vector<int> &path, const search_fixed_bitset &visited, const double cost);
        
        /**
         * @brief Default constructor - creates empty node info
         */
        node_info(void);
        
        virtual ~node_info(void);

        /// @brief Get vertex ID (const)
        inline int get_id(void) const { return id_; }
        /// @brief Get vertex ID (mutable)
        inline int &get_id(void) { return id_; }
        /// @brief Get path from source (const)
        inline const std::vector<int> &get_path(void) const { return path_; }
        /// @brief Get path from source (mutable)
        inline std::vector<int> &get_path(void) { return path_; }
        /// @brief Get visited set (const)
        inline const search_fixed_bitset &get_visited(void) const { return visited_; }
        /// @brief Get visited set (mutable)
        inline search_fixed_bitset &get_visited(void) { return visited_; }
        /// @brief Get accumulated cost (const)
        inline double get_cost(void) const { return cost_; }
        /// @brief Get accumulated cost (mutable)
        inline double& get_cost(void) { return cost_; } 

        /**
         * @brief Check if vertex has been visited
         * @param i Vertex ID to check
         * @return true if vertex i is in visited set
         */
        bool is_visited(const int i) const { return visited_.contains(i + 1); }

        /**
         * @brief Copy state from another node_info
         * @param node_info_v Source node info to copy from
         */
        void set(const node_info &node_info_v);

        /**
         * @brief Initialize as source vertex
         * @param i Vertex ID for source
         */
        void set(const int i);

    private:
        /**
         * @brief Clear all node info data
         */
        void clear_(void);
    };

    /**
     * @class node_info_stack
     * @brief Stack of node_info for DFS traversal
     * 
     * Pre-allocated vector-based stack for efficient node storage
     * during DFS. Avoids dynamic allocation during traversal.
     * Size is pre-allocated to n*(n-1) to accommodate all possible
     * edges in a complete graph.
     */
    class node_info_stack : public std::vector<node_info>
    {
    public:
        int top_; ///< Index of top element (-1 when empty)

    public:
        /**
         * @brief Default constructor - empty stack
         */
        node_info_stack(void);
        
        /**
         * @brief Construct with pre-allocated size
         * @param n Number of vertices (allocates n*(n-1) space)
         */
        node_info_stack(const int n);
        
        virtual ~node_info_stack(void);

        /**
         * @brief Set stack capacity based on graph size
         * @param n Number of vertices
         */
        void set_n(const int n) { resize(n * (n - 1)); }

        /**
         * @brief Push node with full state onto stack
         * @param id Vertex ID
         * @param path Path from source
         * @param bs Visited set (will be updated with id)
         */
        void push(const int id, const vector<int> &path, const search_fixed_bitset &bs);
        
        /**
         * @brief Push node with only ID (creates new path)
         * @param id Vertex ID
         */
        void push(const int id);
        
        /**
         * @brief Remove top element from stack
         */
        void pop(void);
        
        /**
         * @brief Get top element (const)
         * @return Reference to top node_info
         */
        const node_info &top(void) const;
        
        /**
         * @brief Check if stack is empty
         * @return true if stack has no elements
         */
        bool empty(void) const { return top_ == -1; }

        /**
         * @brief Clear stack (reset to empty)
         */
        void clear(void) { top_ = -1; }

        /**
         * @brief Get current stack size
         * @return Number of elements in stack
         */
        int cap(void) const { return top_ + 1; }
    };

    /**
     * @class node_features
     * @brief Simplified node information (legacy/alternative representation)
     * 
     * Lighter-weight version of node_info using integer index instead
     * of bitset for visited tracking. May be used for simpler searches
     * or as intermediate representation.
     */
    class node_features
    {
    private:
        int id_;                ///< Vertex identifier
        std::vector<int> path_; ///< Path from source
        int visited_;           ///< Visited vertices index/count

    public:
        /**
         * @brief Construct with full state
         * @param i Vertex ID
         * @param path Path from source
         * @param visited Visited index
         */
        node_features(const int i, const std::vector<int> &path, const int visited);
        
        /**
         * @brief Construct with pre-allocated path
         * @param i Vertex ID
         * @param n_vertices Reserve space for path
         * @param visited Visited index
         */
        node_features(const int i, const int n_vertices, const int visited);
        
        /**
         * @brief Default constructor
         */
        node_features(void);
        
        virtual ~node_features(void);

        /// @brief Get vertex ID
        inline int get_id(void) const { return id_; }
        /// @brief Get path from source
        inline const std::vector<int> &get_path(void) const { return path_; }
        /// @brief Get visited index
        inline int get_visited_inx(void) const { return visited_; }
    };

    /**
     * @class succ_list
     * @brief Adjacency list for directed graph with arc costs
     * 
     * Efficient successor list representation using arrays.
     * For each vertex i, maintains:
     * - Array of successor vertices
     * - Array of arc costs to successors
     * - Bitset for O(1) arc existence checking
     * 
     * Supports dynamic arc addition/removal during graph construction.
     */
    class succ_list
    {
    private:
        size_t n_vertices_;           ///< Number of vertices in graph

        int **succ_;                  ///< Successor arrays (one per vertex)
        double **cost_;               ///< Arc cost arrays (one per vertex)
        int *top_;                    ///< Top index for each successor list

        int *base_succ_;              ///< Base successor index (for clear)

        search_fixed_bitset *in_succ_; ///< Bitsets for fast arc existence check

    public:
        /**
         * @brief Construct successor list for n vertices
         * @param n_vertices Number of vertices
         */
        succ_list(const size_t n_vertices);
        
        /**
         * @brief Default constructor - empty list
         */
        succ_list(void);
        
        ~succ_list(void);

        /**
         * @brief Clear all arcs (reset to empty graph)
         */
        void clear(void);

        /**
         * @brief Check if arc (i,j) exists
         * @param i Source vertex
         * @param j Target vertex
         * @return true if arc exists
         */
        bool is_arc(const int i, const int j) const;

        /**
         * @brief Add arc (i,j) with default cost 0
         * @param i Source vertex
         * @param j Target vertex
         */
        void add_arc(const int i, const int j);
        
        /**
         * @brief Add arc (i,j) with specified cost
         * @param i Source vertex
         * @param j Target vertex
         * @param cost Arc cost/weight
         */
        void add_arc(const int i, const int j, const double cost);
        
        /**
         * @brief Remove arc (i,j)
         * @param i Source vertex
         * @param j Target vertex
         */
        void remove_arc(const int i, const int j);
        
        /**
         * @brief Get successors of vertex i
         * @param i Vertex
         * @param succ Output: pointer to successor array
         * @param sz Output: number of successors
         */
        void successors(const int i, int *&succ, size_t &sz) const;
        
        /**
         * @brief Get successors and costs of vertex i
         * @param i Vertex
         * @param succ Output: pointer to successor array
         * @param cost Output: pointer to cost array
         * @param sz Output: number of successors
         */
        void successors(const int i, int *&succ, double *&cost, size_t &sz) const;

        /**
         * @brief Get number of successors of vertex
         * @param vertex_i Vertex ID
         * @return Number of outgoing arcs from vertex_i
         */
        size_t get_n_succ(const int vertex_i) const { return top_[vertex_i] + 1; }

    private:
        /**
         * @brief Deallocate all internal arrays
         */
        void clear_(void);
    };

    /**
     * @class search_fixed_bitsetable
     * @brief Dynamic array of fixed bitsets
     * 
     * Pre-allocated array of bitsets for storing visited sets
     * during graph search. Provides stack-like push_back interface.
     */
    class search_fixed_bitsetable
    {
    private:
        search_fixed_bitset *table_; ///< Array of bitsets
        int sz_;                     ///< Allocated capacity
        int top_;                    ///< Index of last element

    public:
        /**
         * @brief Construct table with specified capacity
         * @param sz Maximum number of bitsets
         */
        search_fixed_bitsetable(int sz);
        
        ~search_fixed_bitsetable(void);

        /**
         * @brief Clear table (reset to empty)
         */
        void clear(void);

        /**
         * @brief Add bitset to end of table
         * @param bs Bitset to add
         */
        void push_back(const search_fixed_bitset &bs);

        /**
         * @brief Get index of last element
         * @return Top index (-1 if empty)
         */
        inline int top(void) const { return top_; }
        
        /**
         * @brief Access bitset at index (const)
         * @param i Index
         * @return Bitset at position i
         */
        const search_fixed_bitset &at(int i) const { return table_[i]; }
        
        /**
         * @brief Access bitset at index (mutable)
         * @param i Index
         * @return Bitset at position i
         */
        search_fixed_bitset &at(int i) { return table_[i]; }
    };

    /**
     * @class search_graph
     * @brief Directed graph with DFS path enumeration
     * 
     * Main graph class for finding all simple paths between two vertices
     * using Depth-First Search. Designed for routing problems where all
     * feasible paths need to be enumerated.
     * 
     * Key features:
     * - Efficient adjacency list representation
     * - Pre-allocated stack for DFS (no dynamic allocation during search)
     * - Cycle detection via visited bitsets
     * - Enumerates ALL simple paths from source to target
     * 
     * Usage:
     * @code
     * search_graph g(n_vertices);
     * g.add_arc(0, 1);
     * g.add_arc(1, 2);
     * vector<vector<int>> paths;
     * g.DFS(0, 2, paths);  // Find all paths from 0 to 2
     * @endcode
     */
    class search_graph
    {
    private:
        size_t n_vertices_;              ///< Number of vertices
        succ_list succ_;                 ///< Adjacency list structure

        node_info_stack stack_;          ///< Pre-allocated stack for DFS

        GOMA::bitset active_vertices_;   ///< Set of vertices with incident arcs

    public:
        /**
         * @brief Construct graph with n vertices
         * @param n_vertices Number of vertices
         */
        search_graph(const size_t n_vertices);
        
        /**
         * @brief Default constructor - empty graph
         */
        search_graph(void);
        
        virtual ~search_graph(void);

        /**
         * @brief Add directed arc from i to j with optional cost
         * @param i Source vertex (0-indexed)
         * @param j Target vertex (0-indexed)
         * @param arc_cost Cost/weight of arc (default 0.0)
         * 
         * Adds arc (i,j) to the graph. Both i and j are marked as active.
         * If arc already exists, has no effect.
         */
        void add_arc(const int i, const int j, const double arc_cost = 0.0);

        /**
         * @brief Clear all arcs and reset internal state
         */
        void clear();

        /**
         * @brief Find all simple paths from source to target using DFS
         * @param source Starting vertex (0-indexed)
         * @param target Destination vertex (0-indexed)
         * @param p Output: vector of all simple paths found
         * 
         * Performs iterative depth-first search to enumerate all simple
         * paths (paths with no repeated vertices) from source to target.
         * Each path is represented as a vector of vertex IDs.
         * 
         * Algorithm:
         * 1. Initialize stack with source vertex
         * 2. While stack not empty:
         *    a. Pop current vertex and path
         *    b. If current == target, add path to results
         *    c. Otherwise, push all unvisited successors onto stack
         * 
         * Time complexity: O(V! / (V-L)!) where V = vertices, L = path length
         * Space complexity: O(V^2) for pre-allocated stack
         * 
         * @note Paths are found in reverse DFS order
         * @note Only simple paths (no cycles) are returned
         */
        void DFS(const int source, const int target, vector<vector<int>> &p);
    };

}