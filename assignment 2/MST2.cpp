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
#include <stack>
#include <cassert>

using namespace std;


struct coor {
    int index;
    double x, y;
    bool operator<(const coor& other) const {
        if (x != other.x) return x < other.x;
        if (y != other.y) return y < other.y;
        return index < other.index;
    }
    bool operator==(const coor& other) const {
        return index == other.index;
    }
    bool operator!=(const coor& other) const {
        return !(*this == other);
}
};
const double INF = numeric_limits<double>::max();

double MST2(int num, vector<coor>& v);
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

// endl 안 하면 cout보다 나중에 나옴
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
    double result = MST2(actual_count, v);
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
    // return (floor(sqrt(dx * dx + dy * dy) * 1e15)) * 1e-15;
    return round((floor(sqrt(dx * dx + dy * dy) * 1e13)) * 1e-13);
}


vector<pair<coor, coor>> prim(int num, vector<coor>& v) {
    double sum = 0;
    vector<pair<coor, coor>> ans;

    // memory 최적화 : tuple<double, coor, coor> -> tuple<double, int, int>로 감소시킴.
    vector<tuple<double, int, int>> pv(num, make_tuple(INF, 0, 0));

    // 전처리 -> 원하는 target만 남기기 = get<2>().index로 접근하기 위함
    for (int i = 0; i < num; i++)
        get<2>(pv[i]) = i;

    int new_pos = 0;

    while ((int)ans.size() < num-1) {
        int mindex = 0;     // index of smallest & padding space for keeping INF
        for (int i = 1; i < (int)pv.size(); i++) {
            if (get<0>(pv[i]) > get_dist(v[new_pos], v[get<2>(pv[i])])) 
                pv[i] = {get_dist(v[new_pos], v[get<2>(pv[i])]), new_pos, get<2>(pv[i])};
            
            if (get<0>(pv[mindex]) > get<0>(pv[i]) || (get<0>(pv[mindex]) == get<0>(pv[i]) && get<2>(pv[mindex]) > get<2>(pv[i]))) 
                mindex = i;
        }
        ans.push_back({v[get<1>(pv[mindex])], v[get<2>(pv[mindex])]});
        // console << ans.size() << " : " << get<1>(pv[mindex]).x << " " << get<1>(pv[mindex]).y << " <=> " << get<2>(pv[mindex]).x << " " << get<2>(pv[mindex]).y << "\n" << flush;
        sum += get<0>(pv[mindex]);
        new_pos = get<2>(pv[mindex]);
        pv[mindex] = pv[pv.size()-1];
        pv.pop_back();

    }
    console << sum << "\n" << flush;    

    return ans;
}


vector<coor> eulercircuit(vector<vector<coor>> (&ntn)) {
    vector<coor> ans = {};
    stack<coor> s;

    s.emplace(ntn[1][0]);       // no element in ntn[0] -> .tsp start from 1 

    while (!s.empty()) {
        coor cur = s.top();
        if (!ntn[cur.index].empty()) {
            
            // euler circuit greedy != tsp greedy
            coor to = ntn[cur.index].back();
            ntn[cur.index].pop_back();

            int cp = -1;   // cur pos
            for (int i = 0; i < (int)ntn[to.index].size(); i++) {
                if (ntn[to.index][i] == cur) {
                    cp = i;
                    break;
                }
            }
            if (cp > -1) {
                ntn[to.index][cp] = ntn[to.index][ntn[to.index].size()-1];
                ntn[to.index].pop_back();
            }
            s.emplace(to);
        }
        else {
            ans.emplace_back(cur);
            s.pop();
        }
    }
    // double ecsum = 0;
    // for (int i = 1; i < (int)ans.size(); i++) {
    //     ecsum += get_dist(ans[i-1], ans[i]);
    // }
    // console << "ecsum : " << ecsum << endl;
    return ans;
}




// vector<pair<coor, coor>> anotherprim(int num, vector<coor>& v, map<coor, int>& m);       // for debugging

double MST2(int num, vector<coor>& v) {
    // no need for map
    vector<pair<coor, coor>> mst = prim(num, v);
    // anotherprim(num, v, m);
    // console << mst.size() << " = " << num-1 << "\n";

    // make adjacency vector (node to node), use coor.index for mapping
    vector<vector<coor>> ntn(mst.size() * 2 + 1);
    for (int i = 0; i < (int)mst.size(); i++) {
        ntn[mst[i].second.index].emplace_back(mst[i].first);
        ntn[mst[i].second.index].emplace_back(mst[i].first);
        ntn[mst[i].first.index].emplace_back(mst[i].second);
        ntn[mst[i].first.index].emplace_back(mst[i].second);
    }

    // approx tsp tour = visit all edges twice = can do the same with euler circuit now
    vector<coor> ec = eulercircuit(ntn);
    ntn.clear();
    int visited[mst.size() * 2 + 1] = {};

    double chtotal = 0;
    coor recent = ec[0];
    visited[recent.index] = 1;
    for (int i = 1; i < (int)ec.size(); i++) {
        if (!(visited[ec[i].index])) {
            visited[ec[i].index] = 1;
            chtotal += get_dist(recent, ec[i]);
            recent = ec[i];
        }
        if (i == (int)ec.size()-1) chtotal += get_dist(recent, ec[0]);
    }
    return chtotal;
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
