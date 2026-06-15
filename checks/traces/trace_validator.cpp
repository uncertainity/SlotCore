#include "../../reference/cpp/trace_checker.h"
#include <bits/stdc++.h>
using namespace std;

//g++ checks/traces/trace_validator.cpp reference/cpp/trace_checker.cpp -std=c++17 -o checks/traces/trace_validator


string readFile(const string& path){
  ifstream in(path);
  if (!in){
    throw runtime_error("Could not open file: " + path);
  }
  stringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();

}

struct TraceBlock {
  string label;
  string trace;
};


vector<string> splitReferenceScenarios(const string& text) {
  vector<string> scenarios;

  string current;

  stringstream ss(text);
  string line;

  while (getline(ss, line)) {
      string t = trim(line);

      if (startsWith(t, "Scenario:")) {
          if (!trim(current).empty()) {
              scenarios.push_back(current);
              current.clear();
          }

          // optional: keep scenario header also
          current += line + "\n";
      }
      else {
          current += line + "\n";
      }
  }

  if (!trim(current).empty()) {
      scenarios.push_back(current);
  }

  return scenarios;
}


vector<TraceBlock> splitTestTraces(const string& text) {
  vector<TraceBlock> blocks;

  TraceBlock current;

  stringstream ss(text);
  string line;

  while (getline(ss, line)) {

      string t = trim(line);

      if (startsWith(t, "===")) {

          if (!trim(current.trace).empty()) {
              blocks.push_back(current);
          }

          current = TraceBlock{};
          current.label = t;
          continue;
      }

      current.trace += line + "\n";
  }

  if (!trim(current.trace).empty()) {
      blocks.push_back(current);
  }

  return blocks;
}


int main(){
  string ref_text = readFile("checks/traces/trace_contract.txt");
  string test_text = readFile("checks/traces/test_traces.txt");
  vector<TraceBlock> test_traces = splitTestTraces(test_text);
  vector<string> ref_traces = splitReferenceScenarios(ref_text);
  ofstream report("checks/traces/trace_report.txt");
  report << "Trace Validation Report\n";
  report << "=======================\n\n";
  for(auto& trace_block:test_traces){
    string test_trace = trace_block.trace;
    string test_trace_label = trace_block.label;
    int pass_flag = 0;
    for(auto& ref_trace:ref_traces){
      vector<TraceNode*> ref_roots = parseTrace(ref_trace);
      vector<TraceNode*> test_roots = parseTrace(test_trace);
      if (traceMatches(test_roots, ref_roots)){
        report<<test_trace_label
            << "PASS"
            <<"\n";
        pass_flag = 1;
        break;
      }
    }
    if(pass_flag == 0){
      report<<test_trace_label
            << "FAIL"
            <<"\n";
    }
  }
  return 0;
}