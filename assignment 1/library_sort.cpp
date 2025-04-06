#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <iomanip>

using namespace std;

vector<int> v;
vector<int> tmp;
vector<bool> mark;
int input_num, lim;
auto bin_checker = std::chrono::duration<double, std::milli>(chrono::steady_clock::now() - chrono::steady_clock::now()).count();
auto ex_checker = std::chrono::duration<double, std::milli>(chrono::steady_clock::now() - chrono::steady_clock::now()).count();
auto insert_checker = std::chrono::duration<double, std::milli>(chrono::steady_clock::now() - chrono::steady_clock::now()).count();
auto time_start = chrono::steady_clock::now();


int binary(int start, int endpoint, int target) {
    int mid = (start + endpoint) / 2;
    while (start <= endpoint) {
        mid = (start + endpoint) / 2;
        if (!mark[mid]) {           // 같으면 탈출 가능 = 1회 덜 시도
            if (mid > start)
                mid--;
            else if (mid < endpoint)
                mid++;
            else
                return start;
        }
        if (tmp[mid] < target)
            start = mid+1;
        else
            endpoint = mid-1;
        
        // if (!mark[mid]) {
        //     if (mid-1 > 0 && tmp[mid-1] < target)
        //         start = mid+1;
        //     else
        //         endpoint = mid-1;
        // }
        // else {
        //     if (tmp[mid] < target)
        //         start = mid+1;
        //     else
        //         endpoint = mid-1;
        // }
    }
    return start;
}

int extend(int start, int lim) {
    if (start == lim)
        start--;
    int found;
    int dist = (start % 2 ? 0 : 1);
    while (true) {
        if (start+dist < lim && !mark[start+dist]) {
            found = start+dist;
            break;
        }
        if (start-dist > 0 && !mark[start-dist]) {
            found = start-dist;
            break;
        }
        dist+=2;
    }
    return found;
}

void insertion(int mid, int canput, int new_insert) {
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
    tmp[canput] = new_insert;
    return;
}

// since we need to shift numbers behind the new insertion
void lib() {
    mark[0] = true;
    input_num = 1;
    lim = 2;
    while (input_num < lim) {
        for (int j = (input_num << 1)-1; j > 0; j-=2) {
            mark[j] = false;
            mark[j-1] = true;
            tmp[j-1] = tmp[j/2];
        }
        
        for (int j = input_num; j < lim; j++) { // search for place to insert
            bin_checker -= std::chrono::duration<double, std::milli>(chrono::steady_clock::now() - time_start).count();
            int mid = binary(1, (input_num << 1)-1, v[j]);
            bin_checker += std::chrono::duration<double, std::milli>(chrono::steady_clock::now() - time_start).count();

            ex_checker -= std::chrono::duration<double, std::milli>(chrono::steady_clock::now() - time_start).count();
            int canput = extend(mid, (input_num << 1));
            ex_checker += std::chrono::duration<double, std::milli>(chrono::steady_clock::now() - time_start).count();

            insert_checker -= std::chrono::duration<double, std::milli>(chrono::steady_clock::now() - time_start).count();
            insertion(mid, canput, v[j]);
            insert_checker += std::chrono::duration<double, std::milli>(chrono::steady_clock::now() - time_start).count();

        }

        input_num = lim;
        lim = min((input_num << 1), (int)v.size());

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
    int num;
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
    cout << bin_checker << ", " << ex_checker << ", " << insert_checker << "\n";
    // ofstream out_file(argv[2], ios::app);
    // if (!out_file) {
    //     cerr << "wrong output file behind\n";
    //     return 1;
    // }
    // out_file << argv[0] << " input = " << argv[1] << "\n";
    // out_file << "Function time: " << std::fixed << std::setprecision(2) << time_duration << " ms\n";


    return 0;
}

