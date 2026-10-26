#define PROBLEM "https://judge.yosupo.jp/problem/static_range_frequency"
#include <iostream>
#include <vector>
#include "../../library/data_structure/wavelet_matrix.hpp"
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int n,q;std::cin>>n>>q;std::vector<long long> values(n);for(auto& x:values)std::cin>>x;
    WaveletMatrix<> matrix(values);
    while(q--){int l,r;long long x;std::cin>>l>>r>>x;std::cout<<matrix.count(l,r,x)<<'\n';}
}
