#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <cmath>
#include <iomanip>

using namespace std;

int num;
int inc;
int lim;
vector<int> v;

// 인접한 두 요소를 비교하여 정렬하는 단순한 알고리즘으로, 작은 값(또는 큰 값)이 점점 이동하여 정렬됩니다.

void comb() {
    lim = v.size()-1;
    inc = lim / 2;
    while (inc) {
        for (int j = 1; j+inc <= lim; j++) {        // jump 거리 늘려서 O(log n)의도
            if (v[j] > v[j+inc])
                iter_swap(v.begin() + j, v.begin() + j+inc);
        }
        inc = floor(3 * inc / 4);
    }
    while (lim > 0) {
        int k = 0;
        for (int i = 1; i < lim; i++) {     // swap 없을 때까지 -> k=i 이유 작은 거가 뒤에 있던 경우면 얼마나 가져올지 몰라서 거기 다시 진행 가능성 존재.
            if (v[i] > v[i+1]) {
                iter_swap(v.begin() + i, v.begin() + i+1);
                k = i;
            }
        }
        lim = k-1;
    }
    return;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "need text file behind\n";
        return 1;
    }

    ifstream file(argv[1]);
    if (!file) {
        cerr << "wrong file behind\n";
        return 1;
    }

    // made padding space at v[0] for sorting.
    v.reserve(1000005);
    v.push_back(0);
    while (file >> num) {
        v.push_back(num);
    }
    file.close();

    // start time calcuation from now
    auto time_start = chrono::steady_clock::now();

    // number of inputs = v.size() - 1 due to padding space
    comb();

    // time took by algorithm
    auto time_end = chrono::steady_clock::now();
    auto time_duration = std::chrono::duration<double, std::milli>(time_end - time_start).count();

    // 측정된 시간을 특정 형식으로 표준 출력에 출력
    cout << "ALGORITHM_TIME_MS:" << std::fixed << std::setprecision(2) <<  time_duration << "\n";

    // int out = 0;
    // for (int i = 1; i < check_size; i++) {
    //     if (v[i] > v[i+1]) {
    //         out = i;
    //         cout << v[i] << " > " << v[i+1] << "\n";
    //         break;
    //     }
    // }
    // if (!out)
    //     cout << "sorted\n";
    // else
    //     cout << "at " << out << " unsorted\n";

    // ofstream out_file(argv[2], ios::app);
    // if (!out_file) {
    //     cerr << "wrong output file behind\n";
    //     return 1;
    // }
    // out_file << argv[0] << " input = " << argv[1] << "\n";
    // out_file << "Function time: " << std::fixed << std::setprecision(2) << time_duration << " ms\n";

    return 0;
}

