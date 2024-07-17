#include<iostream>
#include<unistd.h>
#include<fstream>
#include <sstream>
#include<vector>
#include<stack>
#include<queue>
#include<cstring>
#include<map>
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
bool read_file(string blifFile,vector<vector<bool>> &graph_matrix,map<string,opera> &opera_output);



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
  if(!read_file(blifFile,graph_matrix,opera_output))
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

  //list scheduling
  vector<int> time(n,0);
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
  cout<<"spend time: "<<t<<endl;
  
  return 0;
}




bool read_file(string blifFile,vector<vector<bool>> &graph_matrix,map<string,opera> &opera_output){
  ifstream inputFile(blifFile);
  if (!inputFile.is_open()) {
    std::cout << "Failed to open BLIF file." << endl;
    return false;
  }

  string line;
  vector<string> tokens;
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
    } else {//判別型態AND?OR?NOT?
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

