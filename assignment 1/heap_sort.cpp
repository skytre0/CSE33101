#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <iomanip>

using namespace std;

int num;
vector<int> v;

void downheap(int index, int heap_size) {
    int larger = index;
    while (true) {
        if (index * 2 <= heap_size && v[index] < v[index * 2])
            larger = index * 2;
        if (index * 2 + 1 <= heap_size && v[larger] < v[index * 2 + 1])
            larger = index * 2 + 1;
        if (larger != index) {
            iter_swap(v.begin() + index, v.begin() + larger);
            index = larger;
        }
        else
            break;
    }
}

void heap_sort() {
    int heap_size = v.size() - 1;
    for (int i = heap_size / 2; i > 0; i--) {
        downheap(i, heap_size);      // reverse order
    }
    for (int i = heap_size; i > 1; i--) {
        iter_swap(v.begin() + 1, v.begin() + i);
        downheap(1, i - 1);
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
    
    // start time calculation from now
    auto time_start = chrono::steady_clock::now();

    // number of inputs = v.size() - 1 due to padding space
    heap_sort();

    // time took by algorithm
    auto time_end = chrono::steady_clock::now();
    auto time_duration = std::chrono::duration<double, std::milli>(time_end - time_start).count();
    
    // 측정된 시간을 특정 형식으로 표준 출력에 출력
    cout << "ALGORITHM_TIME_MS:" << std::fixed << std::setprecision(2) <<  time_duration << "\n";

    // int out = 0;
    // for (size_t i = 1; i < v.size()-1; i++) {
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

