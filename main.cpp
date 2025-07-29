#define _USE_MATH_DEFINES

#include <iostream>
#include <iomanip>
#include <string>

#include <vector>
#include <queue>
#include <deque>
#include <stack>
#include <set>
#include <map>
#include <unordered_map>
#include <climits>
#include <algorithm>
#include <bitset>
#include <unordered_set>

using namespace std;

const long long max_n = 1000;

long long n, m, k;

set<long long> g[max_n];
vector<long long> g2[max_n]; // Unused, but kept for potential future use
bitset<max_n> g_bit[max_n];
// bitset<max_n> n2[max_n];  // REMOVED: unused array wasting ~125KB

long long current_subgraph = 0; // Global variable to track current subgraph index

vector<bitset<max_n>> check_result(const vector<bitset<max_n>> &res)
{
    vector<bitset<max_n>> result;
    vector<bitset<max_n>> sorted_res = res;
    sort(sorted_res.begin(), sorted_res.end(),
         [](const bitset<max_n> &a, const bitset<max_n> &b)
         {
             return a.count() > b.count();
         });

    for (const bitset<max_n> &R : sorted_res)
    {
        bool flag = false;
        for (bitset<max_n> &x : result)
        {
            if ((x | R) == x)
            {
                // R is a subset of x, skip it
                flag = true;
                break;
            }
        }
        if (!flag)
        {
            result.push_back(R);
        }
    }
    return result;
}

// Custom hash function for bitset - improved version using more bits
struct BitsetHash
{
    size_t operator()(const bitset<max_n> &bs) const
    {
        size_t seed = 0;
        // Hash multiple blocks to reduce collisions - use up to 256 bits
        size_t blocks_to_hash = min(size_t(256), bs.size());

        for (size_t i = 0; i < blocks_to_hash; i += 64)
        {
            size_t block = 0;
            for (size_t j = 0; j < 64 && i + j < bs.size(); ++j)
            {
                if (bs[i + j])
                {
                    block |= (1ULL << j);
                }
            }
            seed ^= hash<size_t>{}(block) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        // Also hash the population count to further differentiate bitsets
        seed ^= hash<size_t>{}(bs.count()) + 0x517cc1b7;
        return seed;
    }
};

// make this global to save stack memory
vector<bitset<max_n>> result;
unordered_set<bitset<max_n>, BitsetHash> visited; // Use unordered_set for faster lookups

priority_queue<pair<long long, long long>, vector<pair<long long, long long>>,
               greater<pair<long long, long long>>>
    pq;

/**
 * @brief Find maximal k-core subgraph in G, we proved that there is at most 1 maximal k-core in G
 *
 * @return set<long long>
 */
set<long long> find_maximal_kcore()
{
    vector<bool> removed(n + 1, false);

    while (!pq.empty() && pq.top().first < k)
    {
        auto [deg, u] = pq.top();
        pq.pop();

        if (removed[u] || g[u].size() < k)
        {
            removed[u] = true;
            for (auto v : g[u])
            {
                g[v].erase(u);
                if (!removed[v] && g[v].size() < k)
                {
                    pq.push({g[v].size(), v});
                }
            }
            g[u].clear();
        }
    }

    set<long long> kcore;
    for (long long i = 1; i <= n; ++i)
    {
        if (!removed[i] && g[i].size() >= k)
            kcore.insert(i);
    }
    return kcore;
}

// void neighbour_decomposition()
// {
//     // REMOVED: This function used n2 array which was unused and wasted memory
// }

long long calc_diameter(const bitset<max_n> &nodes)
{
    if (nodes.count() <= 1)
        return 0;

    // Create a list of active nodes for faster iteration
    vector<int> active_nodes;
    active_nodes.reserve(nodes.count());
    for (int i = 1; i <= n; ++i)
    {
        if (nodes[i])
            active_nodes.push_back(i);
    }

    int num_active = active_nodes.size();

    // First check connectivity with optimized BFS
    bitset<max_n> visited;
    queue<int> q;
    q.push(active_nodes[0]);
    visited[active_nodes[0]] = 1;
    int connected_count = 1;

    while (!q.empty())
    {
        int u = q.front();
        q.pop();

        // Only check actual neighbors instead of all n nodes
        for (auto v : g2[u])
        {
            if (nodes[v] && !visited[v])
            {
                visited[v] = 1;
                q.push(v);
                connected_count++;
            }
        }
    }

    // Check if graph is connected
    if (connected_count != num_active)
        return LLONG_MAX;

    // Check for diameter 1 (complete graph) - optimized
    bool is_diameter_1 = true;
    for (int i = 0; i < num_active && is_diameter_1; ++i)
    {
        int u = active_nodes[i];
        for (int j = i + 1; j < num_active && is_diameter_1; ++j)
        {
            int v = active_nodes[j];
            if (!g_bit[u][v])
            {
                is_diameter_1 = false;
            }
        }
    }
    if (is_diameter_1)
        return 1;

    // Optimized diameter 2 check using bitwise operations
    for (int i = 0; i < num_active; ++i)
    {
        int u = active_nodes[i];

        for (int j = i + 1; j < num_active; ++j)
        {
            int v = active_nodes[j];

            if (g_bit[u][v])
                continue; // Already connected

            // Check if u and v have a common neighbor using bitwise AND
            // This replaces the O(n) inner loop with O(1) bitwise operation
            if ((g_bit[u] & g_bit[v] & nodes).none())
            {
                return 3; // Found a pair with distance > 2
            }
        }
    }

    return 2; // All pairs have distance <= 2
}

bool check_kcore(const bitset<max_n> &nodes)
{
    for (int u = 1; u <= n; ++u)
    {
        if (!nodes[u])
            continue;
        long long degree = (g_bit[u] & nodes).count();
        if (degree < k)
        {
            return false;
        }
    }
    return true;
}

bool degeneracy_prune(const bitset<max_n> &nodes, long long k)
{
    // If degeneracy < k, no k-core exists
    vector<int> degree(n + 1, 0);
    for (int i = 1; i <= n; i++)
    {
        if (nodes[i])
        {
            degree[i] = (g_bit[i] & nodes).count();
        }
    }

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;
    for (int i = 1; i <= n; i++)
    {
        if (nodes[i])
        {
            pq.push({degree[i], i});
        }
    }

    bitset<max_n> remaining = nodes;
    while (!pq.empty() && pq.top().first < k)
    {
        auto [deg, u] = pq.top();
        pq.pop();

        if (!remaining[u])
            continue;
        remaining[u] = 0;

        // Update neighbors
        for (int v = 1; v <= n; v++)
        {
            if (remaining[v] && g_bit[u][v])
            {
                degree[v]--;
                if (degree[v] < k)
                {
                    pq.push({degree[v], v});
                }
            }
        }
    }

    return remaining.count() >= k;
}

void find_kcore_from_bitset(bitset<max_n> &nodes)
{

    vector<int> degree(n + 1, 0);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;

    // Initialize degrees and priority queue
    for (int i = 1; i <= n; ++i)
    {
        if (nodes[i])
        {
            degree[i] = (g_bit[i] & nodes).count();
            if (degree[i] < k)
            {
                pq.push({degree[i], i});
            }
        }
    }

    // Remove nodes with degree < k and update neighbors
    while (!pq.empty())
    {
        auto [deg, u] = pq.top();
        pq.pop();

        if (!nodes[u] || degree[u] >= k)
            continue;

        nodes[u] = 0;

        for (auto v : g2[u])
        {
            if (nodes[v] && g_bit[u][v])
            {
                degree[v]--;
                if (degree[v] < k)
                {
                    pq.push({degree[v], v});
                }
            }
        }
    }
}
bool is_subgraph_of_result(const bitset<max_n> &nodes)
{
    for (const auto &res : result)
    {
        if ((res | nodes) == res)
        {
            return true; // nodes is a subgraph of res
        }
    }
    return false; // no subgraph found
}

void update_visited(const bitset<max_n> &nodes)
{
    // Create a list of elements to remove (can't modify set while iterating)
    vector<bitset<max_n>> to_remove;

    for (const auto &v : visited)
    {
        if ((nodes | v) == nodes && v != nodes)
        {
            // If v is a subset of nodes, mark it for removal
            to_remove.push_back(v);
        }
        else if ((v | nodes) == v && v != nodes)
        {
            // If nodes is a subset of v, we can skip updating visited
            update_visited(v);
            return;
        }
    }

    // Remove all marked elements
    for (const auto &v : to_remove)
    {

        visited.erase(v);
    }

    // cerr << "Visited nodes updated. Size: " << visited.size() << "\n";
}

bool is_contained_in_visited(const bitset<max_n> &nodes)
{
    for (const auto &v : visited)
    {
        if ((v | nodes) == v)
        {
            return true; // nodes is contained in v
        }
    }
    return false; // no superset found
}

int compute_2hop_coverage(const bitset<max_n> &nodes, int u)
{
    bitset<max_n> covered;
    for (auto v : g2[u])
    {
        if (nodes[v] && g_bit[u][v])
        {
            covered[v] = 1;
            covered |= g_bit[v] & nodes;
        }
    }
    return covered.count();
}

/**
 * @brief This function will peel the minimum degree node from the subgraph until it reaches the diameter of 2
 *
 * @param nodes
 * @return true if the subgraph after peeling is k-core and has diameter <= 2
 * @return false
 */
bool special_check(bitset<max_n> nodes, long long diameter)
{
    while (calc_diameter(nodes) > diameter)
    {
        long long min_degree = LLONG_MAX;
        long long min_node = -1;
        for (int i = 1; i <= n; i++)
        {
            if (nodes[i])
            {
                long long degree = (g_bit[i] & nodes).count();
                if (degree < min_degree)
                {
                    min_degree = degree;
                    min_node = i;
                }
            }
        }
        nodes[min_node] = 0; // Peel the node
        // Check if the remaining nodes still form a k-core
    }

    return check_kcore(nodes);
}
/**
 * @brief This function will peel the maximal k-core to find maximal k-core subgraph with diameter 2
 *
 */
bool solve(bitset<max_n> &nodes, long long diameter, int depth, int branch_index = -1)
{
    // assume that nodes is k-core, we need to check if it has diameter 2

    if (depth <= 20)
        cerr << "Current subgraph: " << nodes.count() << " " << current_subgraph << " "
             << visited.size() << " " << depth << " " << branch_index << "\n";
    current_subgraph++;

    if (nodes.count() >= k + 1 && nodes.count() <= 2 * k + 1)
    {
        result.push_back(nodes);
        cerr << "Found: " << nodes.count() << "\n";

        for (int i = 1; i <= n; i++)
        {
            if (nodes[i])
            {
                cerr << i << " ";
            }
        }
        cerr << "\n";

        result.push_back(nodes);

        if (result.size() == 50000)
        {
            cerr << "Reached 50000 results, checking for duplicates..." << "\n";
            result = check_result(result);
        }

        return true;
    }

    long long current_diameter = calc_diameter(nodes);

    if (current_diameter > diameter)
    {
        if (current_diameter == LLONG_MAX)
        {
            visited.insert(nodes);
            return false; // disconnected graph
        }

        bool flag = false;

        // Pre-calculate degrees once and reuse
        // vector<pair<long long, long long>> nodes_degrees;
        // nodes_degrees.reserve(nodes.count());

        // for (int u = 1; u <= n; ++u)
        // {
        //     if (nodes[u])
        //     {
        //         long long degree = (g_bit[u] & nodes).count();
        //         nodes_degrees.push_back({degree, u});
        //     }
        // }

        // sort(nodes_degrees.begin(), nodes_degrees.end());

        vector<pair<int, int>> two_hop_coverage;
        two_hop_coverage.reserve(nodes.count());
        for (int u = 1; u <= n; ++u)
        {
            if (nodes[u])
            {
                int cov = compute_2hop_coverage(nodes, u);
                two_hop_coverage.emplace_back(cov, u);
            }
        }
        sort(two_hop_coverage.begin(), two_hop_coverage.end());

        for (auto &[cov, u] : two_hop_coverage)
        {
            if (!nodes[u])
                continue;

            bitset<max_n> new_nodes = nodes;
            new_nodes[u] = 0;

            // Early pruning: if removing this node makes graph too small for k-core
            if (new_nodes.count() <= k || is_subgraph_of_result(new_nodes))
                continue;

            if (!check_kcore(new_nodes))
            {
                find_kcore_from_bitset(new_nodes);
            }

            // Pruning: check if result is meaningful and not already found
            if (new_nodes.count() >= k + 1 && !is_subgraph_of_result(new_nodes) && !is_contained_in_visited(new_nodes) && special_check(new_nodes, 2))
            {
                if (solve(new_nodes, diameter, depth + 1, branch_index))
                {
                    flag = true;
                }
            }
        }

        // Mark this configuration as processed (regardless of success)
        visited.insert(nodes);
        update_visited(nodes);
        return flag;
    }
    else if (check_kcore(nodes))
    {
        cerr << "Found: " << nodes.count() << "\n";

        for (int i = 1; i <= n; i++)
        {
            if (nodes[i])
            {
                cerr << i << " ";
            }
        }
        cerr << "\n";

        result.push_back(nodes);

        if (result.size() == 50000)
        {
            cerr << "Reached 50000 results, checking for duplicates..." << "\n";
            result = check_result(result);
        }

        return true;
    }
    else
    {
        // If the nodes do not form a k-core, mark this bitset as visited
        visited.insert(nodes);
        cerr << "Not a k-core: " << nodes.count() << "\n";
        return false;
    }
}

vector<bitset<max_n>> find_kcore_with_diameter(set<long long> maximal_kcore, long long diameter = 2)
{
    // find all subsets
    // check if k_core
    // calculate diameter
    // if current_diameter < diameter, add more node to see if maximal

    // if there is another "maximal", then skip the current subset, otherwise add the subset to result

    result.clear();
    visited.clear(); // Memory fix: clear visited set between different runs

    bitset<max_n> kcore_bitset;
    for (auto &node : maximal_kcore)
    {
        kcore_bitset[node] = 1;
    }

    if (calc_diameter(kcore_bitset) <= 2)
    {
        result.push_back(kcore_bitset);
        return result;
    }

    vector<pair<int, int>> two_hop_coverage;
    two_hop_coverage.reserve(kcore_bitset.count());
    for (int u = 1; u <= n; ++u)
    {
        if (kcore_bitset[u])
        {
            int cov = compute_2hop_coverage(kcore_bitset, u);
            two_hop_coverage.emplace_back(cov, u);
        }
    }
    sort(two_hop_coverage.begin(), two_hop_coverage.end());

    for (auto &[cov, node] : two_hop_coverage)
    {
        bitset<max_n> current_set = kcore_bitset;
        current_set[node] = 0;
        if (!check_kcore(current_set))
        {
            find_kcore_from_bitset(current_set);
        }

        solve(current_set, diameter, 0, node);
    }

    return result;
}

/**
 * @brief Main application to find maximal k-core subgraph with and without diameter
 * Assuming that there is no self-loop and the graph is undirected
 *
 * @return int
 */
int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    cout.tie(NULL);

    cin >> n >> m >> k;

    for (int i = 1; i <= m; i++)
    {
        long long u, v;
        cin >> u >> v;

        g[u].insert(v);
        g[v].insert(u);
        g2[u].push_back(v);
        g2[v].push_back(u);
        g_bit[u][v] = g_bit[v][u] = 1;
    }

    for (int i = 1; i <= n; i++)
    {
        if (g[i].size() < k)
            pq.push({g[i].size(), i});
    }

    set<long long> result = find_maximal_kcore();

    cout << "Maximal " << k << "-core is " << "\n";
    for (auto &it : result)
    {
        cout << it << " ";
    }
    cout << "\n";

    vector<bitset<max_n>> res = check_result(find_kcore_with_diameter(result, 2));

    for (bitset<max_n> &x : res)
    {
        cout << "Maximal " << k << "-core with 2-diameter is: ";
        for (int i = 1; i <= n; i++)
        {
            if (x[i])
                cout << i << " ";
        }
        cout << "\n";
    }

    return 0;
}