#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <cmath>
#include <iomanip>

using namespace std;

int num;
vector<int> v;


void downheap(int index, int heap_size, int diff) {
    while (true) {
        int larger = index;
        int left = (index - diff) * 2 + diff;
        int right = left + 1;
        if (left <= heap_size && v[index] < v[left])
            larger = left;
        if (right <= heap_size && v[larger] < v[right])
            larger = right;
        if (larger != index) {
            iter_swap(v.begin() + index, v.begin() + larger);
            // downheap(larger, heap_size, diff);
            index = larger;
        }
        else
            break;
    }
}

void heap_sort(int start, int endpoint) {
    for (int i = (start + endpoint - 1) / 2; i >= start; i--) {     // 여기서 st, end에 따라 tree 형태 결정되기 때문에 diff 고정됨
        downheap(i, endpoint, start-1);      // reverse order
    }
    for (int i = endpoint; i > start; i--) {
        iter_swap(v.begin() + start, v.begin() + i);
        downheap(start, i - 1, start-1);
    }
    return;
}

void insertion(int start, int endpoint) {
    for (int i = start+1; i <= endpoint; i++) {
        for (int j = i; j > start; j--) {
            if (v[j-1] > v[j]) {
                iter_swap(v.begin() + j-1, v.begin() + j);
            }
            else
                break;
        }
    }
    return;
}

void sweep(int start, int endpoint, int depth) {
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
        else {
            iter_swap(v.begin() + pivot, v.begin() + left);
        }

        // divide between insertion(16 or lower size) / heap_sort
        if (depth < floor(log2(v.size()-1))) {
            sweep(start, left - 1, depth+1);
            sweep(left + 1, endpoint, depth+1);
        }
        else {
            if (left - 1 - start < 16) 
                insertion(start, left - 1);
            else 
                heap_sort(start, left - 1);

            if (endpoint - left - 1 < 16) 
                insertion(left + 1, endpoint);
            else 
                heap_sort(left + 1, endpoint);
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
    sweep(1, v.size()-1, 0);

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


// references : D. R. Musser. Introspective sorting and selection algorithms. Software: Practice and Experience, 27(8):983–993, 1997.
// https://webpages.charlotte.edu/rbunescu/courses/ou/cs4040/introsort.pdf