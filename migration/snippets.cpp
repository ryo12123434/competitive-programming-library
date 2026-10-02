// Reference snippets prepared for the Codex migration task.
// Do not include this file from contest code. See migration/TASK.md.
// The implementations are intentionally kept close to the original snippets.

#if 0

// ============================================================
// UnionFind
// ============================================================
struct UnionFind {
    vector<int> parent, rank, size, edge_count;

    // 初期化: 根付き木の親、高さ、頂点数、辺の数
    UnionFind(int n) : parent(n, -1), rank(n, 0), size(n, 1), edge_count(n, 0) { }

    // 根を求める
    int root(int x) {
        if (parent[x] == -1) return x;
        return parent[x] = root(parent[x]);
    }

    // xとyが同じグループかどうか
    bool issame(int x, int y) {
        return root(x) == root(y);
    }

    // xとyを含むグループを合体
    bool unite(int x, int y) {
        int rx = root(x), ry = root(y);
        if (rx == ry) {
            edge_count[rx]++;
            return false;
        }
        if (rank[rx] < rank[ry]) swap(rx, ry);
        parent[ry] = rx;
        if (rank[rx] == rank[ry]) rank[rx]++;
        size[rx] += size[ry];
        edge_count[rx] += edge_count[ry] + 1;
        return true;
    }

    // 頂点数を取得
    int getsize(int x) {
        return size[root(x)];
    }

    // 辺の数を取得
    int getedges(int x) {
        return edge_count[root(x)];
    }
};

// ============================================================
// IntervalSet
// ============================================================
// 半開区間 [l, r) を管理する区間セット (被覆長・包含判定・mex付き)
template <typename T>
struct IntervalSet {
    set<pair<T, T>> st;
    T INF_VAL;
    T MINF_VAL;
    long long covered_length;

    IntervalSet() : covered_length(0) {
        if constexpr (numeric_limits<T>::has_infinity) {
            INF_VAL = numeric_limits<T>::infinity();
            MINF_VAL = -numeric_limits<T>::infinity();
        } else {
            INF_VAL = numeric_limits<T>::max() / 2;
            MINF_VAL = numeric_limits<T>::lowest() / 2;
        }
        st.insert({MINF_VAL, MINF_VAL});
        st.insert({INF_VAL, INF_VAL});
    }

    void insert(T l, T r) {
        auto it = st.upper_bound({l, INF_VAL});
        it--;
        if (it->second < l) it++;

        T nl = l, nr = r;
        while (it->first <= r) {
            nl = min(nl, it->first);
            nr = max(nr, it->second);
            if (it->first != MINF_VAL && it->first != INF_VAL) {
                covered_length -= (long long)(it->second - it->first);
            }
            it = st.erase(it);
        }
        if (nl != MINF_VAL && nl != INF_VAL) {
            covered_length += (long long)(nr - nl);
        }
        st.insert({nl, nr});
    }

    void erase(T l, T r) {
        auto it = st.upper_bound({l, INF_VAL});
        it--;
        if (it->second <= l) it++;

        vector<pair<T, T>> leftovers;
        while (it->first < r) {
            if (it->first != MINF_VAL && it->first != INF_VAL) {
                covered_length -= (long long)(it->second - it->first);
            }
            if (it->first < l) leftovers.push_back({it->first, l});
            if (it->second > r) leftovers.push_back({r, it->second});
            it = st.erase(it);
        }
        for (auto p : leftovers) {
            covered_length += (long long)(p.second - p.first);
            st.insert(p);
        }
    }

    bool intersects(T l, T r) const {
       if (l >= r) return false; // 無効な区間、または長さ0の区間

       // 左端が l 以上の最初の区間を検索
       auto it = st.upper_bound({l, INF_VAL});

       // 1つ前の区間（左端が l 以下の最大の区間）と交差しているか確認
       it--;
       if (it->second > l) return true;

       // 1つ後の区間（左端が l より大きい最初の区間）と交差しているか確認
       it++;
       if (it->first < r) return true;

       return false;
    }

    bool contains(T x) const {
        auto it = st.upper_bound({x, INF_VAL});
        it--;
        return it->first <= x && x < it->second;
    }

    T mex(T x = 0) const {
        auto it = st.upper_bound({x, INF_VAL});
        it--;
        if (it->first <= x && x < it->second) return it->second;
        return x;
    }

    void print() const {
        for (auto p : st) {
            if (p.first == MINF_VAL || p.first == INF_VAL) continue;
            cout << p.first << " " << p.second << '\n';
        }
    }

    void debug_print() const {
        cout << \"[ \";
        for (auto p : st) {
            if (p.first == MINF_VAL || p.first == INF_VAL) continue;
            cout << \"[\" << p.first << \", \" << p.second << \") \";
        }
        cout << \"]\\\n\";
    }
};

// ============================================================
// 2D Imos Method (Half-open)
// ============================================================
template<typename T>
struct Imos2D {
    int H, W;
    vector<vector<T>> data;

    Imos2D(int H, int W) : H(H), W(W), data(H + 1, vector<T>(W + 1, 0)) {}

    // 半開区間 [u, d) x [l, r) に val を加算
    void add(int u, int d, int l, int r, T val = 1) {
        u = max(0, min(u, H)); d = max(0, min(d, H));
        l = max(0, min(l, W)); r = max(0, min(r, W));
        if (u >= d || l >= r) return;
        
        data[u][l] += val;
        data[u][r] -= val;
        data[d][l] -= val;
        data[d][r] += val;
    }

    // 累積和を構築
    void build() {
        for (int i = 0; i <= H; i++) {
            for (int j = 1; j <= W; j++) {
                data[i][j] += data[i][j - 1];
            }
        }
        for (int j = 0; j <= W; j++) {
            for (int i = 1; i <= H; i++) {
                data[i][j] += data[i - 1][j];
            }
        }
    }

    // 構築後の値を取得
    T get(int i, int j) const {
        return data[i][j];
    }
};


// ============================================================
// Count Inversions (BIT / ACL)
// ============================================================
// 配列 A の転倒数（i < j かつ A[i] > A[j] となる組の数）を返す
template <typename T>
long long count_inversions(const std::vector<T>& A) {
    int n = A.size();
    if (n <= 1) return 0;

    // 1. 座標圧縮
    std::vector<T> B = A;
    std::sort(B.begin(), B.end());
    B.erase(std::unique(B.begin(), B.end()), B.end());

    // 2. 転倒数の計算
    atcoder::fenwick_tree<int> fw(B.size());
    long long inv_count = 0;

    for (int i = 0; i < n; ++i) {
        int rank = std::lower_bound(B.begin(), B.end(), A[i]) - B.begin();
        inv_count += i - fw.sum(0, rank + 1);
        fw.add(rank, 1);
    }
    
    return inv_count;
}

// ============================================================
// Trie Tree
// ============================================================
// trie木 char_size : 文字の種類数, base : 文字種の0番目
// Trie<26, 'a'>のように初期化 'a'は0, 'c'は2
template<int char_size, int base>
struct Trie{
    struct Node { // 頂点
        vector<int> next;   // 子の頂点番号、なければ-1
        vector<int> accept; // 末端がこの頂点になるstr_id
        int c;              // baseからの間隔
        int common;         // この頂点を共有する文字列数
        Node(int c_) : c(c_), common(0) {
            // 文字の種類をchar_sizeで初期化
            next.assign(char_size, -1);
        }
    };

    vector<Node> nodes;     // trie 木本体
    int root;               // 根
    Trie() : root(0) {
        // 根のみで初期化。根は文字を持たないのでダミーとして-1を渡す
        nodes.push_back(Node(-1));
    }
    
    // 単語の挿入 単語 : str、単語番号 : str_id
    void insert(const string &word, int word_id) {
        int node_id = 0;
        for(int i = 0; i < (int)word.size(); i++) {
            int c = (int)(word[i] - base);
            if(nodes[node_id].next[c] == -1) { // 次の頂点がなければ頂点を追加
                nodes[node_id].next[c] = (int)nodes.size(); // 現在のNodeのnext[c]を更新
                nodes.push_back(Node(c));
            }
            ++nodes[node_id].common;
            node_id = nodes[node_id].next[c];
        }
        ++nodes[node_id].common;
        nodes[node_id].accept.push_back(word_id); // 単語の終端なので頂点に追加
    }

    void insert(const string &word) {
        insert(word, nodes[0].common);
    }

    // 単語とprefixの探索
    bool search_word(const string &word, bool prefix = false) {
        int node_id = 0;
        for(int i = 0; i < (int)word.size(); i++) {
            int c = (int)(word[i] - base);
            int next_id = nodes[node_id].next[c];
            if(next_id == -1) { // 次の頂点が存在しなければ終了
                return false; 
            }
            node_id = next_id;
        }
        // prefix == trueならprefixが存在するかを返す
        return (prefix) ? true : nodes[node_id].accept.size() > 0; // 最後の頂点が受理状態か確認
    }
    // prefixの探索
    bool search_prefix(const string &prefix) {
        return search_word(prefix, true);
    }

    // 最長共通接頭辞LCPの探索
    int search_LCP(const string &word) {
        int node_id = 0;
        int length = 0;
        for(int i = 0; i < (int)word.size(); i++) {
            int c = (int)(word[i] - base);
            int next_id = nodes[node_id].next[c];
            if(next_id == -1 || nodes[next_id].common <= 1) { // 次の頂点が存在しないか、共有先がなくなったら終了
                break; 
            }
            node_id = next_id;
            length++;
        }
        return length;
    }
    
    // 挿入した単語の数=根の頂点を共有している単語の数
    int count() const {return (nodes[0].common);}

    // Trie木のノード数
    int size() const {return (int)nodes.size();}
};

// ============================================================
// Rolling Hash (2^61-1)
// ============================================================
// 2^61 - 1 を法とする安全で高速なRolling Hash
struct RollingHash {
    static const uint64_t mod = (1ULL << 61) - 1;
    using uint128_t = __uint128_t;
    vector<uint64_t> hash, power; // hash : 累積hash, power : 基数の累乗
    static uint64_t base;

    // 実行時に1度だけランダムな基数を生成
    static inline uint64_t generate_base() {
        mt19937_64 mt(chrono::steady_clock::now().time_since_epoch().count());
        uniform_int_distribution<uint64_t> rand(128, mod - 1);
        return rand(mt);
    }

    // a * b mod (2^61 - 1) を計算
    static inline uint64_t mul(uint128_t a, uint128_t b) {
        uint128_t t = a * b;
        t = (t >> 61) + (t & mod);
        if (t >= mod) t -= mod;
        return (uint64_t)t;
    }

    // コンストラクタ O(N)でhash, power配列を構築
    RollingHash(const string &s) {
        if (base == 0) base = generate_base();
        int n = (int)s.size();
        hash.assign(n + 1, 0);
        power.assign(n + 1, 1);
        for (int i = 0; i < n; i++) {
            hash[i + 1] = mul(hash[i], base) + s[i];
            if (hash[i + 1] >= mod) hash[i + 1] -= mod;
            power[i + 1] = mul(power[i], base);
        }
    }

    // [l, r) のハッシュ値を取得 (0-indexed) O(1)
    uint64_t get(int l, int r) const {
        uint64_t res = hash[r] + mod - mul(hash[l], power[r - l]);
        if (res >= mod) res -= mod;
        return res;
    }

    // 長さ h2_len のhash h2 を h1 の後ろに結合
    static uint64_t connect(uint64_t h1, uint64_t h2, int h2_len, const vector<uint64_t>& p) {
        uint64_t res = mul(h1, p[h2_len]) + h2;
        if (res >= mod) res -= mod;
        return res;
    }
};
uint64_t RollingHash::base = 0;

// ============================================================
// Topological Sort (Kahn's Algorithm)
// ============================================================
/**
 * トポロジカルソートを記録した配列を返す関数
 * @param G 隣接リスト形式のグラフ
 * @param indegree 各頂点の入次数を格納した配列
 * @param V 頂点数
 * @return ソート済みの頂点配列（閉路がある場合は要素数がV未満になる）
 */
vector<int> topological_sort(vector<vector<int>> &G, vector<int> &indegree, int V) {
    vector<int> sorted_vertices;
    queue<int> que;

    // 入次数が0の頂点をキューに追加
    for (int i = 0; i < V; i++) {
        if (indegree[i] == 0) {
            que.push(i);
        }
    }

    while (!que.empty()) {
        int v = que.front();
        que.pop();

        for (int u : G[v]) {
            indegree[u]--;
            if (indegree[u] == 0) {
                que.push(u);
            }
        }
        sorted_vertices.push_back(v);
    }

    return sorted_vertices;
}

// ============================================================
// Bellman-Ford Algorithm
// ============================================================
struct Edge{
   long long from;
   long long to;
   long long cost;
};
using Edges = vector<Edge>;

/* bellman_ford(Es,s,t,dis)
    入力: 全ての辺Es, 頂点数V, 開始点 s, 最短経路を記録するdis
    出力: 負の閉路が存在するなら ture
    計算量：O(|E||V|)
    副作用：dis が書き換えられる
    辺に-1をかけることで距離を最大化できる
*/
bool bellman_ford(const Edges &Es, int V, int s, vector<long long> &dis) {
    dis.resize(V, INF);
    dis[s] = 0;
    int cnt = 0;
    for(int i = 0; i < V - 1; i++) {
        for(auto e : Es) {
            if(dis[e.from] != INF && dis[e.from] + e.cost < dis[e.to]) {
                dis[e.to] = dis[e.from] + e.cost;
            }
        }
        cnt++;
    }
    /*
        V 回目のループで「さらに更新されるか」をチェック
        もし V 回目にもコストが下がる頂点があり、かつそこがスタートから到達可能なら、
        それは「スタートから辿り着ける負の閉路」が存在する証拠。
    */
    for(auto e : Es) {
        if(dis[e.from] != INF && dis[e.from] + e.cost < dis[e.to]) {
            return true;
        }
    }
    return false;
}

// ============================================================
// Dijkstra
// ============================================================
struct Dijkstra {
    struct Edge {
        int to;
        long long cost;
    };
    using P = pair<long long, int>;

    int V;
    vector<vector<Edge>> G;
    vector<long long> dis;
    vector<int> prev;

    // 頂点数 V で初期化
    Dijkstra(int V) : V(V), G(V) {}

    // 有向辺の追加 (無向グラフの場合は両方向から追加する)
    void add_edge(int from, int to, long long cost) {
        G[from].push_back({to, cost});
    }

    // 始点 s からダイクストラ法を実行
    // 計算量: O(|E|log|V|)
    void solve_dijk(int s) {
        dis.assign(V, INF);
        prev.assign(V, -1);
        priority_queue<P, vector<P>, greater<P>> pq;
        
        dis[s] = 0;
        pq.emplace(dis[s], s);
        
        while (!pq.empty()) {
            P p = pq.top();
            pq.pop();
            int v = p.second;
            
            if (dis[v] < p.first) continue;
            
            for (auto &e : G[v]) {
                if (dis[e.to] > dis[v] + e.cost) {
                    dis[e.to] = dis[v] + e.cost;
                    prev[e.to] = v;
                    pq.emplace(dis[e.to], e.to);
                }
            }
        }
    }

    // 始点から頂点 t までの最短距離を返す（到達不可の場合は INF）
    long long get_dist(int t) const {
        return dis[t];
    }

    // 始点から頂点 t までのパスを取得
    vector<int> get_path(int t) const {
        vector<int> path;
        for (int cur = t; cur != -1; cur = prev[cur]) {
            path.push_back(cur);
        }
        reverse(path.begin(), path.end());
        return path;
    }
};


// ============================================================
// 最長増加部分列 (LIS)
// ============================================================
/// @brief 最長増加部分列（LIS）のインデックスを返します
/// @tparam Strict 狭義単調増加の場合 true, 広義単調増加の場合 false
/// @tparam Type 数列の要素の型
/// @param v 数列
/// @return 最長増加部分列（LIS）のインデックス
/// @note 1.4 最長増加部分列の復元
/// @see https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/lis
template <bool Strict, class Type>
std::vector<int> LIS(std::vector<Type>& v)
{
    std::vector<Type> dp;

    auto it = dp.begin();

    std::vector<int> positions;

    for (const auto& elem : v)
    {
        if constexpr (Strict)
        {
            it = std::lower_bound(dp.begin(), dp.end(), elem);
        }
        else
        {
            it = std::upper_bound(dp.begin(), dp.end(), elem);
        }

        positions.push_back(static_cast<int>(it - dp.begin()));

        if (it == dp.end())
        {
            dp.push_back(elem);
        }
        else
        {
            *it = elem;
        }
    }

    std::vector<int> subseq(dp.size());

    int si = static_cast<int>(subseq.size()) - 1;

    int pi = static_cast<int>(positions.size()) - 1;

    while ((0 <= si) && (0 <= pi))
    {
        if (positions[pi] == si)
        {
            subseq[si] = pi;

            --si;
        }

        --pi;
    }

    return subseq;
}

#endif
