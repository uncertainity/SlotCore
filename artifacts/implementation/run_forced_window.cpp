#include <functional>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

enum Symbol { HV1, HV2, HV3, HV4, HV5, LV1, LV2, LV3, LV4, LV5, LV6, WILD, SCAT, COR, COLLECT };
enum class FeatureChoice { None = 0, FreeSpins = 1, BlitzSpins = 2 };
using PayWindow = vector<vector<Symbol>>;
using CorValueWindow = vector<vector<double>>;

struct ProbTables {
    vector<int> ReelsetWeights;
    vector<int> corTrigger;
    vector<int> numCorWeight;
    vector<int> wildMult;
    vector<int> freeGamenumCollect;
    vector<int> freeGameCOROccur;
    vector<int> goodCORTable;
    vector<int> badCORTable;
    vector<double> CORValues;
    vector<double> baseCORValues;
    vector<int> baseCORWeights;
};

extern ProbTables probTable;

void setTraceEnabled(bool enabled);
void setRngSeed(unsigned int seed);
void clearForcedWindows();
void setForcedBaseWindow(const PayWindow& w, const CorValueWindow& cv = CorValueWindow());
void setForcedFreeWindows(const vector<PayWindow>& windows);
void setForcedBlitzWindows(const vector<PayWindow>& windows, const vector<CorValueWindow>& values);
void setForcedWildMultipliers(const vector<int>& multipliers);
PayWindow additionalCOR(PayWindow pay_window);
int waysWinCalculation(const PayWindow& pay_window);
int waysWinCalculationWithMultiplier(const PayWindow& pay_window);
int resolveCollect(const PayWindow& pay_window, const CorValueWindow& cor_values);
int FreeGameFeature_1(int num_of_free_spins);
int FreeGameFeature_2(int num_of_free_spins);
int runOneSpin(FeatureChoice selectedFeature = FeatureChoice::None);

static const char* TRACE_OUTPUT_PATH = "checks/traces/test_traces.txt";

struct CaseResult {
    string id;
    string category;
    string status = "EXECUTED";
    string observed;
    vector<string> errors;
};

static PayWindow noWinWindow() {
    return {
        {HV1, HV5, LV4, LV5, LV6, LV1},
        {HV2, LV1, LV5, LV6, HV3, HV4},
        {HV3, LV2, LV6, LV5, HV4, HV5},
        {HV4, LV3, LV4, LV6, HV5, LV2}
    };
}

static CorValueWindow zeroValues() {
    return CorValueWindow(4, vector<double>(6, 0.0));
}

static PayWindow blitzWindowA() {
    return {
        {COLLECT, COR, COR, COR, COR, COLLECT},
        {COR, COR, COR, COR, COR, COR},
        {COR, COR, COR, COR, COR, COR},
        {COR, COR, COR, COR, COR, COR}
    };
}

static PayWindow blitzWindowB() {
    return {
        {COLLECT, COR, COR, COR, COR, LV6},
        {COR, COR, COR, COR, COR, COR},
        {COR, COR, COR, COR, COR, COR},
        {COR, COR, COR, COR, COR, COLLECT}
    };
}

static CorValueWindow blitzValuesA() {
    CorValueWindow v(4, vector<double>(6, 0.5));
    v[1][2] = 3.0;
    v[2][4] = 25.0;
    return v;
}

static CorValueWindow blitzValuesB() {
    CorValueWindow v(4, vector<double>(6, 1.0));
    v[0][4] = 10.0;
    v[3][1] = 2.5;
    return v;
}

static vector<PayWindow> repeatedNoWin(int count) {
    vector<PayWindow> windows;
    for (int i = 0; i < count; ++i) windows.push_back(noWinWindow());
    return windows;
}

static string escapeJson(const string& s) {
    string out;
    for (char ch : s) {
        if (ch == '\\') out += "\\\\";
        else if (ch == '"') out += "\\\"";
        else if (ch == '\n') out += "\\n";
        else out += ch;
    }
    return out;
}

static string captureTrace(function<int()> fn, int& value) {
    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    setTraceEnabled(true);
    try {
        value = fn();
        setTraceEnabled(false);
        cout.rdbuf(old);
    } catch (...) {
        setTraceEnabled(false);
        cout.rdbuf(old);
        throw;
    }
    return buffer.str();
}

static void emitTraceBlock(const string& label, const string& trace) {
    ofstream out(TRACE_OUTPUT_PATH, ios::app);
    if (!out) throw runtime_error(string("unable to open trace output: ") + TRACE_OUTPUT_PATH);

    string header = string("=== run_forced_window: ") + label + " ===\n";
    cerr << header << trace;
    if (trace.empty() || trace.back() != '\n') cerr << "\n";
    out << header << trace;
    if (trace.empty() || trace.back() != '\n') out << "\n";
}

static string captureAndEmitTrace(const string& label, function<int()> fn, int& value) {
    string trace = captureTrace(fn, value);
    emitTraceBlock(label, trace);
    return trace;
}

static int countSubstr(const string& text, const string& needle) {
    int count = 0;
    size_t pos = 0;
    while ((pos = text.find(needle, pos)) != string::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}

static int countSymbolInWindow(const PayWindow& w, Symbol symbol) {
    int count = 0;
    for (const auto& row : w) for (Symbol cell : row) if (cell == symbol) ++count;
    return count;
}

static int countProtectedChanges(const PayWindow& before, const PayWindow& after) {
    int changes = 0;
    for (size_t r = 0; r < before.size(); ++r) {
        for (size_t c = 0; c < before[r].size(); ++c) {
            if ((before[r][c] == COLLECT || before[r][c] == WILD || before[r][c] == SCAT) && before[r][c] != after[r][c]) ++changes;
        }
    }
    return changes;
}

static string observedBase(int ways, int collect, int total, int freeCalls, int blitzCalls) {
    return string("{\"ways_win\":") + to_string(ways) +
        ",\"collect_win\":" + to_string(collect) +
        ",\"run_one_spin_total\":" + to_string(total) +
        ",\"free_spins_evaluator_calls\":" + to_string(freeCalls) +
        ",\"blitz_resolve_collect_calls\":" + to_string(blitzCalls) +
        ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
}

template<typename Fn>
static CaseResult runCase(const string& id, const string& category, Fn fn) {
    CaseResult result;
    result.id = id;
    result.category = category;
    try {
        clearForcedWindows();
        setTraceEnabled(false);
        setRngSeed(100 + static_cast<unsigned int>(id.back()));
        result.observed = fn();
    } catch (const exception& e) {
        result.status = "FAILED";
        result.observed = "null";
        result.errors.push_back(e.what());
    }
    clearForcedWindows();
    setTraceEnabled(false);
    return result;
}

static vector<CaseResult> executeCases() {
    vector<CaseResult> results;

    results.push_back(runCase("CASE_EDGE_001", "Base Game / Empty Payout Boundary", [] {
        PayWindow w = noWinWindow();
        int ways = waysWinCalculation(w);
        int collect = resolveCollect(w, zeroValues());
        setForcedBaseWindow(w, zeroValues());
        int total = 0;
        string trace = captureAndEmitTrace("CASE_EDGE_001 runOneSpin none", [] { return runOneSpin(FeatureChoice::None); }, total);
        return observedBase(ways, collect, total, countSubstr(trace, "->waysWinCalculationWithMultiplier"), countSubstr(trace, "->FreeGameFeature_2"));
    }));

    results.push_back(runCase("CASE_EDGE_002", "Base Game / One Below Feature Trigger", [] {
        PayWindow w = {
            {SCAT, HV2, SCAT, HV4, HV5, LV1},
            {LV2, LV3, LV4, LV5, LV6, HV2},
            {LV3, LV4, LV5, LV6, HV3, HV4},
            {LV4, LV5, LV6, HV5, LV1, LV2}
        };
        int ways = waysWinCalculation(w);
        setForcedBaseWindow(w, zeroValues());
        int total = 0;
        string trace = captureAndEmitTrace("CASE_EDGE_002 runOneSpin one below trigger", [] { return runOneSpin(FeatureChoice::FreeSpins); }, total);
        return string("{\"ways_win\":") + to_string(ways) +
            ",\"run_one_spin_total\":" + to_string(total) +
            ",\"free_spins_evaluator_calls\":" + to_string(countSubstr(trace, "->waysWinCalculationWithMultiplier")) +
            ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
    }));

    results.push_back(runCase("CASE_EDGE_003", "Base Game / Exact Trigger With Free Spins Selection", [] {
        PayWindow w = {
            {SCAT, HV1, HV2, HV3, HV4, HV5},
            {LV1, LV2, SCAT, LV3, LV4, LV5},
            {LV2, LV3, LV4, LV5, LV6, HV1},
            {LV3, LV4, LV5, LV6, HV2, SCAT}
        };
        setForcedBaseWindow(w, zeroValues());
        setForcedFreeWindows(repeatedNoWin(40));
        int total = 0;
        string trace = captureAndEmitTrace("CASE_EDGE_003 runOneSpin FreeGameFeature_1", [] { return runOneSpin(FeatureChoice::FreeSpins); }, total);
        return string("{\"run_one_spin_total\":") + to_string(total) +
            ",\"free_spins_evaluator_calls\":" + to_string(countSubstr(trace, "->waysWinCalculationWithMultiplier")) +
            ",\"feature_function_calls\":" + to_string(countSubstr(trace, "->FreeGameFeature_1")) +
            ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
    }));

    results.push_back(runCase("CASE_EDGE_004", "Base Game / Trigger Plus One With Blitz Spins Selection", [] {
        PayWindow w = {
            {SCAT, HV1, HV2, HV3, HV4, SCAT},
            {LV1, LV2, SCAT, LV3, LV4, LV5},
            {LV2, LV3, LV4, LV5, LV6, HV1},
            {LV3, LV4, LV5, LV6, HV2, SCAT}
        };
        vector<PayWindow> blitzWindows(20, blitzWindowA());
        vector<CorValueWindow> blitzValues(20, blitzValuesA());
        setForcedBaseWindow(w, zeroValues());
        setForcedBlitzWindows(blitzWindows, blitzValues);
        int total = 0;
        string trace = captureAndEmitTrace("CASE_EDGE_004 runOneSpin FreeGameFeature_2", [] { return runOneSpin(FeatureChoice::BlitzSpins); }, total);
        return string("{\"run_one_spin_total\":") + to_string(total) +
            ",\"blitz_resolve_collect_calls\":" + to_string(countSubstr(trace, "->resolveCollect")) +
            ",\"feature_function_calls\":" + to_string(countSubstr(trace, "->FreeGameFeature_2")) +
            ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
    }));

    results.push_back(runCase("CASE_EDGE_005", "Base Game / additionalCOR Protected Symbols", [] {
        vector<int> oldTrigger = probTable.corTrigger;
        vector<int> oldNum = probTable.numCorWeight;
        probTable.corTrigger = {1, 0};
        probTable.numCorWeight = {0, 0, 0, 0, 1};
        PayWindow w = {
            {COLLECT, WILD, SCAT, WILD, SCAT, COLLECT},
            {SCAT, WILD, SCAT, WILD, SCAT, WILD},
            {WILD, SCAT, WILD, LV6, WILD, SCAT},
            {SCAT, WILD, SCAT, WILD, SCAT, WILD}
        };
        PayWindow after = additionalCOR(w);
        int protectedChanges = countProtectedChanges(w, after);
        int corCount = countSymbolInWindow(after, COR);
        probTable.corTrigger = oldTrigger;
        probTable.numCorWeight = oldNum;
        return string("{\"protected_position_changes\":") + to_string(protectedChanges) +
            ",\"post_additional_cor_count\":" + to_string(corCount) +
            ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
    }));

    results.push_back(runCase("CASE_EDGE_006", "Ways Evaluation / First Reel Wild Forced Boundary", [] {
        PayWindow w = {
            {WILD, HV2, HV2, HV2, HV2, HV2},
            {LV1, LV3, LV4, LV5, LV6, HV3},
            {LV2, LV4, LV5, LV6, HV4, HV5},
            {LV3, LV5, LV6, HV3, HV4, HV5}
        };
        int ways = waysWinCalculation(w);
        return string("{\"ways_win\":") + to_string(ways) +
            ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
    }));

    results.push_back(runCase("CASE_EDGE_007", "Ways Evaluation / Last Reel Wild Boundary", [] {
        PayWindow w = {
            {HV2, HV2, HV2, HV2, HV2, WILD},
            {LV1, LV3, LV4, LV5, LV6, HV3},
            {LV2, LV4, LV5, LV6, HV4, HV5},
            {LV3, LV5, LV6, HV3, HV4, HV5}
        };
        int ways = waysWinCalculation(w);
        return string("{\"ways_win\":") + to_string(ways) +
            ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
    }));

    results.push_back(runCase("CASE_EDGE_008", "Ways Evaluation / Full Winning Occupancy", [] {
        PayWindow w(4, vector<Symbol>(6, HV1));
        int ways = waysWinCalculation(w);
        return string("{\"ways_win\":") + to_string(ways) +
            ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
    }));

    results.push_back(runCase("CASE_EDGE_009", "Free Spins / No Wild Multiplier", [] {
        PayWindow w = {
            {HV1, HV1, HV1, HV1, HV1, HV1},
            {LV1, LV2, LV3, LV4, LV5, LV6},
            {LV2, LV3, LV4, LV5, LV6, HV2},
            {LV3, LV4, LV5, LV6, HV3, HV4}
        };
        setForcedWildMultipliers({});
        int win = waysWinCalculationWithMultiplier(w);
        return string("{\"free_spins_win\":") + to_string(win) +
            ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
    }));

    results.push_back(runCase("CASE_EDGE_010", "Free Spins / Maximum SCAT Retrigger Boundary", [] {
        PayWindow first = {
            {SCAT, SCAT, SCAT, SCAT, SCAT, SCAT},
            {HV1, LV2, LV3, LV4, LV5, LV6},
            {LV2, LV3, LV4, LV5, LV6, HV2},
            {LV3, LV4, LV5, LV6, HV3, HV4}
        };
        vector<PayWindow> windows;
        windows.push_back(first);
        vector<PayWindow> filler = repeatedNoWin(40);
        windows.insert(windows.end(), filler.begin(), filler.end());
        setForcedFreeWindows(windows);
        int total = 0;
        string trace = captureAndEmitTrace("CASE_EDGE_010 FreeGameFeature_1 retrigger boundary", [] { return FreeGameFeature_1(1); }, total);
        return string("{\"free_spins_session_win\":") + to_string(total) +
            ",\"free_spins_evaluator_calls\":" + to_string(countSubstr(trace, "->waysWinCalculationWithMultiplier")) +
            ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
    }));

    results.push_back(runCase("CASE_EDGE_011", "Blitz Spins / Multi-Spin State Reset", [] {
        setForcedBlitzWindows({blitzWindowA(), blitzWindowB()}, {blitzValuesA(), blitzValuesB()});
        int total = 0;
        string trace = captureAndEmitTrace("CASE_EDGE_011 FreeGameFeature_2 multi-spin", [] { return FreeGameFeature_2(2); }, total);
        return string("{\"blitz_session_win\":") + to_string(total) +
            ",\"resolve_collect_calls\":" + to_string(countSubstr(trace, "->resolveCollect")) +
            ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
    }));

    results.push_back(runCase("CASE_EDGE_012", "Collect / No COR Boundary", [] {
        PayWindow w = {
            {COLLECT, HV1, HV2, HV3, HV4, COLLECT},
            {LV1, LV2, LV3, LV4, LV5, LV6},
            {LV2, LV3, LV4, LV5, LV6, HV2},
            {LV3, LV4, LV5, LV6, HV3, HV4}
        };
        int collect = resolveCollect(w, zeroValues());
        setForcedBaseWindow(w, zeroValues());
        int total = 0;
        captureAndEmitTrace("CASE_EDGE_012 runOneSpin collect no COR", [] { return runOneSpin(FeatureChoice::None); }, total);
        return string("{\"collect_win\":") + to_string(collect) +
            ",\"run_one_spin_total\":" + to_string(total) +
            ",\"final_state\":{\"persistent_state\":\"none_exposed\"}}";
    }));

    return results;
}

int main() {
    setTraceEnabled(false);
    vector<CaseResult> results = executeCases();
    cout << "{\n  \"cases\": [\n";
    for (size_t i = 0; i < results.size(); ++i) {
        const CaseResult& r = results[i];
        cout << "    {\n";
        cout << "      \"case_id\": \"" << r.id << "\",\n";
        cout << "      \"category\": \"" << escapeJson(r.category) << "\",\n";
        cout << "      \"status\": \"" << r.status << "\",\n";
        cout << "      \"observed_result\": " << r.observed << ",\n";
        cout << "      \"errors\": [";
        for (size_t j = 0; j < r.errors.size(); ++j) {
            if (j) cout << ", ";
            cout << "\"" << escapeJson(r.errors[j]) << "\"";
        }
        cout << "]\n";
        cout << "    }" << (i + 1 == results.size() ? "\n" : ",\n");
    }
    cout << "  ]\n}\n";
    return 0;
}
