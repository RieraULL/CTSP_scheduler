/**
 * @file graph.cpp
 * @brief Implementation of graph data structures for DFS path enumeration
 * 
 * Implements directed graph with adjacency list representation and
 * depth-first search to find all simple paths between two vertices.
 * Optimized for routing problems requiring complete path enumeration.
 * 
 * Key implementation details:
 * - Pre-allocated stack (n*(n-1) capacity) avoids dynamic allocation during DFS
 * - Bitsets provide O(1) visited checks and arc existence queries
 * - Adjacency lists use dynamically resized arrays with doubling strategy
 * - Iterative DFS implementation avoids recursion stack overflow
 */

#include "graph.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <cassert>
#include <algorithm>
#include <limits>

#include <cstdint>

#include "bitset.hpp"

namespace GOMA
{
    // ========================================================================
    // succ_list: Adjacency list implementation
    // ========================================================================
    
    /**
     * Constructor: Allocate adjacency list arrays for all vertices
     * 
     * Initializes with capacity (n-1) for each vertex (complete graph capacity).
     * Uses base_succ_[] to track current array capacity for dynamic resizing.
     */
    succ_list::succ_list(const size_t n_vertices) : n_vertices_(n_vertices)
    {
        succ_ = new int *[n_vertices_];
        cost_ = new double *[n_vertices_];
        top_ = new int[n_vertices_];
        base_succ_ = new int[n_vertices_];

        in_succ_ = new search_fixed_bitset[n_vertices_];

        for (size_t i{0}; i < n_vertices_; i++)
        {
            succ_[i] = new int[n_vertices_ - 1];
            cost_[i] = new double[n_vertices_ - 1];
            top_[i] = -1;
            base_succ_[i] = -1;
        }
    }

    /**
     * Default constructor: Initialize empty adjacency list
     * All pointers set to nullptr for safe destruction.
     */
    succ_list::succ_list(void) : n_vertices_{0},
                                 succ_{nullptr},
                                 cost_{nullptr},
                                 top_{nullptr},
                                 base_succ_{nullptr},
                                 in_succ_{nullptr}
    {
    }

    /**
     * Deallocate all internal arrays
     * Frees succ_[], cost_[], top_[], in_succ_[], base_succ_[]
     */
    void succ_list::clear_(void)
    {
        for (size_t i{0}; i < n_vertices_; i++)
        {
            delete[] succ_[i];
            delete[] cost_[i];
        }

        if (succ_)
            delete[] succ_;

        if (cost_)
            delete[] cost_;

        if (top_)
            delete[] top_;

        if (in_succ_)
            delete[] in_succ_;

        if (base_succ_)
            delete[] base_succ_;
    }

    /**
     * Destructor: Free all allocated memory
     */
    succ_list::~succ_list(void)
    {
        clear_();
    }

    /**
     * Reset adjacency list to empty (preserve allocated memory)
     * Resets top_[] indices and clears in_succ_[] bitsets.
     * Does not deallocate memory - allows reuse.
     */
    void succ_list::clear(void)
    {
        for (size_t i{0}; i < n_vertices_; i++)
        {
            top_[i] = base_succ_[i];
            in_succ_[i].clear();
        }
    }

    /**
     * Remove arc (i,j) from adjacency list
     * 
     * Searches for j in succ_[i] and removes it by shifting remaining elements.
     * Updates in_succ_[i] bitset. O(degree(i)) complexity.
     */
    void succ_list::remove_arc(const int i, const int j)
    {
        assert(i >= 0 && i < (int)(n_vertices_));
        assert(j >= 0 && j < (int)(n_vertices_));

        if (!is_arc(i, j))
            return;

        in_succ_[i].remove(j + 1);

        int *succ{succ_[i]};
        double *cost{cost_[i]};
        int &top{top_[i]};

        int k = 0;
        while ((k <= top) && (succ[k] != j))
            k++;

        assert(k <= top);

        succ[k] = succ[top];
        cost[k] = cost[top];
        top--;
    }

    /**
     * Check if arc (i,j) exists
     * 
     * Uses bitset for O(1) lookup via in_succ_[i].contains(j+1)
     * Note: j is stored as (j+1) in bitset (1-indexed)
     * 
     * @return true if arc exists, false otherwise
     */
    bool succ_list::is_arc(const int i, const int j) const
    {
        assert(i >= 0 && i < (int)n_vertices_);
        assert(j >= 0 && j < (int)n_vertices_);

        return in_succ_[i].contains(j + 1);
    }

    /**
     * Add arc (i,j) without cost
     * 
     * Appends j to succ_[i] and marks in bitset.
     * Increments top_[i]. No effect if arc already exists.
     */
    void succ_list::add_arc(const int i, const int j)
    {
        assert(i >= 0 && i < (int)n_vertices_);
        assert(j >= 0 && j < (int)n_vertices_);

        if (is_arc(i, j))
            return;

        top_[i]++;
        succ_[i][top_[i]] = j;
        in_succ_[i].insert(j + 1);
    }

    /**
     * Add arc (i,j) with cost
     * 
     * Calls add_arc(i,j) then stores cost in cost_[i][top_[i]].
     * Cost is stored at same index as successor vertex.
     */
    void succ_list::add_arc(const int i, const int j, const double cost)
    {
        add_arc(i, j);
        cost_[i][top_[i]] = cost;
    }

    /**
     * Get successor array for vertex i
     * 
     * Returns pointer to internal succ_[i] array and its size.
     * Array contains vertex IDs of all neighbors.
     * 
     * @param[in] i Source vertex
     * @param[out] succ Pointer to successor array (not copied, direct reference)
     * @param[out] sz Number of successors (top_[i] + 1)
     */
    void succ_list::successors(const int i, int *&succ, size_t &sz) const
    {
        assert(i >= 0 && i < (int)n_vertices_);

        sz = top_[i] + 1;

        succ = succ_[i];
    }

    /**
     * Get successor array and costs for vertex i
     * 
     * Returns pointers to both succ_[i] and cost_[i] arrays.
     * Delegates to successors(i, succ, sz) then adds cost pointer.
     * 
     * @param[in] i Source vertex
     * @param[out] succ Pointer to successor array
     * @param[out] cost Pointer to cost array (parallel to succ)
     * @param[out] sz Number of successors
     */
    void succ_list::successors(const int i, int *&succ, double *&cost, size_t &sz) const
    {
        successors(i, succ, sz);
        cost = cost_[i];
    }

    // ========================================================================
    // search_fixed_bitsetable: Dynamic bitset array
    // ========================================================================

    /**
     * Constructor: Allocate array of sz bitsets
     * Initializes empty table (top_ = -1)
     */
    search_fixed_bitsetable::search_fixed_bitsetable(int sz) : sz_(sz),
                                                               top_(-1)
    {
        table_ = new search_fixed_bitset[sz_];

        assert(table_ != NULL);
    }

    /**
     * Destructor: Free bitset array
     */
    search_fixed_bitsetable::~search_fixed_bitsetable(void)
    {
        if (table_)
            delete[] table_;
    }

    /**
     * Clear table: Reset to empty without deallocation
     */
    void search_fixed_bitsetable::clear(void)
    {
        top_ = -1;
    }

    /**
     * Add bitset to end of table
     * Increments top_ and copies bitset via assignment operator.
     */
    void search_fixed_bitsetable::push_back(const search_fixed_bitset &bs)
    {
        assert(top_ < sz_ - 1);
        top_++;
        table_[top_] = bs;
    }

    // ========================================================================
    // node_features: Lightweight node state with int visited
    // ========================================================================

    /**
     * Constructor: Initialize from path and visited integer
     */
    node_features::node_features(const int id, const std::vector<int> &path, const int visited) : id_(id), path_(path), visited_(visited)
    {
    }

    /**
     * Constructor: Initialize with empty path, pre-reserve capacity
     */
    node_features::node_features(const int id, const int n_vertices, const int visited) : id_(id), path_(), visited_(visited)
    {
        path_.reserve(n_vertices);
    }

    /**
     * Default constructor: Invalid node
     */
    node_features::node_features(void) : id_(-1), path_(), visited_(-1)
    {
    }

    /**
     * Destructor
     */
    node_features::~node_features(void)
    {
    }

    // ========================================================================
    // node_info: Full node state with bitset visited
    // ========================================================================

    /**
     * Constructor: Initialize all fields from parameters
     */
    node_info::node_info(const int id, const std::vector<int> &path, const search_fixed_bitset &visited, const double cost) : id_(id),
                                                                                                                              path_(path),
                                                                                                                              visited_(visited),
                                                                                                                              cost_(cost)
    {
    }

    /**
     * Default constructor: Invalid node with empty state
     */
    node_info::node_info(void) : id_(-1),
                                 path_(),
                                 visited_(),
                                 cost_(0.0)
    {
    }

    /**
     * Destructor
     */
    node_info::~node_info(void)
    {
    }

    /**
     * Copy all fields from another node_info
     */
    void node_info::set(const node_info &node_info_v)
    {
        id_ = node_info_v.id_;
        path_ = node_info_v.path_;
        visited_ = node_info_v.visited_;
        cost_ = node_info_v.cost_;
    }

    /**
     * Set vertex ID only (reset other fields to defaults)
     */
    void node_info::set(const int i)
    {
        id_ = i;
        path_.push_back(i);
        visited_.clear();
        visited_.insert(i + 1);
    }

    /**
     * Clear all fields to default values
     */
    void node_info::clear_(void)
    {
        id_ = -1;
        path_.clear();
        visited_.clear();
        cost_ = 0.0;
    }

    // ========================================================================
    // node_info_stack: Pre-allocated stack for DFS
    // ========================================================================

    /**
     * Default constructor: Empty stack
     */
    node_info_stack::node_info_stack(void) : vector<node_info>(), top_(-1)
    {
    }

    /**
     * Constructor: Pre-allocate stack with capacity n*(n-1)
     * 
     * Allocates enough space for worst-case DFS (all simple paths).
     * Avoids dynamic allocation during search.
     */
    node_info_stack::node_info_stack(const int n) : vector<node_info>(n * (n - 1)), top_(-1)
    {
    }

    /**
     * Destructor
     */
    node_info_stack::~node_info_stack(void)
    {
    }

    /**
     * Push vertex with path and visited set onto stack
     * 
     * Increments top_, then updates node_info at(top_) with:
     * - id: vertex ID
     * - path: copied from parameter
     * - visited: copied from bs, then inserts id+1
     */
    void node_info_stack::push(const int id, const vector<int> &path, const search_fixed_bitset &bs)
    {
        top_++;

        node_info &node = at(top_);

        node.get_id() = id;
        node.get_path() = path;
        node.get_visited() = bs;
        node.get_visited().insert(id + 1);
    }

    /**
     * Push vertex with empty path onto stack
     * 
     * Used for initial vertex in DFS. Initializes:
     * - id: vertex ID
     * - path: empty
     * - visited: {id+1}
     * - cost: 0.0
     */
    void node_info_stack::push(const int id)
    {
        top_++;

        node_info &node = at(top_);

        node.get_id() = id;
        node.get_path() = {};
        node.get_visited().clear();
        node.get_visited().insert(id + 1);
        node.get_cost() = 0.0;
    }

    /**
     * Pop top element from stack
     * Decrements top_ (does not deallocate memory)
     */
    void node_info_stack::pop(void)
    {
        top_--;
    }

    /**
     * Access top element without removing
     * @return Const reference to node_info at top of stack
     */
    const node_info &node_info_stack::top(void) const
    {
        return at(top_);
    }

    // ========================================================================
    // search_graph: Main graph class with DFS path enumeration
    // ========================================================================

    /**
     * Constructor: Initialize graph with n vertices
     * 
     * Pre-allocates:
     * - succ_: adjacency list for n+1 vertices (0-indexed + extra)
     * - stack_: capacity n*(n-1) for DFS
     * - active_vertices_: bitset for n vertices
     */
    search_graph::search_graph(const size_t n_vertices) : n_vertices_(n_vertices), succ_(n_vertices + 1), stack_(n_vertices),
                                                          active_vertices_(n_vertices)
    {
    }

    /**
     * Default constructor: Empty graph
     */
    search_graph::search_graph(void) : n_vertices_(0), succ_(), stack_(), active_vertices_()
    {
    }

    /**
     * Destructor
     */
    search_graph::~search_graph(void)
    {
    }

    /**
     * Clear all arcs and reset stack
     * Empties graph while preserving allocated memory.
     */
    void search_graph::clear(void)
    {
        succ_.clear();
        stack_.clear();
    }

    /**
     * Add directed arc (i,j) with cost
     * 
     * Delegates to succ_.add_arc() then marks both vertices as active.
     * Active vertices are those with at least one incident arc.
     */
    void search_graph::add_arc(const int i, const int j, const double arc_cost)
    {
        succ_.add_arc(i, j, arc_cost);

        active_vertices_.insert(i + 1);
        active_vertices_.insert(j + 1);
    }

    /**
     * Depth-First Search to find all simple paths from s to t
     * 
     * Iterative DFS implementation using pre-allocated stack.
     * Enumerates ALL simple paths (paths without repeated vertices)
     * from source s to target t.
     * 
     * Algorithm:
     * 1. Initialize: clear output p, clear stack, push source s
     * 2. Main loop while stack not empty:
     *    a. Pop current vertex (id) and its path from source
     *    b. Extend path with current vertex
     *    c. If id == t: found complete path, add to results
     *    d. Otherwise: for each unvisited successor j of id,
     *       push (j, extended_path, updated_visited) onto stack
     * 3. Return when stack exhausted (all paths explored)
     * 
     * @param[in] s Source vertex (0-indexed)
     * @param[in] t Target vertex (0-indexed)
     * @param[out] p Vector of all simple paths found (cleared initially)
     * 
     * Each path in p is a vector<int> of vertex IDs from s to t.
     * Paths include both endpoints.
     * 
     * Time complexity: O(V! / (V-L)!) where V = |vertices|, L = avg path length
     * Space complexity: O(V^2) for pre-allocated stack
     * 
     * Example:
     * @code
     * search_graph g(4);
     * g.add_arc(0, 1); g.add_arc(1, 3);
     * g.add_arc(0, 2); g.add_arc(2, 3);
     * vector<vector<int>> paths;
     * g.DFS(0, 3, paths);
     * // paths = {{0,1,3}, {0,2,3}}
     * @endcode
     */
    void search_graph::DFS(const int s, const int t, vector<vector<int>> &p)
    {
        p.clear();
        stack_.clear();

        stack_.push(s);

        // Main DFS loop: process stack until empty
        while (!stack_.empty())
        {
            // Pop current vertex and its state
            const node_info si{stack_.top()};
            stack_.pop();

            const int id{si.get_id()};

            // Extend path with current vertex
            vector<int> path{si.get_path()};
            path.push_back(id);

            // Check if reached target
            if (id == t)
            {
                // Found complete path from s to t
                p.push_back(path);
            }
            else
            {
                // Explore successors of current vertex
                size_t n_succ = 0;
                int *succ = NULL;

                succ_.successors(id, succ, n_succ);

                if (n_succ != 0)
                {
                    // Get visited set for cycle detection
                    const search_fixed_bitset visited{si.get_visited()};

                    // Push all unvisited successors onto stack
                    for (size_t k{0}; k < n_succ; k++)
                    {
                        const int j = *succ;

                        // Only explore if j not yet visited (avoid cycles)
                        if (!visited.contains(j + 1))
                        {
                            // Push successor with extended path and updated visited set
                            stack_.push(j, path, si.get_visited());
                        }

                        succ++;  // Move to next successor in array
                    }
                }
            }
        }
    }

}