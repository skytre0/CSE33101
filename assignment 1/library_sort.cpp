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

int binary(int start, int endpoint, int target) {
    int mid = (start + endpoint) / 2;
    while (start <= endpoint) {
        mid = (start + endpoint) / 2;        // 계속 start = true 유도할 것
        if (mark[mid] == false) {
            if (mid-1 > 0 && tmp[mid-1] < target)
                start = mid+1;
            else if (mid-1 > 0 && tmp[mid-1] > target)
                endpoint = mid-3;
            else if (mid+1 < lim && tmp[mid+1] < target)
                start = mid+3;
            else
                endpoint = mid-1;
        }
        else {
            if (tmp[mid] < target) {
                if (mid+1 < lim && mark[mid+1] != false)
                    start = mid+1;
                else
                    start = mid+2;
            }
            else {
                if (mid-1 > 0 && mark[mid-1] != false)
                    endpoint = mid-1;
                else
                    endpoint = mid-2;
            }
        }
    }
    return mid;
}

int extend(int start, int lim) {
    int left = start;
    int right = start;
    int found;
    while (true) {
        if (left > 0 && mark[left] == false) {
            found = left;
            break;
        }
        else if (right < lim && mark[right] == false) {
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
    tmp.push_back(0);
    mark.push_back(true);
    input_num = 1;
    lim = min(input_num*2, (int)v.size());
    for ( ; input_num < lim; ) {
        //initialize tmp & mark
        vector<int> new_tmp = {};       // changed from insert due to time issue
        vector<bool> new_mark = {};
        new_tmp.resize(tmp.size() * 2, 0);
        new_mark.resize(mark.size() * 2, false);

        for (int j = 0; j < input_num; j++) {
            new_tmp[j*2] = tmp[j];
            new_mark[j*2] = mark[j];
        }
        tmp.swap(new_tmp);
        mark.swap(new_mark);

        // for (int j = 0; j < input_num*2; j += 2) {
        //     if (mark[j+1] == true) {
        //         tmp.insert(tmp.begin() + j+1, 0);
        //         mark.insert(mark.begin() + j+1, false);     // 공간 없으면 제공함
        //     }
        // }
        
        for (int j = input_num; j < lim; j++) { // search for place to insert
            int put = v[j];
            int mid = binary(1, input_num*2-1, put);
            int canput = extend(mid, input_num*2);
            tmp[canput] = put;
            mark[canput] = true;

            // insertion sort
            while (canput+1 <= input_num*2-1 && mark[canput+1] != false && tmp[canput] > tmp[canput+1]) {
                iter_swap(tmp.begin() + canput, tmp.begin() + canput+1);
                canput++;
            }
            while (canput-1 > 0 && mark[canput-1] != false && tmp[canput-1] > tmp[canput]) {
                iter_swap(tmp.begin() + canput-1, tmp.begin() + canput);
                canput--;
            }
        }
        
        // upgrade
        input_num = lim;
        lim = min(input_num*2, (int)v.size());

    }


    // remove the blank = initialiize v -> just change all v[i] -> can do -> put if true
    int cnt = 1;
    for (size_t i = 1; i < tmp.size(); i++) {
        if (mark[i] == true) {
            v[cnt++] = tmp[i];
        }
    } 
    // for (size_t i = 1; i < tmp.size(); i++) {
    //     if (mark[i] == false) {
    //         tmp.erase(tmp.begin() + i);
    //         mark.erase(mark.begin() + i);
    //     }
    // } 
    // v.swap(tmp);
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
    lib();

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




// references : M. A. Bender, M. Farach-Colton, and M. A. Mosteiro. Insertion sort is o (n log n). Theory of Computing systems, 39:391–397, 2006.
// https://www.researchgate.net/publication/266703295_Insertion_Sort_is_On_log_n