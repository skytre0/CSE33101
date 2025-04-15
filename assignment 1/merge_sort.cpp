#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <iomanip>

using namespace std;

int num;
vector<int> v;

// 참고할 swap 방법
// v.erase(v.begin() + 3, v.begin() + 5);  // 3, 4 삭제
// v.insert(v.begin() + 3, vv.begin(), vv.end()); // 6, 7 삽입

void merge(int fs, int fe, int ss, int se) {
    vector<int> tmp(se-fs+1);
    int first_start = fs;
    while (fs <= fe && ss <= se) {
        if (v[fs] < v[ss])
            tmp.push_back(v[fs++]);
        else
            tmp.push_back(v[ss++]);
    }
    while (fs <= fe)
        tmp.push_back(v[fs++]);
    while (ss <= se)
        tmp.push_back(v[ss++]);
    for (int i = first_start; i < se+1; i++) {
        v[i] = tmp[i-first_start];
    }
    // v.erase(v.begin() + first_start, v.begin() + se + 1);
    // v.insert(v.begin() + first_start, tmp.begin(), tmp.end());
    return;
}

void divide(int start, int endpoint) {
    int cal = endpoint - start;
    if (cal > 1) {
        divide(start, (start + endpoint) / 2);
        divide((start + endpoint) / 2 + 1, endpoint);
        merge(start, (start + endpoint) / 2, (start + endpoint) / 2 + 1, endpoint);
    }
    else if (cal) {
        if (v[start] > v[endpoint])
            iter_swap(v.begin()+start, v.begin()+endpoint);
    }
    return;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "need text file behind\n";
        return 1;
    }

    ifstream in_file(argv[1]);
    if (!in_file) {
        cerr << "wrong input file behind\n";
        return 1;
    }

    // made padding space at v[0] for sorting.
    v.reserve(1000005);
    v.push_back(0);
    while (in_file >> num) {
        v.push_back(num);
    }
    in_file.close();

    // start time calcuation from now
    auto time_start = chrono::steady_clock::now();

    // number of inputs = v.size() - 1 due to padding space
    divide(1, v.size()-1);

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


// references : 
// https://m.blog.naver.com/dorergiverny/223052685676
