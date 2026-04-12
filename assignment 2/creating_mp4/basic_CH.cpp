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

double basic_CH(int num, vector<coor>& v, const string& dataset_name);
// 기존 prim을 아래처럼 교체
vector<pair<coor, coor>> prim(int num, vector<coor>& v, map<coor, int>& m, ofstream& viz_out);
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

// --- [추가할 부분] 파일명에서 데이터셋 이름 추출 ---
    string dataset_name = tspfile;
    size_t pos = dataset_name.find_last_of("/\\");
    if (pos != string::npos) dataset_name = dataset_name.substr(pos + 1);
    pos = dataset_name.find_last_of(".");
    if (pos != string::npos) dataset_name = dataset_name.substr(0, pos);
    
    // 시간 측정 시작
    auto start = chrono::steady_clock::now();
    // basic_CH 함수에 dataset_name을 추가로 넘겨줍니다.
    double result = basic_CH(actual_count, v, dataset_name); 
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


vector<pair<coor, coor>> prim(int num, vector<coor>& v, map<coor, int>& m, ofstream& viz_out) {
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
            
            if (get<0>(pv[mindex]) > get<0>(pv[i]) || (get<0>(pv[mindex]) == get<0>(pv[i]) && get<2>(pv[mindex]) > get<2>(pv[i]))) 
                mindex = i;
        }
        ans.push_back({v[get<1>(pv[mindex])], v[get<2>(pv[mindex])]});
        // console << ans.size() << " : " << get<1>(pv[mindex]).x << " " << get<1>(pv[mindex]).y << " <=> " << get<2>(pv[mindex]).x << " " << get<2>(pv[mindex]).y << "\n" << flush;
        
        // --- [추가 부분] ---
        viz_out << "FRAME: MST Formation\n";
        for (const auto& e : ans) {
            viz_out << e.first.x << " " << e.first.y << " " << e.second.x << " " << e.second.y << " 0\n"; // 파란색(0)
        }
        viz_out << "\n";
        // -------------------
        
        sum += get<0>(pv[mindex]);
        m[v[get<1>(pv[mindex])]]++;
        m[v[get<2>(pv[mindex])]]++;
        new_pos = get<2>(pv[mindex]);
        pv[mindex] = pv[pv.size()-1];
        pv.pop_back();

    }
    m[v[0]]--;  
    // console << sum << "\n" << flush;    

    return ans;
}

// 리모델링 -> 그냥 single tree로 구현하기로 함

struct node;
struct edge;
double get_slack(const edge* e);

// 새 struct 제작 예정 -> edge -> head = +, tail = -..?
// initialize에 각 연결된 coor 설정하고, 한 coor을 범위 버리는 방식이면 최소한만 만들기 가능 + 찾기 가능한 상태
// 그냥 new 쓰고, 마지막에 모두 연결되면 for(0~n) for(i~n)으로 전부 delete하면 됨 = 약간의 위험 부담 있지만 그나마 최소 -> cleanup 추가 예정
struct edge {
    double dist;
    node* u;
    node* v;
    node* uo;
    node* vo;
    bool operator<(const edge& other) const {
        return get_slack(this) < get_slack(&other);
    }
};

struct node {       // 일반 node 기준 지표 = 주석 (blossom 제외)
    // 영구 보존
    coor cur;
    vector<edge*> el;

    // dual update에만 변경
    double dual;

    // augment에 변경
    int treeidx;
    int depth;  // for dual update : even = +, odd = -, 여기에 blossom의 depth는 tbase 따라감
    node* matched;      // if not, nullptr. 
    node* parent;       // to get augmenting path

    // expand에 변경
    vector<node*> nl;
    node* up;
    node* down;

    // double dfore;     // dual variable update가 tree에 언제 들어왔는지에 따라서 각 시점의 dsum 달라짐 -> 내가 걸리면 그때 내 dual = dual + dsum - dfore하는 거가 맞음
};

double slagma(node* place);

double get_slack(const edge* e) {
    return (e->dist - e->uo->dual - e->vo->dual - slagma(e->uo) - slagma(e->vo));
}

node* matched(edge* e, node* oneside) {
    if (e->u == oneside)    return e->v;
    else                    return e->u;
}

// slack = weight - e->uo->dual - e->uv->dual - (slagma(e->uo) + slagma(e->uv))
struct esgreater {
    bool operator()(const edge* a, const edge* b) const {
        // slack이 작은 것이 우선순위가 높게 (min-heap)
        return get_slack(a) > get_slack(b);
    }
};




// 원래 vector 가장 가까운 7개 느낌 생각했지만, 원 외부 고려하면 안 됨 = 무조건 메모리 O(m)이 나와야 함 -> 원 위 6개 2개씩 거의 0에 가까운 거리, 원 중심 1개 원 외부 1개인 경우.
void initialize(vector<coor>& odds, map<coor, node>& mn, map<pair<node*, node*>, edge>& me) {
    // to not use new & delete -> called m with reference + elacing values
    node* mu;       // will use in both for loop
    node* mv;
    for (int i = 0; i < (int)odds.size(); i++) {
        mu = &mn[odds[i]];       // set mu
        // initialize node  
        mu->cur = odds[i];
        mu->dual = 0;
        mu->treeidx = -1;       // will later be having index i of odds if included in a tree -> for pq equality
        mu->depth = 0;      
        mu->parent = nullptr;   
        mu->el.reserve(odds.size() * odds.size());
        mu->nl.reserve(odds.size()-1);
        // can also be used to mark ancestor(beginning -> if use as m[odds[treeidx]])
    }
    // sort with priority
    for (int i = 0; i < (int)odds.size(); i++) {
        mu = &mn[odds[i]];       // set mu
        double mindist = INF;
        for (int j = i+1; j < (int)odds.size(); j++) {
            mv = &mn[odds[j]];
            // initialize edge

            edge* e = &me[{mu, mv}];
            e->dist = get_dist(odds[i], odds[j]);
            e->u = mu;
            e->uo = mu;
            e->v = mv;
            e->vo = mv;
            mu->el.push_back(e);
            mv->el.push_back(e);
        }
        // set dual variable
        for (int j = 0; j < (int)mu->el.size(); j++)
            mindist = min(mindist, mu->el[j]->dist);
        mu->dual = mindist / 2;
    }
    return;
}




double slagma(node* place) {
    if (place == nullptr)    return 0;
    double dsum = 0;
    while (place->up != nullptr) {
        place = place->up;
        dsum += place->dual;    // 원래 blossom 없는 곳에서 시작해서 blossom으로 올라가서 +해야 함.
    }
    return dsum;
}


edge* edge_update(edge* (&e)) {
    while (e->u->up != nullptr)  e->u = e->u->up;
    while (e->v->up != nullptr)  e->v = e->v->up;
    return e;
}

void grow(node* to_extend, node* target, vector<node*> (&nodes)[2], vector<edge*> (&edges)) {
    // console << "growing" << endl;
    node* np = target->matched;

    // add data to tree's new nodes
    target->parent = to_extend;
    target->depth = to_extend->depth + 1;
    target->treeidx = to_extend->treeidx;

    np->parent = target;
    np->depth = target->depth + 1;
    np->treeidx = target->treeidx;

    // add outer to nodes[0], inner to nodes[1]
    nodes[0].emplace_back(np);         // outer (+)
    nodes[1].emplace_back(target);     // inner (-)

    // edge linking inner node not allowed
    edge* e;
    for (int i = 0; i < (int)edges.size(); ) {
        e = edge_update(edges[i]);
        if (e->u->depth % 2 || e->v->depth % 2) {
            edges[i] = edges[edges.size()-1];
            edges.pop_back();
        }
        else i++;
    }

    // remove edge to np node -> since will put soon -> no need for same multiple 
    for (int i = 0; i < (int)edges.size(); ) {      // edge updated above
        if (edges[i]->u == np || edges[i]->v == np) {
            edges[i] = edges[edges.size()-1];
            edges.pop_back();
        }
        else i++;
    }

    // add new outer's edges to only free / outer nodes == np's edge's both end depth even
    for (int i = 0; i < (int)np->el.size(); i++) {
        if (!((np->el[i]->u->depth) % 2 || (np->el[i]->v->depth) % 2)) {
            edges.emplace_back(np->el[i]);
        }
    }
    // console << np->depth << " growing done" << endl;
    return;
}

void shrink(node* tbase, node* to_extend, node* target, vector<node*> (&nodes)[2], vector<edge*> (&edges), vector<node*> (&btainer)) {
    // console << "shrinking" << endl;

    // create new node for blossom
    node* blossom = new node;


    // initialize blossom with tbase
    blossom->dual = 0.0;
    blossom->treeidx = tbase->treeidx;
    blossom->depth = tbase->depth;

    // matching & parent 대상 수정 (상대방도 여기서 같이 수정함)
    blossom->matched = tbase->matched;
    if (tbase->matched != nullptr)
        tbase->matched->matched = blossom;
    blossom->parent = tbase->parent;

    // blossom을 구성하는 node 이외의 node의 parent가 blossom을 구성하는 node의 경우
    // blossom 내부의 node의 up = blossom으로 하고, 조건 일치하면 blossom을 parent로 아래에서 조정

    // blossom 중첩 대충이라도 구분 위해서
    blossom->cur = tbase->cur;
    if (blossom->cur.index < 0)
        blossom->cur.index--;
    else
        blossom->cur.index = -1 * abs(blossom->cur.index);      // to see that it is a blossom

    // blossom could have been made by free node == tbase
    if (tbase->matched != nullptr)      
        tbase->matched->matched = blossom;
    
    // blossom just created = 100% have down, 100% doesn't have up
    blossom->up = nullptr;
    blossom->down = tbase;
    // blossom->nl.reserve(to_extend->depth + target->depth - (2 * tbase->depth) + 1);

    // put all nodes creating blossom to blossom->nl
    while (to_extend != tbase) {
        blossom->nl.emplace_back(to_extend);
        to_extend->up = blossom;
        to_extend = to_extend->parent;
    }
    blossom->nl.emplace_back(tbase);
    tbase->up = blossom;

    // reverse direction of parent = creating cycle of lower level nodes
    node* nverse = blossom->nl[0];
    node* nave;
    while (target != tbase) {
        blossom->nl.emplace_back(target);
        target->up = blossom;
        nave = target;
        target = target->parent;
        nave->parent = nverse;
        nverse = nave;
    }

    // target == tbase 경우 second while passed = tbase at the back
    if (tbase == blossom->nl[blossom->nl.size()-1])
        tbase->parent = blossom->nl[0];
    else 
        tbase->parent = blossom->nl[blossom->nl.size()-1];

    // 이걸로 blossom 내부 cycle + 모두의 up = blossom 완성


    // debug cycle
    // node* downnode = tbase;
    // while (downnode->parent != tbase)      {console << downnode->cur.index << ", "; downnode = downnode->parent;}
    // console << downnode->cur.index << endl;
    // downnode = downnode->parent;

    // nodes에 있는 blossom 내부의 node 제거, blossom은 추가
    for (int i = 0; i < (int)blossom->nl.size(); i++) {
        int ooe = (blossom->nl[i]->depth % 2);
        for (int j = 0; j < (int)nodes[ooe].size(); j++) {
            if (nodes[ooe][j] == blossom->nl[i]) {
                nodes[ooe][j] = nodes[ooe][nodes[ooe].size()-1];
                nodes[ooe].pop_back();
                i--;
                break;
            }
        }
    }
    nodes[0].emplace_back(blossom);     // blossom 생성 = 무조건 depth even한 경우에만


    // blossom 외부 node의 parent가 blossom 내부에 있다면, 그들의 parent를 blossom으로 재조정
    // blossom 형성되었다 = matching이 모두 blossom 형성에 사용되었다 = 외부에서 matching으로 연결되는 것은 tbase 말고 없다.
    // 심지어 tbase = outer라서 tbase의 parent와 matched = inner와 parent 관계 = nodes[1]은 무조건 parent 존재함
    // 위에서 inner node를 제거했으니까, 이제 무조건 외부 inner만 남은 상태
    for (int i = 0; i < (int)nodes[1].size(); i++) {
        if (nodes[1][i]->parent->up != nullptr)     nodes[1][i]->parent = nodes[1][i]->parent->up;
    }


    // 일단 edges에서 up이 한쪽이라도 존재하면 제거 -> 내부 node끼리 연결 여부, 기존 depth % 2 여부 확인 번거로워서
    for (int i = 0; i < (int)edges.size(); ) {
        if (edges[i]->u->up != nullptr || edges[i]->v->up != nullptr) {
            edges[i] = edges[edges.size() - 1];
            edges.pop_back();
        }
        else    i++;
    }

    // 기존 방식 ->
    // for (int i = 0; i < (int)blossom->nl.size(); i++) {
    //     for (int j = 0; j < (int)blossom->nl[i]->el.size(); j++) {
    //         if (blossom->nl[i]->el[j]->u == blossom->nl[i]) {
    //             if (blossom->nl[i]->el[j]->v->up == nullptr)
    //                 blossom->el.emplace_back(blossom->nl[i]->el[j]);
    //         }
    //         if (blossom->nl[i]->el[j]->v == blossom->nl[i]) {
    //             if (blossom->nl[i]->el[j]->u->up == nullptr)
    //                 blossom->el.emplace_back(blossom->nl[i]->el[j]);
    //         }
    //     }
    // }

    // debug
    // for (edge* ee : edges) {
    //     assert(get_slack(ee) >= 0);
    // }

    // for (node* bn : blossom->nl) {
    //     console << bn->cur.index << ", " << bn->depth << endl;
    // }

    // 다 blossom에 넣되, 이제는 기존에 inner였던 node의 edge도 추가 가능함
    // 외부와 연결된 edge를 blossom에 넣으면, 본인은 안 들고 있게 제거
    for (int i = 0; i < (int)blossom->nl.size(); i++) {
        for (int j = 0; j < (int)blossom->nl[i]->el.size(); ) {
            if (blossom->nl[i]->el[j]->u == blossom->nl[i]) {
                if (blossom->nl[i]->el[j]->v->up == nullptr) {
                    blossom->el.emplace_back(blossom->nl[i]->el[j]);
                    blossom->nl[i]->el[j] = blossom->nl[i]->el.back();
                    blossom->nl[i]->el.pop_back();
                }
                else j++;
            }
            else if (blossom->nl[i]->el[j]->v == blossom->nl[i]) {
                if (blossom->nl[i]->el[j]->u->up == nullptr) {
                    blossom->el.emplace_back(blossom->nl[i]->el[j]);
                    blossom->nl[i]->el[j] = blossom->nl[i]->el.back();
                    blossom->nl[i]->el.pop_back();
                }
                else j++;
            }
        }
        blossom->nl[i]->el.shrink_to_fit();
    }

    // nl의 경계 간선 추출 종료 = blossom의 inner와 연결된 edge 제외 나머지 edges에 추가 + 이제 edge update해서 blossom pointing하게
    for (int i = 0; i < (int)blossom->el.size(); i++) {
        edge_update(blossom->el[i]);
        if (!((blossom->el[i]->u->depth) % 2 || (blossom->el[i]->v->depth) % 2)) {
            edges.emplace_back(blossom->el[i]);
        }
    }


    // blossom이기에 btainer에 본인 추가 -> btainer는 순서 중요함
    btainer.emplace_back(blossom);

    // debug
    // int countfree = 0;
    // for (node* fn : nodes[0]) {
    //     if (fn->matched == nullptr) countfree++;
    // }
    // for (edge* be : blossom->el) {
    //     if (get_slack(be) < 0) {
    //         console << countfree << "th, " << get_slack(be) << " : " << be->u->cur.index << ", " << be->u->treeidx << ", " << be->u->depth << " vs " << be->v->cur.index << ", " << be->v->treeidx << ", " << be->v->depth << endl;
    //     }
    //     assert(get_slack(be) >= 0);
    // }

    // console << blossom->nl.size() << " nodes shrinking done" << endl;
    return;
}


node* expand(node* target, vector<node*> (&nodes)[2], vector<edge*> (&edges), vector<node*> (&btainer)) {
    // console << "expanding " << target->cur.index << ", " << " : size : " << target->nl.size() << endl;
    // for (node* bn : target->nl) {
    //     console << bn->cur.index << ", " << bn->depth << endl;
    // }
    
    // expand only in dual_update -> check if depth % 2 & dual == 0 & down != nullptr
    int bdx = -1;

    // remove blossom(target) from nodes vector
    for (int i = 0; i < (int)nodes[1].size(); i++) {
        if (target == nodes[1][i]) {
            bdx = i;
            break;
        }
    }
    if (bdx != -1) {
        nodes[1][bdx] = nodes[1][nodes[1].size()-1];
        nodes[1].pop_back();
    }
    // blossom 내부 순서 유지로 인해 pop_back 사용 못함 = erase -> 이러면 일단 target nodes, btainer에서 모두 제거됨
    for (int i = (int)btainer.size()-1; i > -1; i--) {
        if (btainer[i] == target)       btainer.erase(btainer.begin() + i);
    }

    
    // 이제는 edges vector에서 target과 연결된 edges 모두 제거
    // 이거 아니면 whether (in / not in) the path +  (inner / outer) node 필요해서 -> will only add outer soon
    for (int i = 0; i < (int)edges.size(); ) {
        if (edges[i]->u == target || edges[i]->v == target) {
            edges[i] = edges[edges.size()-1];
            edges.pop_back();
        }
        else    i++;
    }


    // blossom->el의 edge들 계승 = all edges shrink에서 update해서 모두 최상단 blossom이 u/v에 존재하는 중
    node* downnode;
    node* pn;
    node* mn;
    vector<edge*> pev = {};
    vector<edge*> mev = {};
    pev.reserve(target->nl.size());
    mev.reserve(target->nl.size());

    // for (int k = 0; k < (int)target->el.size(); k++)
    //     target->el[k] = edge_update(target->el[k]);

    // blossom(target)을 matched와 이어진 node ~ parent와 연결된 node를 기준으로 alternating path 설정을 다시해야 함
    // blossom 무조건 matched 상태 -> 본인보다 depth 큰 outer와
    // blossom (-) = inner에 위치한 상태 = parent 무조건 존재함

    // 기존 방식 -> 
    // for (int i = 0; i < (int)target->nl.size(); i++) {
    //     downnode = target->nl[i];            
    //     downnode->treeidx = target->treeidx;        // blossom could have been made in the past
    //     for (int j = 0; j < (int)downnode->el.size(); j++) {
    //         if (downnode->el[j]->u == target) {
    //             // edge의 한쪽 끝을 target -> 본인으로 edge 방향 조정
    //             downnode->el[j]->u = downnode;
    //             // 본인 반대쪽 parent / matched 여부 확인하기
    //             if (downnode->el[j]->v == target->parent)   pev.emplace_back(downnode->el[j]);
    //             if (downnode->el[j]->v == target->matched)  mev.emplace_back(downnode->el[j]);

    //         }
    //         if (downnode->el[j]->v == target) {
    //             downnode->el[j]->v = downnode;
    //             if (downnode->el[j]->u == target->parent)   pev.emplace_back(downnode->el[j]);
    //             if (downnode->el[j]->u == target->matched)  mev.emplace_back(downnode->el[j]);
    //         }
    //     }
    // }


    for (int i = 0; i < (int)target->el.size(); i++) {
        node* uside = target->el[i]->uo;
        node* vside = target->el[i]->vo;
        while (uside->up != nullptr && uside->up != target) uside = uside->up;
        while (vside->up != nullptr && vside->up != target) vside = vside->up;
        if (uside->up != nullptr && uside->up == target) {
            // edge의 한쪽 끝을 target -> 본인으로 edge 방향 조정
            target->el[i]->u = uside;
            uside->treeidx = target->treeidx;
            // shrink에서 blossom에 넘겨 주고 본인에게서 삭제했기에 다시 가져오기
            uside->el.emplace_back(target->el[i]);
            // 본인 반대쪽 parent / matched 여부 확인하기
            if (target->el[i]->v == target->parent)   pev.emplace_back(target->el[i]);
            if (target->el[i]->v == target->matched)  mev.emplace_back(target->el[i]);
        }
        else if (vside->up != nullptr && vside->up == target) {
            target->el[i]->v = vside;
            vside->treeidx = target->treeidx;
            vside->el.emplace_back(target->el[i]);
            if (target->el[i]->u == target->parent)   pev.emplace_back(target->el[i]);
            if (target->el[i]->u == target->matched)  mev.emplace_back(target->el[i]);
        }
    } 


    // expand all에서 상위 blossom 강제 해체하면서 일부 slack 보정 사라졌기에 get slack끼리의 크기 비교
    // slack 보정을 하면 blossom 내부 slack이 0보다 작게 나올 수 있고, 안 하면 외부와 slack이 0보다 큼

    // parent & matched와 blossom인 상태에서 무조건 tight -> 풀기 전까지 (모든 탐색 끝나고 강제 해제의 경우로 blossom 해제 이후는 보장 못함)
    int pepos = 0;
    // parent와 연결되는 node는 결국 matched node에서 방향만 바꾸면 되기에 slack == 0 아무거나 골라도 됨 -> 이 edge 바꾼다고 blossom 내부 matchin이 달라지는 것이 아니라서
    for (int i = 0; i < (int)pev.size(); i++) {
        if (get_slack(pev[pepos]) > get_slack(pev[i]))    pepos = i;  
    }
    
    // matched와 연결하는 edge에서 slack == 0이 다수 나오면, 뭘 고르는 가에 따라서 matching 전체 weight 달라져서 모두 확인해야 함.
    int mepos = 0;
    for (int i = 0; i < (int)mev.size(); i++) {
        if (get_slack(mev[mepos]) > get_slack(mev[i])) mepos = i;
    }




    // pn, mn 지정 -> pn은 pm 끝나고 expand all할 때 parent 없기에 조건 확인해서 없으면 pn = mn으로
    if (mev[mepos]->u == target->matched)    mn = mev[mepos]->v;
    else    mn = mev[mepos]->u;

    if (pev.size() == 0)    pn = mn;
    else {
        if (pev[pepos]->u == target->parent)    pn = pev[pepos]->v;
        else    pn = pev[pepos]->u;
    }

    
    // console << target->cur.index << ", " << target->cur.x << ", " << target->cur.y << "'s dual : " << target->dual << endl;
    // console << "matched within : " << mn->cur.index << ", " << mn->cur.x << ", " << mn->cur.y << "'s dual : " << mn->dual << endl;
    // console << "matched outside : " << target->matched->cur.index << ", " << target->matched->cur.x << ", " << target->matched->cur.y << "'s dual : " << target->matched->dual << endl;
    // coor a = {mn->cur.index, mn->cur.x, mn->cur.y};
    // coor b = {target->matched->cur.index, target->matched->cur.x, target->matched->cur.y};
    // console << "both dist : " << get_dist(a, b) << endl;

    // console << target->matched->cur.index << ", " << target->matched->cur.x << ", " << target->matched->cur.y << "'s dual : " << target->matched->dual << endl;
    // if (target->cur.index == -51) {
    //     for (int i = 0; i < (int)target->nl.size(); i++) {
    //         console << target->nl[i]->cur.index << ", " << target->nl[i]->cur.x << ", " << target->nl[i]->cur.y << "'s dual : " << target->nl[i]->dual << endl;
    //     }
    // }


    pev.clear();
    mev.clear();


    // slack 계산 다 한 뒤에 up & down 풀어야 함 -> 아니면 slack에서 0인 거가 아니게 됨
    for (int i = 0; i < (int)target->nl.size(); i++) {
        target->nl[i]->up = nullptr;
    }
    target->down = nullptr;


    // blossom 구성하는 node들 순서대로 넣을 공간 마련
    vector<node*> cycleseq;
    cycleseq.reserve(target->nl.size());

    // parent + depth 설정 확실하게 해야 함 -> parent와 연결된 대상이 depth 승계(정확히는 odd/even)
    pn->depth = target->depth;

    // 일단 parent에서 한바퀴 돌면서 depth 직전 +1로 전부 돌리기 -> pepos = 0 = cycleseq에서의 pn의 위치
    pepos = 0;
    downnode = pn;
    cycleseq.emplace_back(downnode);

    while (downnode->parent != pn) {
        downnode->parent->depth = downnode->depth+1;
        downnode = downnode->parent;
        cycleseq.emplace_back(downnode);
    }

    for (int i = 0; i < (int)cycleseq.size(); i++) {
        if (cycleseq[i] == mn)  mepos = i;
    }
    
    // [pn, mn]으로 둘다 방향에 포함해야 함.
    // cycle이 pn에서 시작해서 parent 따라서 내려감 -> 지금 depth 증가 = parent 감소하는 반비례로 이상하다는 점 주의.
    // 어차피 path 이외의 부분은 depth, parent, treeidx 모두 notree에 모은 뒤에 한번에 삭제 예정.
    vector<node*> notree = {};
    notree.reserve(cycleseq.size());


    // pn + mn = odd -> 현재 cycle parent 방향이 맞음 = depth가 이상함
    if ((pepos + mepos) % 2) {
        // pn, mn 제외 = path 아닌 구역
        for (int i = mepos-1; i > 0; i-=2) {
            // cycleseq[i]->matched = cycleseq[i-1];
            // cycleseq[i-1]->matched = cycleseq[i];

            notree.emplace_back(cycleseq[i]);
            notree.emplace_back(cycleseq[i-1]);
        }
        // mn 포함 = path 구역 -> 어차피 pn은 depth 정상, parent 받을 예정으로 for문에 없어도 됨.
        for (int i = (int)cycleseq.size()-1; i >= mepos; i-=2) {
            // depth 바꾸기, % 연산 i+1 = size()가 되기에 0으로 바꾸기 위해서
            cycleseq[i]->depth = cycleseq[(i+1) % cycleseq.size()]->depth+1;
            cycleseq[i-1]->depth = cycleseq[i]->depth+1;

            // cycleseq[i]->matched = cycleseq[i-1];
            // cycleseq[i-1]->matched = cycleseq[i];
        }
    }

    // pn + mn = even -> 현재 cycle parent 방향이 틀렸지만, depth가 맞음
    else {
        // mn 포함, pn 비슷한 이유로 포함 X.
        for (int i = mepos; i > 0; i-=2) {
            // parent 방향 바꾸기.
            cycleseq[i]->parent = cycleseq[i-1];
            cycleseq[i-1]->parent = cycleseq[i-2];

            // cycleseq[i]->matched = cycleseq[i-1];
            // cycleseq[i-1]->matched = cycleseq[i];
        }
        // pn, mn 제외.
        for (int i = mepos+1; i < (int)cycleseq.size(); i+=2) {
            // cycleseq[i]->matched = cycleseq[i+1];
            // cycleseq[i+1]->matched = cycleseq[i];

            notree.emplace_back(cycleseq[i]);
            notree.emplace_back(cycleseq[i+1]);
        }
    }


    for (int i = mepos-1; i > 0; i-=2) {
        cycleseq[i]->matched = cycleseq[i-1];
        cycleseq[i-1]->matched = cycleseq[i];
    }
    for (int i = mepos+1; i < (int)cycleseq.size(); i+=2) {
        cycleseq[i]->matched = cycleseq[(i+1) % cycleseq.size()];
        cycleseq[(i+1) % cycleseq.size()]->matched = cycleseq[i];
    }


    cycleseq.clear();

    // initialize nodes that are not on the path
    for (int i = 0; i < (int)notree.size(); i++) {
        notree[i]->depth = 0;
        notree[i]->parent = nullptr;
        notree[i]->treeidx = -1;
    }

    // pn에 target의 parent 계승 여기서
    pn->parent = nullptr;
    pn->parent = target->parent;
    target->matched->parent = mn;       // 다시 말하지만 matched = 무조건 child라서 가능

    // mn에 matched 계승
    target->matched->matched = mn;
    mn->matched = target->matched;


    // debug
    // console << "total nodes : " << target->nl.size() << ", not tree anymore : " << notree.size() << endl;


    // path 상의 node만 추가
    for (int i = 0; i < (int)notree.size(); i++) {
        for (int j = 0; j < (int)target->nl.size(); ) {
            if (notree[i] == target->nl[j]) {
                target->nl[j] = target->nl[target->nl.size()-1];
                target->nl.pop_back();      // 이제 target delete 예정이라서 가능함.
            }
            else j++;
        }
    }

    // 하단의 notree edge로 인해 잠시 treeidx = -1로 -> 이후 복구 예정
    for (node* tmpn : target->nl) {
        if (tmpn != mn) tmpn->treeidx = -1;
    }  

    // notree에 있는 것들과 이제 outer 연결 가능으로 그것들만 뽑아서 연결
    for (int i = 0; i < (int)notree.size(); i++) {
        for (edge* te : notree[i]->el) {
            if (te->u == notree[i] && te->v->treeidx != -1 && !(te->v->depth % 2)) edges.emplace_back(te);
            else if (te->v == notree[i] && te->u->treeidx != -1 && !(te->u->depth % 2)) edges.emplace_back(te);
        }
    }

    notree.clear();

    for (node* tmpn : target->nl) {
        tmpn->treeidx = mn->treeidx;
    }  

    // target->nl에 남은 것 = path 상에 존재하는 것.
    for (int i = 0; i < (int)target->nl.size(); i++)
        nodes[(target->nl[i]->depth) % 2].emplace_back(target->nl[i]);



    // 이제 target->nl[i] % depth == 0 & target->nl[i]->el[j] not to inner 검사하고 outer의 edge만 추가 -> grow와 같은 방식 사용
    for (int i = 0; i < (int)target->nl.size(); i++) {
        if ((target->nl[i]->depth) % 2)   continue;       // doesn't accept inner node
        for (int j = 0; j < (int)target->nl[i]->el.size(); j++) {
            if (!((target->nl[i]->el[j]->u->depth) % 2 || (target->nl[i]->el[j]->v->depth) % 2)) {
                edges.emplace_back(edge_update(target->nl[i]->el[j]));
            }
        }
    }


    // debug
    // console << target->nl.size() << endl;
    // for (int i = 0; i < (int)target->nl.size(); i++) {
    //     console << target->nl[i]->depth << " : " << target->nl[i]->cur.index << ", " << target->nl[i]->cur.x << ", " << target->nl[i]->cur.y << "matched with " << target->nl[i]->matched->depth << " : "  << target->nl[i]->matched->cur.index << ", " << target->nl[i]->matched->cur.x << ", " << target->nl[i]->matched->cur.y << endl;
    // }


    target->el.clear();
    target->nl.clear();
    // delete target
    delete target;


    node* endnode = pn->parent;

    downnode = mn;
    // console << "up out : " << (downnode->depth) % 2 << " + " << downnode->dual << " + " << (downnode->down != nullptr) << " & " << downnode->cur.index << ", " << downnode->cur.x << ", " << downnode->cur.y << endl;
    while (1) {
        // console << "up in : " << downnode << " & " << (((downnode->depth) % 2) && (downnode->dual == 0) && (downnode->down != nullptr)) << " & " << downnode->cur.index << ", " << downnode->cur.x << ", " << downnode->cur.y << endl;
        if ((downnode->depth % 2) && (downnode->dual == 0) && (downnode->down != nullptr)) {
            downnode = expand(downnode, nodes, edges, btainer);
        }
        if (downnode->parent == endnode || downnode->parent == nullptr)    break;
        downnode = downnode->parent;
        // console << "endofloop" << endl;
    }
    // console << "up free : " << (downnode->depth) % 2 << " + " << downnode->dual << " + " << (downnode->down != nullptr) << " & " << downnode->cur.index << ", " << downnode->cur.x << ", " << downnode->cur.y << endl;


    // console << "expanding done" << endl;
    return downnode;      // mn -> parent -> 이미 내부 정렬 다 해서 = 들어올 때의 blossom과 동일한 depth의 node 반환
}


int dual_update(double delta, vector<node*> (&nodes)[2], vector<edge*> (&edges), vector<node*> (&btainer)) {     // assuming blossom removed its nodes inside
    for (int i = 0; i < (int)nodes[0].size(); i++) {
        nodes[0][i]->dual += delta;
    }
    for (int i = 0; i < (int)nodes[1].size(); i++) {
        nodes[1][i]->dual -= delta;
    }
    // expand only after cur' tree's all nodes' dual update completed = no multiple update allowed
    int passed = 0;
    for (int i = 0; i < (int)nodes[1].size(); i++) {
        if ((nodes[1][i]->depth % 2) && nodes[1][i]->dual == 0 && nodes[1][i]->down != nullptr) {       // expand only if inner node & dual = 0 & is blossom
            expand(nodes[1][i], nodes, edges, btainer);
            passed = 1;
        }
    }

    return passed;
}


void augment(node* target, vector<node*> (&nodes)[2], vector<edge*> (&edges), vector<node*> (&btainer)) {        // always target == free node
    // console << "augmenting" << endl;
    // console << "new : " << target->cur.index << " -> ";

    // 이미 dual update 하고 와서, expand 필요한 경우 이미 실시한 상태.
    int tar_tree = target->treeidx;
    int ext_tree = target->parent->treeidx;

    while (target->parent != nullptr) {
        if (target->depth % 2) {
            target->parent->matched = target;
            target->matched = target->parent;
        }
        target = target->parent;
        // console << target->cur.index << " -> ";
    }
    // console << endl;

    // remove edges related to alternating tree    -> for multiple tree
    for (int i = 0; i < (int)edges.size(); ) {
        if (edges[i]->u->treeidx == tar_tree || edges[i]->u->treeidx == ext_tree || edges[i]->v->treeidx == tar_tree || edges[i]->v->treeidx == ext_tree) {
            edges[i] = edges.back();
            edges.pop_back();
        }
        else    i++;
    }

    for (node* remainn : nodes[0]) {
        if (remainn->treeidx == tar_tree || remainn->treeidx == ext_tree)   continue;
        for (edge* nie : remainn->el) {
            if (nie->u->treeidx == tar_tree || nie->u->treeidx == ext_tree) edges.emplace_back(nie);
            if (nie->v->treeidx == tar_tree || nie->v->treeidx == ext_tree) edges.emplace_back(nie);
        }
    }

    // intialize every node we looked at -> in multiple, only trees that met
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < (int)nodes[i].size(); ) {
            node* tmpn = nodes[i][j];
            if (tmpn->treeidx == tar_tree || tmpn->treeidx == ext_tree) {
                tmpn->treeidx = -1;
                tmpn->depth = 0;
                tmpn->parent = nullptr;
                nodes[i][j] = nodes[i].back();
                nodes[i].pop_back();
            }
            else    j++;
        }
    }


    // console << "augmenting done" << endl;
    return;
}

node* nca(edge* e) {
    node* to_extend = e->u;
    node* target = e->v;
    // for easy computation, make to_extend always have more depth -> depth only sure about odd / even -> change it
    int tep = 0;
    int tp = 0;
    // count number of parents
    node* ncachecker = to_extend;
    while (ncachecker->parent != nullptr) {
        tep++;
        ncachecker = ncachecker->parent;
    }
    ncachecker = target;
    while (ncachecker->parent != nullptr) {
        tp++;
        ncachecker = ncachecker->parent;
    }
    if (tep < tp) {
        swap(to_extend, target);
        swap(tep, tp);
    }

    // find nca
    while (tep-- > tp)   to_extend = to_extend->parent;
    while (to_extend != target) {
        to_extend = to_extend->parent;
        target = target->parent;
    }
    return to_extend;
}

void blossomV(vector<coor>& odds, map<coor, node>& mn, map<pair<node*, node*>, edge>& me, int& lasttry) {
    vector<node*> btainer;
    btainer.reserve(odds.size());
    for (int mnt = 0; mnt < (int)odds.size() / 2; mnt++) {
        int odx = mnt;

        // console << odx+1 << "/" << odds.size() / 2 << ", coor nums : " << odds.size() << endl;
        

        vector<node*> nodes[2] = {};
        nodes[0].reserve(odds.size());       // for outer (+)
        nodes[1].reserve(odds.size());       // for inner (-)
        vector<edge*> edges = {};
        edges.reserve((odds.size() * odds.size()) / 2);

        double delta;
        edge* e;
        node* n;

        vector<edge*> starter = {};
        // int vsize = min(starter.max_size(), odds.size() * odds.size());
        // starter.reserve(vsize);

        // 기존 방식 ->
        // for (int i = 0; i < (int)odds.size(); i++) {
        //     for (int j = i; j < (int)odds.size()-1; j++) {
        //         if ((mn[odds[i]].el[j]->u->matched == nullptr && mn[odds[i]].el[j]->u->up == nullptr && mn[odds[i]].el[j]->u->down == nullptr) || 
        //             (mn[odds[i]].el[j]->v->matched == nullptr && mn[odds[i]].el[j]->v->up == nullptr && mn[odds[i]].el[j]->v->down == nullptr))
        //             starter.emplace_back(mn[odds[i]].el[j]);
        //     }
        // }


        // get only edges that are connected to free node
        for (pair<const pair<node*, node*>, edge>& edx : me) {
            if ((edx.second.u->matched == nullptr && edx.second.u->up == nullptr && edx.second.u->down == nullptr) || 
                (edx.second.v->matched == nullptr && edx.second.v->up == nullptr && edx.second.v->down == nullptr))
                starter.emplace_back(&edx.second);            
        }
        
        // multiple tree : 
        if (odds.size() * 0.475 < odx) {
            // console << "now multiple" << endl;
            for (int i = 0; i < (int)starter.size(); i++) {
                e = edge_update(starter[i]);
                edges.emplace_back(e);
                if (starter[i]->u->matched == nullptr && starter[i]->u->treeidx == -1) {
                    starter[i]->u->treeidx = starter[i]->u->cur.index;
                    nodes[0].emplace_back(starter[i]->u);
                }
                if (starter[i]->v->matched == nullptr && starter[i]->v->treeidx == -1) {
                    starter[i]->v->treeidx = starter[i]->v->cur.index;
                    nodes[0].emplace_back(starter[i]->v);
                }
            }
            starter.clear();
            starter.shrink_to_fit();
        }


        // single tree : 
        else {
            // get free node with smallest slack from those edges
            int stedge = 0;
            for (int i = 0; i < (int)starter.size(); i++) {
                e = edge_update(starter[i]);
                if (get_slack(starter[stedge]) > get_slack(starter[i])) {
                    stedge = i;
                }
            }
            if (starter[stedge]->u->matched == nullptr) n = starter[stedge]->u;
            else    n = starter[stedge]->v;
            starter.clear();
            starter.shrink_to_fit();

            // n == root of the tree -> insert all edges of root
            n->treeidx = n->cur.index;
            for (int i = 0; i < (int)n->el.size(); i++) {
                edges.emplace_back(n->el[i]);
            }
            nodes[0].emplace_back(n);            
        }



        // loop until augment is made = (shrink / grow / expand doesn't break the loop)
        while (1) {
            // // debug
            // for (int i = 0; i < (int)nodes[1].size(); i++) {
            //     console << nodes[1][i]->cur.index << ", " << nodes[1][i]->cur.x << ", " << nodes[1][i]->cur.y << " matched with " << nodes[1][i]->matched->cur.index << ", " << nodes[1][i]->matched->cur.x << ", " << nodes[1][i]->matched->cur.y << endl;
            // }
            // // debug
            // if (btainer.size() > 0) {
            //     node* pn = btainer[btainer.size()-1]->down;
            //     node* downnode = pn;
            //     while (downnode->parent != pn)      {console << downnode->cur.index << ", " << downnode->cur.x << ", " << downnode->cur.y << endl; downnode = downnode->parent;}
            //     console << downnode->cur.index << ", " << downnode->cur.x << ", " << downnode->cur.y << endl;
            //     downnode = downnode->parent;
            // }
            

            // dual_update에서 expand함 = expand 안 하고 나오면 grow / shrink / augment 중 하나되도록 dual_update까지를 loop 시켜야 함.
            int noexpand = 1;   // see if it expanded = need to get new delta
            int adx = 0;        // if not expanded, use it to get minimum slack edge
            while (noexpand) {
                // debug
                // console << "no expand start : " << noexpand << endl;

                // partly unsorted -> should I?
                // sort(edges.begin(), edges.end());

                // initialize delta, adx
                delta = INF;
                adx = 0;
                // make delta be value that make blossom expand
                for (int i = 0; i < (int)nodes[1].size(); i++) {
                    if (nodes[1][i]->down != nullptr && delta > nodes[1][i]->dual)   delta = nodes[1][i]->dual;
                }

                // console << "expand lim : " << delta << ", ";


                for (int i = 0; i < (int)edges.size(); i++) {
                    // no need to check inner edge here now.
                    if (edges[i]->u->treeidx != -1 && edges[i]->v->treeidx != -1) {       // edge's two end = both tree
                        if ((edges[i]->u->depth % 2) || (edges[i]->v->depth % 2))    continue;
                        if (delta > get_slack(edges[i]) / 2) {      // since removed edge with inner node, delta = get_slack() / 2
                            delta = get_slack(edges[i]) / 2;
                            adx = i;
                        }
                    }
                    else if (delta > get_slack(edges[i])) {     // only one end is a tree = other side (free node / already matched not tree node )
                        delta = get_slack(edges[i]);
                        adx = i;
                    }
                }

                // since single tree = dual update now.
                noexpand = dual_update(delta, nodes, edges, btainer);


                // debug
                // console << "no expand end : " << noexpand << endl;
            }

            // selected new edge to check
            e = edges[adx];
            swap(edges[adx], edges[edges.size()-1]);    // edge 선택 다시 불가하게 제거
            edges.pop_back();

            
            node* to_extend;
            node* target;

            // to make to_extend 100% part of tree
            if (e->u->treeidx != -1) {    // node shown (both u, v could be blossom)
                to_extend = e->u;
                target = e->v;
            }
            else {
                to_extend = e->v;
                target = e->u;
            }

            // debug
            // console << "to_extend : " << to_extend->cur.index << ", " << to_extend->cur.x << ", " << to_extend->cur.y << endl;
            // console << "target : " << target->cur.index << ", " << target->cur.x << ", " << target->cur.y << endl;


            // target is a tree = (augment / shrink)
            if (target->treeidx != -1) {
                if (target->treeidx != to_extend->treeidx) {    // different tree met = augment (not implemented yet)
                    node* wastarget;
                    while (target->parent != nullptr) {
                        wastarget = target;
                        target = target->parent;
                        wastarget->parent = to_extend;
                        wastarget->depth = to_extend->depth+1;
                        wastarget->treeidx = to_extend->treeidx;
                        to_extend = wastarget;
                    }
                    target->parent = to_extend;
                    target->depth = to_extend->depth+1;
                    augment(target, nodes, edges, btainer);
                    lasttry--;
                    if (nodes[0].size() + nodes[1].size() == 0)
                        break;
                }
                else {      // same tree = since no edge to inner node = shrink
                    node* tbase = nca(e);
                    // if ((to_extend->depth + target->depth - (2 * tbase->depth) + 1) % 2)     // odd cycle = need to make a blossom
                    shrink(tbase, to_extend, target, nodes, edges, btainer);       // 서로를 cl에서 지우기 해야 함 -> 나중에 augment 위해
                    // even cycle = just ignore and continue
                }

            }
            // target is not a tree = (augment / grow)
            else {
                if (target->matched == nullptr) {   // target is free node = augment
                    // to make augmenting path
                    target->parent = to_extend;
                    target->depth = to_extend->depth+1;
                    target->treeidx = to_extend->treeidx;
                    nodes[1].emplace_back(target);

                    augment(target, nodes, edges, btainer);
                    lasttry--;
                    if (nodes[0].size() + nodes[1].size() == 0)
                        break;
                }
                else {      // target is already matched = grow
                    grow(to_extend, target, nodes, edges);
                }

            }
        }
        
        // last turn = free all blossoms
        if (lasttry == 0) {
            // console << "expand all " << btainer.size() << endl;
            while (!btainer.empty()) {
                // debug
                // console << "to expand : " << btainer[(int)btainer.size()-1]->cur.index << ", " << btainer[(int)btainer.size()-1]->cur.x << ", " << btainer[(int)btainer.size()-1]->cur.y << " & matched with : " << 
                //         btainer[(int)btainer.size()-1]->matched->cur.index << ", " << btainer[(int)btainer.size()-1]->matched->cur.x << ", " << btainer[(int)btainer.size()-1]->matched->cur.y << endl;
                // for (int i = 0; i < (int)btainer[(int)btainer.size()-1]->el.size(); i++) {if ((btainer[(int)btainer.size()-1]->el[i]->u == btainer[(int)btainer.size()-1]->matched) || (btainer[(int)btainer.size()-1]->el[i]->v == btainer[(int)btainer.size()-1]->matched)) console << "is there" << endl; break;}
                
                node* tn = btainer[(int)btainer.size()-1]->matched;
                tn->parent = btainer[(int)btainer.size()-1];
                btainer[(int)btainer.size()-1]->depth++;
                tn->depth = btainer[(int)btainer.size()-1]->depth+1;
                // btainer[(int)btainer.size()-1]->parent = tn;
                expand(btainer[(int)btainer.size()-1], nodes, edges, btainer);
            }
            return;
        }

    }
        for (int i = 0; i < (int)odds.size(); i++) {
            if (mn[odds[i]].matched->matched != &mn[odds[i]] || mn[odds[i]].matched->matched->matched != mn[odds[i]].matched)   console << "bad matching" << endl;  
        }

    
    return;
}



vector<pair<coor, coor>> mwpm(vector<coor>& odds) {
    map<coor, node> mn;
    map<pair<node*, node*>, edge> me;
    vector<pair<coor, coor>> ans;
    initialize(odds, mn, me);
    int ohalf = odds.size() / 2;
    blossomV(odds, mn, me, ohalf);

    ans.reserve(odds.size() / 2);
    double check = 0;
    for (int i = 0; i < (int)odds.size(); i++) {
        node* final = &mn[odds[i]];
        if (final->matched != nullptr) {
            check += get_dist(final->cur, final->matched->cur);
            ans.emplace_back(final->cur, final->matched->cur);
            final->matched->matched = nullptr;
            final->matched = nullptr;
        }
    }
    // console << "should be same : " << check << endl;

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

double basic_CH(int num, vector<coor>& v, const string& dataset_name) {
    string out_filename = "basic_CH_" + dataset_name + ".txt";
    ofstream viz_out(out_filename);

    map<coor, int> m;
    // 과정 1: MST 생성 (내부에서 파란색 출력)
    vector<pair<coor, coor>> mst = prim(num, v, m, viz_out);

    vector<coor> odds;
    for (int i = 0; i < num; i++) {
        if (m[v[i]] % 2 != 0) odds.emplace_back(v[i]);
    }
    
    vector<pair<coor, coor>> pm = mwpm(odds);

    // [과정 2] MWPM 생성 (MST: 회색 1, MWPM: 초록 2로 하나씩 추가)
    vector<pair<coor, coor>> current_matching;
    for (const auto& pme : pm) {
        current_matching.push_back(pme);
        viz_out << "FRAME: Finding Minimum Weight Perfect Matching\n";
        for (const auto& e : mst) {
            viz_out << e.first.x << " " << e.first.y << " " << e.second.x << " " << e.second.y << " 1\n"; // 뼈대 MST는 회색(1)
        }
        for (const auto& e : current_matching) {
            viz_out << e.first.x << " " << e.first.y << " " << e.second.x << " " << e.second.y << " 2\n"; // 매칭선은 초록(2)
        }
        viz_out << "\n";
    }

    // 인접 리스트 생성
    vector<vector<coor>> ntn(mst.size() + pm.size() + 1);
    for (int i = 0; i < (int)mst.size(); i++) {
        ntn[mst[i].second.index].emplace_back(mst[i].first);
        ntn[mst[i].first.index].emplace_back(mst[i].second);
    }
    for (int i = 0; i < (int)pm.size(); i++) {
        ntn[pm[i].second.index].emplace_back(pm[i].first);
        ntn[pm[i].first.index].emplace_back(pm[i].second);
    }
    
    vector<coor> ec = eulercircuit(ntn);
    ntn.clear();
    int visited[mst.size() + pm.size() + 1] = {};

    double chtotal = 0;
    coor recent = ec[0];
    visited[recent.index] = 1;
    vector<coor> tsp_path;
    tsp_path.push_back(recent);

    // [과정 3] 오일러 회로 숏컷 탐색 (배경: 회색 1, 탐색 경로: 파랑 0)
    // [과정 3] 오일러 회로 숏컷 탐색 
    for (int i = 1; i < (int)ec.size(); i++) {
        if (!(visited[ec[i].index])) {
            visited[ec[i].index] = 1;
            chtotal += get_dist(recent, ec[i]);
            recent = ec[i];
            tsp_path.push_back(recent);

            viz_out << "FRAME: Euler Circuit Shortcut (Using MWPM Bridges)\n";
            
            // 1. 뼈대인 MST는 회색(1)으로 출력
            for (const auto& e : mst) {
                viz_out << e.first.x << " " << e.first.y << " " << e.second.x << " " << e.second.y << " 1\n";
            }
            // 2. [핵심 포인트!] MWPM 매칭선은 초록색(2)으로 계속 살려둡니다.
            // 이렇게 하면 파란색 숏컷 선이 초록색 다리를 타고 다른 트리 가지로 건너뛰는 것을 눈으로 볼 수 있습니다!
            for (const auto& e : pm) {
                viz_out << e.first.x << " " << e.first.y << " " << e.second.x << " " << e.second.y << " 2\n";
            }
            
            // 3. 현재 뻗어나가는 오일러 회로 숏컷 경로는 파란색(0)
            for (size_t j = 0; j + 1 < tsp_path.size(); ++j) {
                viz_out << tsp_path[j].x << " " << tsp_path[j].y << " " << tsp_path[j+1].x << " " << tsp_path[j+1].y << " 0\n";
            }
            viz_out << "\n";
        }
        if (i == (int)ec.size()-1) {
            chtotal += get_dist(recent, ec[0]);
            tsp_path.push_back(ec[0]);
            
            // [과정 4] 최종 완성 시에는 모두 깔끔하게 보라색(3)으로 통일!
            viz_out << "FRAME: Final TSP Path\n";
            for (size_t j = 0; j + 1 < tsp_path.size(); ++j) {
                viz_out << tsp_path[j].x << " " << tsp_path[j].y << " " << tsp_path[j+1].x << " " << tsp_path[j+1].y << " 3\n";
            }
            viz_out << "\n";
        }
    }
    viz_out.close();
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
