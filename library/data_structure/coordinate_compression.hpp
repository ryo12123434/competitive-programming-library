#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * @brief 整数列を座標圧縮する。
 *
 * 各要素を、元の列に現れる異なる値を昇順に並べたときの 0-indexed の順位へ変換する。
 *
 * @param a 圧縮する整数列
 * @return 座標圧縮後の整数列
 * @par Complexity
 * O(N log N)
 */
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
