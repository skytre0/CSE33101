#include <iostream>
#include <fstream>
#include <random>
#include <limits>  // INT_MIN, INT_MAX
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;

vector<int> v = {};

int main() {
    int n = 100000;  // 생성할 숫자의 개수
    ofstream file("test.txt");  // ascending- , descending- , random- , part-random- 
    if (!file) {
        cerr << "Error: Cannot open file\n";
        return 1;
    }

    // random_device rd;  // 난수 생성기
    // mt19937 gen(rd()); // Mersenne Twister 엔진 사용
    // uniform_int_distribution<int> dist(numeric_limits<int>::min(), numeric_limits<int>::max()); // 범위 설정

    unsigned seed = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count(); // 밀리초 기반 시드
    mt19937 gen(seed);// Mersenne Twister 엔진 사용
    uniform_int_distribution<int> dist(numeric_limits<int>::min(), numeric_limits<int>::max()); // 범위 설정

    // padding vector in case of needing sorted text file.
    for (int i = 0; i < n; i++) 
        v.push_back(dist(gen));
    // sort(v.begin(), v.end());
    // reverse(v.begin(), v.end());
    // for (int i = 47; i < n; i += 73) {
    //     sort(v.begin() + (i-47), v.begin() + i);
    // }
    
   
    for (int i = 0; i < n; i++) {
        file << v[i] << " ";
    } 

    file.close();
    cout << n << " numbers saved\n";

    return 0;
}

// reference : 
// https://m.blog.naver.com/dorergiverny/223067218069
