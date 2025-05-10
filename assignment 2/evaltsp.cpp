#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <sys/stat.h>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <numeric>
#include <filesystem>
#include <sys/wait.h>

namespace fs = std::filesystem;

using namespace std;

// ======= 설정 영역 =======

std::vector<std::string> programs = {
    // "./basic_HK", 
    "./basic_CH"
    // 필요시 다른 프로그램 추가
};

std::vector<std::string> tsp_files = {
    // "a280.tsp",
    // "xql662.tsp",
    // "kz9976.tsp",
    "mona-lisa100K.tsp"
};

std::string tsp_dir = "./tsp_dataset/";
std::string result_csv = "eval_tsp_result.csv";
// 0부터 시작함
int start_index = 0;
int num_coor = 100000;
int repeat = 1; // 반복 횟수

// ======= 구조체 및 유틸 =======

struct RunResult {
    double time_ms;
    double memory_mb;
    double result_val;
};

bool file_exists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

int get_dimension_from_tsp(const std::string& tsp_path) {
    std::ifstream fin(tsp_path);
    std::string line;
    while (std::getline(fin, line)) {
        if (line.find("DIMENSION") != std::string::npos) {
            size_t pos = line.find(":");
            return std::stoi(line.substr(pos + 1));
        }
    }
    return -1;
}

// 파일 이름이 이미 존재하면 뒤에 숫자를 붙여 새로운 이름 반환
std::string get_unique_filename(const std::string& base_name) {
    std::string name = base_name;
    size_t dot_pos = base_name.find_last_of('.');
    std::string prefix = (dot_pos == std::string::npos) ? base_name : base_name.substr(0, dot_pos);
    std::string ext = (dot_pos == std::string::npos) ? "" : base_name.substr(dot_pos);
    int count = 1;
    while (fs::exists(name)) {
        name = prefix + std::to_string(count) + ext;
        ++count;
    }
    return name;
}

// 실행 함수: result도 파싱
void execute(const std::string& program, const std::string& tsp_path, int start, int num_coor, RunResult& result) {
    std::string temp_output = "temp_output.txt";
    std::string temp_mem = "temp_mem.txt";
    std::string command = program + " " + tsp_path + " " + std::to_string(start) + " " + std::to_string(num_coor);
    std::string full_cmd = "/usr/bin/time -f \"%M\" -o " + temp_mem + " " + command + " > " + temp_output;
    int ret_code = std::system(full_cmd.c_str());
    if (ret_code != 0) {
        if (WIFSIGNALED(ret_code)) {
            int sig = WTERMSIG(ret_code);
            std::cerr << "프로그램이 시그널 " << sig << " (";
            switch(sig) {
                case SIGKILL: std::cerr << "SIGKILL"; break;
                case SIGSEGV: std::cerr << "SIGSEGV"; break;
                case SIGABRT: std::cerr << "SIGABRT"; break;
                case SIGFPE:  std::cerr << "SIGFPE";  break;
                case SIGBUS:  std::cerr << "SIGBUS";  break;
                // 필요한 시그널 추가
                default: std::cerr << "알 수 없음"; break;
            }
            std::cerr << ")에 의해 종료되었습니다.\n";
        } else if (WIFEXITED(ret_code)) {
            int exit_code = WEXITSTATUS(ret_code);
            std::cerr << "프로그램이 비정상 종료(exit code: " << exit_code << "): " << command << "\n";
        } else {
            std::cerr << "프로그램이 알 수 없는 이유로 종료됨: " << command << "\n";
        }
        result.time_ms = -1;
        result.memory_mb = -1;
        result.result_val = -1;
        return;
    }
    // 시간/결과 추출
    std::ifstream fout(temp_output);
    std::string line;
    double algorithm_time = -1.0;
    double result_val = -1.0;
    while (std::getline(fout, line)) {
        if (line.find("ALGORITHM_TIME_MS:") != std::string::npos) {
            std::string time_str = line.substr(line.find(":") + 1);
            time_str.erase(0, time_str.find_first_not_of(" \t"));
            time_str.erase(time_str.find_last_not_of(" \t") + 1);
            try {
                algorithm_time = std::stod(time_str);
            } catch (...) {
                algorithm_time = -1.0;
            }
        }
        if (line.find("RESULT:") != std::string::npos) {
            std::string result_str = line.substr(line.find(":") + 1);
            result_str.erase(0, result_str.find_first_not_of(" \t"));
            result_str.erase(result_str.find_last_not_of(" \t") + 1);
            try {
                result_val = std::stod(result_str);
            } catch (...) {
                result_val = -1.0;
            }
        }
    }
    fout.close();
    // 메모리 추출
    std::ifstream mem_file(temp_mem);
    long mem_kb = 0;
    mem_file >> mem_kb;
    mem_file.close();
    result.time_ms = algorithm_time;
    result.memory_mb = mem_kb / 1024.0;
    result.result_val = result_val;
    std::remove(temp_output.c_str());
    std::remove(temp_mem.c_str());
}

// 반복 실행 및 평균 기록
void runProgram(const std::string& program, const std::string& tsp_file, int start, int num_coor, int repeat,
                double& avg_time, double& avg_memory, double& avg_result) {
    std::vector<double> times, memories, results;
    std::string tsp_path = tsp_dir + tsp_file;
    for (int r = 0; r < repeat; ++r) {
        RunResult result;
        execute(program, tsp_path, start, num_coor, result);
        // start++;
        if (result.time_ms >= 0 && result.memory_mb >= 0 && result.result_val >= 0) {
            times.push_back(result.time_ms);
            memories.push_back(result.memory_mb);
            results.push_back(result.result_val);
            std::cout << "Algorithm: " << program.substr(2)
                    << " | TSP File: " << tsp_file
                    << " | Start: " << start
                    << " | Count: " << num_coor
                    << " | Run: " << (r + 1)
                    << " | Time: " << std::fixed << std::setprecision(2) << result.time_ms << "ms"
                    << " | Memory: " << std::fixed << std::setprecision(2) << result.memory_mb << "MB"
                    << " | Result: " << std::fixed << std::setprecision(10) << result.result_val << "\n";
        }
    }
    if (!times.empty()) {
        avg_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        avg_memory = std::accumulate(memories.begin(), memories.end(), 0.0) / memories.size();
        avg_result = std::accumulate(results.begin(), results.end(), 0.0) / results.size();
    } else {
        avg_time = -1;
        avg_memory = -1;
        avg_result = -1;
    }
}

// ======= 메인 =======

int main() {
    std::string unique_csv = get_unique_filename(result_csv);
    std::ofstream csv(unique_csv);
    csv << "Algorithm,TSPFile,StartIndex,Count,AverageTime(ms),AverageMemory(MB),AverageResult\n";
    for (const auto& program : programs) {
        if (!file_exists(program)) {
            std::cerr << "실행 파일 없음: " << program << "\n";
            continue;
        }
        for (const auto& tsp_file : tsp_files) {
            std::string tsp_path = tsp_dir + tsp_file;
            if (!file_exists(tsp_path)) {
                std::cerr << "TSP 파일 없음: " << tsp_path << "\n";
                continue;
            }
            int dimension = get_dimension_from_tsp(tsp_path);
            if (dimension <= 0) {
                std::cerr << "DIMENSION 파싱 실패: " << tsp_file << "\n";
                continue;
            }
            if (start_index >= dimension) {
                std::cerr << "시작 인덱스(" << start_index << ")가 DIMENSION(" << dimension << ") 이상입니다. 건너뜀.\n";
                continue;
            }
            int actual_count = std::min(num_coor, dimension - start_index);
            double avg_time = 0.0, avg_memory = 0.0, avg_result = 0.0;
            runProgram(program, tsp_file, start_index, actual_count, repeat, avg_time, avg_memory, avg_result);
            csv << program.substr(2) << ","
                << tsp_file << ","
                << start_index << ","
                << actual_count << ","
                << std::fixed << std::setprecision(2) << avg_time << ","
                << std::fixed << std::setprecision(2) << avg_memory << ","
                << std::fixed << std::setprecision(10) << avg_result << "\n";
            std::cout << "---------------------------------------------------\n";
        }
    }
    csv.close();
    std::cout << "CSV 저장 완료: " << unique_csv << "\n";
    return 0;
}
