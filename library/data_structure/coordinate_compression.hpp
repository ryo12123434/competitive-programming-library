#pragma once

#include <bits/stdc++.h>
using namespace std;

vector<int> compress(vector<int> a){
    // コピー
    vector<int> b = a;
    
    // B を小さい順にソート
    sort(b.begin(), b.end());
    
    // B から重複を除去する
    b.erase(unique(b.begin(), b.end()), b.end());

    // 座標圧縮した結果を求める
    vector<int> res(a.size());
    for (int i = 0; i < a.size(); ++i) {
        res[i] = lower_bound(b.begin(), b.end(), a[i]) - b.begin();
    }
    return res;
};
