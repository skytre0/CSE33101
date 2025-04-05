#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <algorithm>
#include <stack>
#include <climits>
#include <iomanip>

using namespace std;

int num;
vector<int> v;
int start = 1;
int minrun = 32;
vector<int> save;        // 숫자 1개로 size 표시함
vector<int> tmp;
int save_pos = 1;

int binary(vector<int>& v, int start, int endpoint, int target) {
    int mid = (start + endpoint) / 2;
    while (start <= endpoint) {
        mid = (start + endpoint) / 2;
        if (v[mid] < target)
            start = mid+1;
        else
            endpoint = mid-1;
    }
    return start;
}

int rbinary(int start, int endpoint, int target) {
    int mid = (start + endpoint) / 2;
    while (start <= endpoint) {
        mid = (start + endpoint) / 2;
        if (v[mid] > target)
            start = mid+1;
        else
            endpoint = mid-1;
    }
    return start;
}

int insertion(int start, bool dir) {
    int width = min(start + minrun, (int)v.size());
    if (dir) {
        for (int i = start+1; i < width; i++) {         // 오름차순 = 순행.
            int loc = binary(v, start, i-1, v[i]);
            for (int j = i; j != loc; j--)
                iter_swap(v.begin() + j, v.begin() + j-1);
        }
    }
    else {
        for (int i = start+1; i < width; i++) {         // 내림차순 -> extend 후 reverse 필요 주의.
            int loc = rbinary(start, i, v[i]);
            for (int j = i; j != loc; j--)
                iter_swap(v.begin() + j, v.begin() + j-1);
        }
    }
    return width;
}

int extend(int start, bool dir) {
    if (dir) {  // 오름차순 = 전보다 큰 경우
        while (start < v.size() && v[start-1] <= v[start])
            start++;
    }
    else {
        while (start < v.size() && v[start-1] >= v[start])
            start++;
    }
    return start;
}

void fromleft(int start, int endpoint, int vs, int ve, int ts, int te) {
    // v가 vs ~ ve, tmp가 ts ~ te
    // vs < ts 반복 = ++, 반대 반복은 --
    // gallop 반복 iter_swap 안 되기에 *2 성공하면 while로 다 자동 입력하기.
    int gallop = 0;
    while (vs <= ve && ts <= te) {
        // gallop
        if (gallop > 1 && v[vs] < tmp[ts]) {
            gallop = 2;
            while (vs + gallop < ve && v[vs + gallop] < tmp[ts])
                gallop *= 2;
            if (vs + gallop <= ve)
                gallop = binary(v, vs + gallop/2, vs + gallop, tmp[ts]);
            else
                gallop = binary(v, vs + gallop/2, ve, tmp[ts]);
            gallop = min(ve, gallop);
            while (vs < gallop) 
                v[start++] = v[vs++];
            gallop = 0;
        }
        if (gallop < -1 && v[vs] >= tmp[ts]) {
            gallop = 2;
            while (ts + gallop < te && tmp[ts + gallop] <= v[vs])
                gallop *= 2;
            if (ts + gallop <= te)
                gallop = binary(tmp, ts + gallop/2, ts + gallop, v[vs]);
            else
                gallop = binary(tmp, ts + gallop/2, te, v[vs]);
            gallop = min(te, gallop);
            while (ts < gallop) 
                v[start++] = tmp[ts++];
            gallop = 0;
        }

        // normal
        if (v[vs] < tmp[ts]) {
            v[start++] = v[vs++];
            if (gallop < 1)
                gallop = 1;
            else
                gallop++;
        }
        else {
            v[start++] = tmp[ts++];
            if (gallop > -1)
                gallop = -1;
            else
                gallop--;
        }
    }
    
    // 남은 거 밀기.
    while (vs <= ve)
        v[start++] = v[vs++];
    while (ts <= te)
        v[start++] = tmp[ts++];
    return;
}

void fromright(int start, int endpoint, int vs, int ve, int ts, int te) {
    // 왼 <- 오 방향 주의, 일단 윗 코드 그대로 들고 옴.
    // v가 vs ~ ve, tmp가 ts ~ te
    // vs > ts 반복 = ++, 반대 반복은 --
    // gallop 반복 iter_swap 안 되기에 *2 성공하면 while로 다 자동 입력하기.
    int gallop = 0;
    while (vs >= ve && ts >= te) {
        // gallop
        if (gallop > 1 && v[vs] < tmp[ts]) {
            gallop = -2;
            while (vs + gallop > ve && v[vs + gallop] > tmp[ts])
                gallop *= 2;
            if (vs + gallop >= ve)
                gallop = binary(v, vs + gallop, vs + gallop/2, tmp[ts]);
            else
                gallop = binary(v, ve, vs + gallop/2, tmp[ts]);
            gallop = max(ve, gallop);
            while (vs > gallop) 
                v[start--] = v[vs--];
            gallop = 0;
        }
        if (gallop < -1 && v[vs] >= tmp[ts]) {
            gallop = -2;
            while (ts + gallop > te && tmp[ts + gallop] >= v[vs])
                gallop *= 2;
            if (ts + gallop >= te)
                gallop = binary(tmp, ts + gallop, ts + gallop/2, v[vs]);
            else
                gallop = binary(tmp, te, ts + gallop/2, v[vs]);
            gallop = max(te, gallop);
            while (ts > gallop) 
                v[start--] = tmp[ts--];
            gallop = 0;
        }

        // normal
        if (v[vs] > tmp[ts]) {
            v[start--] = v[vs--];
            if (gallop < 1)
                gallop = 1;
            else
                gallop++;
        }
        else {
            v[start--] = tmp[ts--];
            if (gallop > -1)
                gallop = -1;
            else
                gallop--;
        }
    }
    
    // 남은 거 밀기.
    while (vs >= ve)
        v[start--] = v[vs--];
    while (ts >= te)
        v[start--] = tmp[ts--];
    return;
}


int merge(int index, int target, int eval) {     // vector 1개 제거해야 하고, save_pos 갱신해야 하고, merge해야 함.
    int fs, fe, ss, se;         // galloping 구현 필요함, 2개 좌우 무조건 확인 X lower_bound / upper_bound 제거 필요함.
    tmp = {};
    if (target == 0) {
        fs = index - save[eval] - save[eval-1];
        fe = index - save[eval] - 1;
        ss = index - save[eval];
        se = index - 1;
        save[eval] += save[eval-1];
    }
    else {
        fs = index - save[eval] - save[eval-1] - save[eval-2];
        fe = index - save[eval] - save[eval-1] - 1;
        ss = index - save[eval] - save[eval-1];
        se = index - save[eval] - 1;
        save[eval-2] += save[eval-1];
    }
    save.erase(save.begin() + eval-1);

    // 이제 save 필요 없음 = merge_sort의 merge하면 됨.
    int to_check;
    fs = binary(v, fs, fe, v[ss]);     // binary search로 lower bound, upper bound 찾기
    to_check = binary(v, ss, se, v[fe]);     // 양쪽 binary return인 start ~ start하면 됨.
    se = min(se, to_check);

    // galloping mode 도입하기.
    // 일단 원본 v = fs ~ se -> fs ~ fe, tmp = 일부 발췌.
    // fromxxx -> start, end, v의 start, v의 end, tmp의 start, tmp의 end.
    // fromright 왼쪽에서 오른쪽으로 채워서 endpoint, fe, se < start, fs, ss 주의.
    if (se - ss > fe - fs) {
        tmp.insert(tmp.begin(), v.begin() + fs, v.begin() + fe+1);
        fromleft(fs, se, ss, se, 0, tmp.size()-1);
    }
    else {
        tmp.insert(tmp.begin(), v.begin() + ss, v.begin() + se+1);
        fromright(se, fs, fe, fs, tmp.size()-1, 0);
    }


    // 예전 코드
    // while (fs <= fe && ss <= se) {
    //     if (v[fs] < v[ss])
    //         tmp.push_back(v[fs++]);
    //     else
    //         tmp.push_back(v[ss++]);
    // }
    // while (fs <= fe)
    //     tmp.push_back(v[fs++]);
    // while (ss <= se)
    //     tmp.push_back(v[ss++]);
    // v.erase(v.begin() + first_start, v.begin() + se + 1);
    // v.insert(v.begin() + first_start, tmp.begin(), tmp.end());
    return eval-1;
}

int resize(int index, int eval) {        // 다음 insertion 시작점 -> 주는 이유 save가 크기 저장이라서 기준점 필요함, eval = save_pos 상대 위치 파악 위해서.
    if (save[eval-1] > save[eval]) {    // resize vector의 resize 아님
        if (save[eval-1] != INT_MAX && save[eval-2] != INT_MAX && save[eval-2] > save[eval-1] + save[eval])
            return eval+1;
        else if (save[eval-1] == INT_MAX)
            return eval+1;
        else if (save[eval-2] == INT_MAX && save[eval] < save[eval-1]) 
            return eval+1;     // 2 조건 모두 만족.

        else {      // 여기부터는 merge 필요
            if (save[eval-1] != INT_MAX && save[eval] > save[eval-2]) {
                eval = merge(index, -2, eval);        // save_pos-2에 merge & 내부적으로 변경 및 가장 마지막 pop_back, save_pos 수정 필요
                if (eval-1 > 0)
                    eval = resize(index-save[eval], eval-1);
                eval = resize(index, eval);       // cur, save_pos-2랑 save_pos-3을 또 해야 함
            }
            else {
                eval = merge(index, 0, eval);        // 본인과 merge
                eval = resize(index, eval);       // 범위 계산 다시 해서 넣어야 함.
            }
        }
    }
    else {      // 위 if의 else와 동일
        if (save[eval] > save[eval-2]) {
            eval = merge(index, -2, eval);        // save_pos-2에 merge & 내부적으로 변경 및 가장 마지막 pop_back, save_pos 수정 필요
            if (eval-1 > 0)
                eval = resize(index-save[eval], eval-1);
            eval = resize(index, eval);       // cur, save_pos-2랑 save_pos-3을 또 해야 함
        }
        else {
            eval = merge(index, 0, eval);        // 본인과 merge
            eval = resize(index, eval);       // 범위 계산 다시 해서 넣어야 함.
        }
    }
    return eval;
}

void tim() {
    start = 1;
    save.push_back(INT_MAX);
    while (start < v.size()) {  //binary + extend run size
        int from = start;
        bool dir = true;
        if (start+1 < v.size())
            dir = (v[start] < v[start+1]);      // dir = 1 -> 오름, dir = 0 -> 내림
        start = insertion(start, dir);
        start = extend(start, dir);     // save에 기록 필요 -> start부터 다시 시작해야 함 = 구한 거에 start 포함 X
        if (!dir)
            reverse(v.begin() + from, v.begin() + start);
        save.push_back(start - from);     // 일단 넣어
        save_pos = resize(start, save_pos);      // 여기 문제.
    }
    save_pos--;     // 이제 새 거 안 넣으니까.
    // 여기서 save에 기록된 것들 전부 merge하면서 합치면 됨.
    while (save_pos > 1) {
        save_pos = merge(v.size(), 0, save_pos);
    }

    return;
}

int getminrun(int n) {
    int r = 0; // 나머지 계산용
    while (n >= 64) { //  64 -> 주로 32 ~ 64 사이라서.
        r |= (n & 1);
        n >>= 1;
    }
    return n + r;
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
    v.push_back(0);
    while (in_file >> num) {
        v.push_back(num);
    }
    in_file.close();
    // minrun = getminrun(v.size()-1);

    // start time calcuation from now
    auto time_start = chrono::steady_clock::now();

    // number of inputs = v.size() - 1 due to padding space
    tim();

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
