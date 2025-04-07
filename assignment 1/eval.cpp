#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <unistd.h>
#include <numeric>

namespace fs = std::filesystem;

struct RunResult {
    double time_ms;    // 실행 시간 (밀리초)
    double memory_mb;  // 메모리 사용량 (MB)
};

void execute(const std::string& program, const std::string& input_file, RunResult& result) {
    // 프로그램 출력을 임시 파일로 리다이렉션
    std::string temp_output_file = "temp_output.txt";
    std::string command = program + " ./tests/" + input_file + " > " + temp_output_file;

    // 메모리 측정 명령어
    std::string time_command = "/usr/bin/time -f \"%M\" -o temp_mem.txt " + command;

    // 프로그램 실행
    int ret_code = std::system(time_command.c_str());
    if (ret_code != 0) {
        std::cerr << "Error: Failed to execute " << program << "\n";
        exit(EXIT_FAILURE);
    }

    // 임시 출력 파일에서 ALGORITHM_TIME_MS 값 파싱
    std::ifstream output_file_stream(temp_output_file);
    if (!output_file_stream.is_open()) {
        std::cerr << "Error: Failed to open " << temp_output_file << "\n";
        exit(EXIT_FAILURE);
    }

    std::string line;
    double algorithm_time = -1.0;
    while (std::getline(output_file_stream, line)) {
        if (line.find("ALGORITHM_TIME_MS:") != std::string::npos) {
            std::string time_str = line.substr(line.find(":") + 1);
            time_str.erase(0, time_str.find_first_not_of(" \t"));
            time_str.erase(time_str.find_last_not_of(" \t") + 1);
            try {
                algorithm_time = std::stod(time_str);
            } catch (const std::exception& e) {
                std::cerr << "Error: Failed to parse time value: " << time_str << "\n";
            }
            break;
        }
    }
    output_file_stream.close();

    // 메모리 사용량 측정
    std::ifstream mem_file("temp_mem.txt");
    if (!mem_file.is_open()) {
        std::cerr << "Error: Failed to open temp_mem.txt\n";
        exit(EXIT_FAILURE);
    }

    long memory_kb;
    mem_file >> memory_kb;
    mem_file.close();

    // 결과 저장
    result.time_ms = algorithm_time;
    result.memory_mb = memory_kb / 1024.0;

    // 임시 파일 삭제
    fs::remove("temp_mem.txt");
    fs::remove(temp_output_file);
}

void createDirectory(const std::string& path) {
    if (!fs::exists(path)) {
        fs::create_directory(path);
    }
}

void runProgram(const std::string& program, const std::string& input_arg,
                const std::string& csv_file, int run,
                std::vector<double>& times, std::vector<double>& memories) {
    RunResult result{0.0, 0.0};
    execute(program, input_arg, result);

    // 입력 파일 분석
    std::string input_base = input_arg.substr(0, input_arg.find(".txt"));
    size_t last_hyphen = input_base.rfind('-');
    std::string input_type = input_base.substr(0, last_hyphen);
    std::string data_size = input_base.substr(last_hyphen + 1);

    // 실행 결과 출력
    std::cout << "Algorithm: " << program.substr(2)
              << " | Input Type: " << input_type
              << " | Data Size: " << data_size
              << " | Run: " << run
              << " | Time: " << std::fixed << std::setprecision(2) << result.time_ms << "ms"
              << " | Memory: " << std::fixed << std::setprecision(2) << result.memory_mb << "MB\n";

    // 데이터 저장
    times.push_back(result.time_ms);
    memories.push_back(result.memory_mb);
}

void writeAverageToCSV(const std::string& program, const std::string& input_arg,
                       const std::string& csv_file,
                       const std::vector<double>& times, const std::vector<double>& memories) {
    // 입력 파일 분석
    std::string input_base = input_arg.substr(0, input_arg.find(".txt"));
    size_t last_hyphen = input_base.rfind('-');
    std::string input_type = input_base.substr(0, last_hyphen);
    std::string data_size = input_base.substr(last_hyphen + 1);

    // 평균 계산
    double avg_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    double avg_memory = std::accumulate(memories.begin(), memories.end(), 0.0) / memories.size();

    // CSV 기록
    std::ofstream csv(csv_file, std::ios_base::app);
    if (csv.tellp() == 0) {
        csv << "Algorithm,InputType,DataSize,AverageTime(ms),AverageMemory(MB)\n";
    }
    csv << program.substr(2) << ","
        << input_type << ","
        << data_size << ","
        << std::fixed << std::setprecision(2) << avg_time << ","
        << std::fixed << std::setprecision(2) << avg_memory << "\n";
}

int main() {
    // 알고리즘 목록
    std::vector<std::string> progs = {
            // "./bubble_sort",
            // "./tim_sort", 
            // "./cocktail_shaker_sort",
            // "./tournament_sort",
            // "./selection_sort",
            // "./comb_sort",
            // "./insertion_sort",
            // "./introsort",
            // "./quick_sort",
            // "./library_sort",
            // "./heap_sort",
            // "./merge_sort"
    };

    // 입력 파일 목록 (단일 인자)
    std::vector<std::string> input_args = {
        // "ascending-1k.txt",
        // "descending-1k.txt",
        // "random-1k.txt",
        // "part-random-1k.txt", 
        // "ascending-10k.txt",
        // "descending-10k.txt",
        // "random-10k.txt",
        // "part-random-10k.txt", 
        // "ascending-100k.txt",
        // "descending-100k.txt",
        // "random-100k.txt",
        // "part-random-100k.txt", 
        // "ascending-1m.txt",
        // "descending-1m.txt",
        // "random-1m.txt",
        // "part-random-1m.txt"
    };

    const int n = 10;  // 실행 반복 횟수

    // 결과 디렉토리 생성
    std::string output_dir = "results_" + std::to_string(
        std::chrono::system_clock::now().time_since_epoch().count());
    createDirectory(output_dir);

    // CSV 파일 설정
    std::string csv_file = output_dir + "/results.csv";

    for (const auto& prog : progs) {
        for (const auto& input_arg : input_args) {
            std::vector<double> times;
            std::vector<double> memories;
            for (int i = 0; i < n; i++) {
                runProgram(prog, input_arg, csv_file, i+1, times, memories);
                std::cout << "---------------------------------------------------\n";
            }

            writeAverageToCSV(prog, input_arg, csv_file, times, memories);
        }
    }
    return 0;
}
