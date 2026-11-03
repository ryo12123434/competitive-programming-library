#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
#include <iostream>
#include <vector>
#include "../../library/data_structure/wavelet_matrix.hpp"
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int n,q;std::cin>>n>>q;std::vector<long long> values(n);for(auto& x:values)std::cin>>x;
    WaveletMatrix<> matrix(values);
    while(q--){int l,r,k;std::cin>>l>>r>>k;std::cout<<*matrix.kth_smallest(l,r,k)<<'\n';}
}
