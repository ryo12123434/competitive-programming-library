#include<bits/stdc++.h>
#include<atcoder/all>
//#include <boost/multiprecision/cpp_int.hpp>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

using namespace std;
using namespace atcoder;
//using boost::multiprecision::cpp_int;
using namespace __gnu_pbds;
 
#pragma region template
using ull = unsigned long long;
using ll = long long;
using vi = vector<int>;
using vll = vector<long long>;
using vvi = vector<vi>;
using vvll = vector<vll>;
using vs = vector<string>;
using pii = pair<int, int>;
using pci = pair<char, int>;
using pll = pair<ll, ll>;
using vpii= vector<pair<int, int>>;
using vpci= vector<pair<char, int>>;
using vpll = vector<pair<ll, ll>>;
using vpcl = vector<pair<char, ll>>;
using vc = vector<char>;
using vb = vector<bool>;
using vvb = vector<vb>;
using Graph = vector<vector<int>>; // グラフ型 頂点数だけvectorを確保！

#define all(x) (x).begin(), (x).end()
#define rall(x) (x).rbegin(), (x).rend()
#define Yes cout << "Yes" << el
#define No cout << "No" << el
#define YN(bool) if(bool){cout<<"Yes"<<endl;}else{cout<<"No"<<endl;}
#define rep(i, s, n) for (int i = (s); i < (n); ++i)
#define rrep(i, n, s) for (int i = (n); i >= (s); --i)
#define mp make_pair
#define pb push_back
#define el '\n'
#define decout(x) cout << fixed << setprecision(20) << x << el
// 4近傍、(一般的に)上右下左 URDL
const int dy4[4] = {-1,0,1,0};
const int dx4[4] = {0,1,0,-1};
// 4近傍  座標平面ver RLUD
//const int dx4[4] = {1,-1,0,0};
//const int dy4[4] = {0,0,1,-1};
// 8方向 左上, 上, 右上, 右, 右下, 下, 左下, 左
const int dy8[8] = {-1,-1,-1,0,1,1,1,0};
const int dx8[8] = {-1,0,1,1,1,0,-1,-1};
int ctoi(const char c) { return ('0' <= c && c <= '9') ? (c - '0') : -1; }
//aとbで大きいほうをaに代入
template<class T> inline bool chmax(T& a, T b) { if (a < b) { a = b; return true; } return false; }
//aとbで小さいほうをaに代入
template<class T> inline bool chmin(T& a, T b) { if (a > b) { a = b; return true; } return false; }
ll digit_sum(string s) {ll ans=0; for(int i=0; i<s.size();i++){ans+=s[i]-'0';}return ans;}
//print_vec用
template<typename T> struct is_vector : false_type {};
template<typename T> struct is_vector<vector<T>> : true_type {};
template<typename T> inline constexpr bool is_vector_v = is_vector<T>::value;
template<typename T> void print_vec(const vector<T>& v);
const long long INF = 1LL<<60;
// 順序統計木
template<typename T>
using ordered_set = tree<T, null_type, less<T>, rb_tree_tag, tree_order_statistics_node_update>;
using ordered_multiset = tree<pii, null_type, less<pii>, rb_tree_tag, tree_order_statistics_node_update>;
#pragma endregion



int main(void) {ios::sync_with_stdio(false); std::cin.tie(nullptr);
    
    
    
    
    return 0;
}

/*
cd atcoder/Argo
g++ main.cpp -o main.exe
main.exe < in.txt > out.txt
*/
template<typename T>
void print_vec(const vector<T>& v) {if (v.empty()) {cout << endl;return;}if constexpr (is_same_v<T, string>) 
{for (const auto& s : v) {cout << s << endl;}}
else if constexpr (is_vector_v<T>) {for (const auto& row : v) {print_vec(row);}}
else{for (size_t i = 0; i < v.size(); ++i) {cout << v[i] << (i == v.size() - 1 ? "" : " ");}cout << el;}}
