#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <climits>
#include <iomanip>

using namespace std;

int num;
int input_size, tree_size = 1;
vector<int> v;

void incline(int i) {
    while (i != 1) {
        i /= 2;
        if (v[v[i]] > v[v[i*2+1]]) {
            v[i] = v[i*2+1];
        }
        if (v[v[i]] > v[v[i*2]]) {
            v[i] = v[i*2];
        }
    }
}

void tournament() {
    vector<int> tmp;        // 마지막에 swap용
    tmp.push_back(0);
    for (int i = tree_size; i >= tree_size/2; i--) {    // tree 바닥에 index 다 설치 완료.
        if (i*2+1 < v.size() && v[i] > v[i*2+1])
            v[i] = i*2+1;
        if (i*2 < v.size() && v[i] > v[i*2])
            v[i] = i*2;
    }
    // 이제 1 ~ tree_size 안에서만 순회 예정. v 안 값은 원래 있어야 할 value의 index = v[v[]]로 2번 해야 함.
    for (int i = (tree_size/2)-1; i > 0; i--) {
        if (v[v[i]] > v[v[i*2+1]])
            v[i] = v[i*2+1];
        if (v[v[i]] > v[v[i*2]])
            v[i] = v[i*2];
    }
    // 이제 추출 시작
    for (int i = 0; i < input_size; i++) {
        tmp.push_back(v[v[1]]);
        v[v[1]] = INT_MAX;
        incline(v[1]/2);
    }
    v.swap(tmp);
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
    v.push_back(INT_MAX);
    while (file >> num) {
        v.push_back(num);
    }
    file.close();
    input_size = v.size()-1;
    while (tree_size < v.size()-1)
        tree_size *= 2;
    v.insert(v.begin() + 1, tree_size, 0);
    // for (int i = 1; i < tree_size; i++)     // aims to invalid place = 0 = INT_MAX
    //     v.insert(v.begin() + 1, 0);      // insert 반복으로 time 측정 바깥에서 에러 발생한 것.
    
    // start time calcuation from now
    auto time_start = chrono::steady_clock::now();

    // number of inputs = v.size() - 1 due to padding space
    tournament();

    // time took by algorithm
    auto time_end = chrono::steady_clock::now();
    auto time_duration = std::chrono::duration<double, std::milli>(time_end - time_start).count();

    // 측정된 시간을 특정 형식으로 표준 출력에 출력
    cout << "ALGORITHM_TIME_MS:" << std::fixed << std::setprecision(2) <<  time_duration << "\n";

    // int out = 0;
    // for (int i = 1; i < v.size()-1; i++) {
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



// references : A. Stepanov and A. Kershenbaum. Using tournament trees to sort. Polytechnical Institute of New York, CATT Technical Report, pages 86–13, 1986.
// https://stepanovpapers.com/TournamentTrees.pdf