#pragma once

#include<bits/stdc++.h>
using namespace std;


// 静的双方向連結リスト
// 要素取得に各要素の総連結要素数nに対してO(n)
struct Static_Doubly_LinkedList {
    int n;
    vector<int> nxt; // 次の要素 (p)
    vector<int> prv; // 前の要素 (q)

    // 初期化
    Static_Doubly_LinkedList(int n) : n(n), nxt(n, -1), prv(n, -1) {}

    // x の後ろに y を連結するO(1)
    void connect(int x, int y) {
        if(nxt[x] != -1) prv[nxt[x]] = -1;
        if(prv[y] != -1) nxt[prv[y]] = -1;
        nxt[x] = y;
        prv[y] = x;
    }

    // x と y の連結を解除するO(1)
    void disconnect(int x, int y) {
        if(nxt[x] == y) nxt[x] = -1;
        if(prv[y] == x) prv[y] = -1;
    }
    
    // xの直後にzを挿入するO(1)
    void insert_after(int x, int z) {
        int y = nxt[x];
        connect(x, z);
        if(y != -1) connect(z, y);
    }

    // xの直前にzを挿入するO(1)
    void insert_before(int x, int z) {
        int y = prv[x];
        connect(z, x);
        if(y != -1) connect(y, z);
    }

    // x が含まれる連結成分を先頭から順に取得する
    vector<int> get_path(int x) {
        // 先頭まで遡る
        int head = x;
        while (prv[head] != -1) {
            head = prv[head];
        }

        // 先頭から順に末尾まで辿る
        vector<int> path;
        int cur = head;
        while (cur != -1) {
            path.push_back(cur);
            cur = nxt[cur];
        }
        return path;
    }
};
