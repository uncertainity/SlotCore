#pragma once

#include <string>
#include <vector>

struct Contract;
struct TraceNode;
struct NodeSignature;

std::vector<TraceNode*> parseTrace(const std::string& text);
bool startsWith(const std::string& line, const std::string& prefix);
std::string trim(const std::string &s);


bool traceMatches(
    const std::vector<TraceNode*>& test_roots,
    const std::vector<TraceNode*>& ref_roots
);