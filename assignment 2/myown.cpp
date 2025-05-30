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

double myown(int num, vector<coor>& v);
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
    double result = myown(actual_count, v);
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
    return (floor(sqrt(dx * dx + dy * dy) * 1e15)) * 1e-15;
    // return sqrt(dx * dx + dy * dy);
}

int ccw(const coor& a, const coor& b, const coor& c) {
    double cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cross > 0) return 1;    // CCW
    if (cross < 0) return -1;   // CW
    return 0;                   // straight
}


vector<coor> convex_hull(vector<coor>& v) {
    int n = v.size();
    if (n < 2) {
        v.emplace_back(v[0]);
        return v;
    } 
    
    vector<coor> hull;
    sort(v.begin(), v.end());
    
    // Lower hull
    vector<coor> lower;
    for (int i = 0; i < n; i++) {
        while (lower.size() >= 2) {
            coor a = lower[lower.size()-2];
            coor b = lower[lower.size()-1];
            if (ccw(a, b, v[i]) <= 0) lower.pop_back();
            else break;
        }
        lower.emplace_back(v[i]);
    }
    
    // Lower hull v 제거 (시작, 끝 제외)
    vector<coor> tmp_v;
    int j = 1;
    for (int i = 0; i < (int)v.size(); i++) {
        if (j < (int)lower.size()-1 && v[i] == lower[j]) j++;
        else tmp_v.emplace_back(v[i]);
    }
    v.swap(tmp_v);
    tmp_v.clear();
    // console << "each size -> n : " << n << ", v : " << v.size() << ", hull : " << lower.size() << endl;
    
    // Upper hull
    vector<coor> upper;
    for (int i = v.size()-1; i > -1; i--) {
        while (upper.size() >= 2) {
            coor a = upper[upper.size()-2];
            coor b = upper[upper.size()-1];
            if (ccw(a, b, v[i]) <= 0) upper.pop_back();
            else break;
        }
        upper.emplace_back(v[i]);
    }
    
    // Upper hull에서 시작, 끝 포함 제거
    j = 0;
    for (int i = v.size()-1; i > -1; i--) {
        if (j < (int)upper.size() && v[i] == upper[j]) j++;
        else tmp_v.emplace_back(v[i]);
    }
    v.swap(tmp_v);
    tmp_v.clear();
    
    // hull merge
    lower.insert(lower.end(), upper.begin() + 1, upper.end());
    if (lower[lower.size()-1] == lower[0]) lower.pop_back();
    // console << "each size -> n : " << n << ", v : " << v.size() << ", hull : " << lower.size() << endl;
    return lower;
}

vector<vector<coor>> divide_layers(vector<coor>& v) {
    vector<vector<coor>> layers;
    while (!v.empty()) {
        vector<coor> hull = convex_hull(v);
        if (hull.size() < 3) {
            if (!hull.empty()) layers.emplace_back(hull);
            break;
        }
        layers.emplace_back(hull);
    }
    // int debugsum = 0;
    // for (int i = 0; i < layers.size(); i++) {
    //     debugsum += layers[i].size();
    // }
    // console << "alright : " << debugsum << endl;
    return layers;
}

// upper[upper.size()-1] == upper.back()으로 -> 더 편함
void merge_layer(vector<coor>& merged, vector<coor>& new_layer) {
    const int N = 10;
    vector<vector<pair<double, coor>>> layer_nodes(new_layer.size());

    // 1. get top N using insertion sort
    for (size_t i = 0; i < new_layer.size(); i++) {
        vector<pair<double, coor>> top(N+1, {INF, coor{}});
        coor p = new_layer[i];

        int num = 0;
        for (size_t j = 0; j < merged.size()-1; j++) {
            double dist = get_dist(p, merged[j]);
            
            // insertion sort with top N
            for (int k = min(num, N-1); k > -1; k--) {
                if (dist < top[k].first) {
                    top[k+1] = top[k];
                    top[k] = {dist, merged[j]};
                    num++;
                } 
                else break;
            }
        }
        
        // remove INFs
        while (top.back().first == INF) top.pop_back();
        layer_nodes[i] = top;
    }

    // 2. find greedy
    while (!new_layer.empty()) {
        int bdx = 0;

        // closest to path
        for (size_t i = 1; i < new_layer.size(); i++) {
            if (layer_nodes[i][0].first < layer_nodes[i][bdx].first) 
                bdx = i;
        }

        // to remove easily later
        swap(new_layer[bdx], new_layer.back());
        swap(layer_nodes[bdx], layer_nodes.back());

        coor p = new_layer.back();
        vector<pair<double, coor>>& cands = layer_nodes.back();

        // 3. find best pos
        double min_cost = INF;
        int ba = 0;
        int bb = 0;
        bool adj = 0;
        bool left = 0;

        // check greedy top N 
        for (size_t i = 0; i < cands.size(); i++) {
            for (size_t j = i+1; j < cands.size(); j++) {
                coor a = cands[i].second;
                coor b = cands[j].second;

                int ap = find(merged.begin(), merged.end(), a) - merged.begin();
                int bp = find(merged.begin(), merged.end(), b) - merged.begin();

                // always ap < bp
                if (ap > bp) swap(ap, bp);

                // since tsp = a - b - c - a
                if (ap == 0 && bp == (int)merged.size()-2) {
                    ap = merged.size()-1;
                    swap(ap, bp);
                }

                // Check adj
                bool is_adj = (bp - ap == 1);
                bool is_left = 1;      // new node left or right to close node in tsp

                // for cost
                double original = INF;
                double new_cost = INF;
                double cost_diff = INF;

                if (is_adj) {
                    original = get_dist(merged[ap], merged[bp]);
                    new_cost = get_dist(merged[ap], p) + get_dist(p, merged[bp]);
                    cost_diff = new_cost - original;
                }
                else {
                    double ori_left;
                    double ori_right;
                    double new_left;
                    double new_right;

                    if (ap == 0) {
                        ori_left = get_dist(merged[ap], merged[merged.size()-2]) + get_dist(merged[bp], merged[bp-1]);
                        new_left = get_dist(merged[ap], p) + get_dist(p, merged[bp]) + get_dist(merged[merged.size()-2], merged[bp-1]);
                    } 
                    else {
                        ori_left = get_dist(merged[ap], merged[ap-1]) + get_dist(merged[bp], merged[bp-1]);
                        new_left = get_dist(merged[ap], p) + get_dist(p, merged[bp]) + get_dist(merged[ap-1], merged[bp-1]);
                    }

                    if (bp == (int)merged.size()-1) {
                        ori_right = get_dist(merged[ap], merged[ap+1]) + get_dist(merged[bp], merged[1]);
                        new_right = get_dist(merged[ap], p) + get_dist(p, merged[bp]) + get_dist(merged[ap+1], merged[1]);
                    }
                    else {
                        ori_right = get_dist(merged[ap], merged[ap+1]) + get_dist(merged[bp], merged[bp+1]);
                        new_right = get_dist(merged[ap], p) + get_dist(p, merged[bp]) + get_dist(merged[ap+1], merged[bp+1]);
                    }

                    if (new_left - ori_left < new_right - ori_right) {
                        cost_diff = new_left - ori_left;
                        is_left = 1;
                    }
                    else {
                        cost_diff = new_right - ori_right;
                        is_left = 0;
                    }
                }


                if (cost_diff < min_cost) {
                    min_cost = cost_diff;
                    ba = ap;
                    bb = bp;
                    adj = is_adj;
                    left = is_left;
                }
            }
        }

        // 4. merge
        if (adj) {
            // adjacent
            merged.insert(merged.begin() + bb, p);
        }
        else {
            // not adjacent
            vector<coor> new_path;
            new_path.reserve(merged.size() + 1);
            if (left) {
                if (ba == 0) {
                    new_path.insert(new_path.end(), merged.begin(), merged.begin() + bb);
                    for (int i = (int)merged.size()-2; i > bb-1; i--) {
                        new_path.emplace_back(merged[i]);
                    }
                    new_path.emplace_back(p);
                    new_path.emplace_back(merged[0]);
                }
                else {
                    new_path.insert(new_path.end(), merged.begin(), merged.begin() + ba);
                    for (int i = bb-1; i > ba-1; i--) {
                        new_path.emplace_back(merged[i]);
                    }
                    new_path.emplace_back(p);
                    new_path.insert(new_path.end(), merged.begin() + bb, merged.end());
                }

            }
            else {
                if (bb == (int)merged.size()-1) {
                    new_path.emplace_back(merged[0]);
                    new_path.emplace_back(p);
                    for (int i = ba; i > 0; i--) {
                        new_path.emplace_back(merged[i]);
                    }
                    new_path.insert(new_path.end(), merged.begin() + ba+1, merged.end());
                }
                else {
                    new_path.insert(new_path.end(), merged.begin(), merged.begin() + ba+1);
                    new_path.emplace_back(p);
                    for (int i = bb; i > ba; i--) {
                        new_path.emplace_back(merged[i]);
                    }
                    new_path.insert(new_path.end(), merged.begin() + bb+1, merged.end());
                }
            }
            
            merged.swap(new_path);
        }

        // 5. remove p related from vector
        new_layer.pop_back();
        layer_nodes.pop_back();

        // 6. add dist with p to layer_nodes
        for (int i = 0; i < (int)layer_nodes.size(); i++) {
            double dist = get_dist(new_layer[i], p);
            layer_nodes[i].emplace_back(INF, coor{});
            for (int j = (int)layer_nodes[i].size()-2; j > -1; j--) {
                if (dist < layer_nodes[i][j].first) {
                    layer_nodes[i][j+1] = layer_nodes[i][j];
                    layer_nodes[i][j] = {dist, p};
                } 
                else break;
            }
            layer_nodes[i].pop_back();
        }

    }
}

double calculate_cycle_length(vector<coor>& path) {
    double total = 0;
    for (int i = 0; i < (int)path.size()-1; i++) {
        total += get_dist(path[i], path[i+1]);
    }
    return total;
}

double myown(int num, vector<coor>& v) {
    vector<vector<coor>> layers = divide_layers(v);
    if (layers.empty()) return 0;
    
    vector<coor> tsp_path = layers[layers.size()-1];
    layers.pop_back();
    for (int i = layers.size()-1; i > -1; i--) {
        merge_layer(tsp_path, layers[i]);
    }
    
    return calculate_cycle_length(tsp_path);
}



