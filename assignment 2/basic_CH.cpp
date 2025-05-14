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
    bool operator==(const coor& other) const {
        return index == other.index;
    }
    bool operator!=(const coor& other) const {
        return !(*this == other);
}
};
const double INF = numeric_limits<double>::max();

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

struct link {       // coor를 key로 하는 map 사용 고민
    int pinos;
    double dual;
    int depth; 
    coor cur;
    int pdx;        // use for search -> since not priority, can't use .top method
    vector<pair<double, coor>> pl;      // since needed multiple time = pq -> pl in initialize step -> same -> index big+min / minimum -> 7
    // weight + to_where 구조
    vector<link*> nl;       // reserve 7 size for number of children -> same as pl
    link* matched;      // if not, nullptr. 
    link* before;
    link* before_ori;       // for blossom
    link* next_ori;         // for blossom
    int is_blossom;         // 기본 0, 한 좌표가 계속 blossom에 포함 가능하기에 bool이 아닌 int로
    bool is_tree;
    bool operator<(const link& other) const {
        if (cur != other.cur) return cur < other.cur;
        if (depth != other.depth) return depth < other.depth;
        return false;
    }
};
// 사용 link a; a->pdx;  

void initialize(vector<coor>& odds, map<coor, link>& m) {
    // to not use new & delete -> called m with reference + placing values
    link* mpy;       // will use in both for loop
    for (int i = 0; i < (int)odds.size(); i++) {
        mpy = &m[odds[i]];       // set mpy
        mpy->pinos = i;
        mpy->depth = 0;        
        mpy->cur = odds[i];
        mpy->pdx = 0;
        mpy->pl.reserve(7);
        mpy->nl.reserve(7);
        mpy->is_blossom = 0;
        // can also be used to mark ancestor(beginning -> if use as m[odds[is_tree]])
        mpy->is_tree = -1;       // will later be having index i of odds if included in a tree -> for pq equality
    }
    // select 7 highest priority only
    for (int i = 0; i < (int)odds.size(); i++) {
        mpy = &m[odds[i]];       // set mpy
        priority_queue<pair<double, coor>, vector<pair<double, coor>>, greater<pair<double, coor>>> tmpq = {};
        for (int j = 0; j < (int)odds.size(); j++) {
            if (i == j) continue;
            tmpq.emplace(get_dist(odds[i], odds[j]), odds[j]);
        }
        // see same coor x/y exists or not
        if (tmpq.top().first == 0) {
            pair<double, coor> fp = tmpq.top();
            while (!tmpq.empty() && tmpq.top().second.index < odds[i].index)    // might only have same x,y coor
                tmpq.pop();
            if (tmpq.empty() || tmpq.top().first != 0)
                mpy->pl.push_back(fp);
            else {
                mpy->pl.push_back(tmpq.top());
                while (!tmpq.empty() || tmpq.top().first == 0)
                    tmpq.pop();
            }
        }
        // push max 6
        for (int j = 0; j < 6; j++) {
            if (tmpq.empty()) break;
            mpy->pl.push_back(tmpq.top());
            tmpq.pop();
        }
        // set dual variable
        mpy->dual = mpy->pl[mpy->pdx].first / 2;
    }
    return;
}

void makesimple(vector<coor>& odds, map<coor, link>& m) {
    int endpos = odds.size();
    for (int i = 0; i < endpos; ) {
        link* me = &m[odds[i]];
        link* another = &m[me->pl[me->pdx].second];
        // if match available
        if (another->matched == nullptr && me->dual == another->dual) {
            // match these two
            another->matched = me;
            me->matched = another;
            // shift these to the back of the odds (since not free node anymore) + also move the endpos(no need to look)
            swap(odds[me->pinos], odds[--endpos]);
            m[odds[me->pinos]].pinos = me->pinos;
            m[odds[endpos]].pinos = endpos;

            swap(odds[another->pinos], odds[--endpos]);
            m[odds[another->pinos]].pinos = another->pinos;
            m[odds[endpos]].pinos = endpos;
        }
        else i++;       // 이거 최소한의 matching 일단 형성
    }
    return;
}

link* grow();
void augment();
void shrink();
void expand();
coor nca();
void clean_tree();// 이 안에 expand 예정

void blossom(vector<coor>& odds, map<coor, link>& m, int free_num) {
    // single, multiple 등하면 각각 pq 필요함 -> 다음 얻기 위해 = pq가 가리키는 dist, 해당 tree의 element가 뭔지 필요함.
    // dual variable 한번에 모두 update 예정, delta는 자주 변경 -> delta 변경 1개에 path 확장 1번
    // 이 priority queue는 dist, coor_in_tree -> pq update 위해서, dist에 결정되면 그냥 해당 coor 가서 하면 되니까
    priority_queue<pair<double, coor>, vector<pair<double, coor>>, greater<pair<double, coor>>> pqs[free_num] = {};
    double delta = 0;
    // put initial mcp
    for (int i = 0; i < free_num; i++) {
        pqs[i].emplace(m[odds[i]].pl[m[odds[i]].pdx].first, m[odds[i]].cur);
    }
    // loop until augment is made
    while (1) {
        int gdx = 0;
        // choose coor to grow
        for (int i = 1; i < free_num; i++) {
            if (pqs[gdx].top().first > pqs[i].top().first) gdx = i;
        }

        // need delta update here

        link* to_extend = &m[pqs[gdx].top().second];
        link* target = &m[to_extend->pl[to_extend->pdx].second];
        pqs[gdx].pop();
        // not matched = free node = destination
        if (target->matched == nullptr) {
            augment();      // flip all edges & reset pinos, dual, depth, pdx, link(matched, before, before_ori, next_ori), is_blossom, is_tree
            break;
        }
        // already matched node + not a tree
        else {
            // matched node is not part of the tree
            if (target->is_tree == -1) {
                target = grow();        // 직전 target과 matched인 대상(outer)로 target을 바꿈
                if (target->pl[target->pdx].second == target->matched->cur) target->pdx++;
                pqs[gdx].emplace(target->pl[to_extend->pdx].first, target->cur);        // 2 since to_extend's next priority + new node's priority

                // 이미 존재하던 node pl 한계일 수 있어서 나중에 확인 -> continue 위해
                // check if all edges selected / next edge == matched edge -> need to plus = need to check all edges selected again
                if (++(to_extend->pdx) == 7) continue;
                if (to_extend->pl[to_extend->pdx].second == to_extend->matched->cur) to_extend->pdx++;
                if (to_extend->pdx == 7) continue;
                pqs[gdx].emplace(to_extend->pl[to_extend->pdx].first, to_extend->cur);
            }
            // if the matched node already part of my tree
            else if (to_extend->is_tree == target->is_tree) {
                coor ncap = nca();
                // need to make a blossom = odd cycle
                if ((to_extend->depth + target->depth - (2 * m[ncap].depth) + 1) % 2)
                    shrink();
                // else : just ignore and continue = even cycle

                if (++(to_extend->pdx) == 7) continue;
                if (to_extend->pl[to_extend->pdx].second == to_extend->matched->cur) to_extend->pdx++;
                if (to_extend->pdx == 7) continue;
                pqs[gdx].emplace(to_extend->pl[to_extend->pdx].first, to_extend->cur);      // only 1 added due to blossom
            }
            // if the matched node is part of another tree
            else {
                augment();      // flip all edges & reset pinos, dual, depth, pdx, link(matched, before, before_ori, next_ori), is_blossom, is_tree
                break;
            }
        }
    }
    return;
}

vector<pair<coor, coor>> mwpm(int num, vector<coor>& odds) {
    map<coor, link> mm;
    vector<pair<coor, coor>> ans;
    initialize(odds, mm);
    // find able right away
    // makesimple(odds, mm);
    while (!odds.empty() || (double)odds.size() <= (double)num * 0.05)      // 이거 pq의 memory issue 발생 가능성 생기면 비율이 아니라 고정값으로 변경 예정
        blossom(odds, mm, 1);
    while (!odds.empty())
        blossom(odds, mm, (int)odds.size());


    
    return ans;
}


// vector<pair<coor, coor>> anotherprim(int num, vector<coor>& v, map<coor, int>& m);       // for debugging

double basic_CH(int num, vector<coor>& v) {
    map<coor, int> m;        // to check if number of nodes linked is odd & save memory -> exchange, time consuming
    vector<pair<coor, coor>> mst = prim(num, v, m);
    // vector<pair<coor, coor>> mst = memprim(num, v, m);
    // anotherprim(num, v, m);
    // console << mst.size() << " = " << num-1 << "\n";
    // for (int i = 0; i < num-1; i++) {
    //     console << i << " : " << mst[i].first.x << " " << mst[i].first.y << " <=> " << mst[i].second.x << " " << mst[i].second.y << "\n" << flush;
    // }
    vector<coor> odds;
    odds.reserve(num);
    for (int i = 0; i < num; i++) {
        if (m[v[i]] % 2 != 0)
            odds.push_back(v[i]);
    }
    vector<pair<coor, coor>> pm = mwpm((int)odds.size(), odds);
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