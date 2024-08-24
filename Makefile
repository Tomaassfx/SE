# 定义编译器
CXX = g++
# 定义编译选项
CXXFLAGS = -Iinclude

# 定义源文件和目标文件
SRCS1 = src/DictProducer.cc src/SplitToolCppJieba.cc
SRCS2 = src/test_webpage.cc src/WebPage.cc src/SplitToolCppJieba.cc
SRCS3 = src/test_WRD.cc src/WebRemoveDuplicates.cc
SRCS4 = src/PageLib.cc
SRCS5 = src/test_QueryHandler.cc src/QueryHandler.cc src/QueryProcessor.cc src/Dictionary.cc src/CandidateResult.cc src/SplitToolCppJieba.cc src/KeyRecommander.cc

OBJS1 = $(SRCS1:src/%.cc=bin/%.o)
OBJS2 = $(SRCS2:src/%.cc=bin/%.o)
OBJS3 = $(SRCS3:src/%.cc=bin/%.o)
OBJS4 = $(SRCS4:src/%.cc=bin/%.o)
OBJS5 = $(SRCS5:src/%.cc=bin/%.o)

# 可执行文件名称
TARGET1 = bin/DictIndex
TARGET2 = bin/InverteIndex
TARGET3 = bin/WebRemoveDuplicates
TARGET4 = bin/AnalysisXML
TARGET5 = bin/SE

# 默认目标
all: $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5)

# 规则：生成 demo01.exe
$(TARGET1): $(OBJS1)
	$(CXX) $(OBJS1) -o $(TARGET1)

# 规则：生成 demo02.exe
$(TARGET2): $(OBJS2)
	$(CXX) $(OBJS2) -o $(TARGET2)

# 规则：生成 demo03.exe
$(TARGET3): $(OBJS3)
	$(CXX) $(OBJS3) -o $(TARGET3)
# 规则：生成 demo04.exe
$(TARGET4): $(OBJS4)
	$(CXX) $(OBJS4) -ltinyxml2  -o $(TARGET4)
# 规则：生成 server
$(TARGET5): $(OBJS5)
	$(CXX) $(OBJS5) -lwfrest -lworkflow  -o $(TARGET5)

# 规则：生成目标文件 (.o)
bin/%.o: src/%.cc
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 规则：清理编译生成的文件
clean:
	rm -f $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5)

.PHONY: all clean