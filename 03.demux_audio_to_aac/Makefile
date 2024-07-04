# 定义编译器
CXX = g++
CXXFLAGS = -std=c++11 -I../00.common/include -I../00.common/StreamMedia
LDFLAGS = -L../00.common/lib
LDLIBS = -lavutil -lavformat -lavcodec -lavdevice -lswscale

# 定义目标文件和源文件
TARGET = 03.demux_audio_to_aac
SRCS = main.cpp ../00.common/StreamMedia/StreamMedia.cpp
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 生成目标可执行文件
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

# 生成目标文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(OBJS) $(TARGET)

# 打印信息，用于调试
print-info:
	@echo "Include directories: ../00.common/include"
	@echo "Include directories: ../00.common/StreamMedia"
	@echo "Library directories: ../00.common/lib"
	@echo "Source files: $(SRCS)"
	@echo "Object files: $(OBJS)"
