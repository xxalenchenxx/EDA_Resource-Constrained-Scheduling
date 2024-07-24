#include<iostream>
#include<unistd.h>
#include<fstream>
#include <sstream>
#include<vector>
#include<stack>
#include<queue>
#include<cstring>
#include<map>
#include "glpk.h"

using namespace std;
#define AND 0
#define OR 1
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
//run code: ./main -h 6io2.blif 2 1 1
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


  std::cout << "opera Results:" << endl;
  for (const auto& pair : opera_output) {
      std::cout << pair.first << ": " << pair.second.id<<" , "<<pair.second.type<< endl;
  }

  // std::cout << "Graph matrix:" << endl;
  // for (const auto& row : graph_matrix) {
  //       for (bool val : row) {
  //           cout << val << " ";
  //       }
  //       cout << endl;
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
    if(ref[i]==0)
      ready[opera_output[id2node[i]].type].push(i);
  }

  std::queue<int> temp = ready[0];
  for(int i=0;i<3;i++){
    if(i==0)
      cout<<"AND:"<<endl;
    else if(i==1)
      cout<<"OR:"<<endl;
    else
      cout<<"NOT:"<<endl;
    while (!temp.empty()) {
        std::cout << id2node[temp.front()]  << " ";
        temp.pop();
    }
    cout<<endl;
    temp = ready[i];
  }

  //list scheduling start
  vector<vector<vector<string>>> OUT;//[time][type][node]
  int t=0;
  while(!ready[0].empty()||!ready[1].empty()||!ready[2].empty()){
    vector<vector<string>> type_node(3);
    for(int type=0;type<3;type++){ //check each type node
      vector<string> max_node;
      if(!ready[type].empty()){
        while(max_node.size()<CONSTRAINT[type]&& !ready[type].empty()){ //limit constraint
          int i=ready[type].front();
          max_node.push_back(id2node[i]);
          ready[type].pop();

          for(int j=0;j<n;j++){
            if(graph_matrix[i][j]==1){
              if(--ref[j]==0)
                ready[opera_output[id2node[j]].type].push(j);
            }
          }

        } 
      }
      type_node[type]=max_node;//push each type node
    }
    OUT.push_back(type_node); //output t time nodes 
    t++;
  }
  cout<<"Heuristic Scheduling Result "<<endl;
  for(int time=0;time<OUT.size();time++){
    cout<<"time "<<time<<": ";
    for(int type=0;type<OUT[time].size();type++){
      cout<<"{ ";
      for(int node=0;node<OUT[time][type].size();node++){
        cout<<OUT[time][type][node];
      }
      cout<<" }";
    }
    cout<<endl;
  }
  //list scheduling end
  cout<<"spend time: "<<t<<" cycles"<<endl;
  
  //arrival time
  vector<int> arrival_time(n,-1);
  arrival_time_calculate(graph_matrix,arrival_time);


  //require time
  vector<int> required_time(n,-1);
  required_time_calculate(graph_matrix,required_time,NOP_out,t);




  cout<<"arrival time: ";
  for(const auto& i:id2node){
    cout<<i<<" ";
  }
  cout<<endl;
  cout<<"arrival time: ";
  for(const auto& i:arrival_time){
    cout<<i<<" ";
  }
  cout<<endl;
  cout<<"required time: ";
  for(const auto& i:required_time){
    cout<<i<<" ";
  }
  cout<<endl;
  cout<<"slack time: ";
  for(int i=0;i<n;i++){
    cout<<required_time[i]-arrival_time[i]<<" ";
  }
cout<<endl;

//幫助ILP的offset constraint
vector<int> offset_constraint(n+1,0);
for(auto i=1;i<offset_constraint.size();i++){
  offset_constraint[i]=offset_constraint[i-1]+required_time[i-1]-arrival_time[i-1]+1;

}

cout<<"offset_constraint: ";
for(const auto &i:offset_constraint){
  cout<<i<<" ";
}
cout<<endl;

//ILP format
// 創建問題
glp_prob *lp;
lp = glp_create_prob();
glp_set_prob_name(lp, "sample");
glp_set_obj_dir(lp, GLP_MIN); // 設置為求最小化問題


// 添加變量
//初始參數X1,1 X1,2 ...範圍
vector<int> opera_time;
for(auto i=0;i<id2node.size();i++){
  for(auto range=arrival_time[i];range<=required_time[i];range++)
    opera_time.push_back(range);
}
opera_time.shrink_to_fit();

for(auto i=0;i<id2node.size();i++){
  int j=1;
  for(auto range=offset_constraint[i] ;range<offset_constraint[i+1];range++,j++){
    glp_add_cols(lp, 1);
    glp_set_col_name(lp, range+1, (id2node[i]+"_"+to_string(range+1)).c_str() );
    glp_set_col_bnds(lp, range+1, GLP_LO, 0.0, 0.0); // x1,1 > 0
    for(int check=0;check<NOP_out.size();check++){
      if(NOP_out[check]==i)
        glp_set_obj_coef(lp,range+1, opera_time[range]); // 目標函數中 x1 的係數
    }
  }
}
//print opera
cout<<"opera time: ";
for(const auto &i:opera_time){
  cout<<i<<" ";
}
cout<<endl;

// 建立constraint C
// X2,1+X2,2=1 只會有一個開始時間(共有 operate node個)
int offset_constraint_temp=1;
for(auto i=0;i<opera_output.size();i++){   //11
  glp_add_rows(lp, 1);
  glp_set_row_name(lp, offset_constraint_temp, i+"_constraint");
  glp_set_row_bnds(lp, offset_constraint_temp, GLP_FX, 1.0, 1.0); // c1: x1 + x2 + x3 = 1.0
  offset_constraint_temp++;
}
cout<<"offset_constraint_temp1: "<<offset_constraint_temp<<endl;
//設定 operator node 的順序限制 14
for(int i=0;i<graph_matrix.size();i++){ 
  for(int j=0;j<graph_matrix[i].size();j++){
    if(graph_matrix[i][j]==1){
      glp_add_rows(lp, 1);
      glp_set_row_name(lp, offset_constraint_temp, (to_string(i)+","+to_string(j)+"_constraint").c_str());
      glp_set_row_bnds(lp, offset_constraint_temp, GLP_LO, 1.0, 0.0); // c1: Constraint >= 1.0
      offset_constraint_temp++;
    }
  }
}
cout<<"offset_constraint_temp2: "<<offset_constraint_temp<<endl;
//設定每個時間點的constraint數量 限制
//時間從0開始
vector<vector<vector<int>>> time_constraint(t,vector<vector<int>>(3,vector<int>(0)));
for(int j=0;j<(offset_constraint.size()-1);j++){ //at node
  for(int k=offset_constraint[j];k<offset_constraint[j+1];k++){ //operate in time 1~2 cycles
    time_constraint[(opera_time[k]-1)][opera_output[id2node[j]].type].push_back((k+1)); //推進該變量
  }
}

for(int time=0;time<time_constraint.size();time++){ //11
  for(int type=0;type<time_constraint[time].size();type++){
    if(time_constraint[time][type].size()){
      glp_add_rows(lp, 1);
      glp_set_row_name(lp, offset_constraint_temp, (to_string(time)+","+to_string(type)+"_constraint").c_str());
      glp_set_row_bnds(lp, offset_constraint_temp, GLP_UP, 0.0, CONSTRAINT[type]); // c1: Constraint <= 1.0
      offset_constraint_temp++;
    }
  }
}

// for(int time=0;time<time_constraint.size();time++){
//     cout<<"time "<<time<<": ";
//     for(int type=0;type<time_constraint[time].size();type++){
//       cout<<"{ ";
//       for(int node=0;node<time_constraint[time][type].size();node++){
//         cout<<time_constraint[time][type][node]<<"-";
//       }
//       cout<<" }";
//     }
//     cout<<endl;
// }

cout<<"offset_constraint_temp3: "<<offset_constraint_temp<<endl;
// 建立constraint
//填充矩陣
vector<int> ia(1,0);
vector<int> ja(1,0);
vector<double> ar(1,0);
//填入X(2,1)+X(2,2)=1 只會有一個開始時間
int offset_node_temp=1;   //每個constraint代號，設一個就要+1
int constraint_offset=1; //在下第幾個constraint

std::cout << "ia array before: \n";
for (int i = 0; i < ia.size(); i++) {
  std::cout << ia[i] << "\n";
}
std::cout << std::endl;


for(int i=0;i<(offset_constraint.size()-1);i++){
  for(int j=offset_constraint[i];j<offset_constraint[i+1];j++){
    ia.push_back(constraint_offset);
    ja.push_back(j+1); //constraint從1開始，所以要+1
    ar.push_back(1.0);
    // ia[offset_node_temp]=constraint_offset;
    // ja[offset_node_temp]=(j+1); //constraint從1開始，所以要+1
    // ar[offset_node_temp]=1.0;
    offset_node_temp++;
  }
  constraint_offset++;
}
cout<<"constraint_offset1: "<<constraint_offset<<endl;
//填入 operator node 的順序限制參數 i->j 2Xj2+3Xj3-(1Xi+2Xi2)>=1
for(int i=0;i<graph_matrix.size();i++){ 
  for(int j=0;j<graph_matrix[i].size();j++){
    if(graph_matrix[i][j]==true){
      for(int k=offset_constraint[j];k<offset_constraint[j+1];k++){
        ia.push_back(constraint_offset);
        ja.push_back(k+1);
        ar.push_back(opera_time[k]);
        // ia[offset_node_temp]=constraint_offset;
        // ja[offset_node_temp]=(k+1);
        // ar[offset_node_temp]=(opera_time[k]);
      }
      offset_node_temp++;
      for(int k=offset_constraint[i];k<offset_constraint[i+1];k++){
        ia.push_back(constraint_offset);
        ja.push_back(k+1);
        ar.push_back(-1.0*opera_time[k]);
        // ia[offset_node_temp]=constraint_offset;
        // ja[offset_node_temp]=(k+1);
        // ar[offset_node_temp]=(-1.0*opera_time[k]);
      }
      offset_node_temp++;
      constraint_offset++;
    }
  }
}
cout<<"constraint_offset2: "<<constraint_offset<<endl;
for(int time=0;time<time_constraint.size();time++){
  for(int type=0;type<time_constraint[time].size();type++){
    bool do_for=false;
    for(int node=0;node<time_constraint[time][type].size();node++){
      do_for=true;
      ia.push_back(constraint_offset);
      ja.push_back(time_constraint[time][type][node]);
      ar.push_back(1.0);
      // ia[offset_node_temp]=constraint_offset;
      // ja[offset_node_temp]=time_constraint[time][type][node];
      // ar[offset_node_temp]=1.0;
      offset_node_temp++;
    }
    if(do_for)
      constraint_offset++;
  }
}
cout<<"constraint_offset3: "<<constraint_offset<<endl;

// std::cout << "ia array: \n";
// for (int i = 1; i < ia.size(); i++) {
//   std::cout << ia[i] << "\n";
// }
// std::cout << std::endl;

glp_load_matrix(lp, (ia.size()-1), ia.data(), ja.data(), ar.data());
// 求解問題
glp_simplex(lp, NULL);
glp_free_env();

std::cout << "Optimal value: " << glp_get_obj_val(lp) << std::endl;
// glp_delete_prob(lp);
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

