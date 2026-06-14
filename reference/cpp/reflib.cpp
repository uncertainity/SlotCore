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

using namespace std;

std::random_device rd;
std::mt19937 rng(
    std::chrono::high_resolution_clock::now()
        .time_since_epoch()
        .count()
);

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

// const Symbol symbolArray[] = {HV1, HV2, HV3, HV4, LV1, LV2, LV3, LV4};
// int NO_OF_SYMBOLS = sizeof(symbolArray)/sizeof(symbolArray[0]);
// const int SYMBOL_COUNT = 8;
// constexpr int MAX_MATCH = 6; // use index 0..5
// const int NO_OF_REELS = 5; 
// const int NO_OF_ROWS = 6;
// const int LINE_COUNT = 5;
// const int bet = 20;


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


int PayTable_2[SYMBOL_COUNT][MAX_MATCH] =
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

    //{ 0,  0,  0,  50,   250, 1000 }      // WILD
};



int PayLine[LINE_COUNT][NO_OF_REELS] = 
{
    {0, 1,  2,  3,  4},
    {5, 6,  7,  8,  9},
    {10,11, 12, 13, 14},
    {0, 6,  2,  8,  4},
    {5, 11, 12, 13, 14}
};



long long int HitTable[SYMBOL_COUNT][MAX_MATCH] = {};
long long int WinTable[SYMBOL_COUNT][MAX_MATCH] = {};


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


enum Symbol CG1_reel1[] = {LV1,LV2,HV1,LV4,SCATTER,LV3,HV2,LV1,LV2,HV3,SCATTER,LV4,LV1,HV1,SCATTER,LV3,HV4,LV2};
enum Symbol CG1_reel2[] = {HV4,LV1,HV1,LV2,LV3,SCATTER,HV2,LV4,WILD,LV1,HV3,LV2,LV4,HV1,LV3,HV2};
enum Symbol CG1_reel3[] ={LV4,HV4,LV1,HV1,SCATTER,LV2,HV3,WILD,LV3,HV2,LV1,LV4,HV1,LV2,HV4,LV3,SCATTER,LV1,HV2,LV4};
enum Symbol CG1_reel4[] ={LV2,HV2,LV4,SCATTER,LV3,HV1,WILD,LV1,HV3,LV2,LV4,HV2,LV3,HV4,LV1,SCATTER,LV2};
enum Symbol CG1_reel5[] ={HV4,LV2,HV3,LV1,SCATTER,LV4,HV1,LV3,WILD,LV2,HV2,LV1,LV4,HV3,LV2,SCATTER,HV1,LV3,HV4};


enum Symbol* CG1_Reels[] = {CG1_reel1,CG1_reel2,CG1_reel3,CG1_reel4,CG1_reel5};
vector<int> CG1_Reelsize = {
    sizeof(CG1_reel1)/sizeof(CG1_reel1[0]),
    sizeof(CG1_reel2)/sizeof(CG1_reel2[0]),
    sizeof(CG1_reel3)/sizeof(CG1_reel3[0]),
    sizeof(CG1_reel4)/sizeof(CG1_reel4[0]),
    sizeof(CG1_reel5)/sizeof(CG1_reel5[0])
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
    vector<vector<Symbol>> pay_window (NO_OF_ROWS,vector<Symbol>(NO_OF_REELS));
    for (int i = 0; i < NO_OF_REELS;i++){
        int start_idx = getRandom(0,ReelSize[i] - 1);
        //cout<<"Index for Reel:"<<start_idx<<endl;
        for (int j = 0; j < NO_OF_ROWS;j++){
            int idx = (start_idx + j) % ReelSize[i];
            pay_window[j][i] = ReelSet[i][idx];
        }
    }
    return pay_window;
}

vector<vector<Symbol>> generatePayWindowMegaways(Symbol* Reelset[], vector<int> ReelSize){
    vector<vector<Symbol>> pay_window (NO_OF_ROWS, vector<Symbol> (NO_OF_REELS, BLANK));
    vector<int> reel_len(NO_OF_REELS,0);
    for(int i = 0; i < NO_OF_REELS; i++){
        int selected_len = getRandom(MIN_REEL,NO_OF_REELS);
        reel_len[i] = selected_len;
        for(int j = 0; j < selected_len; j++){
            int idx = getRandom(0,ReelSize[i] - 1);
            pay_window[j][i] = Reelset[i][idx];
        }
    }
    cout<<"The selectd reel lengths are:"<<endl;
    print1D(reel_len);
    return pay_window;
}


void handleMystSymbols(vector<vector<Symbol>> &pay_window, string s){
    //replace every Myst Symbol with same symbol
    probTables pb;
    if (s == "SAME"){
        float p_myst = getUniform();
        int selected_idx = cumProbability(pb.mystStackProb,p_myst);
        for(int i = 0; i < NO_OF_ROWS; i++){
            for(int j = 0; j < NO_OF_REELS; j++){
                if(pay_window[i][j] == MYST) pay_window[i][j] = symbolArray[selected_idx];
            }
        }
    }
    else if(s == "DIFFERENT"){
        //replace every Myst symbol with a different symbol
        for(int i = 0; i < NO_OF_ROWS; i++){
            for(int j = 0; j < NO_OF_REELS; j++){
                if (pay_window[i][j] == MYST){
                    float p_myst = getUniform();
                    int selected_idx = cumProbability(pb.mystStackProb,p_myst);
                    pay_window[i][j] = symbolArray[selected_idx];
                }
            }
        }
    }

}

vector<vector<Symbol>> mergeGridPay(vector<vector<Symbol>> &pay_window, vector<vector<infoGrid>> &grid_window){
    vector<vector<Symbol>> updated_pay_window(pay_window.size(),vector<Symbol> (pay_window[0].size()));
    for(int i = 0; i < pay_window.size(); i++){
        for(int j = 0; j < pay_window[0].size(); j++){
            if(grid_window[i][j].cascade == 1){
                updated_pay_window[i][j] = CASC;
            }
            else{
                updated_pay_window[i][j] = pay_window[i][j];
            }
        }
    }
    return updated_pay_window;
}

vector<vector<Symbol>> cascade(vector<vector<Symbol>> &pay_window,Symbol* ReelSet[],vector<int>ReelSize){
    vector<vector<Symbol>> cascaded_pay_window(pay_window.size(),vector<Symbol>(pay_window[0].size(),BLANK));
    vector<int>end_idx(NO_OF_REELS);
    for(int i = 0; i < NO_OF_REELS; i++){
        int casc_row = NO_OF_ROWS - 1;
        for(int j = NO_OF_ROWS - 1; j >= 0; j--){
            if (pay_window[j][i] != CASC){
                cascaded_pay_window[casc_row][i] = pay_window[j][i];
                casc_row --;
                end_idx[i] ++;
            }
        }
    }
    //printPayWindow(cascaded_pay_window);
    for(int i = 0; i < NO_OF_REELS; i++){
        int cr_idx = getRandom(0,ReelSize[i] - 1);
        for (int j = 0; j < NO_OF_ROWS - end_idx[i]; j++){
            int idx = (cr_idx + j) % ReelSize[i];
            cascaded_pay_window[j][i] = ReelSet[i][idx];
        }
    }
    return cascaded_pay_window;
}


// Win Evaluation Types

int waysWinCalculation(vector<vector<Symbol>>& pay_window,int start_idx){
    //vector<int>symbolWin{0,NO_OF_SYMBOLS};
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
        HitTable[uniqueSymbols[s]][left2right] ++;
        WinTable[uniqueSymbols[s]][left2right] = symbol_win;
        // cout<<"Symbol:"<<symbolArray[s]<<",left2right"<<left2right<<",Win:"<<symbol_win<<endl;
        cout
            << "Symbol : " << SymbolChar[uniqueSymbols[s]]
            << " | Count : " << left2right
            << " | Ways : " << ways
            << " | Pay : " << PayTable_1[uniqueSymbols[s]][left2right]
            << " | Total Win : " << symbol_win
            << endl;
        win += symbol_win;
    }
    return win;

}


int waysWinCalculationWithCascade(vector<vector<Symbol>>& pay_window, vector<vector<infoGrid>>& grid_window, int start_idx){
    //vector<int>symbolWin{0,NO_OF_SYMBOLS};
    // vector<vector<infoGrid>> grid_window;
    // grid_window.resize(pay_window.size(),vector<infoGrid>pay_window[0].size());

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
    
    for(int s = 0; s < uniqueSymbols.size();s++){
        int left2right = 0;
        int ways = 1;
        vector<pair<int,int>> coord;
        for (int idx = 0;idx<NO_OF_REELS;idx++){
            vector<Symbol>sliced_reel = sliceReels(pay_window,idx,start_idx);
            //print1D(sliced_reel);
            int occ_in_reel = 0;
            for(int i = 0; i < sliced_reel.size();i++){
                if ((sliced_reel[i] == uniqueSymbols[s]) || (sliced_reel[i] == WILD)){
                    coord.push_back({i,idx});
                    occ_in_reel ++;
                } 
            }
            if (occ_in_reel > 0){
                ways = ways * occ_in_reel;
                left2right ++;
            }
            else break;
        }

        //cout<<"Ways:"<<ways<<endl;
        int symbol_win = PayTable_1[uniqueSymbols[s]][left2right] * ways;

        if (symbol_win > 0){
            for (const auto&[row,col]: coord){
                grid_window[row][col].cascade = 1;
            }
        }

        HitTable[uniqueSymbols[s]][left2right] ++;
        WinTable[uniqueSymbols[s]][left2right] = symbol_win;
        // cout<<"Symbol:"<<symbolArray[s]<<",left2right"<<left2right<<",Win:"<<symbol_win<<endl;
        cout
            << "Symbol : " << SymbolChar[uniqueSymbols[s]]
            << " | Count : " << left2right
            << " | Ways : " << ways
            << " | Pay : " << PayTable_1[uniqueSymbols[s]][left2right]
            << " | Total Win : " << symbol_win
            << endl;
        win += symbol_win;
    }
    return win;

}



int linesWinCalculation(vector<vector<Symbol>>& pay_window){
    int round_win = 0;
    //cout <<"Inside the function"<<endl;
    for(int line = 0; line < LINE_COUNT; line++){
        vector<Symbol> oneLine(NO_OF_REELS,BLANK);
        int line_win = 0;
        Symbol winning_symbol = BLANK;
        for(int i = 0; i < NO_OF_REELS; i++){
            int row_no = PayLine[line][i] / NO_OF_REELS;
            int reel_no = PayLine[line][i] % NO_OF_REELS;
            oneLine[i] = pay_window[row_no][reel_no];
        }
        Symbol start_symbol = oneLine[0];
        int count = 1;
        int cons_wild_count = 0;
        if (start_symbol == WILD){
            while (start_symbol == WILD){
                start_symbol = oneLine[cons_wild_count];
                cons_wild_count ++;
            }
            cons_wild_count --;
        }
        //cout<<"Start Symbol:"<<SymbolChar[start_symbol]<<endl;
        //cout<<"Consecutive Wild Count:"<<cons_wild_count<<endl;
        for(int i = 1; i < oneLine.size(); i++){
            if ((start_symbol == oneLine[i]) || (oneLine[i] == WILD)){
                count ++;
            }
        }
        //cout<<"Actual Count:"<<count<<endl;

        if (PayTable_1[WILD][cons_wild_count] > PayTable_1[start_symbol][count]){
            line_win = PayTable_1[WILD][cons_wild_count];
            winning_symbol = WILD;
            count = cons_wild_count;
        }
        else {
            line_win = PayTable_1[start_symbol][count];
            winning_symbol = start_symbol;
        }
        cout
            << "Symbol : " << SymbolChar[winning_symbol]
            << " | Line No : " << line
            << " | Kind : " << count
            << " | Pay : " << line_win
            << endl;
        round_win += line_win;
    }
    return round_win;
}

int payAnyWhereWinCalculation(vector<vector<Symbol>> &pay_window){
    vector<int> freqSymbols(NO_OF_SYMBOLS,0);
    int wild_add = 0;
    int round_win = 0;
    for(int i = 0; i < NO_OF_ROWS; i++){
        for(int j = 0; j < NO_OF_REELS; j++){
            if(pay_window[i][j] == WILD){
                wild_add ++;
            }
            else freqSymbols[pay_window[i][j]] ++;
        }
    }
    //print1D(freqSymbols);
    for(int i = 0; i < freqSymbols.size(); i++){
        freqSymbols[i] = min(MAX_MATCH - 1,freqSymbols[i] + wild_add);
        
        int symbol_win = PayTable_2[symbolArray[i]][freqSymbols[i]];
        round_win += symbol_win;
        if (symbol_win > 0){
            cout
            << "Symbol : " << SymbolChar[symbolArray[i]]
            << " | Count : " << freqSymbols[i]
            << " | Pay : " << symbol_win
            << endl;
        }
    }
    //print1D(freqSymbols);
    return round_win;
    
}

// Popular Features

int holdAndSpin(vector<vector<Symbol>>& pay_window){
    
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
        printCoinWindow(coin_window);
        if (flag == 1) count = 3;
        else count --;
        if (number_filled == coin_window.size() * coin_window[0].size()) break;
    }
    return feature_win;
}


void experimentalPayWindow(){
    // for testing dummy cases
    // vector<vector<Symbol>> pay_window = generatePayWindow(BG1_Reels,BG1_Reelsize);
    // pay_window[0][0] = WILD;
    // pay_window[0][1] = LV4;
    // pay_window[0][2] = WILD;
    // pay_window[0][3] = LV4;
    //pay_window[0][4] = LV4;
    
    // printPayWindow(pay_window);
    // //int round_win = linesWinCalculation(pay_window);
    // int round_win = payAnyWhereWinCalculation(pay_window);
    // vector<vector<Symbol>> pay_window = generatePayWindowMegaways(BG1_Reels,BG1_Reelsize);
    // printPayWindow(pay_window);
    // int round_win = waysWinCalculation(pay_window,0);    
    // cout<<"The Round Win:"<<round_win<<endl;
    // vector<vector<Symbol>> pay_window = generatePayWindow(BG1_Reels,BG1_Reelsize);
    // pay_window[0][1] = MYST;
    // pay_window[2][2] = MYST;
    // pay_window[2][3] = MYST;
    // pay_window[1][2] = MYST;
    // cout<<"With only mystery stack:"<<endl;
    // printPayWindow(pay_window);
    // handleMystSymbols(pay_window,"DIFFERENT");
    // cout<<"After handling mystery stacks:"<<endl;
    // printPayWindow(pay_window);
    
    // vector<vector<Symbol>> pay_window = generatePayWindow(BG1_Reels,BG1_Reelsize);
    // pay_window[1][1] = WILD;
    // pay_window[2][1] = WILD;
    // printPayWindow(pay_window);
    // vector<vector<infoGrid>> grid_window(pay_window.size(),vector<infoGrid> (pay_window[0].size()));
    // int round_win = waysWinCalculationWithCascade(pay_window,grid_window,0);
    // vector<vector<Symbol>> updated_pay_window = mergeGridPay(pay_window,grid_window);
    // printPayWindow(updated_pay_window);
    // vector<vector<Symbol>> cascaded_pay_window = cascade(updated_pay_window,CG1_Reels,CG1_Reelsize);
    // printPayWindow(cascaded_pay_window);

    vector<vector<Symbol>> pay_window = generatePayWindow(BG1_Reels,BG1_Reelsize);
    pay_window[1][2] = SCATTER;
    pay_window[2][4] = SCATTER;
    int feature_win = holdAndSpin(pay_window);
    cout<<"Feature Win:"<<feature_win<<endl;
}

//g++ -std=c++17 main.cpp -o main

struct Trace{
    static inline int depth = 0;
    std::string fn;

    Trace(const char* function_name) : fn(function_name){
        std::cout << std::string(depth * 2, ' ') << "->" << fn << "\n";
        depth ++;
    }

    template<typename T>
    void input(const std:: string& name, const T& value){
        std::cout << std::string(depth * 2, ' ')
                  << "input " << name
                  << ": " << typeid(T).name()
                  << "\n";
    }

    template<typename T>
    void output(const std::string& name, const T& value) {
        std::cout << std::string(depth * 2, ' ')
                  << "output " << name
                  << ": " << typeid(T).name()
                  << "\n";
    }

    ~Trace() {
        depth--;
        std::cout << std::string(depth * 2, ' ')
                  << "<- " << fn << "\n";
    }
};




int main(){
    experimentalPayWindow();
    return 0;
}