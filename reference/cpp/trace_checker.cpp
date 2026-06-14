#include <bits/stdc++.h>
using namespace std;

struct Contract {
    string kind;   // input / global / output
    string name;   // pay_window
    string type;   // std::vector<...>
    string n_type;
    string shape;  // 4x6
};

struct TraceNode {
    string name;
    vector<Contract> inputs;
    vector<Contract> globals;
    vector<Contract> outputs;
    vector<TraceNode*> children;
};

// remove leading and trailing white spaces
string trim(const string &s){
    int l = 0, r = (int)s.size()-1;

    while(l <= r && isspace(s[l])) l++;
    while(r >= l && isspace(s[r])) r--;

    if (l > r) return "";
    return s.substr(l,r-l+1);

}

string removeSpace(string s){
    s.erase(
        remove_if(s.begin(),s.end(),::isspace),
        s.end()
    );
    return s;
}

bool startsWith(const string& line, const string& prefix){
    return line.rfind(prefix,0) == 0;
}

string parseFunctionName(const string& line, const string& arrow){
    string name = trim(line.substr(arrow.size()));
    return name;
}

// vector<TraceNode*> parseTrace(const string& text){
//     vector<TraceNode*> roots;
//     vector<TraceNode*> st;
    
//     stringstream ss(text);
//     string line;

//     while (getline(ss,line)){
//         line = trim(line);
//         if(line.empty()) continue;

//         if (startsWith(line,"->")){
//             string fn_name = parseFunctionName(line,"->");
//             TraceNode* node = new TraceNode();
//             node->name = fn_name;

//             if(st.empty()){
//                 roots.push_back(node);
//             }
//             else{
//                 st.back()->children.push_back(node);
//             }
//             st.push_back(node);
//         }
//         else if(startsWith(line,"<-")){
//             if(!st.empty()){
//                 st.pop_back();
//             }
//         }
//     }
//     return roots;
// }

Contract parseContract(string line){
    Contract c;
    line = trim(line);

    if(startsWith(line,"input")){
        c.kind = "input";
        line = trim(line.substr(6));
    }
    else if (startsWith(line,"Global/Struct")){
        c.kind = "global";
        line = trim(line.substr(14));
    }
    else if(startsWith(line,"output")){
        c.kind = "output";
        line = trim(line.substr(7));
    }

    size_t shape_pos = line.rfind("shape:["); //this gives the start of s in shape:[
    if(shape_pos != string::npos){
        size_t start = shape_pos + 7;
        size_t end = line.find("]", start);
        c.shape = line.substr(start,end - start);
        line = trim(line.substr(0, shape_pos));
    }

    size_t colon_pos = line.find(":");
    if (colon_pos != string::npos) {
        c.name = trim(line.substr(0, colon_pos));
        c.type = trim(line.substr(colon_pos + 1));
        c.n_type = removeSpace(c.type);
    } else {
        c.name = trim(line);
        c.type = "";
        c.n_type = "";
    }
    return c;
}

vector<TraceNode*> parseTrace(const string& text) {
    vector<TraceNode*> roots;
    vector<TraceNode*> st;

    stringstream ss(text);
    string line;

    while (getline(ss, line)) {
        line = trim(line);
        if (line.empty()) continue;

        if (startsWith(line, "->")) {
            string fn_name = parseFunctionName(line, "->");

            TraceNode* node = new TraceNode();
            node->name = fn_name;

            if (st.empty()) roots.push_back(node);
            else st.back()->children.push_back(node);

            st.push_back(node);
        }

        else if (startsWith(line, "<-")) {
            if (!st.empty()) st.pop_back();
        }

        else if (startsWith(line, "input ") ||
                 startsWith(line, "Global/Struct ") ||
                 startsWith(line, "output ")) {

            if (st.empty()) continue;

            Contract c = parseContract(line);

            if (c.kind == "input")
                st.back()->inputs.push_back(c);
            else if (c.kind == "global")
                st.back()->globals.push_back(c);
            else if (c.kind == "output")
                st.back()->outputs.push_back(c);
        }
    }

    return roots;
}


struct NodeSignature {
    vector<string> input_shapes;
    vector<string> global_names;
    vector<string> global_shapes;
    vector<string> output_shapes;
};


NodeSignature makeSignature(const TraceNode* node){
    NodeSignature sig;
    for(const auto &c : node->inputs){
        sig.input_shapes.push_back(c.shape);
    }
    for (const auto& c : node->globals) {
        sig.global_names.push_back(c.name);
        sig.global_shapes.push_back(c.shape);
    }

    for (const auto& c : node->outputs) {
        sig.output_shapes.push_back(c.shape);
    }
    return sig;
}





void printContract(const Contract& c, int depth) {
    for (int i = 0; i < depth; i++)
        cout << "  ";

    cout << c.kind
         << " "
         << c.name
         << " : "
         << c.n_type
         << " shape:["
         << c.shape
         << "]\n";
}

void printTree(TraceNode* node, int depth = 0) {

    for (int i = 0; i < depth; i++)
        cout << "  ";

    cout << "->" << node->name << "\n";

    for (const auto& input : node->inputs)
        printContract(input, depth + 1);

    for (const auto& global : node->globals)
        printContract(global, depth + 1);

    for (const auto& output : node->outputs)
        printContract(output, depth + 1);

    for (TraceNode* child : node->children)
        printTree(child, depth + 1);

    for (int i = 0; i < depth; i++)
        cout << "  ";

    cout << "<-" << node->name << "\n";
}

void printSignature(TraceNode* node, int depth = 0){
    for(int i = 0;  i < depth; i++){
        cout<<" ";
    }
    cout<<node->name<<endl;
    NodeSignature sig = makeSignature(node);
    for (const auto &s : sig.input_shapes){
        cout<<"Input Shapes:" 
        << s 
        <<"   "
        <<"\n";
    }
    for(int i = 0; i < sig.global_names.size(); i++){
        cout<<sig.global_names[i]
            <<":"
            <<sig.global_shapes[i]
            <<"\n";
    }
    for (const auto &s : sig.output_shapes){
        cout<<"Output Shapes:" 
        << s 
        <<"   "
        <<"\n";
    }

    for(TraceNode* child : node->children){
        printSignature(child,depth + 1);
    }
    for (int i = 0; i < depth; i++)
        cout << "  ";
}

bool signatureCompatible(const NodeSignature& a, const NodeSignature& b) {
    return a.input_shapes == b.input_shapes
        && a.global_names == b.global_names
        && a.global_shapes == b.global_shapes
        && a.output_shapes == b.output_shapes;
}

bool nodeCompatible(TraceNode* test, TraceNode* ref) {
    NodeSignature ts = makeSignature(test);
    NodeSignature rs = makeSignature(ref);

    return signatureCompatible(ts, rs);
}


bool treeCompatible(TraceNode* test, TraceNode* ref);

bool childrenCompatible(const vector<TraceNode*>& test_children,const vector<TraceNode*>& ref_children){
    int j = 0;
    for(int i = 0; i < (int)test_children.size(); i++){
        bool found = false;

        while(j < (int)ref_children.size()){
            if(treeCompatible(test_children[i],ref_children[j])){
                found = true;
                j ++;
                break;
            }
            j++;
        }
        if(!found) return false;
    }
    return true;
}

bool treeCompatible(TraceNode* test, TraceNode* ref) {
    if (!nodeCompatible(test, ref)){
        cout<<test->name
            << " and "
            << ref->name
            <<" are not compatible"<<endl;
        return false;
    }
    return childrenCompatible(test->children, ref->children);
}

bool existsSubtree(TraceNode* test, TraceNode* ref){
    if(treeCompatible(test,ref)){
        return true;
    }
    for(const auto &child : ref->children){
        if(existsSubtree(test,child)){
            return true;
        }
    }
    return false;
}

bool traceMatches(const vector<TraceNode*>& test_roots,const vector<TraceNode*>& ref_roots){
    for (TraceNode* test_root : test_roots) {
        bool matched = false;

        for (TraceNode* ref_root : ref_roots) {
            if (existsSubtree(test_root, ref_root)) {
                matched = true;
                break;
            }
        }

        if (!matched) {
            cout << "No match for test root: "
                 << test_root->name
                 << "\n";
            return false;
        }
    }

    return true;
}



int main() {
    string ref_trace = R"(
->runOneSpin
  input bet_fixed_coins: int shape:[1]
  input feature_choice: FeatureChoice shape:[1]
  Global/Struct probTable.ReelsetWeights: std::vector<int, std::allocator<int> > shape:[2]
  ->generatePayWindow
    input ReelSet: Symbol[6][variable] shape:[6][variable]
    input ReelSize: std::vector<int, std::allocator<int> > shape:[6]
    Global/Struct NO_OF_ROWS: int shape:[1]
    Global/Struct NO_OF_REELS: int shape:[1]
    output pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
  <- generatePayWindow
  ->additionalCOR
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    Global/Struct probTable.corTrigger: std::vector<int, std::allocator<int> > shape:[2]
    Global/Struct probTable.numCorWeight: std::vector<int, std::allocator<int> > shape:[5]
    Global/Struct probTable.baseCORValueWeights.values: std::vector<double, std::allocator<double> > shape:[22]
    Global/Struct probTable.baseCORValueWeights.weights: std::vector<int, std::allocator<int> > shape:[22]
    output pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
  <- additionalCOR
  ->waysWinCalculation
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    Global/Struct PayTable: int shape:[12x7]
    Global/Struct STANDARD_SYMBOL_COUNT: int shape:[1]
    output win: int shape:[1]
  <- waysWinCalculation
  ->resolveCollect
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    input bet_fixed_coins: int shape:[1]
    Global/Struct probTable.CORValues: std::vector<double, std::allocator<double> > shape:[22]
    output CollectResult: CollectResult shape:[1]
  <- resolveCollect
  ->FreeGameFeature_2
    input num_of_blitz_spins: int shape:[1]
    Global/Struct probTable.freeGamenumCollect: std::vector<int, std::allocator<int> > shape:[2]
    Global/Struct probTable.freeGameCOROccur: std::vector<int, std::allocator<int> > shape:[2]
    Global/Struct probTable.goodCORTable: std::vector<int, std::allocator<int> > shape:[22]
    Global/Struct probTable.badCORTable: std::vector<int, std::allocator<int> > shape:[22]
    Global/Struct probTable.CORValues: std::vector<double, std::allocator<double> > shape:[22]
    ->resolveCollect
      input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
      input bet_fixed_coins: int shape:[1]
      Global/Struct probTable.CORValues: std::vector<double, std::allocator<double> > shape:[22]
      output CollectResult: CollectResult shape:[1]
    <- resolveCollect
    output FeatureResult.session_win: int shape:[1]
  <- FreeGameFeature_2
  output SpinResult.total_win: int shape:[1]
<- runOneSpin

)";
    string test_trace = R"(
->waysWinCalculation
  input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
  Global/Struct PayTable: std::array<std::array<int, 7ul>, 12ul> shape:[12x7]
  Global/Struct STANDARD_SYMBOL_COUNT: int shape:[1]
  output win: int shape:[1]
<- waysWinCalculation
->resolveCollect
  input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
  input bet_fixed_coins: int shape:[1]
  Global/Struct probTable.CORValues: std::vector<double, std::allocator<double> > shape:[22]
  output CollectResult: CollectResult shape:[1]
<- resolveCollect
)";

    // vector<TraceNode*> roots = parseTrace(trace);

    // for (TraceNode* root : roots) {
    //     printSignature(root);
    //     //printTree(root);
    // }

    //same node check
    vector<TraceNode*> test_roots = parseTrace(test_trace);
    vector<TraceNode*> ref_roots = parseTrace(ref_trace);
    if (traceMatches(test_roots, ref_roots)) {
        cout << "PASS\n";
    } else {
        cout << "FAIL\n";
    }
    return 0;
}