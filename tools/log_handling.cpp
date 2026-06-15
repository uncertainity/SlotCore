#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <chrono>

//g++ tools/log_handling.cpp -std=c++17 -o tools/log_handling


namespace fs = std::filesystem;
using namespace std;

template <typename T>
void print_vecs(vector<T>& vec){
    for(size_t t = 0;  t < vec.size(); t++){
        cout<<vec[t]<<"\n";
    }
    //cout<<"\n";
}

vector<string> getSummaryLogFiles(const string& log_dir){
    vector<string>summary_files;
    for(const auto& entry: fs::directory_iterator(log_dir)){
        string filename = entry.path().filename().string();
        cout<<"Filename:"<<filename<<endl;
        if (filename.rfind("summary_",0) == 0){
            summary_files.push_back(filename);
        }
    }
    return summary_files;
}

vector<string> getPipelineLogFiles(const string& log_dir){
    vector<string>pipeline_files;
    for(const auto& entry: fs::directory_iterator(log_dir)){
        string filename = entry.path().filename().string();
        if (filename.rfind("pipeline_",0) == 0){
            pipeline_files.push_back(filename);
        }
    }
    return pipeline_files;
}


vector<string> readSummaryLogs(string log_dir,const vector<string>& summary_files){
    vector<string> all_lines;
    for(const auto& filepath : summary_files){
        ifstream fin(log_dir+ "/" + filepath);
        string line;
        while (getline(fin,line)){
            all_lines.push_back(line);
        }
    }
    //sort(all_lines.begin(),all_lines.end());
    return all_lines;
}

// vector<string> cleanSummaryLogs(const vector<string>& all_lines){
//     vector<string>stack;
//     vector<string>clean_logs;
//     int validation_success = 0;
//     int timestamp_idx = all_lines[0].rfind("]") + 1;
//     int cur_line = 0;
//     int total_lines = all_lines.size();
//     int stack_active = 0;

//     while(cur_line < total_lines){
//         if (all_lines[cur_line].rfind("Running validate_specs.sh", timestamp_idx) == timestamp_idx){
//             if  (all_lines[cur_line+1].rfind("Input validation passed",timestamp_idx) == timestamp_idx) &&
//                 (validation_success == 0){
//                 clean_logs.push_back(all_lines[cur_line]);
//                 clean_logs.push_back(all_lines[cur_line + 1]);
//                 cur_line += 2;
//                 validation_success = 1;
//             }
//             else{
//                 clean_logs.push_back(all_lines[cur_line]);
//                 clean_logs.push_back("Input validation failed");
//                 cur_line ++;
//             }
//         }
//         if(all_lines[cur_line].rfind("Running Stage:",timestamp_idx) == timestamp_idx){
//             if (stack_active == 0){
//                 stack.push_back(all_lines[cur_line]);
//                 stack_active = 1;
//                 cur_line ++;
//             }
//             else{
//                 stack.clear();
//                 stack_active = 0;
//             }
           
//         }
//         if (stack_active == 1){
//             stack.push_back(all_lines[cur_line]);
//             if (all_lines[cur_line].rfind("Finished Stage:",timestamp_idx) == timestamp_idx) && (stack_active == 1){
//                 stack_active = 0;
//                 for (int i = 0; i < stack.size();i++){
//                     clean_logs.push_back(stack[i]);
//                 }
//             }  
//             cur_line ++;
//         }
//     }
// }

string trim(string s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";

    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}


vector<string> cleanSummaryLogs(const vector<string>& all_lines) {
    vector<string> clean_logs;
    vector<string> stage_block;

    bool validation_seen = false;
    bool in_stage = false;

    for (int i = 0; i < (int)all_lines.size(); i++) {
        const string& line = all_lines[i];

        size_t rb = line.find("]");
        if (rb == string::npos) continue;

        string msg = trim(line.substr(rb + 2)); // after "] "

        if (msg == "Running validate_specs.sh") {
            if (!validation_seen &&
                i + 1 < (int)all_lines.size() &&
                all_lines[i + 1].find("Input validation passed") != string::npos) {

                clean_logs.push_back(line);
                clean_logs.push_back(all_lines[i + 1]);
                validation_seen = true;
                i++;
            }
            continue;
        }

        if (msg.rfind("Running stage:", 0) == 0) {
            stage_block.clear();
            stage_block.push_back(line);
            in_stage = true;
            continue;
        }

        if (in_stage) {
            stage_block.push_back(line);

            if (msg.rfind("Finished stage:", 0) == 0) {
                for (const auto& x : stage_block) {
                    clean_logs.push_back(x);
                }
                stage_block.clear();
                in_stage = false;
            }
        }
    }

    return clean_logs;
}


void printTokenCount(const unordered_map<string, long long>& tokens) {
    for (const auto& [stage, count] : tokens) {
        cout << stage << " : " << count << endl;
    }
}


bool isTokenNumber(const string& s) {
    bool has_digit = false;

    for (unsigned char c : s) {
        if (isdigit(c)) {
            has_digit = true;
        } else if (c == ',' || isspace(c)) {
            continue;
        } else {
            return false;
        }
    }

    return has_digit;
}

unordered_map<string, long long>
getTokenCount(const vector<string>& clean_logs) {
    unordered_map<string, long long> tokens;

    vector<string> stages{
        "planner",
        "planner_verifier",
        "simulator",
        "coder",
        "consensus",
        "tester",
        "interactive pass",
        "generate test traces",
        "trace validation"
    };

    for (const auto& stage : stages) {
        tokens[stage] = 0;
    }

    for (const auto& line : clean_logs) {
        size_t rb = line.find(']');
        size_t colon = line.rfind(':');

        if (rb == string::npos || colon == string::npos || colon <= rb)
            continue;

        string stage = line.substr(rb + 2, colon - rb - 2);
        string token_str = line.substr(colon + 1);

        if (!isTokenNumber(token_str))
            continue;

        token_str.erase(
            remove(token_str.begin(), token_str.end(), ','),
            token_str.end()
        );

        long long token_count = stoll(token_str);

        auto it = tokens.find(stage);
        if (it != tokens.end()) {
            it->second += token_count;
        }
    }

    return tokens;
}


string timestampNow() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);

    stringstream ss;
    ss << put_time(localtime(&t), "%Y%m%d_%H%M%S");
    return ss.str();
}

void writeLinesToFile(const string& filepath, const vector<string>& lines) {
    ofstream fout(filepath);

    for (const auto& line : lines) {
        fout << line << '\n';
    }
}

void writeCleanSummary(
    const string& filepath,
    const vector<string>& clean_logs,
    const unordered_map<string, long long>& tokens
) {
    ofstream fout(filepath);

    fout << "Clean Summary Logs\n";
    fout << "==================\n\n";

    for (const auto& line : clean_logs) {
        fout << line << '\n';
    }

    fout << "\nToken Counts\n";
    fout << "============\n";

    for (const auto& [stage, count] : tokens) {
        fout << stage << " : " << count << '\n';
    }
}


void deleteOtherLogFiles(
    const string& log_dir,
    const vector<string>& keep_files
) {
    for (const auto& entry : fs::directory_iterator(log_dir)) {
        if (!entry.is_regular_file())
            continue;

        string path = entry.path().string();

        bool keep = false;
        for (const auto& k : keep_files) {
            if (path == k) {
                keep = true;
                break;
            }
        }

        if (!keep) {
            fs::remove(path);
        }
    }
}


int main(){
    string log_dir = "logs";
    vector <string> summary_files = getSummaryLogFiles(log_dir);
    sort(summary_files.begin(),summary_files.end());
    //print_vecs(summary_files);

    vector <string> pipeline_files = getPipelineLogFiles(log_dir); 
    sort(pipeline_files.begin(),pipeline_files.end());
    //print_vecs(pipeline_files);
    
    vector<string>all_lines = readSummaryLogs(log_dir,summary_files);
    cout<<"Summary Logs:"<<endl;
    //print_vecs(all_lines);

    vector<string>dump_lines = readSummaryLogs(log_dir,pipeline_files);
    cout<<"Pipeline Dumps:"<<endl;
    //print_vecs(dump_lines);

    cout<<"Clean Logs"<<endl;
    vector<string> clean_logs = cleanSummaryLogs(all_lines);
    print_vecs(clean_logs);
    
    unordered_map<string,long long> tokens = getTokenCount(clean_logs);
    printTokenCount(tokens);

    string ts = timestampNow();

    string summary_dump_path = log_dir + "/summary_dump_" + ts + ".log";
    string pipeline_dump_path = log_dir + "/pipeline_dump_" + ts + ".log";
    string clean_summary_path = log_dir + "/clean_summary_" + ts + ".log";

    writeLinesToFile(summary_dump_path, all_lines);
    writeLinesToFile(pipeline_dump_path, dump_lines);
    writeCleanSummary(clean_summary_path, clean_logs, tokens);

    deleteOtherLogFiles(
        log_dir,
        {summary_dump_path, pipeline_dump_path, clean_summary_path}
    );

    return 0;
}
