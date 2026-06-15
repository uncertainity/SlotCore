#include <algorithm>
#include <array>
#include <cxxabi.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

using namespace std;

enum Symbol { HV1, HV2, HV3, HV4, HV5, LV1, LV2, LV3, LV4, LV5, LV6, WILD, SCAT, COR, COLLECT };
enum class FeatureChoice { None = 0, FreeSpins = 1, BlitzSpins = 2 };

constexpr int NO_OF_REELS = 6;
constexpr int NO_OF_ROWS = 4;
constexpr int STANDARD_SYMBOL_COUNT = 11;
constexpr int PAYTABLE_SYMBOL_COUNT = 12;
constexpr int BASE_BET_FIXED_COINS = 20;

using PayWindow = vector<vector<Symbol>>;
using CorValueWindow = vector<vector<double>>;

std::mt19937 rng(1);

static bool trace_enabled = true;
void setTraceEnabled(bool enabled) { trace_enabled = enabled; }
void setRngSeed(unsigned int seed) { rng.seed(seed); }

template<typename T>
string type_name() {
    int status = 0;
    unique_ptr<char, void(*)(void*)> res{abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status), free};
    return status == 0 ? res.get() : typeid(T).name();
}

template<typename T> string shape_of(const T&) { return "[1]"; }
template<typename T> string shape_of(const vector<T>& v) { return "[" + to_string(v.size()) + "]"; }
template<typename T> string shape_of(const vector<vector<T>>& v) { return "[" + to_string(v.size()) + "x" + to_string(v.empty() ? 0 : v[0].size()) + "]"; }
template<typename T, size_t N> string shape_of(T* const (&)[N]) { return "[" + to_string(N) + "][variable]"; }
template<typename T, size_t N> string shape_of(const T (&)[N]) { return "[" + to_string(N) + "]"; }
template<typename T, size_t R, size_t C> string shape_of(const T (&)[R][C]) { return "[" + to_string(R) + "x" + to_string(C) + "]"; }

struct Trace {
    static inline int depth = 0;
    string fn;
    explicit Trace(const char* function_name) : fn(function_name) {
        if (trace_enabled) cout << string(depth * 2, ' ') << "->" << fn << "\n";
        depth++;
    }
    template<typename T> void input(const string& name, const T& value) {
        if (trace_enabled) cout << string(depth * 2, ' ') << "input " << name << ": " << type_name<T>() << " shape:" << shape_of(value) << "\n";
    }
    template<typename T> void output(const string& name, const T& value) {
        if (trace_enabled) cout << string(depth * 2, ' ') << "output " << name << ": " << type_name<T>() << " shape:" << shape_of(value) << "\n";
    }
    template<typename T> void GlobalandStruct(const string& name, const T& value) {
        if (trace_enabled) cout << string(depth * 2, ' ') << "Global/Struct " << name << ": " << type_name<T>() << " shape:" << shape_of(value) << "\n";
    }
    ~Trace() {
        depth--;
        if (trace_enabled) cout << string(depth * 2, ' ') << "<- " << fn << "\n";
    }
};

const Symbol StandardSymbols[STANDARD_SYMBOL_COUNT] = {HV1, HV2, HV3, HV4, HV5, LV1, LV2, LV3, LV4, LV5, LV6};
int PayTable[PAYTABLE_SYMBOL_COUNT][7] = {
    {0,0,20,30,40,50,100}, {0,0,10,20,30,40,50}, {0,0,10,20,30,40,50},
    {0,0,4,16,24,30,40}, {0,0,4,16,24,30,40}, {0,0,0,10,20,24,30},
    {0,0,0,10,20,24,30}, {0,0,0,8,16,20,24}, {0,0,0,8,16,20,24},
    {0,0,0,6,10,16,20}, {0,0,0,6,10,16,20}, {0,0,0,20,40,60,100}
};

struct ProbTables {
    vector<int> ReelsetWeights{58, 42};
    vector<int> corTrigger{1, 9};
    vector<int> numCorWeight{240, 200, 150, 100, 80};
    vector<int> wildMult{480, 545, 360};
    vector<int> freeGamenumCollect{75, 25};
    vector<int> freeGameCOROccur{26, 74};
    vector<int> goodCORTable{0,0,0,0,0,0,0,606,893,600,600,500,500,500,500,400,300,100,30,10,10,3};
    vector<int> badCORTable{17500,7000,3000,2500,2000,2000,1500,1000,1000,500,500,100,100,50,50,50,30,20,10,0,0,0};
    vector<double> CORValues{0.5,1,1.5,2,2.5,3,3.5,4,5,6,7,7.5,8,9,10,12.5,25,50,125,250,500,2000};
    vector<double> baseCORValues{0.5,1,1.5,2,2.5,3,3.5,4,5,6,7,7.5,8,9,10,12.5,25,50,125,250,500,2000};
    vector<int> baseCORWeights{7000,6000,6000,5000,3000,3000,2500,2000,1000,1000,1000,100,100,50,50,50,30,20,10,10,5,1};
} probTable;

std::vector<Symbol> BG1_reel1 = {LV2, LV4, LV3, HV2, HV1, LV2, LV4, LV2, LV3, LV4, LV6, COLLECT, LV2, LV3, HV3, LV2, LV3, HV1, LV4, LV2, LV2, HV2, LV4, HV1, LV2, LV4, LV3, LV4, SCAT, LV2, HV4, LV3, HV5, LV4, LV5, HV5, LV3, HV1, LV6, LV2, LV4, LV3, LV4, LV4, HV1, LV2, HV3, LV3, LV2, HV1, LV3, LV4, LV3, LV4, HV1, LV2, HV4, LV2, LV3, LV4, HV1, LV2, LV3, LV2, LV4, LV3, HV1, LV4, LV2, SCAT, LV3, LV4, LV6, LV3, LV2, LV3, HV1, HV1, HV1, HV2, LV3, LV4, LV2, LV5, HV1, LV1, LV2, COR, COR};
std::vector<Symbol> BG1_reel2 = {HV5, LV1, LV6, HV1, HV1, HV1, LV5, HV5, HV5, LV3, LV1, COR, COR, HV3, LV1, LV3, HV2, HV3, LV4, HV5, LV5, LV1, HV5, HV2, LV1, HV5, LV5, WILD, LV1, HV5, HV5, LV5, LV1, HV4, HV5, LV5, LV1, HV5, LV2, HV5, LV5, SCAT, LV1, LV5, LV5, HV5, LV5, WILD, LV5, LV1, HV5, LV5, LV5, LV1, LV5, HV4, LV5, HV5, LV1, HV5, HV5, LV1, LV5, LV1, HV3, HV2, LV1, LV5, LV1, LV5, HV5, LV1, HV1, LV1, LV5, LV1, HV5, LV5, LV5, LV1, LV5, HV5};
std::vector<Symbol> BG1_reel3 = {LV4, HV2, HV4, LV4, LV4, HV2, LV5, LV5, HV3, LV4, LV6, LV5, LV5, LV3, SCAT, LV4, LV5, HV1, HV4, LV5, LV6, LV5, LV5, HV4, HV1, SCAT, LV5, LV4, HV3, LV4, HV4, LV5, LV4, HV4, LV5, HV4, LV5, HV4, HV4, LV3, LV5, LV5, HV4, LV5, WILD, LV4, HV4, LV5, HV4, LV6, LV1, LV5, LV5, HV4, HV4, HV2, HV4, LV3, LV3, HV1, LV2, HV4, HV5, LV6, LV4, HV2, HV4, LV4, LV4, HV2, LV5, LV5, SCAT, HV3, LV4, LV6, LV5, LV5, LV3, HV1, HV1, HV1, HV4, LV5, SCAT, LV5, LV5, HV4, HV1, LV5, LV4, HV3, LV4, HV4, LV5, WILD, HV4, LV5, HV4, LV5, HV4, HV4, LV3, LV5, LV5, HV4, LV5, WILD, LV4, HV4, LV5, HV4, LV6, LV1, LV5, LV5, HV4, HV4, HV2, HV4, LV3, LV3, HV1, LV2, HV4, HV5, LV6, COR, COR, COR};
std::vector<Symbol> BG1_reel4 = {HV1, HV1, HV1, LV2, LV5, LV2, LV3, LV5, LV1, WILD, LV5, LV2, LV5, LV3, LV6, LV6, LV5, LV3, LV5, LV2, HV1, LV6, LV3, LV5, HV1, HV5, HV4, HV3, HV3, SCAT, HV3, HV3, LV5, LV5, HV2, LV5, LV5, HV3, LV1, LV5, LV1, LV4, HV1, LV1, LV1, LV5, LV1, WILD, LV5, LV1, LV1, LV5, HV3, LV1, LV1, HV1, LV5, HV2, LV2, HV3, LV3, COR, COR, COR, LV1};
std::vector<Symbol> BG1_reel5 = {LV2, LV2, HV5, HV3, HV3, LV3, LV3, LV1, HV4, HV3, LV5, LV5, LV5, WILD, HV2, HV2, HV3, LV1, LV1, LV4, HV5, LV6, HV1, LV1, LV1, LV3, LV2, HV4, HV5, LV1, SCAT, LV5, LV5, HV1, HV4, HV4, LV5, HV4, HV3, LV1, LV1, HV5, HV4, HV4, HV5, HV3, LV3, LV5, HV3, HV1, LV1, HV5, HV2, HV3, LV1, LV4, HV1, HV1, HV1, LV4, HV2, LV2, LV4, HV5, LV3, HV1, WILD, LV2, LV2, HV5, LV3, HV2, HV2, LV5, LV5, HV4, HV3, HV5, LV2, LV2, COR, COR, COR};
std::vector<Symbol> BG1_reel6 = {COR, LV3, COLLECT, HV4, HV3, HV5, LV5, COR, COR, HV2, HV2, LV5, LV3, COLLECT, HV3, LV3, LV5, HV3, HV1, HV5, HV4, HV3, SCAT, HV4, LV1, HV5, HV4, HV4, LV1, SCAT, LV1, HV4, HV5, LV2, HV3, HV3, HV5, LV5, HV1, HV1, HV1, LV2, HV3, LV1, LV5, LV6, LV1, LV4, WILD, HV3, HV4, LV1, LV5, HV1, LV1, HV4, HV1, LV1, LV1, HV3, HV3, HV3, LV1, HV1, LV1, LV5, HV2, LV4, LV1, HV5, HV1, LV6, LV2, LV2, LV2, HV3, HV2, LV1, WILD, LV3, LV2, HV5};
std::vector<Symbol>* BG1_Reels[NO_OF_REELS] = {&BG1_reel1, &BG1_reel2, &BG1_reel3, &BG1_reel4, &BG1_reel5, &BG1_reel6};
std::vector<int> BG1_ReelSize = {89, 82, 130, 65, 83, 82};

std::vector<Symbol> BG2_reel1 = {HV1, LV1, LV2, LV4, LV1, HV3, LV6, LV6, HV3, LV4, HV1, LV6, HV5, HV1, LV1, LV4, LV1, HV3, LV3, LV6, LV6, HV4, HV3, LV4, HV1, LV5};
std::vector<Symbol> BG2_reel2 = {HV2, LV3, LV2, LV2, LV5, HV2, HV4, HV5, LV3, LV5, HV4, HV5, LV1, LV4, HV2, LV3, LV2, LV2, LV5, LV6, HV2, HV4, HV5, LV3, LV5};
std::vector<Symbol> BG2_reel3 = {LV1, LV2, LV4, LV1, HV3, LV6, LV6, HV3, LV4, HV1, LV6, HV5, HV1, LV1, HV2, LV4, LV1, HV3, LV3, LV6, LV6, HV4, HV3, LV4, HV1, LV5};
std::vector<Symbol> BG2_reel4 = {HV2, LV3, LV2, LV2, LV5, HV2, HV4, HV5, LV3, LV5, HV4, HV5, HV1, LV1, LV4, HV2, LV3, LV2, HV3, LV2, LV5, LV6, HV2, HV4, HV5, LV3, LV5};
std::vector<Symbol> BG2_reel5 = {HV1, LV1, LV2, LV4, LV1, HV3, LV6, LV6, HV3, LV4, HV1, LV6, HV5, HV1, LV1, HV2, LV4, LV1, HV3, LV3, LV6, LV6, HV4, HV3, LV4, HV1, LV5};
std::vector<Symbol> BG2_reel6 = {HV2, LV3, LV2, LV2, LV5, HV2, HV4, HV5, LV3, LV5, HV4, HV5, HV1, LV1, LV4, HV2, LV3, LV2, HV3, LV2, LV5, LV6, HV2, HV4, HV5, LV3, LV5};
std::vector<Symbol>* BG2_Reels[NO_OF_REELS] = {&BG2_reel1, &BG2_reel2, &BG2_reel3, &BG2_reel4, &BG2_reel5, &BG2_reel6};
std::vector<int> BG2_ReelSize = {26, 25, 26, 27, 27, 27};

std::vector<Symbol> FG1_reel1 = {HV2, LV2, LV3, HV3, HV1, LV2, HV4, HV3, LV4, LV2, LV3, HV3, LV4, HV2, LV2, LV4, LV3, HV3, HV2, LV2, LV1, LV6, LV3, LV4, HV1, HV2, HV3, HV4, LV5, LV2, LV4, LV3, HV1, HV2, HV5, LV2, LV4, LV3, LV2, LV6, HV2, LV4, LV3, LV4, HV1, LV2, HV2, LV1, LV3, LV4, HV1, LV2, LV3, HV5, LV2, LV3, HV2, LV4, LV2, LV6, LV3, LV4, LV2, HV3, LV2, LV3, LV6, LV4, LV3, HV2, LV4, LV4, HV4, LV3, LV2, LV3, LV6, LV4, LV3, HV5, LV3, LV4, LV2, LV4, LV3, HV2, HV1, LV1, LV4, LV2, LV3, LV4, HV2, LV2, LV2, HV3, LV4, LV2, LV3, HV3, LV4, LV2, HV4, LV3, LV4, HV2, LV2, LV4, LV3, LV4, LV6, LV2, LV1, LV3, LV3, LV4, LV2, LV5, HV1, LV6, LV5, LV2, LV4, LV3, LV4, HV1, HV2, LV4, LV4, LV2, LV3, LV3, HV1, LV4, LV2, SCAT, LV3, LV4, HV1, HV3, LV2, LV3, HV1, SCAT};
std::vector<Symbol> FG1_reel2 = {LV1, LV6, LV1, LV5, SCAT, LV1, LV4, HV1, LV1, LV4, LV1, WILD, LV5, LV1, LV4, LV2, LV4, HV2, HV4, HV2, LV6, LV4, HV4, HV1, HV5, LV3, LV3, LV1, LV5, WILD, LV4, LV1, LV5, HV1, LV6, HV5, SCAT, HV1, LV1, HV3, HV2, LV1, LV1, HV1, LV1, HV5, LV1, HV4, HV2, LV1, HV5, HV2, HV1, HV3, HV4, HV2, LV5, LV5, HV4, HV2, HV1, LV4, HV4, LV4, HV5, LV3, HV1, LV1, LV5, LV3, WILD, LV1, HV4, LV5, HV1, HV5, HV4, HV1, SCAT, HV3, HV2, LV1, LV1, HV1, LV1, HV5, LV1, HV4, LV4, LV1, HV5};
std::vector<Symbol> FG1_reel3 = {LV4, HV5, LV3, LV4, LV6, WILD, LV2, LV3, LV4, HV4, HV1, LV4, HV5, SCAT, HV3, HV2, LV4, LV6, HV4, HV4, HV4, LV5, HV1, HV2, LV5, LV4, LV5, LV4, LV6, LV5, LV3, HV4, LV5, HV4, LV3, HV3, HV4, LV2, HV1, LV5, LV3, LV5, HV4, LV5, HV2, HV1, HV4, LV4, HV3, LV4, LV6, LV2, SCAT, HV4, LV2, LV3, LV5, WILD, LV6, LV2, LV5, HV4, LV1, HV3, LV4, HV5, HV4, LV4, HV2, LV4, HV4, HV1, LV4, HV5, HV2, HV3, HV2, LV4, HV1, LV6, LV2, LV5, WILD, LV1, LV4, LV6, LV5, HV1, HV2, HV4, LV5, HV4, LV3, HV3, HV4, LV2, HV1, HV1, HV4, LV4, HV5};
std::vector<Symbol> FG1_reel4 = {HV1, HV3, LV6, LV5, HV1, HV5, LV5, SCAT, HV1, HV2, HV4, HV3, LV5, HV5, HV4, HV2, LV6, LV5, LV4, WILD, LV3, LV1, LV4, LV1, LV1, SCAT, HV4, LV1, LV4, HV1, HV5, LV1, HV2, LV2, LV3, LV6, WILD, LV2, LV1, LV5, HV1, LV1, HV3, HV4, LV5, HV2, LV4, HV1, HV3, HV4, HV5, LV1, LV5, LV4, LV6, WILD, LV1, LV4, LV5, LV6};
std::vector<Symbol> FG1_reel5 = {LV2, HV2, HV5, HV3, LV3, HV1, LV1, HV4, SCAT, LV5, LV4, HV1, HV3, HV2, HV2, HV1, LV1, LV2, LV4, HV5, HV1, HV5, HV4, HV3, LV5, HV1, SCAT, HV2, LV1, HV5, HV1, HV4, HV2, HV3, LV3, LV5, HV3, HV1, LV1, HV5, HV2, HV3, LV1, LV4, LV6, LV2, HV1, LV4, HV2, LV2, LV4, HV5, LV3, HV4, HV1, LV2, LV2, HV5, LV3, HV2, HV2, LV5, LV5, HV4, HV1, LV5, LV5, LV5, LV4, HV3, LV3, LV3, HV4, LV2, HV1, HV1, LV2, LV3, HV3, LV3, HV2, HV5, LV3, SCAT, HV2, HV4, HV1, HV5, HV2, HV5, HV1, HV2, LV3, LV5, LV5, HV3, HV1, HV3, LV3, HV1, HV2, LV3, LV1, HV4, HV3, HV1, HV3, HV3, LV2, HV1, LV6, LV1, HV5, HV4, LV5, LV3, HV1, HV3, HV2, HV1, LV3, HV5, LV5, HV4, LV5, HV4, LV5, LV3, HV1, HV2, LV3, HV3, HV3, LV2, HV1, HV5, HV2, HV3, HV4, HV5, LV1, HV1, HV2, HV1, LV5, HV3, LV2, LV6, HV4, LV5, HV2, HV5, HV1, HV4, HV2, HV1, HV1, HV3, LV5, LV2, LV5, LV3, HV1, HV2, LV3, LV5, HV3};
std::vector<Symbol> FG1_reel6 = {HV4, LV3, HV2, LV1, HV3, HV5, LV5, HV1, HV3, HV2, HV2, LV5, LV3, LV6, HV3, LV3, LV5, HV3, HV1, HV5, HV4, HV3, LV1, HV2, LV1, HV5, HV3, HV4, LV1, LV6, LV1, HV4, HV5, SCAT, HV3, HV5, LV5, HV1, HV2, HV4, HV3, LV1, LV5, HV1, LV1, LV4, HV2, HV3, HV4, LV1, LV5, HV1, LV1, HV4, LV6, LV1, LV1, HV3, HV3, HV3, LV1, HV1, LV1, LV5, HV2, LV4, LV1, HV5, HV1, HV5, LV1, LV1, LV1, HV3, HV2, LV1, LV1, LV1, HV1, HV5, LV5, HV4, LV4, HV5, LV1, LV1, HV1, HV3, LV5, HV3, HV1, HV5, LV5, HV3, HV5, LV5, HV1, HV5, HV3, LV1, LV6, HV3, HV5, LV1, HV3, HV4, HV5, LV1, HV4, LV1, HV5, HV4, HV4, LV1, LV1, LV4, HV4, HV5, SCAT, HV3, HV4, LV6, HV2, HV3, HV4, HV5, HV3, HV5, HV4, HV2, HV3, HV5, HV4, HV3, HV1, HV1, HV5, LV1, HV3, HV5, HV5, HV4, HV3, LV1, HV4, HV3, LV2, LV2, HV4, LV4, HV5, LV1, HV4, HV3, LV2, HV4, HV3, HV5, HV3, HV2, LV2, HV5, HV4, SCAT, HV3, HV5, HV5, HV4, LV5, HV2, HV1};
std::vector<Symbol>* FG1_Reels[NO_OF_REELS] = {&FG1_reel1, &FG1_reel2, &FG1_reel3, &FG1_reel4, &FG1_reel5, &FG1_reel6};
std::vector<int> FG1_ReelSize = {144, 91, 101, 60, 167, 171};


static CorValueWindow last_cor_values(NO_OF_ROWS, vector<double>(NO_OF_REELS, 0.0));
static bool forced_base_window_active = false;
static PayWindow forced_base_window;
static CorValueWindow forced_base_cor_values;
static vector<PayWindow> forced_free_windows;
static vector<PayWindow> forced_blitz_windows;
static vector<CorValueWindow> forced_blitz_cor_values;
static vector<int> forced_wild_multipliers;

void clearForcedWindows() {
    forced_base_window_active = false;
    forced_base_window.clear();
    forced_base_cor_values.clear();
    forced_free_windows.clear();
    forced_blitz_windows.clear();
    forced_blitz_cor_values.clear();
    forced_wild_multipliers.clear();
}
void setForcedBaseWindow(const PayWindow& w, const CorValueWindow& cv = CorValueWindow()) { forced_base_window_active = true; forced_base_window = w; forced_base_cor_values = cv; }
void setForcedFreeWindows(const vector<PayWindow>& windows) { forced_free_windows = windows; }
void setForcedBlitzWindows(const vector<PayWindow>& windows, const vector<CorValueWindow>& values) { forced_blitz_windows = windows; forced_blitz_cor_values = values; }
void setForcedWildMultipliers(const vector<int>& multipliers) { forced_wild_multipliers = multipliers; }

static void validateWindowShape(const PayWindow& w) {
    if (w.size() != NO_OF_ROWS) throw runtime_error("pay_window row count is not 4");
    for (const auto& row : w) if (row.size() != NO_OF_REELS) throw runtime_error("pay_window reel count is not 6");
}
static int weightedIndex(const vector<int>& weights) {
    int total = accumulate(weights.begin(), weights.end(), 0);
    uniform_int_distribution<int> dist(1, total);
    int pick = dist(rng), running = 0;
    for (size_t i = 0; i < weights.size(); ++i) { running += weights[i]; if (pick <= running) return static_cast<int>(i); }
    return static_cast<int>(weights.size() - 1);
}
static double weightedValue(const vector<double>& values, const vector<int>& weights) { return values.at(weightedIndex(weights)); }
static bool inCorDomain(double v) { return find(probTable.CORValues.begin(), probTable.CORValues.end(), v) != probTable.CORValues.end(); }
static int countSymbol(const PayWindow& w, Symbol s) { int c = 0; for (const auto& row : w) for (Symbol cell : row) if (cell == s) c++; return c; }
static void assignMissingBaseCorValues(const PayWindow& w) {
    for (int r = 0; r < NO_OF_ROWS; ++r) for (int c = 0; c < NO_OF_REELS; ++c) {
        if (w[r][c] == COR && last_cor_values[r][c] == 0.0) last_cor_values[r][c] = weightedValue(probTable.baseCORValues, probTable.baseCORWeights);
    }
}
static PayWindow emptyWindow(Symbol fill = LV6) { return PayWindow(NO_OF_ROWS, vector<Symbol>(NO_OF_REELS, fill)); }

PayWindow generatePayWindow(vector<Symbol>* Reelset[], vector<int> ReelSize) {
    Trace trace("generatePayWindow");
    trace.input("Reelset", Reelset);
    trace.input("ReelSize", ReelSize);
    trace.GlobalandStruct("NO_OF_ROWS", NO_OF_ROWS);
    trace.GlobalandStruct("NO_OF_REELS", NO_OF_REELS);
    PayWindow pay_window(NO_OF_ROWS, vector<Symbol>(NO_OF_REELS));
    for (int reel = 0; reel < NO_OF_REELS; ++reel) {
        uniform_int_distribution<int> dist(0, ReelSize[reel] - 1);
        int start_idx = dist(rng);
        for (int row = 0; row < NO_OF_ROWS; ++row) pay_window[row][reel] = (*Reelset[reel])[(start_idx + row) % ReelSize[reel]];
    }
    trace.output("pay_window", pay_window);
    return pay_window;
}

PayWindow additionalCOR(PayWindow pay_window) {
    Trace trace("additionalCOR");
    trace.input("pay_window", pay_window);
    trace.GlobalandStruct("probTable.corTrigger", probTable.corTrigger);
    trace.GlobalandStruct("probTable.numCorWeight", probTable.numCorWeight);
    trace.GlobalandStruct("probTable.baseCORValueWeights.values", probTable.baseCORValues);
    trace.GlobalandStruct("probTable.baseCORValueWeights.weights", probTable.baseCORWeights);
    validateWindowShape(pay_window);
    last_cor_values.assign(NO_OF_ROWS, vector<double>(NO_OF_REELS, 0.0));
    assignMissingBaseCorValues(pay_window);
    if (weightedIndex(probTable.corTrigger) == 1) { trace.output("pay_window", pay_window); return pay_window; }
    int number_to_add = weightedIndex(probTable.numCorWeight) + 1;
    vector<pair<int,int>> candidates;
    for (int r = 0; r < NO_OF_ROWS; ++r) for (int c = 0; c < NO_OF_REELS; ++c) {
        Symbol s = pay_window[r][c];
        if (s != COLLECT && s != WILD && s != SCAT) candidates.push_back({r,c});
    }
    shuffle(candidates.begin(), candidates.end(), rng);
    for (int i = 0; i < number_to_add && i < static_cast<int>(candidates.size()); ++i) {
        auto [r,c] = candidates[i];
        pay_window[r][c] = COR;
        last_cor_values[r][c] = weightedValue(probTable.baseCORValues, probTable.baseCORWeights);
    }
    trace.output("pay_window", pay_window);
    return pay_window;
}

int waysWinCalculation(const PayWindow& pay_window) {
    Trace trace("waysWinCalculation");
    trace.input("pay_window", pay_window);
    trace.GlobalandStruct("PayTable", PayTable);
    trace.GlobalandStruct("STANDARD_SYMBOL_COUNT", STANDARD_SYMBOL_COUNT);
    trace.GlobalandStruct("PAYTABLE_SYMBOL_COUNT", PAYTABLE_SYMBOL_COUNT);
    validateWindowShape(pay_window);
    int win = 0;
    for (int sym_idx = 0; sym_idx < STANDARD_SYMBOL_COUNT; ++sym_idx) {
        Symbol target = StandardSymbols[sym_idx];
        int ways = 1, consecutive_reels = 0;
        for (int reel = 0; reel < NO_OF_REELS; ++reel) {
            int count = 0;
            for (int row = 0; row < NO_OF_ROWS; ++row) if (pay_window[row][reel] == target || pay_window[row][reel] == WILD) count++;
            if (count == 0) break;
            ways *= count;
            consecutive_reels++;
        }
        win += PayTable[sym_idx][consecutive_reels] * ways;
    }
    int scat_count = countSymbol(pay_window, SCAT);
    if (scat_count < 0 || scat_count > NO_OF_REELS) throw runtime_error("invalid SCAT count");
    win += PayTable[11][scat_count];
    trace.output("win", win);
    return win;
}

int waysWinCalculationWithMultiplier(const PayWindow& pay_window) {
    Trace trace("waysWinCalculationWithMultiplier");
    trace.input("pay_window", pay_window);
    trace.GlobalandStruct("probTable.wildMult", probTable.wildMult);
    trace.GlobalandStruct("PayTable", PayTable);
    trace.GlobalandStruct("STANDARD_SYMBOL_COUNT", STANDARD_SYMBOL_COUNT);
    trace.GlobalandStruct("PAYTABLE_SYMBOL_COUNT", PAYTABLE_SYMBOL_COUNT);
    validateWindowShape(pay_window);
    int standard_win = 0;
    for (int sym_idx = 0; sym_idx < STANDARD_SYMBOL_COUNT; ++sym_idx) {
        Symbol target = StandardSymbols[sym_idx];
        int ways = 1, consecutive_reels = 0;
        for (int reel = 0; reel < NO_OF_REELS; ++reel) {
            int count = 0;
            for (int row = 0; row < NO_OF_ROWS; ++row) if (pay_window[row][reel] == target || pay_window[row][reel] == WILD) count++;
            if (count == 0) break;
            ways *= count;
            consecutive_reels++;
        }
        standard_win += PayTable[sym_idx][consecutive_reels] * ways;
    }
    int total_wild_multiplier = 1;
    size_t forced_idx = 0;
    for (const auto& row : pay_window) for (Symbol s : row) if (s == WILD) {
        int picked_mult = weightedIndex(probTable.wildMult);
        int mult = forced_idx < forced_wild_multipliers.size() ? forced_wild_multipliers[forced_idx++] : (picked_mult == 0 ? 2 : picked_mult == 1 ? 3 : 5);
        total_wild_multiplier *= mult;
    }
    int scat_count = countSymbol(pay_window, SCAT);
    int scatter_win = PayTable[11][scat_count];
    int win = standard_win * total_wild_multiplier + scatter_win;
    trace.output("win", win);
    return win;
}

int resolveCollect(const PayWindow& pay_window, const CorValueWindow& cor_values) {
    Trace trace("resolveCollect");
    trace.input("pay_window", pay_window);
    trace.input("cor_values", cor_values);
    trace.GlobalandStruct("BASE_BET_FIXED_COINS", BASE_BET_FIXED_COINS);
    trace.GlobalandStruct("probTable.CORValues", probTable.CORValues);
    validateWindowShape(pay_window);
    if (cor_values.size() != NO_OF_ROWS || cor_values[0].size() != NO_OF_REELS) throw runtime_error("cor_values shape is not 4x6");
    vector<pair<int,int>> collects;
    vector<double> cor_values_visible;
    for (int r = 0; r < NO_OF_ROWS; ++r) for (int c = 0; c < NO_OF_REELS; ++c) {
        if (pay_window[r][c] == COLLECT) {
            if (c != 0 && c != 5) throw runtime_error("COLLECT on invalid reel");
            collects.push_back({r,c});
        }
        if (pay_window[r][c] == COR) {
            if (!inCorDomain(cor_values[r][c])) throw runtime_error("COR value outside CORValues domain");
            cor_values_visible.push_back(cor_values[r][c]);
        }
    }
    double collect_win = 0.0;
    for (size_t i = 0; i < collects.size(); ++i) for (double v : cor_values_visible) collect_win += v * BASE_BET_FIXED_COINS;
    int rounded = static_cast<int>(collect_win + 0.5);
    trace.output("collect_win", rounded);
    return rounded;
}

static int retriggerAward(int scat_count) { static int awards[7] = {0,0,5,8,12,20,30}; return scat_count >= 2 && scat_count <= 6 ? awards[scat_count] : 0; }

int FreeGameFeature_1(int num_of_free_spins) {
    Trace trace("FreeGameFeature_1");
    trace.input("num_of_free_spins", num_of_free_spins);
    trace.GlobalandStruct("FG1_Reels", FG1_Reels);
    trace.GlobalandStruct("FG1_ReelSize", FG1_ReelSize);
    trace.GlobalandStruct("probTable.wildMult", probTable.wildMult);
    int spins_remaining = num_of_free_spins, session_win = 0, forced_idx = 0;
    while (spins_remaining > 0) {
        spins_remaining--;
        PayWindow pay_window = forced_idx < static_cast<int>(forced_free_windows.size()) ? forced_free_windows[forced_idx++] : generatePayWindow(FG1_Reels, FG1_ReelSize);
        for (const auto& row : pay_window) for (Symbol s : row) if (s == COR || s == COLLECT) throw runtime_error("COR/COLLECT in Free Spins window");
        session_win += waysWinCalculationWithMultiplier(pay_window);
        spins_remaining += retriggerAward(countSymbol(pay_window, SCAT));
    }
    trace.output("session_win", session_win);
    return session_win;
}

static PayWindow buildRandomBlitzWindow(CorValueWindow& cor_values) {
    PayWindow w = emptyWindow(COR);
    cor_values.assign(NO_OF_ROWS, vector<double>(NO_OF_REELS, 0.0));
    for (int r = 0; r < NO_OF_ROWS; ++r) for (int c = 0; c < NO_OF_REELS; ++c) w[r][c] = LV6;
    int collect_count = weightedIndex(probTable.freeGamenumCollect) + 1;
    vector<pair<int,int>> collect_candidates;
    for (int r = 0; r < NO_OF_ROWS; ++r) { collect_candidates.push_back({r,0}); collect_candidates.push_back({r,5}); }
    shuffle(collect_candidates.begin(), collect_candidates.end(), rng);
    vector<int> reels_used;
    for (auto [r,c] : collect_candidates) {
        if (find(reels_used.begin(), reels_used.end(), c) != reels_used.end()) continue;
        w[r][c] = COLLECT;
        reels_used.push_back(c);
        if (static_cast<int>(reels_used.size()) == collect_count) break;
    }
    vector<pair<int,int>> cors;
    for (int r = 0; r < NO_OF_ROWS; ++r) for (int c = 0; c < NO_OF_REELS; ++c) if (w[r][c] != COLLECT) {
        if (weightedIndex(probTable.freeGameCOROccur) == 0) { w[r][c] = COR; cors.push_back({r,c}); }
    }
    if (cors.empty()) {
        for (int r = 0; r < NO_OF_ROWS && cors.empty(); ++r) for (int c = 0; c < NO_OF_REELS && cors.empty(); ++c) if (w[r][c] != COLLECT) { w[r][c] = COR; cors.push_back({r,c}); }
    }
    int good = weightedIndex(vector<int>(cors.size(), 1));
    for (int i = 0; i < static_cast<int>(cors.size()); ++i) {
        auto [r,c] = cors[i];
        cor_values[r][c] = probTable.CORValues[weightedIndex(i == good ? probTable.goodCORTable : probTable.badCORTable)];
    }
    for (int r = 0; r < NO_OF_ROWS; ++r) for (int c = 0; c < NO_OF_REELS; ++c) if (w[r][c] == LV6) w[r][c] = COR, cor_values[r][c] = probTable.CORValues[0];
    return w;
}

int FreeGameFeature_2(int num_of_free_spins) {
    Trace trace("FreeGameFeature_2");
    trace.input("num_of_free_spins", num_of_free_spins);
    trace.GlobalandStruct("probTable.freeGamenumCollect", probTable.freeGamenumCollect);
    trace.GlobalandStruct("probTable.freeGameCOROccur", probTable.freeGameCOROccur);
    trace.GlobalandStruct("probTable.goodCORTable", probTable.goodCORTable);
    trace.GlobalandStruct("probTable.badCORTable", probTable.badCORTable);
    trace.GlobalandStruct("probTable.CORValues", probTable.CORValues);
    trace.GlobalandStruct("BASE_BET_FIXED_COINS", BASE_BET_FIXED_COINS);
    int spins_remaining = num_of_free_spins, session_win = 0, forced_idx = 0;
    while (spins_remaining > 0) {
        spins_remaining--;
        CorValueWindow values;
        PayWindow w;
        if (forced_idx < static_cast<int>(forced_blitz_windows.size())) { w = forced_blitz_windows[forced_idx]; values = forced_blitz_cor_values[forced_idx]; forced_idx++; }
        else w = buildRandomBlitzWindow(values);
        if (countSymbol(w, COLLECT) < 1 || countSymbol(w, COR) < 1) throw runtime_error("Blitz window must contain at least one COLLECT and COR");
        session_win += resolveCollect(w, values);
    }
    trace.output("session_win", session_win);
    return session_win;
}

int runOneSpin(FeatureChoice selectedFeature = FeatureChoice::None) {
    Trace trace("runOneSpin");
    trace.input("selectedFeature", selectedFeature);
    trace.GlobalandStruct("probTable.ReelsetWeights", probTable.ReelsetWeights);
    PayWindow pay_window;
    if (forced_base_window_active) { pay_window = forced_base_window; last_cor_values = forced_base_cor_values.empty() ? CorValueWindow(NO_OF_ROWS, vector<double>(NO_OF_REELS, 0.0)) : forced_base_cor_values; assignMissingBaseCorValues(pay_window); }
    else pay_window = weightedIndex(probTable.ReelsetWeights) == 0 ? generatePayWindow(BG1_Reels, BG1_ReelSize) : generatePayWindow(BG2_Reels, BG2_ReelSize);
    if (!forced_base_window_active) pay_window = additionalCOR(pay_window);
    int base_win = waysWinCalculation(pay_window);
    int collect_win = resolveCollect(pay_window, last_cor_values);
    int total_win = base_win + collect_win;
    int scat_count = countSymbol(pay_window, SCAT);
    if (scat_count >= 3) {
        if (selectedFeature == FeatureChoice::FreeSpins) { static int awards[7] = {0,0,0,8,12,20,30}; total_win += FreeGameFeature_1(awards[scat_count]); }
        if (selectedFeature == FeatureChoice::BlitzSpins) { static int awards[7] = {0,0,0,3,4,6,8}; total_win += FreeGameFeature_2(awards[scat_count]); }
    }
    trace.output("total_win", total_win);
    return total_win;
}
