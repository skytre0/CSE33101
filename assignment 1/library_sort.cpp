#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <iomanip>

using namespace std;

int num;
vector<int> v;
vector<int> tmp;
vector<bool> mark;
int input_num, lim;
auto time_start = chrono::steady_clock::now();


int binary(int start, int endpoint, int target) {
    int mid = (start + endpoint) / 2;
    while (start <= endpoint) {
        mid = (start + endpoint) / 2;        // 계속 start = true 유도할 것
        if (mark[mid] == false) {
            if (mid-1 > 0 && tmp[mid-1] < target)
                start = mid+1;
            else
                endpoint = mid-1;
        }
        else {
            if (tmp[mid] < target)
                start = mid+1;
            else
                endpoint = mid-1;
        }
    }
    // if (start > input_num*2-1)
    //     start = input_num*2-1;
    return start;
}

int extend(int start, int lim) {
    if (start == lim)
        start--;
    int left = start;
    int right = start;
    int found;
    while (true) {
        if (left > 0 && mark[left] == false) {
            found = left;
            break;
        }
        if (right < lim && mark[right] == false) {
            found = right;
            break;
        }
        left--;
        right++;
    }
    return found;
}


// since we need to shift numbers behind the new insertion
void lib() {
    mark[0] = true;
    input_num = 1;
    lim = min(input_num*2, (int)v.size());
    while (input_num < lim) {
        for (int j = input_num*2-1; j > 0; j-=2) {
            mark[j] = false;
            mark[j-1] = true;
            tmp[j-1] = tmp[j/2];
        }
        
        for (int j = input_num; j < lim; j++) { // search for place to insert
            int put = v[j];
            int mid = binary(1, input_num*2-1, put);
            int canput = extend(mid, input_num*2);
            // tmp[canput] = put;
            mark[canput] = true;
            if (canput < mid) {
                while (canput < mid-1) {
                    tmp[canput] = tmp[canput+1];
                    canput++;
                }
            }
            else if (canput != mid) {
                while (canput > mid) {
                    tmp[canput] = tmp[canput-1];
                    canput--;
                }
            }
            tmp[canput] = put;

            // insertion sort
            // while (canput+1 < input_num*2 && mark[canput+1] != false && tmp[canput] > tmp[canput+1]) {
            //     iter_swap(tmp.begin() + canput, tmp.begin() + canput+1);
            //     canput++;
            // }
            // while (canput-1 > 0 && mark[canput-1] != false && tmp[canput-1] > tmp[canput]) {
            //     iter_swap(tmp.begin() + canput-1, tmp.begin() + canput);
            //     canput--;
            // }

        }

        input_num = lim;
        lim = min(input_num*2, (int)v.size());

    }

    int cnt = 1;
    for (size_t i = 1; i < v.size()*2; i++) {
        if (mark[i] == true) {
            v[cnt++] = tmp[i];
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
    tmp.resize(v.size() * 2, 0);
    mark.resize(v.size() * 2, false);
    file.close();
    
    // start time calcuation from now
    time_start = chrono::steady_clock::now();

    // number of inputs = v.size() - 1 due to padding space
    lib();

    // time took by algorithm
    auto time_end = chrono::steady_clock::now();
    auto time_duration = std::chrono::duration<double, std::milli>(time_end - time_start).count();

    // 측정된 시간을 특정 형식으로 표준 출력에 출력
    cout << "ALGORITHM_TIME_MS:" << std::fixed << std::setprecision(2) <<  time_duration << "\n";

    int out = 0;
    for (size_t i = 1; i < v.size()-1; i++) {
        if (v[i] > v[i+1]) {
            out = i;
            cout << i << " : " << v[i] << " > " << v[i+1] << "\n";
            // break;
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
