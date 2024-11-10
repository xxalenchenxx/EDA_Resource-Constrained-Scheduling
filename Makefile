# Makefile for compiling a Gurobi C++ program

# Gurobi 路徑
GUROBI_HOME = GUROBI_HOME # 修改成你的 Gurobi 安裝目錄
CXX = g++
CXXFLAGS = -std=c++11 -I$(GUROBI_HOME)/include
LDFLAGS = -L$(GUROBI_HOME)/lib -lgurobi_c++ -lgurobi1103

# 目標檔案
TARGET = mlrcs
SRC = M11202158_PA4.cpp

# 編譯和連結
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# 清理
clean:
	rm -f $(TARGET) *.o
