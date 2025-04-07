#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <queue>
#include <iomanip>

using namespace std;

int num;
vector<int> v;
queue<pair<int, int>> q;

void sweep(int start, int endpoint) {
    int pivot = start;
    int left = start + 1;
    int right = endpoint;
    int cal = endpoint - start;
    if (cal > 0) {
        while (left < right) {
            if (v[left] < v[pivot])
                left++;
            else if (v[right] > v[pivot])
                right--;
            else 
                iter_swap(v.begin() + left++, v.begin() + right--);
        }

        if (v[pivot] < v[left]) {
            iter_swap(v.begin() + pivot, v.begin() + --left);
        }
            // v.insert(v.begin() + left, v[pivot]);
        else {
            iter_swap(v.begin() + pivot, v.begin() + left);
        }
            // v.insert(v.begin() + (++left), v[pivot]);
        // v.erase(v.begin() + pivot);
        // left--;     // due to erasing the original pivot
        q.push({start, left - 1});
        q.push({left + 1, endpoint});
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
    q.push({1, v.size()-1});        // due to stack overflow, I used queue.
    while (!q.empty()) {
        sweep(q.front().first, q.front().second);
        q.pop();
    }

    // time took by algorithm
    auto time_end = chrono::steady_clock::now();
    auto time_duration = std::chrono::duration<double, std::milli>(time_end - time_start).count();
    
    // 측정된 시간을 특정 형식으로 표준 출력에 출력
    cout << "ALGORITHM_TIME_MS:" << std::fixed << std::setprecision(2) <<  time_duration << "\n";

    int out = 0;
    for (size_t i = 1; i < v.size()-1; i++) {
        if (v[i] > v[i+1]) {
            out = i;
            cout << v[i] << " > " << v[i+1] << "\n";
            break;
        }
    }
    if (!out)
        cout << "sorted\n";
    else
        cout << "at " << out << " unsorted\n";

    // ofstream out_file(argv[2], ios::app);
    // if (!out_file) {
    //     cerr << "wrong output file behind\n";
    //     return 1;
    // }
    // out_file << argv[0] << " input = " << argv[1] << "\n";
    // out_file << "Function time: " << std::fixed << std::setprecision(2) << time_duration << " ms\n";

    
    return 0;
}
