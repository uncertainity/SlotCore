#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <cxxabi.h>
#include <memory>
#include <typeinfo>
#include <string>

using namespace std;

std::random_device rd;
std::mt19937 rng(
    std::chrono::high_resolution_clock::now()
        .time_since_epoch()
        .count()
);

template<typename T>
std::string type_name() {
    int status = 0;

    std::unique_ptr<char, void(*)(void*)> res{
        abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status),
        std::free
    };

    return status == 0 ? res.get() : typeid(T).name();
}

enum Symbol {
    HV1, HV2, HV3, HV4, LV1, LV2, LV3, LV4, WILD, SCATTER, MYST,CASC,BLANK
};

// pay anywhere, cluster, megaways
// mystery stacks, multiplier

const Symbol symbolArray[] = {HV1, HV2, HV3, HV4, LV1, LV2, LV3, LV4};
constexpr int NO_OF_SYMBOLS = 8;
constexpr int SYMBOL_COUNT = 8;

// The highest kind of a win possible is actually MAX_MATCH - 1. 0 is included in MAX_MATCH to correspond to PayTable
constexpr int MAX_MATCH = 6;

// this variable is for megaways/multi ways only 
constexpr int MIN_REEL = 2; 

constexpr int NO_OF_REELS = 5;
constexpr int NO_OF_ROWS = 6;
constexpr int LINE_COUNT = 5;
constexpr int BET = 20;

const char* SymbolChar[] =
{
    "HV1",
    "HV2",
    "HV3",
    "HV4",
    "LV1",
    "LV2",
    "LV3",
    "LV4",
    "WILD",
    "SCATTER"
};

int PayTable_1[SYMBOL_COUNT + 1][MAX_MATCH] =
{
    // 0x  1x  2x  3x   4x    5x

    { 0,  0,  0,  20,  100,  500 },   // HV1
    { 0,  0,  0,  15,  80,   400 },   // HV2
    { 0,  0,  0,  10,  60,   300 },   // HV3
    { 0,  0,  0,  8,   40,   200 },   // HV4

    { 0,  0,  0,  5,   25,   100 },   // LV1
    { 0,  0,  0,  5,   20,   80  },   // LV2
    { 0,  0,  0,  3,   15,   60  },   // LV3
    { 0,  0,  0,  3,   10,   50  },   // LV4

    { 0,  0,  0,  50,   250, 1000 }      // WILD
};



enum Symbol BG1_reel1[] = {LV1,LV2,HV1,LV4,SCATTER,LV3,HV2,LV1,LV2,HV3,SCATTER,LV4,LV1,HV1,SCATTER,LV3,HV4,LV2};
enum Symbol BG1_reel2[] = {HV4,LV1,HV1,LV2,LV3,SCATTER,HV2,LV4,WILD,LV1,HV3,LV2,LV4,HV1,LV3,HV2};
enum Symbol BG1_reel3[] ={LV4,HV4,LV1,HV1,SCATTER,LV2,HV3,WILD,LV3,HV2,LV1,LV4,HV1,LV2,HV4,LV3,SCATTER,LV1,HV2,LV4};
enum Symbol BG1_reel4[] ={LV2,HV2,LV4,SCATTER,LV3,HV1,WILD,LV1,HV3,LV2,LV4,HV2,LV3,HV4,LV1,SCATTER,LV2};
enum Symbol BG1_reel5[] ={HV4,LV2,HV3,LV1,SCATTER,LV4,HV1,LV3,WILD,LV2,HV2,LV1,LV4,HV3,LV2,SCATTER,HV1,LV3,HV4};


enum Symbol* BG1_Reels[] = {BG1_reel1,BG1_reel2,BG1_reel3,BG1_reel4,BG1_reel5};
vector<int> BG1_Reelsize = {
    sizeof(BG1_reel1)/sizeof(BG1_reel1[0]),
    sizeof(BG1_reel2)/sizeof(BG1_reel2[0]),
    sizeof(BG1_reel3)/sizeof(BG1_reel3[0]),
    sizeof(BG1_reel4)/sizeof(BG1_reel4[0]),
    sizeof(BG1_reel5)/sizeof(BG1_reel5[0])
};


enum Symbol FG1_reel1[] = {LV1,LV2,HV1,LV4,SCATTER,LV3,SCATTER,LV1,LV2,HV3,LV4,LV1,HV1,SCATTER,LV3,HV4,LV2};
enum Symbol FG1_reel2[] = {HV4,LV1,HV1,LV2,LV3,SCATTER,HV2,LV4,WILD,LV1,HV3,LV2,LV4,HV1,LV3,HV2};
enum Symbol FG1_reel3[] ={LV4,HV4,LV1,HV1,SCATTER,LV2,SCATTER,WILD,LV3,HV2,LV1,LV4,HV1,LV2,HV4,LV3,SCATTER,LV1,HV2,LV4};
enum Symbol FG1_reel4[] ={LV2,HV2,LV4,SCATTER,LV3,HV1,WILD,SCATTER,HV3,LV2,LV4,HV2,LV3,HV4,LV1,SCATTER,LV2};
enum Symbol FG1_reel5[] ={HV4,LV2,HV3,LV1,SCATTER,LV4,HV1,LV3,WILD,SCATTER,HV2,LV1,LV4,HV3,LV2,SCATTER,HV1,LV3,HV4};

enum Symbol* FG1_Reels[] = {FG1_reel1,FG1_reel2,FG1_reel3,FG1_reel4,FG1_reel5};
vector<int> FG1_Reelsize = {
    sizeof(FG1_reel1)/sizeof(FG1_reel1[0]),
    sizeof(FG1_reel2)/sizeof(FG1_reel2[0]),
    sizeof(FG1_reel3)/sizeof(FG1_reel3[0]),
    sizeof(FG1_reel4)/sizeof(FG1_reel4[0]),
    sizeof(FG1_reel5)/sizeof(FG1_reel5[0])
};

template<typename T>
std::string shape_of(const T&) {
    return "[1]";
}

template<typename T>
std::string shape_of(const std::vector<T>& v) {
    return "[" + std::to_string(v.size()) + "]";
}

template<typename T>
std::string shape_of(const std::vector<std::vector<T>>& v) {

    size_t rows = v.size();
    size_t cols = rows ? v[0].size() : 0;

    return "[" +
           std::to_string(rows) +
           "x" +
           std::to_string(cols) +
           "]";
}

template<typename T, size_t N>
std::string shape_of(const T (&)[N]) {
    return "[" + std::to_string(N) + "]";
}

template<typename T, size_t R, size_t C>
std::string shape_of(const T (&)[R][C]) {
    return "[" + std::to_string(R) + "x" + std::to_string(C) + "]";
}


struct Trace{
    static inline int depth = 0; //Trace::depth
    std::string fn;

    Trace(const char* function_name) : fn(function_name){
        std::cout << std::string(depth * 2, ' ') << "->" << fn << "\n";
        depth ++;
    }

    template<typename T>
    void input(const std:: string& name, const T& value){
        std::cout << std::string(depth * 2, ' ')
                  << "input " << name
                  << ": " <<  type_name<T>()
                  << " shape:" << shape_of(value)
                  << "\n";
    }

    template<typename T>
    void output(const std::string& name, const T& value) {
        std::cout << std::string(depth * 2, ' ')
                  << "output " << name
                  << ": " << type_name<T>()
                  << " shape:" << shape_of(value)
                  << "\n";
    }

    template<typename T>
    void GlobalandStruct(const std::string& name, const T& value){
        std::cout << std::string(depth * 2, ' ')
                  << "Global/Struct " << name
                  << ": " << type_name<T>()
                  << " shape:" << shape_of(value)
                  << "\n";
    }

    ~Trace() {
        depth--;
        std::cout << std::string(depth * 2, ' ')
                  << "<- " << fn << "\n";
    }
};



struct probTables {
    //all probability threshold arguments go here
    vector <float> mystStackProb{0.2,0.4,0.35,0.05};
    vector <float> coinProb{0.2,0.4,0.35,0.05};
    float p_coin_thresh = 0.05;
    vector <int> coins{2,5,10,25};
};

struct infoBase{
    // all base game statistics go here
    int round_win = 0;
    int free_game = 0;
};

struct infoFree{
    int session_win = 0;
    int no_of_spins = 0;
    int feature_win = 0;
};

struct infoGrid{
    int cascade = 0;
    int multiplier = 0;
};



// matrix utilies

vector<Symbol> sliceReels(vector<vector<Symbol>> &pay_window,int reel_idx, int start_idx){
    
    vector<Symbol>sliced_reel;
    for (int i = start_idx; i < NO_OF_ROWS;i++){
        sliced_reel.push_back(pay_window[i][reel_idx]);
    }
    return sliced_reel;

}

int countNonzero(const vector<vector<Symbol>> &pay_window, Symbol target_symbol){
    // count nonzero
    int count = 0;
    for (int i = 0; i < pay_window.size(); i++){
        for(int j = 0; j < pay_window[0].size(); j++){
            if(pay_window[i][j] == target_symbol) count ++;
        }
    }
    return count;
}

template<typename Predicate>
int countIf(const vector<vector<Symbol>> &pay_window, Predicate pred){
    int count = 0;
    for (const auto& row: pay_window){
        count += (count_if(row.begin(),row.end(),pred));
    }
    return count;
}


struct symbolWhere{

    vector <pair <int,int>> positions;
    Symbol current_symbol;
    vector<int> rawPositions;
    int count = 0;
};


template<typename Predicate>
symbolWhere findSymbolWhere(const vector<vector<Symbol>> &pay_window, Symbol target_symbol, Predicate pred){
    // np.where implementation
    // example of pred -> findSymbol(pay_window,Symbol target_symbol, [](Symbol s){return s == target_symbol})
    symbolWhere target_info;
    int rows = pay_window.size();
    int reels = pay_window[0].size();
    target_info.current_symbol = target_symbol;

    for(int i = 0; i < rows; i++){
        for(int j = 0; j < reels; j++){
            if (pred(pay_window[i][j])){
                target_info.positions.push_back({i,j});
                target_info.rawPositions.push_back(i*reels + j);
                target_info.count ++;
            }
        }
    }
    return target_info;
}

symbolWhere findSymbol(const vector<vector<Symbol>> &pay_window, Symbol target_symbol){
    // implementation of np.where

    symbolWhere target_info;
    int rows = pay_window.size();
    int reels = pay_window[0].size();
    target_info.current_symbol = target_symbol;
    for (int i = 0; i < rows; i++){
        for(int j = 0; j < reels; j++){
            if (pay_window[i][j] == target_symbol){
                target_info.positions.push_back({i,j});
                target_info.rawPositions.push_back(i*reels + j);
                target_info.count ++;
            }
        }
    }
    return target_info;
}



// pretty print functions
void printPayWindow(const vector<vector<Symbol>>& window) {
    cout << "\n===== PAY WINDOW =====\n";
    int rows = window.size();
    int reels = window[0].size();
    for (int row = 0; row < rows; row++) {
        for (int reel = 0; reel < reels; reel++) {

            switch (window[row][reel]) {
                case LV1:  cout << "LV1\t"; break;
                case LV2:  cout << "LV2\t"; break;
                case LV3:  cout << "LV3\t"; break;
                case LV4:  cout << "LV4\t"; break;

                case HV1:  cout << "HV1\t"; break;
                case HV2:  cout << "HV2\t"; break;
                case HV3:  cout << "HV3\t"; break;
                case HV4:  cout << "HV4\t"; break;

                case WILD: cout << "WILD\t"; break;
                case SCATTER: cout << "SCAT\t"; break;
                case BLANK: cout <<"   \t"; break;
                case MYST: cout<<"MYST\t";break;
                case CASC: cout<<"###\t";break;

                default:   cout << "UNK\t"; break;
            }
        }
        cout << "\n";
    }

    cout << "======================\n";
}

void printCoinWindow(const vector<vector<int>>& coin_window) {
    cout << "Coin Window:\n";

    for (const auto& row : coin_window) {
        for (const auto& val : row) {
            cout << setw(4) << val << " ";
        }
        cout << '\n';
    }

    cout << "------------------------\n";
}




template<typename T>
void print1D(const std::vector<T>& arr)
{
    for (const auto& x : arr)
        std::cout << x << ",";

    std::cout << "\n";
}



// sampling functions 
int getRandom(int a,int b){
    std::uniform_int_distribution<int> dist(a, b);
    return dist(rng);
}

double getUniform(){
    std::uniform_real_distribution<double>dist(0.0,1.0);
    return dist(rng);
}

vector<int> SRSWOR(vector<int>allowedPositions,int number){
    vector<int>selectedPositions;
    int totalSize = allowedPositions.size();
    shuffle(
        allowedPositions.begin(),
        allowedPositions.end(),
        rng
    );
    for(int i = 0; i < number && i < allowedPositions.size(); i++){
        selectedPositions.push_back(allowedPositions[i]);
    }
    return selectedPositions;
}

int cumProbability(vector<float>probArray,float p){
    vector<float>cumProb(probArray.size(),0);
    float running_sum = 0.0f;
    for(int i = 0; i < probArray.size(); i++){
        running_sum += probArray[i];
        cumProb[i] = running_sum;
    }
    for(int i = 0; i < probArray.size();i++){
        if (p < cumProb[i]) return i;
    }
    return probArray.size() - 1;

}





//Slot Utility

vector<vector<Symbol>> generatePayWindow(Symbol* ReelSet[],vector<int>ReelSize){
    Trace trace(__func__);
    trace.input("Reelset",ReelSet);
    trace.input("ReelSize",ReelSize);
    
    vector<vector<Symbol>> pay_window (NO_OF_ROWS,vector<Symbol>(NO_OF_REELS));
    for (int i = 0; i < NO_OF_REELS;i++){
        int start_idx = getRandom(0,ReelSize[i] - 1);
        //cout<<"Index for Reel:"<<start_idx<<endl;
        for (int j = 0; j < NO_OF_ROWS;j++){
            int idx = (start_idx + j) % ReelSize[i];
            pay_window[j][i] = ReelSet[i][idx];
        }
    }
    trace.GlobalandStruct("NO_OF_ROWS",NO_OF_ROWS);
    trace.GlobalandStruct("NO_OF_REELS",NO_OF_REELS);
    
    trace.output("pay_window",pay_window);
    return pay_window;
}

// Win Evaluation Types

int waysWinCalculation(vector<vector<Symbol>>& pay_window,int start_idx){
    //vector<int>symbolWin{0,NO_OF_SYMBOLS};
    Trace trace(__func__);
    trace.input("pay_window",pay_window);
    trace.input("start_idx",start_idx);
    
    int win = 0;
    vector<Symbol>firstReel = sliceReels(pay_window,0,start_idx);
    //print1D(firstReel);
    vector<int>symbol_count(NO_OF_SYMBOLS,0);
    for (int i = 0; i < firstReel.size();i++){
        if (firstReel[i] <= LV4) symbol_count[firstReel[i]] ++;
    }
    //print1D(symbol_count);
    vector<Symbol>uniqueSymbols;
    for (int i = 0; i < NO_OF_SYMBOLS;i++){
        if (symbol_count[i] > 0) uniqueSymbols.push_back(symbolArray[i]);
    }
    // cout<<"Unique Symbols:"<<endl;
    // print1D(uniqueSymbols);
    vector<pair<int,int>> coord;
    for(int s = 0; s < uniqueSymbols.size();s++){
        int left2right = 0;
        int ways = 1;
        for (int idx = 0;idx<NO_OF_REELS;idx++){
            vector<Symbol>sliced_reel = sliceReels(pay_window,idx,start_idx);
            //print1D(sliced_reel);
            int occ_in_reel = 0;
            for(int i = 0; i < sliced_reel.size();i++){
                if ((sliced_reel[i] == uniqueSymbols[s]) || (sliced_reel[i] == WILD)) occ_in_reel ++; 
            }
            if (occ_in_reel > 0){
                ways = ways * occ_in_reel;
                left2right ++;
            }
            else break;
        }

        //cout<<"Ways:"<<ways<<endl;
        int symbol_win = PayTable_1[uniqueSymbols[s]][left2right] * ways;
        //HitTable[uniqueSymbols[s]][left2right] ++;
        //WinTable[uniqueSymbols[s]][left2right] = symbol_win;
        // cout<<"Symbol:"<<symbolArray[s]<<",left2right"<<left2right<<",Win:"<<symbol_win<<endl;
        // cout
        //     << "Symbol : " << SymbolChar[uniqueSymbols[s]]
        //     << " | Count : " << left2right
        //     << " | Ways : " << ways
        //     << " | Pay : " << PayTable_1[uniqueSymbols[s]][left2right]
        //     << " | Total Win : " << symbol_win
        //     << endl;
        win += symbol_win;
    }
    trace.GlobalandStruct("PayTable_1",PayTable_1);
    trace.GlobalandStruct("NO_OF_SYMBOLS",NO_OF_SYMBOLS);

    trace.output("win",win);
    return win;

}



// Popular Features

int holdAndSpin(vector<vector<Symbol>>& pay_window){
    Trace trace(__func__);
    trace.input("pay_window",pay_window);
    vector<vector<int>> coin_window(pay_window.size(),vector<int> (pay_window[0].size(),0));
    int count = 3;
    probTables pt;
    int feature_win = 0;

    for(int i = 0; i < pay_window.size(); i++){
        for(int j = 0; j < pay_window[0].size(); j++){
            if (pay_window[i][j] == SCATTER){
                float p = getUniform();
                if (p < pt.p_coin_thresh){
                    float p_temp = getUniform();
                    int idx = cumProbability(pt.coinProb,p_temp);
                    coin_window[i][j] = pt.coins[idx];
                    feature_win += coin_window[i][j];
                }
            }
        }
    }
    // cout<<"Coin Window Just after trigger:"<<endl;
    // printCoinWindow(coin_window);

    while (count != 0){
        int flag = 0;
        int number_filled = 0;
        for(int i = 0;i < coin_window.size(); i++){
            for(int j = 0; j < coin_window[0].size(); j++){
                if(coin_window[i][j] == 0){
                    float p = getUniform();
                    if (p < pt.p_coin_thresh){
                        float p_temp = getUniform();
                        int idx = cumProbability(pt.coinProb,p_temp);
                        coin_window[i][j] = pt.coins[idx];
                        feature_win += coin_window[i][j];
                        flag = 1;
                    }
                }
                else{
                    number_filled += 1;
                }
            }
        }
        //printCoinWindow(coin_window);
        if (flag == 1) count = 3;
        else count --;
        if (number_filled == coin_window.size() * coin_window[0].size()) break;
    }
    trace.GlobalandStruct("probTable.p_coin_thresh",pt.p_coin_thresh);
    trace.GlobalandStruct("probTable.coinProb",pt.coinProb);
    trace.GlobalandStruct("probTable.coins", pt.coins);
    trace.output("feature_win",feature_win);
    return feature_win;
}

int runOneSpin(){
    Trace trace(__func__);

    infoBase base_stats;
    int start_idx = 0;
    vector<vector<Symbol>> pay_window = generatePayWindow(BG1_Reels,BG1_Reelsize);
    //printPayWindow(pay_window);
    int round_win = waysWinCalculation(pay_window,start_idx);
    base_stats.round_win = round_win;
    int feature_win = 0;
    int scatters = 0;
    for(int i = 0;i < pay_window.size(); i++){
        for(int j = 0; j < pay_window[0].size(); j++){
            if(pay_window[i][j] == SCATTER){
                scatters ++;
            }
        }
    }
    //cout<<"Scatters:"<<scatters<<endl;
    if (scatters >= 3){
        base_stats.free_game = 1;
        feature_win = holdAndSpin(pay_window);
    }
    int total_win = round_win + feature_win;
    trace.output("total_win",total_win);
    return total_win;
}

int main(){
    int total_win = runOneSpin();
    return 0;
}