#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <iomanip>
#include <map>
#include <queue>

using namespace std;

struct coor {
    int index;
    double x, y;
    bool operator<(const coor& other) const {
        if (x != other.x) return x < other.x;
        if (y != other.y) return y < other.y;
        return index < other.index;
    }
};

double basic_CH(int num, vector<coor>& v);
double get_dist(coor& a, coor& b);

std::string trim(const string& s) {
    auto b = s.find_first_not_of(" \t\r\n");
    auto e = s.find_last_not_of(" \t\r\n");
    return (b == string::npos) ? "" : s.substr(b, e - b + 1);
}

bool extract_key_value(const string& line, string& key, string& value) {
    std::string s = trim(line);
    if (s.empty()) return false;
    auto pos = s.find(':');
    if (pos != string::npos) {
        key = trim(s.substr(0, pos));
        value = trim(s.substr(pos + 1));
        return true;
    }
    istringstream iss(s);
    if (iss >> key) {
        std::getline(iss, value);
        value = trim(value);
        return true;
    }
    return false;
}

std::ofstream console("/dev/tty");      // use this instead of cout to see in console

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " tspfile start_index count\n";
        return 1;
    }
    string tspfile = argv[1];
    int start_index = stoi(argv[2]);
    int count = stoi(argv[3]);

    ifstream infile(tspfile);
    if (!infile) {
        cerr << "wrong file: " << tspfile << "\n";
        return 1;
    }
    string line;
    int num_inputs = -1;

    // 헤더 파싱
    while (getline(infile, line)) {
        string key, value;
        if (!extract_key_value(line, key, value)) continue;
        transform(key.begin(), key.end(), key.begin(), ::toupper);
        if (key == "DIMENSION") {
            num_inputs = stoi(value);
        }
        if (key == "NODE_COORD_SECTION") break;
    }
    if (num_inputs <= 0) {
        cerr << "DIMENSION not found or invalid\n";
        return 1;
    }
    if (start_index < 0 || start_index >= num_inputs) {
        cerr << "start_index out of range\n";
        return 1;
    }
    int actual_count = std::min(count, num_inputs - start_index);
    vector<coor> v;
    v.reserve(actual_count);

    // 좌표 읽기: start_index부터 actual_count개만 추출
    int current_idx = 0;
    while (getline(infile, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line == "EOF") break;
        istringstream iss(line);
        int idx;
        double x, y;
        if (!(iss >> idx >> x >> y)) continue;
        if (current_idx >= start_index && (int)v.size() < actual_count) {
            v.push_back({idx, x, y});
        }
        current_idx++;
        if ((int)v.size() == actual_count) break;
    }
    infile.close();

    if ((int)v.size() < actual_count) {
        cerr << "Not enough coordinates in file\n";
        return 1;
    }

    if (v.size() < 2) {
        cerr << "Not enough nodes for TSP\n";
        cout << "ALGORITHM_TIME_MS:0\nRESULT:0\n";
        return 0;
    }

    // 시간 측정 시작
    auto start = chrono::steady_clock::now();
    double result = basic_CH(actual_count, v);
    auto end = chrono::steady_clock::now();
    auto duration_ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    // 결과 출력
    cout << "ALGORITHM_TIME_MS:" << duration_ms << endl;
    cout << "RESULT:" << std::fixed << std::setprecision(10) << result << endl;
    cout << "Number of inputs (used): " << actual_count << "\n";
    cout << "Start index: " << start_index << "\n";
    cout << "Vector size: " << v.size() << "\n";
    // for (size_t i = 0; i < std::min(v.size(), size_t(5)); ++i) {
    //     cout << v[i].x << " " << v[i].y << "\n";
    // }
    return 0;
}

double get_dist(coor& a, coor& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}


vector<pair<coor, coor>> prim(int num, vector<coor>& v, map<coor, int>& m) {
    double sum = 0;
    vector<pair<coor, coor>> ans;

    // memory 최적화 : tuple<double, coor, coor> -> tuple<double, int, int>로 감소시킴.
    const double INF = numeric_limits<double>::max();
    vector<tuple<double, int, int>> pv(num, make_tuple(INF, 0, 0));

    // 전처리 -> 원하는 target만 남기기 = get<2>().index로 접근하기 위함
    for (int i = 0; i < num; i++)
        get<2>(pv[i]) = i;

    int new_pos = 0;
    m[v[new_pos]] = 1;

    while ((int)ans.size() < num-1) {
        int mindex = 0;     // index of smallest & padding space for keeping INF
        for (int i = 1; i < (int)pv.size(); i++) {
            if (get<0>(pv[i]) > get_dist(v[new_pos], v[get<2>(pv[i])])) 
                pv[i] = {get_dist(v[new_pos], v[get<2>(pv[i])]), new_pos, get<2>(pv[i])};
            
            if (get<0>(pv[mindex]) > get<0>(pv[i]))
                mindex = i;
        }
        ans.push_back({v[get<1>(pv[mindex])], v[get<2>(pv[mindex])]});
        // console << ans.size() << " : " << get<1>(pv[mindex]).x << " " << get<1>(pv[mindex]).y << " <=> " << get<2>(pv[mindex]).x << " " << get<2>(pv[mindex]).y << "\n" << flush;
        sum += get<0>(pv[mindex]);
        m[v[get<1>(pv[mindex])]]++;
        m[v[get<2>(pv[mindex])]]++;
        new_pos = get<2>(pv[mindex]);
        pv[mindex] = pv[pv.size()-1];
        pv.pop_back();

    }
    m[v[0]]--;  
    console << sum << "\n" << flush;    

    return ans;
}


// vector<pair<coor, coor>> anotherprim(int num, vector<coor>& v, map<coor, int>& m);


double basic_CH(int num, vector<coor>& v) {
    map<coor, int> m;        // to check if number of nodes linked is odd & save memory -> exchange, time consuming
    vector<pair<coor, coor>> mst = prim(num, v, m);
    // vector<pair<coor, coor>> mst = memprim(num, v, m);
    // anotherprim(num, v, m);
    // console << mst.size() << " = " << num-1 << "\n";
    // for (int i = 0; i < num-1; i++) {
    //     console << i << " : " << mst[i].first.x << " " << mst[i].first.y << " <=> " << mst[i].second.x << " " << mst[i].second.y << "\n" << flush;
    // }
    return 0;
}




/*
// for debugging -> modified by AI + use pq -> useful in sparse graph = not good for tsp
vector<pair<coor, coor>> anotherprim(int num, vector<coor>& v, map<coor, int>& m) {
    vector<pair<coor, coor>> mst_edges;
    vector<bool> in_mst(num, false);
    vector<double> min_dist(num, numeric_limits<double>::max());
    vector<int> parent(num, -1);

    // Min-heap: (distance, node_idx, from_idx)
    using T = tuple<double, int, int>;
    priority_queue<T, vector<T>, greater<T>> pq;

    min_dist[0] = 0.0;
    pq.emplace(0.0, 0, -1);

    double sum = 0.0;

    while (!pq.empty()) {
        auto [cost, u, from] = pq.top();
        pq.pop();
        if (in_mst[u]) continue;
        in_mst[u] = true;
        sum += cost;
        if (from != -1) {
            mst_edges.emplace_back(v[from], v[u]);
            m[v[from]]++;
            m[v[u]]++;
            // console << mst_edges.size() << " : " << v[from].x << " " << v[from].y << " <=> " << v[u].x << " " << v[u].y << "\n" << flush;
        }
        for (int v_idx = 0; v_idx < num; ++v_idx) {
            if (!in_mst[v_idx]) {
                double d = get_dist(v[u], v[v_idx]);
                if (d < min_dist[v_idx]) {
                    min_dist[v_idx] = d;
                    parent[v_idx] = u;
                    pq.emplace(d, v_idx, u);
                }
            }
        }
    }

    // sum에 MST 총 길이가 저장됨
    console << "MST length: " << sum << endl;
    return mst_edges;
}

*/



/* 
prim record

vector<pair<coor, coor>> prim(int num, vector<coor>& v, map<coor, int>& m) {
    double sum = 0;
    vector<pair<coor, coor>> ans;

    // vector 이용한 최적화
    const double INF = numeric_limits<double>::max();
    vector<tuple<double, coor, coor>> pv(num, make_tuple(INF, coor{0, 0, 0}, coor{0, 0, 0}));

    // 전처리 -> 원하는 target만 남기기 = get<2>().index로 접근하기 위함
    for (int i = 0; i < num; i++)
        get<2>(pv[i]) = v[i];

    coor new_pos = v[0];
    m[new_pos] = 1;

    while ((int)ans.size() < num-1) {
        int mindex = 0;     // index of smallest & padding space for keeping INF
        for (int i = 1; i < (int)pv.size(); i++) {
            if (get<0>(pv[i]) > get_dist(new_pos, get<2>(pv[i]))) 
                pv[i] = {get_dist(new_pos, get<2>(pv[i])), new_pos, get<2>(pv[i])};
            
            if (get<0>(pv[mindex]) > get<0>(pv[i]))
                mindex = i;
        }
        ans.push_back({get<1>(pv[mindex]), get<2>(pv[mindex])});
        // console << ans.size() << " : " << get<1>(pv[mindex]).x << " " << get<1>(pv[mindex]).y << " <=> " << get<2>(pv[mindex]).x << " " << get<2>(pv[mindex]).y << "\n" << flush;
        sum += get<0>(pv[mindex]);
        m[get<1>(pv[mindex])]++;
        m[get<2>(pv[mindex])]++;
        new_pos = get<2>(pv[mindex]);
        pv[mindex] = pv[pv.size()-1];
        pv.pop_back();

    }
    m[v[0]]--;  
    console << sum << "\n" << flush;    

    return ans;
}



vector<pair<coor, coor>> prim(int num, vector<coor>& v, map<coor, int>& m) {
    double sum = 0;
    vector<pair<coor, coor>> ans;

    // pq 없이 try
    const double INF = numeric_limits<double>::max();
    tuple<double, coor, coor> pa[num+1];
    fill_n(pa, num+1, make_tuple(INF, coor{0, 0, 0}, coor{0, 0, 0}));
    coor new_pos = v[0];
    m[new_pos] = 1;

    while ((int)ans.size() < num-1) {
        int mindex = num;     // index of smallest
        for (int i = 0; i < num; i++) {
            if (m[v[i]] == 0) {
                if (get<0>(pa[i]) > get_dist(new_pos, v[i])) 
                    pa[i] = {get_dist(new_pos, v[i]), new_pos, v[i]};
                
                if (get<0>(pa[mindex]) > get<0>(pa[i]))
                    mindex = i;
            }
        }
        ans.push_back({get<1>(pa[mindex]), get<2>(pa[mindex])});
        // console << ans.size() << " : " << get<1>(pa[mindex]).x << " " << get<1>(pa[mindex]).y << " <=> " << get<2>(pa[mindex]).x << " " << get<2>(pa[mindex]).y << "\n" << flush;
        m[get<1>(pa[mindex])]++;
        m[get<2>(pa[mindex])]++;
        new_pos = get<2>(pa[mindex]);

    }
    m[v[0]]--;

    return ans;


    priority_queue<tuple<double, coor, coor>, vector<tuple<double, coor, coor>>, greater<tuple<double, coor, coor>>> pq;
    // tuple 사용법 : get<n>argument_name -> 여기서 n = 0,1,2
    // pq : {dist, from, to} 모양

    // get initial pos
    coor new_pos = v[0];
    m[new_pos] = 1;     // will subtrack when while finish -> due to for loop
    while ((int)ans.size() < num-1) {
        console << ans.size() << " , " << pq.size() << "\n" << flush;
        // new args for pq
        for (int i = 0; i < num; i++) {
            if (m[v[i]] == 0)    // not in map == node not linked
                pq.push({get_dist(new_pos, v[i]), new_pos, v[i]});
        }
        while (1) {
            // got possible edge
            coor from = get<1>(pq.top());
            new_pos = get<2>(pq.top());
            pq.pop();

            // record in ans, map is edge is linked to new node
            if (m[new_pos] == 0) {
                ans.push_back({from, new_pos});
                m[from]++;
                m[new_pos]++;
                break;
            }
        }

        // to avoid oom
        // // if (64 * (long long)pq.size() > 4000000000) {
        // if (pq.size() > 30000000) {
        //     priority_queue<tuple<double, coor, coor>, vector<tuple<double, coor, coor>>, greater<tuple<double, coor, coor>>> tmp_pq = {};
        //     bool indexing[num+1] = {};
        //     while (!pq.empty()) {
        //         coor moving = get<2>(pq.top());
        //         if (m[moving] == 0 && indexing[moving.index] == false) {
        //             indexing[moving.index] = true;
        //             tmp_pq.push(pq.top());
        //         }
        //         pq.pop();
        //     }
        //     pq.swap(tmp_pq);
        // }
    }
    m[v[0]]--;


    return ans;
}



*/