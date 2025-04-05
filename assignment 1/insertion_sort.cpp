#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <iomanip>

using namespace std;

int num;
vector<int> v;

// since we need to shift numbers behind the new insertion
void insertion() {
    for (size_t i = 1; i < v.size(); i++) {
        for (int j = i-1; j > 0; j--) {
            if (v[j] > v[j+1]) {
                iter_swap(v.begin() + j, v.begin() + j+1);
            }
            else
                break;
        }
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
    insertion();

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

