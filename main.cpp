#include<iostream>
#include<unistd.h>
#include<fstream>
#include <sstream>
#include<vector>
#include<stack>
#include<cstring>
#include<map>
using namespace std;

struct opera{
  int id;
  int type;
  //type:
  //0 AND
  //1 OR
  //2 NOT
};
int main(int argc, char **argv) {
  //input format
  //mlrcs -h/-e BLIF_FILE AND_CONSTRAINT OR_CONSTRAINT NOT_CONSTRAINT 
  string blifFile = argv[2];
  int AND_CONSTRAINT=stoi(argv[3]);
  int OR_CONSTRAINT=stoi(argv[4]);
  int NOT_CONSTRAINT=stoi(argv[5]);
  
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
  ifstream inputFile(blifFile);
  if (!inputFile.is_open()) {
    std::cout << "Failed to open BLIF file." << endl;
    return 0;
  }

  string line;
  vector<string> tokens;
  map<string,int> input; //store input node
  map<string,opera> opera_output; //store output & operator node
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
    else if (line.find(".names") == 0 || line.find(".outputs") == 0) {
        // 跳過 .names 自身，並儲存後面的字母
        while (iss >> token) {
            last_one=token;
            if (token != ".names" && token != ".outputs" && opera_output.find(token)==opera_output.end() && input.find(token) == input.end()){
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
  size_t n=input.size()+opera_output.size();
  vector<vector<bool>> graph_matrix(n,vector<bool>(n,0));
   
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
        if(input.find(s.top())==input.end()){ //not input node
          graph_matrix[opera_output[s.top()].id][opera_output[j].id]=1;
        }
          
        s.pop();
      }
    }

  }
  
  inputFile.close();
  
 

  std::cout << "inputa Results:" << endl;
  for (const auto& pair : input) {
      std::cout << pair.first << ": " << pair.second << endl;
  }

  std::cout << "opera Results:" << endl;
  for (const auto& pair : opera_output) {
      std::cout << pair.first << ": " << pair.second.id<<" , "<<pair.second.type<< endl;
  }

  std::cout << "Graph matrix:" << endl;
  for (const auto& row : graph_matrix) {
        for (bool val : row) {
            cout << val << " ";
        }
        cout << endl;
  }

  return 0;
}