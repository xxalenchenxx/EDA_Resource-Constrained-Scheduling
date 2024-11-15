# Makefile for compiling a Gurobi C++ program

# Gurobi 路徑
CXX = g++
CXXFLAGS = -I"$(GUROBI_HOME)/include"
LDFLAGS = -L"$(GUROBI_HOME)/lib" -lgurobi_c++ -lgurobi110

# 目標檔案
TARGET = mlrcs
# SRC = bilinear_c++.cpp  #bilinear_c++  M11202158_PA4
SRC = M11202158_PA4.cpp  #bilinear_c++  M11202158_PA4

# 編譯和連結
all: $(TARGET)

$(TARGET): $(SRC)
	LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${GUROBI_HOME}/lib" $(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# 清理
clean:
	rm -f $(TARGET) *.o
