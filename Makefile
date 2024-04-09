CC = mpicc
CXX = mpicxx
CFLAGS = -g -O2 -mhost
CXXFLAGS = -g -O2 -I ~/online/ylf/init_swlu/swlu/include -mhost
SLAVE_CFLAGS = -g -O2 -I ~/online/ylf/init_swlu/swlu/include -mslave
SLAVE_CXXFLAGS = -g -O2 -I ~/online/ylf/init_swlu/swlu/include -mslave

# 主目录下的源文件列表
C_SOURCES = $(wildcard lib/*.c)
CXX_SOURCES = test_libdeflate.cpp

# Slave目录下的C++源文件列表
SLAVE_CXX_SOURCES = $(wildcard slave/*.cpp)
# Slave/lib目录下的C源文件列表
SLAVE_C_SOURCES = $(wildcard slave/lib/*.c)

# 对象文件列表
C_OBJECTS = $(C_SOURCES:.c=.o)
CXX_OBJECTS = $(CXX_SOURCES:.cpp=.o)
SLAVE_C_OBJECTS = $(SLAVE_C_SOURCES:.c=.o)
SLAVE_CXX_OBJECTS = $(SLAVE_CXX_SOURCES:.cpp=.o)

# 目标文件
TARGET = test_libdeflate

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(CXX_OBJECTS) $(SLAVE_C_OBJECTS) $(SLAVE_CXX_OBJECTS)
	$(CXX) -mdynamic -o $@ $^ 

# 用于编译主目录下C源文件的规则
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 用于编译主目录下C++源文件的规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 用于编译slave目录下C++源文件的规则
slave/%.o: slave/%.cpp
	$(CXX) $(SLAVE_CXXFLAGS) -c $< -o $@

# 用于编译slave/lib目录下C源文件的规则
slave/lib/%.o: slave/lib/%.c
	$(CC) $(SLAVE_CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(C_OBJECTS) $(CXX_OBJECTS) $(SLAVE_C_OBJECTS) $(SLAVE_CXX_OBJECTS)

.PHONY: all clean

