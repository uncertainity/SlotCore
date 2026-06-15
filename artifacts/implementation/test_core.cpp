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

void setTraceEnabled(bool enabled);
void setRngSeed(unsigned int seed);
void clearForcedWindows();
void setForcedBaseWindow(const PayWindow& w, const CorValueWindow& cv = CorValueWindow());
void setForcedFreeWindows(const vector<PayWindow>& windows);
void setForcedBlitzWindows(const vector<PayWindow>& windows, const vector<CorValueWindow>& values);
void setForcedWildMultipliers(const vector<int>& multipliers);
PayWindow generatePayWindow(vector<Symbol>* Reelset[], vector<int> ReelSize);
PayWindow additionalCOR(PayWindow pay_window);
int waysWinCalculation(const PayWindow& pay_window);
int waysWinCalculationWithMultiplier(const PayWindow& pay_window);
int resolveCollect(const PayWindow& pay_window, const CorValueWindow& cor_values);
int FreeGameFeature_1(int num_of_free_spins);
int FreeGameFeature_2(int num_of_free_spins);
int runOneSpin(FeatureChoice selectedFeature = FeatureChoice::None);

static int failures = 0;
static const char* TRACE_OUTPUT_PATH = "checks/traces/test_traces.txt";

PayWindow filled(Symbol s = COR) { return PayWindow(4, vector<Symbol>(6, s)); }
CorValueWindow values(double v = 0.0) { return CorValueWindow(4, vector<double>(6, v)); }
PayWindow noWinFreeWindow() {
    PayWindow w(4, vector<Symbol>(6, HV1));
    Symbol reelSymbols[6] = {HV1, HV2, HV3, HV4, HV5, LV1};
    for (int r = 0; r < 4; ++r) for (int c = 0; c < 6; ++c) w[r][c] = reelSymbols[c];
    return w;
}

void expectEqual(const string& name, int got, int expected) {
    if (got != expected) {
        cout << "FAIL " << name << ": got " << got << ", expected " << expected << "\n";
        failures++;
    } else {
        cout << "PASS " << name << "\n";
    }
}

void expectTrue(const string& name, bool ok) {
    if (!ok) { cout << "FAIL " << name << "\n"; failures++; }
    else cout << "PASS " << name << "\n";
}

string captureTrace(function<void()> fn) {
    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    setTraceEnabled(true);
    try {
        fn();
        setTraceEnabled(false);
        cout.rdbuf(old);
    } catch (...) {
        setTraceEnabled(false);
        cout.rdbuf(old);
        throw;
    }
    return buffer.str();
}

void emitTraceBlock(const string& label, const string& trace) {
    ofstream out(TRACE_OUTPUT_PATH, ios::app);
    if (!out) throw runtime_error(string("unable to open trace output: ") + TRACE_OUTPUT_PATH);

    string header = string("=== test_core: ") + label + " ===\n";
    cerr << header << trace;
    if (trace.empty() || trace.back() != '\n') cerr << "\n";
    out << header << trace;
    if (trace.empty() || trace.back() != '\n') out << "\n";
}

size_t countSubstr(const string& text, const string& needle) {
    size_t count = 0, pos = 0;
    while ((pos = text.find(needle, pos)) != string::npos) { count++; pos += needle.size(); }
    return count;
}

int main() {
    try {
        setTraceEnabled(false);
        setRngSeed(7);

        PayWindow shape = {
            {HV1, HV2, HV3, HV4, HV5, LV1},
            {LV2, LV3, LV4, LV5, LV6, HV2},
            {LV3, LV4, LV5, LV6, HV3, HV4},
            {LV4, LV5, LV6, HV5, LV1, LV2}
        };
        expectTrue("TC-001 pay window shape", shape.size() == 4 && shape[0].size() == 6);

        PayWindow hv1 = filled();
        for (int c = 0; c < 6; ++c) hv1[0][c] = HV1;
        expectEqual("TC-002 HV1 six of a kind", waysWinCalculation(hv1), 100);

        PayWindow lv1 = filled();
        lv1[0][0] = LV1; lv1[0][1] = LV1; lv1[0][2] = LV1;
        expectEqual("TC-003 LV1 three of a kind", waysWinCalculation(lv1), 10);

        PayWindow multi = filled();
        multi[0][0] = HV2;
        multi[0][1] = HV2; multi[1][1] = HV2;
        multi[0][2] = HV2; multi[1][2] = HV2;
        multi[0][3] = HV2; multi[1][3] = HV2;
        expectEqual("TC-004 multiple ways count", waysWinCalculation(multi), 240);

        PayWindow wild = filled();
        wild[0][0] = HV2; wild[0][1] = WILD; wild[0][2] = WILD; wild[0][3] = HV2;
        expectEqual("TC-005 WILD substitution", waysWinCalculation(wild), 30);

        PayWindow scat2 = filled();
        scat2[0][0] = SCAT; scat2[0][1] = WILD; scat2[0][2] = SCAT;
        expectEqual("TC-006 WILD does not substitute for SCAT", waysWinCalculation(scat2), 0);

        PayWindow scat3 = filled();
        scat3[0][0] = SCAT; scat3[1][2] = SCAT; scat3[3][5] = SCAT;
        expectEqual("TC-007 base SCAT feature scatter win", waysWinCalculation(scat3), 20);
        clearForcedWindows();
        setForcedBaseWindow(scat3, values());
        vector<PayWindow> eightNoWin(8, noWinFreeWindow());
        setForcedFreeWindows(eightNoWin);
        expectEqual("TC-007 runOneSpin free feature trigger path", runOneSpin(FeatureChoice::FreeSpins), 20);

        PayWindow collect1 = filled(LV6);
        CorValueWindow cv1 = values();
        collect1[0][0] = COLLECT;
        collect1[0][1] = COR; cv1[0][1] = 1.0;
        collect1[2][4] = COR; cv1[2][4] = 2.5;
        collect1[1][5] = COR; cv1[1][5] = 10.0;
        expectEqual("TC-008 COLLECT accumulates COR values", resolveCollect(collect1, cv1), 270);

        PayWindow collect2 = filled(LV6);
        CorValueWindow cv2 = values();
        collect2[0][0] = COLLECT; collect2[0][5] = COLLECT;
        collect2[0][1] = COR; cv2[0][1] = 1.0;
        collect2[2][3] = COR; cv2[2][3] = 4.0;
        expectEqual("TC-009 two COLLECT symbols", resolveCollect(collect2, cv2), 200);

        PayWindow fsWild = filled();
        fsWild[0][0] = HV1; fsWild[0][1] = WILD; fsWild[0][2] = HV1; fsWild[0][3] = WILD;
        setForcedWildMultipliers({2, 5});
        expectEqual("TC-010 Free Spins WILD multipliers", waysWinCalculationWithMultiplier(fsWild), 400);

        PayWindow fsRetrigger = noWinFreeWindow();
        fsRetrigger[0][0] = SCAT; fsRetrigger[0][1] = WILD; fsRetrigger[0][2] = SCAT;
        vector<PayWindow> retriggerWindows;
        retriggerWindows.push_back(fsRetrigger);
        for (int i = 0; i < 5; ++i) retriggerWindows.push_back(noWinFreeWindow());
        clearForcedWindows();
        setForcedFreeWindows(retriggerWindows);
        setForcedWildMultipliers({3});
        string retrace = captureTrace([] { FreeGameFeature_1(1); });
        emitTraceBlock("TC-011 Free Spins retrigger adds five spins", retrace);
        expectTrue("TC-011 Free Spins retrigger adds five spins", countSubstr(retrace, "->waysWinCalculationWithMultiplier") == 6);

        PayWindow blitz = filled(COR);
        CorValueWindow bcv = values(0.5);
        blitz[0][0] = COLLECT; blitz[0][5] = COLLECT;
        bcv[0][1] = 0.5; bcv[1][2] = 3.0; bcv[2][4] = 25.0;
        for (int r = 0; r < 4; ++r) for (int c = 0; c < 6; ++c) if (blitz[r][c] == COR && bcv[r][c] == 0.0) bcv[r][c] = 0.5;
        clearForcedWindows();
        setForcedBlitzWindows({blitz}, {bcv});
        expectEqual("TC-012 Blitz collect resolution", FreeGameFeature_2(1), 1520);

        clearForcedWindows();
        setForcedFreeWindows({noWinFreeWindow(), noWinFreeWindow()});
        expectEqual("TC-013 Free Spins completion", FreeGameFeature_1(2), 0);
        clearForcedWindows();
        setForcedBlitzWindows({collect1}, {cv1});
        expectEqual("TC-013 Blitz completion", FreeGameFeature_2(1), 270);

        clearForcedWindows();
        setForcedBaseWindow(noWinFreeWindow(), values());
        string trace = captureTrace([] { runOneSpin(FeatureChoice::None); });
        emitTraceBlock("Trace call order and shapes", trace);
        size_t run = trace.find("->runOneSpin");
        size_t ways = trace.find("->waysWinCalculation");
        size_t collect = trace.find("->resolveCollect");
        size_t out = trace.find("output total_win");
        expectTrue("Trace call order and shapes", run < ways && ways < collect && collect < out && trace.find("shape:[4x6]") != string::npos);
    } catch (const exception& e) {
        cout << "UNCAUGHT " << e.what() << "\n";
        failures++;
    }

    if (failures == 0) {
        cout << "ALL TESTS PASSED\n";
        return 0;
    }
    cout << failures << " TESTS FAILED\n";
    return 1;
}
