#include <algorithm>
#include <bitset>
#include <climits>
#include <functional>
#include <iostream>
#include <queue>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

const long long max_n = 1000;

long long n, m, k;

set<long long> g[max_n];
vector<long long> g2[max_n];
bitset<max_n> g_bit[max_n];

bitset<max_n> maximal_kcore; // Global variable to store the maximal k-core found

vector<pair<long long, long long>> nodes_degrees;
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

vector<bitset<max_n>> result;
unordered_set<bitset<max_n>, BitsetHash> visited; // Use unordered_set for faster lookups

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

bool diameter_must_be_more_than_2(const bitset<max_n> &nodes)
{
    vector<int> current_nodes;
    for (int i = 1; i <= n; ++i)
        if (nodes[i])
            current_nodes.push_back(i);

    for (int v : current_nodes)
    {
        bitset<max_n> two_hop_neighbors;
        two_hop_neighbors[v] = 1;
        for (int u : g2[v])
        {
            if (nodes[u])
            {
                two_hop_neighbors[u] = 1;
                for (int w : g2[u])
                {
                    if (nodes[w])
                        two_hop_neighbors[w] = 1;
                }
            }
        }
        for (int u : current_nodes)
        {
            if (!two_hop_neighbors[u])
            {
                return true; // Not reachable within 2 hops
            }
        }
    }
    return false; // All pairs reachable within 2 hops
}

bool solve(bitset<max_n> nodes, long long diameter)
{
    const long long node_count = nodes.count();
    cerr << "Current subgraph: " << nodes.count() << " " << current_subgraph << "\n";

    visited.insert(nodes);

    current_subgraph++;
    if (k >= n)
        return false;
    if (node_count <= k)
    {
        bool flag = false;
        for (auto &[degree, u] : nodes_degrees)
        {
            if (nodes[u])
                continue;
            bitset<max_n> new_nodes = nodes;

            bool check = false;

            for (int i = 1; i <= n; ++i)
            {
                if (!(g_bit[u][i] && nodes[i]))
                {
                    check = true;
                    break;
                }
            }
            if (!check)
                continue; // u is already connected to all nodes in nodes
            new_nodes[u] = 1;
            if (visited.count(new_nodes) > 0)
                continue;

            if (solve(new_nodes, diameter) && (maximal_kcore | new_nodes) == maximal_kcore)
            {
                flag = true;
            }
        }

        return flag;
    }
    else if (node_count >= k + 1 && node_count <= 2 * k + 1 && nodes.count() < maximal_kcore.count())
    {
        bool flag = false;
        for (auto &[degree, u] : nodes_degrees)
        {
            if (nodes[u])
                continue;
            bitset<max_n> new_nodes = nodes;

            bool check = false;

            for (int i = 1; i <= n; ++i)
            {
                if (!(g_bit[u][i] && nodes[i]))
                {
                    check = true;
                    break;
                }
            }
            if (!check)
                continue; // u is already connected to all nodes in nodes

            new_nodes[u] = 1;

            if (visited.count(new_nodes) > 0 || diameter_must_be_more_than_2(new_nodes))
                continue;

            if (solve(new_nodes, diameter) && (maximal_kcore | new_nodes) == maximal_kcore)
            {
                flag = true;
            }
        }
        if (check_kcore(nodes))
        {
            if (flag == false)
            {
                result.push_back(nodes);
                flag = true;
            }
        }

        return flag;
    }
    else
    {
        if (calc_diameter(nodes) > diameter)
        {
            return false;
        }

        if (nodes.count() == maximal_kcore.count())
        {
            if (check_kcore(nodes))
            {
                result.push_back(nodes);

                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            bool flag = false;
            for (auto &[degree, u] : nodes_degrees)
            {
                if (nodes[u])
                    continue;
                bitset<max_n> new_nodes = nodes;

                bool check = false;

                for (int i = 1; i <= n; ++i)
                {
                    if (!(g_bit[u][i] && nodes[i]))
                    {
                        check = true;
                        break;
                    }
                }
                if (!check)
                    continue; // u is already connected to all nodes in nodes
                new_nodes[u] = 1;
                if (visited.count(new_nodes) > 0 || diameter_must_be_more_than_2(new_nodes))
                    continue;
                if (solve(new_nodes, diameter) && (maximal_kcore | new_nodes) == maximal_kcore)
                {
                    flag = true;
                }
            }
            if (check_kcore(nodes))
            {
                if (flag == false)
                {
                    result.push_back(nodes);
                    flag = true;
                }
            }

            return flag;
        }
    }
    return false; // Should not reach here
}

vector<bitset<max_n>> find_kcore_with_diameter(long long diameter = 2)
{
    result.clear();

    nodes_degrees.reserve(n);

    for (int u = 1; u <= n; ++u)
    {
        long long degree = g2[u].size();
        if (degree < k)
            continue;
        nodes_degrees.push_back({degree, u});
    }

    sort(nodes_degrees.begin(), nodes_degrees.end(), greater<pair<long long, long long>>());

    for (auto &[_, v] : nodes_degrees)
    {
        if (static_cast<long long>(g[v].size()) < k)
            continue;

        bitset<max_n> current_set;
        current_set[v] = 1;

        solve(current_set, diameter);
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
        g_bit[u][v] = g_bit[v][u] = 1; // Initialize adjacency bitset
    }

    for (int i = 1; i <= n; i++)
    {
        maximal_kcore[i] = 1;
    }

    find_kcore_from_bitset(maximal_kcore);

    vector<bitset<max_n>> res = check_result(find_kcore_with_diameter(2));

    for (bitset<max_n> &x : res)
    {
        cout << "Maximal " << k << "-core with 2-diameter is: ";
        for (int i = 1; i <= n; i++)
        {
            if (x[i])
                cout << i << " ";
        }
        cout << endl;
    }

    return 0;
}
