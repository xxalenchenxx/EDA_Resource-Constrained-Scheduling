#include<iostream>
#include<unistd.h>
#include<fstream>
#include <sstream>
#include<vector>
#include<stack>
#include<queue>
#include<cstring>
#include<map>
#include <algorithm>
#include "gurobi_c++.h" //gurobi_c++.h
using namespace std;
#define AND 0
#define OR  1
#define NOT 2

struct opera{
  int id;
  int type;
  //type:
  //0 AND
  //1 OR
  //2 NOT
};
bool read_file(string blifFile,vector<vector<bool>> &graph_matrix,map<string,opera> &opera_output,vector<int> &NOP_out);

bool check_slack_time(vector<int> arrival_time){
  for(const auto& i:arrival_time){
    if(i==-1)
      return false;
  }
  return true;
}


void arrival_time_calculate(vector<vector<bool>> graph_matrix,vector<int> &arrival_time){
  //arrival time
  //transepose graph matrix
  size_t n=arrival_time.size();
  vector<vector<int>> transpose(n,vector<int>(n,0));
  for(int i=0;i<n;i++){
    for(int j=0;j<n;j++){
      transpose[j][i]=graph_matrix[i][j];
    }
  }
  
  //initial node of arrival time
  for(auto i=0;i<transpose.size();i++){
    for(auto j=0;j<transpose[i].size();j++){
      arrival_time[i]=1;
      if(transpose[i][j]==1){
        arrival_time[i]=-1;
        break;
      }
    }
  }
  //do until all node have arrival time
  while(!check_slack_time(arrival_time)){ 
    for(int i=0;i<n;i++){
      bool process=true;
      if(arrival_time[i]==-1){
        for(int j=0;j<n;j++){
          if(arrival_time[j]==-1 && transpose[i][j]==1){
            process=false;
            break;
          }
        }
        if(process){
        int max=0;
        for(int j=0;j<n;j++){
          if(transpose[i][j]==1 && arrival_time[j]>max)
            max=arrival_time[j]; 
        }
        arrival_time[i]=max+1; //delay 1 cycle
        }
      }
    }
  }
  return;
}


void required_time_calculate(vector<vector<bool>> graph_matrix,vector<int> &required_time,vector<int> NOP_out, int t){
  size_t n= required_time.size();
  //initial required time
  for(int i=0;i<NOP_out.size();i++){
    required_time[NOP_out[i]]=t;
  }

  while(!check_slack_time(required_time)){ 
    for(int i=0;i<n;i++){
      bool process=true;
      if(required_time[i]==-1){
        for(int j=0;j<n;j++){
          if(required_time[j]==-1 && graph_matrix[i][j]==1){
            process=false;
            break;
          }
        }

        if(process){
        int min=INT32_MAX;
        for(int j=0;j<n;j++){
          if(graph_matrix[i][j]==1){
            if(required_time[j]<min)
              min=required_time[j];
          }
        }
        required_time[i]=min-1; //delay 1 cycle
        }
      }
    }
  }
  return;
}

//compile:  g++ main.cpp -o main -lglpk
//run code: ./mlrcs -h 6io2.blif 2 1 1
int main(int argc, char **argv) {
  //input format
  //mlrcs -h/-e BLIF_FILE AND_CONSTRAINT OR_CONSTRAINT NOT_CONSTRAINT 
  string blifFile = argv[2];
  int CONSTRAINT[3]={0,0,0};
  for(int i=0;i<3;i++){
    CONSTRAINT[i]=stoi(argv[3+i]);
    if(i==0)
      cout<<"AND constraint: "<<CONSTRAINT[i]<<endl;
    else if(i==1)
      cout<<"OR constraint: "<<CONSTRAINT[i]<<endl;
    else
      cout<<"NOT constraint: "<<CONSTRAINT[i]<<endl;
  }
    
  
  //read optin start
  //-h list scheduling
  //-e list scheduling+ILP
  char option;
  int opt;
  while((opt = getopt(argc,(char **)argv, "he")) !=-1 ){
    switch(opt){
      case 'h':
      case 'e':
        option=opt;
        break;
      
      default:
        std::cout<<"No such option\n";
        return 0;
        break;

    }
  }
  std::cout<<"option:"<<option<<endl;
  //read optin end
  //read file

  vector<vector<bool>> graph_matrix;
  map<string,opera> opera_output;
  vector<int> NOP_out;
  if(!read_file(blifFile,graph_matrix,opera_output,NOP_out))
    return 0;
  size_t n=opera_output.size(); //matrix size


  // std::cout << "opera Results:" << endl;
  // for (const auto& pair : opera_output) {
  //     std::cout << pair.first << ": " << pair.second.id<<" , "<<pair.second.type<< endl;
  // }


  //input inital ready node
  vector<int> ref(n,0); //check node is ready or not
  vector<queue<int>> ready(3);
  vector<string> id2node(n,"");
  for (const auto& entry : opera_output) 
    id2node[entry.second.id]=entry.first;
    

  //calculate number of predecessor
  for(int i=0;i<n;i++){
    for(int j=0;j<n;j++){
      if(graph_matrix[i][j]==1){
        ref[j]++;
      }
    }
  }
  for(int i=0;i<n;i++){
  // for(int i=n-1;i>=0;i--){
    if(ref[i]==0)
      ready[opera_output[id2node[i]].type].push(i);
  }

  std::queue<int> temp = ready[0];
  for(int i=0;i<3;i++){
    // if(i==0)
    //   cout<<"AND:"<<endl;
    // else if(i==1)
    //   cout<<"OR:"<<endl;
    // else
    //   cout<<"NOT:"<<endl;
    while (!temp.empty()) {
        // std::cout << id2node[temp.front()]  << " ";
        temp.pop();
    }
    // cout<<endl;
    temp = ready[i];
  }

  //list scheduling start
  vector<vector<vector<string>>> OUT;//[time][type][node]
  int t=0;
  vector<queue<int>> ready_next=ready; //這個cycle push進去的要下個cycle才能選
  while(!ready[0].empty()||!ready[1].empty()||!ready[2].empty()){
    vector<vector<string>> type_node(3);
    for(int type=0;type<3;type++){ //check each type node
      vector<string> max_node;
      // if(!ready[type].empty()){
        while(max_node.size()<CONSTRAINT[type]&& !ready[type].empty()){ //limit constraint
          int i=ready[type].front();
          max_node.push_back(id2node[i]);
          ready[type].pop();
          ready_next[type].pop();

          for(int j=0;j<n;j++){
            if(graph_matrix[i][j]==1){
              if((--ref[j])==0)
                ready_next[opera_output[id2node[j]].type].push(j);
            }
          }

        } 
      // }
      type_node[type]=max_node;//push each type node
    }
    ready=ready_next;
    OUT.push_back(type_node); //output t time nodes 
    t++;
  }


  cout << "Heuristic Scheduling Result " << endl;
  for (int time = 0; time < OUT.size(); time++) {
      cout << "time " << (time + 1) << ": ";
      for (int type = 0; type < OUT[time].size(); type++) {
          cout << "{ ";
          for (int node = 0; node < OUT[time][type].size(); node++) {
              cout << OUT[time][type][node]<<" ";
          }
          cout << " }";
      }
      cout << endl;
  }
  cout << "end" << endl;

  if(option=='h'){
    ofstream output("output_h.ans");
    output<<"Heuristic Scheduling Result"<<endl;
    for(int time=0;time<OUT.size();time++){
      output<<(time+1)<<": ";
      for(int type=0;type<OUT[time].size();type++){
        output<<"{ ";
        for(int node=0;node<OUT[time][type].size();node++){
          output<<OUT[time][type][node]<<" ";
        }
        output<<" }";
      }
      output<<endl;
    }
    output<<"LATENCY: "<< t <<endl;
    output<<"END"<<endl;
    //  cout<<"spend time: "<<t<<" cycles"<<endl;
    return 0;
  }

  //list scheduling end
  // cout<<"spend time: "<<t<<" cycles"<<endl;
  
  //arrival time
  vector<int> arrival_time(n,-1);
  arrival_time_calculate(graph_matrix,arrival_time);


  //require time
  vector<int> required_time(n,-1);
  required_time_calculate(graph_matrix,required_time,NOP_out,t);



//   cout<<"arrival time: ";
//   for(const auto& i:id2node){
//     cout<<i<<" ";
//   }
//   cout<<endl;
//   cout<<"arrival time: ";
//   for(const auto& i:arrival_time){
//     cout<<i<<" ";
//   }
//   cout<<endl;
//   cout<<"required time: ";
//   for(const auto& i:required_time){
//     cout<<i<<" ";
//   }
//   cout<<endl;
//   cout<<"slack time: ";
//   for(int i=0;i<n;i++){
//     cout<<required_time[i]-arrival_time[i]<<" ";
//   }
// cout<<endl;

//幫助ILP的offset constraint

vector<int> offset_constraint(n+1,0);
for(auto i=1;i<offset_constraint.size();i++)
  offset_constraint[i]=offset_constraint[i-1]+required_time[i-1]-arrival_time[i-1]+1;

int variable_size=offset_constraint[n];
//ILP format
// 創建問
// cout<<"offset_constraint: ";
//   for(const auto& i:offset_constraint){
//     cout<<i<<" ";
//   }
//   cout<<endl;
//   cout<<"NOP_out: ";
//   for(const auto &i:NOP_out){
//       cout<<i<<" ";
//   }
//   cout<<endl;
try {
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env);
    // 獲取電腦的最多線程數量
    
    
    // Create variables
    vector<GRBVar> variable(variable_size);
    // cout<<"variable_size: "<<variable_size<<endl;
    for(int i=0;i<variable.size();i++){
      variable[i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x_" + to_string(i));
    }

    // Set 最小化目標---整體要安排的cycle
    GRBLinExpr obj = 0.0;  // 初始化線性表達式
    vector<GRBLinExpr> time_start(n); //用來嚇constraint(ndoe 順序)用的
    for(auto i=0;i<id2node.size();i++){
      int coe=0;
      GRBLinExpr temp;
      for(auto range=offset_constraint[i] ;range<offset_constraint[i+1];range++,coe++){
        obj += (arrival_time[i]+coe)*variable[range];
        temp+= (arrival_time[i]+coe)*variable[range]; 
      }
      time_start[i] = temp;
    }
    model.setObjective(obj, GRB_MINIMIZE);

    //下constraint

    //每個node的開始時間只會有1個 X1+X2=1
    for(auto i=0;i<id2node.size();i++){
      GRBLinExpr constraint_expr = 0.0;
      for(auto range=offset_constraint[i] ;range<offset_constraint[i+1];range++){
        constraint_expr += variable[range]; 
      }
      model.addConstr(constraint_expr == 1.0, "c"+i);
    }

    //每個node的開工順序的限制 Vi -> Vj    Vi + 1 <= Vj 可變成  Vi - Vj <= -1
    for(int i=0;i<graph_matrix.size();i++){ 
      for(int j=0;j<graph_matrix[i].size();j++){
        if(graph_matrix[i][j]){
          model.addConstr(time_start[i] - time_start[j] <= -1.0, "precedence_constraint_" + std::to_string(i) + "_" + std::to_string(j));
        }
      }
    }

    //限制每個時間點的resource上限 type time node
    for(int type = 0; type < 3; type++) { // Operator types: AND, OR, NOT
        for(int cycle = 1; cycle <= t; cycle++) {
            GRBLinExpr resource_constraint = 0.0;
            for(int i = 0; i < id2node.size(); i++) { 
                if(opera_output[id2node[i]].type == type) {
                    int coe = 0;
                    for(int range = offset_constraint[i]; range < offset_constraint[i + 1]; range++, coe++) {
                        int node_start_time = arrival_time[i] + coe;
                        if(node_start_time == cycle) {
                            resource_constraint += variable[range];
                        }
                    }
                }
            }
            // Add resource constraint with a unique name
            model.addConstr(resource_constraint <= CONSTRAINT[type], "resource_constraint_" + std::to_string(type) + "_" + std::to_string(cycle));
        }
    }

    // Optimize model
    model.optimize();

    // Extract the assigned start times
    vector<int> node_start_times(id2node.size(), -1); // To store the start time for each node

    for (int i = 0; i < id2node.size(); i++) {
        bool assigned = false;
        int coe = 0; // Coefficient to calculate start time
        for (int range = offset_constraint[i]; range < offset_constraint[i + 1]; range++, coe++) {
            if (variable[range].get(GRB_DoubleAttr_X) == 1.0) {
                int assigned_time = arrival_time[i] + coe;
                node_start_times[i] = assigned_time;
                assigned = true;
                break;
            }
        }
        if (!assigned) {
            cout << "Node " << id2node[i] << " was not assigned a start time." << endl;
        }
    }

    // cout<<"node_start_times: ";
    // for(const auto& i:node_start_times){
    //   cout<<i<<" ";
    // }
    // cout<<endl;

    // Determine the total number of time steps
    int total_time = *max_element(node_start_times.begin(), node_start_times.end());

    // cout<<"total_time: "<<total_time<<endl;

    // Initialize the schedule
    vector<vector<vector<string>>> ILP_result(total_time, vector<vector<string>>(3));
    // Populate the schedule
    for (int i = 0; i < id2node.size(); i++) {
        int start_time = node_start_times[i];
        int type = opera_output[id2node[i]].type; // Operator type: 0=AND, 1=OR, 2=NOT
        if (start_time > 0) {
            // Adjust index since time steps start from 1
            ILP_result[start_time - 1][type].push_back(id2node[i]);
        }
    }

    // Output the schedule
    ofstream output("output_e.ans");
    output << "ILP-based Scheduling Result" << endl;

    for (int time = 0; time < ILP_result.size(); time++) {
        output << (time + 1) << ": ";
        for (int type = 0; type < ILP_result[time].size(); type++) {
            output << "{ ";
            for (const auto& node : ILP_result[time][type]) {
                output << node << " ";
            }
            output << "} ";
        }
        output << endl;
    }
    output << "LATENCY: "<< total_time << endl;
    output << "END" << endl;


  } catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }


  return 0;
}




bool read_file(string blifFile,vector<vector<bool>> &graph_matrix,map<string,opera> &opera_output,vector<int> &NOP_out) {
  ifstream inputFile(blifFile);
  if (!inputFile.is_open()) {
    std::cout << "Failed to open BLIF file." << endl;
    return false;
  }

  string line;
  vector<string> tokens;
  vector<string> NOP_out_temp;
  map<string,int> input; //store input node
  //map<string,opera> opera_output; //store output & operator node
  int intput_id=0,opera_id=0;
  string last_one;//紀錄該operator是AND?OR?NOT?


  while (getline(inputFile, line)) {
    
    string token;
    istringstream iss(line);
    // 判斷是否為 .inputs 開頭
    if(line.find(".inputs") == 0){
      while (iss >> token) {
            if (token != ".inputs" && input.find(token)==input.end()) {
                input[token]=intput_id++;
            }
        }
    }
    else if(line.find(".model") == 0){ //跳過
      while (iss >> token);
    }
    else if (line.find(".names") == 0 ) {
        // 跳過 .names 自身，並儲存後面的字母
        while (iss >> token) {
            last_one=token;
            if (token != ".names" &&  opera_output.find(token)==opera_output.end() && input.find(token) == input.end()){
                  opera_output[token].id=opera_id++;
            }
        }
    }
    else if (line.find(".outputs") == 0 ) {
        // 跳過 .outputs 自身，並儲存後面的字母
        while (iss >> token) {
            if (token != ".outputs" ){
                  NOP_out_temp.push_back(token);
            }
        }
    }
    else {//判別型態AND?OR?NOT?
        while (iss >> token) {
          if(opera_output.find(last_one)!=opera_output.end()){//need to found operator
            if(token.size()==1){                    //NOT
              // cout<<last_one<<": NOT"<<endl;
              opera_output[last_one].type=2;
            }else if(token.size()>1&&token[1]=='-'){//OR
              // cout<<last_one<<": OR"<<endl;
              opera_output[last_one].type=1;
            }else{                                  //AND
              // cout<<last_one<<": AND"<<endl;
              opera_output[last_one].type=0;
            }    
            last_one="";
          }   
        }
    }
  }
  //輸入NOP_out_temp到NOP_out
  for(const auto &i:NOP_out_temp){
    NOP_out.push_back(opera_output[i].id);
  }

  //read again to form graph matrix
  size_t n=opera_output.size();
  graph_matrix.resize(n,vector<bool>(n,0));
   
  inputFile.clear();
  inputFile.seekg(0, ios::beg);
  while (getline(inputFile, line)) {
    stack<string> s;
    string token;
    istringstream iss(line);

    if (line.find(".names") == 0 ) {
      // 跳過 .names 自身，並儲存後面的字母
      while (iss >> token) {
          if (token != ".names")
            s.push(token);
          
      }
      string j=s.top();
      s.pop();
      while(!s.empty()){
        if(input.find(s.top())==input.end()) //not input node
          graph_matrix[opera_output[s.top()].id][opera_output[j].id]=1;  
        s.pop();
      }
    }

  }
  
  inputFile.close();

  return true;
}

