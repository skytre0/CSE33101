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

using namespace std;

struct coor {
    int index;
    double x, y;
};

double basic_HK(int num, vector<coor>& v);
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
    double result = basic_HK(actual_count, v);
    auto end = chrono::steady_clock::now();
    auto duration_ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    // 결과 출력
    cout << "ALGORITHM_TIME_MS:" << duration_ms << endl;
    cout << "RESULT:" << std::fixed << std::setprecision(10) << result << endl;
    cout << "Number of inputs (used): " << actual_count << "\n";
    cout << "Start index: " << start_index << "\n";
    cout << "Vector size: " << v.size() << "\n";
    for (size_t i = 0; i < std::min(v.size(), size_t(5)); ++i) {
        cout << v[i].x << " " << v[i].y << "\n";
    }
    return 0;
}

double get_dist(coor& a, coor& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

double basic_HK(int num, vector<coor>& v) {
    const double INF = numeric_limits<double>::max();
    vector<vector<double>> dp(num);

    for (int i = 0; i < num; i++)
        dp[i].assign((1 << num) - (1 << i), INF);       // reduce memory by deleting space lower than (1 << i)

    dp[0][0] = 0.0;
    for (int i = 1; i < num; i++)
        dp[i][0] = get_dist(v[0], v[i]);

    for (long long bm = 1; bm < (1 << num); bm++) {
        for (int cur = 1; cur < num; cur++) {
            if (!(bm & (1 << cur))) continue;       // is not in cur
            if (dp[cur][bm - (1 << cur)] == INF) continue;
            for (int nxt = 1; nxt < num; nxt++) {
                if (bm & (1 << nxt)) continue;      // 이미 방문
                int nbm = (bm | (1 << nxt)) - (1 << nxt);
                dp[nxt][nbm] = min(dp[nxt][nbm], dp[cur][bm - (1 << cur)] + get_dist(v[cur], v[nxt]));
            }
        }
    }

    double ans = INF;
    for (int i = 1; i < num; i++) {
        long long idx = (1 << num) - 2 - (1 << i);
        ans = min(ans, dp[i][idx] + get_dist(v[i], v[0]));
    }
    return ans;
}








/*
pure basic -> without memory optimization : 

double basic_HK(int num, vector<coor>& v) {
    const double INF = numeric_limits<double>::max();
    vector<vector<double>> dp(num, vector<double>(1 << num, INF));
    dp[0][0] = 0.0;
    for (int i = 1; i < num; i++)
        dp[i][1 << i] = get_dist(v[0], v[i]);

    for (long long bm = 1; bm < (1 << num); bm++) {
        for (int cur = 1; cur < num; cur++) {
            if (!(bm & (1 << cur))) continue;
            if (dp[cur][bm] == INF) continue;
            for (int nxt = 1; nxt < num; nxt++) {
                if (bm & (1 << nxt)) continue; // 이미 방문
                dp[nxt][bm | (1 << nxt)] = min(dp[nxt][bm | (1 << nxt)], dp[cur][bm] + get_dist(v[cur], v[nxt]));
            }
        }
    }
    double ans = INF;
    for (int i = 1; i < num; i++)
        ans = min(ans, dp[i][(1 << num) - 2] + get_dist(v[i], v[0]));
    return ans;
}
*/