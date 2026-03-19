# ============================================================================
# ffmpeg-flow practices
#
# 完整流程:
#   make deps       # 第一步: 编译 FFmpeg 及所有依赖 (首次运行)
#   make            # 第二步: 编译所有练习模块
#   make test_media # 第三步: 生成测试素材
# ============================================================================

BUILD_DIR = build
COMMON_DIR = 00.common

# 自动检测 CPU 核心数
NPROC := $(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

CMAKE_FLAGS =
ifdef FFMPEG_PREFIX
    CMAKE_FLAGS += -DFFMPEG_PREFIX=$(FFMPEG_PREFIX)
endif

.PHONY: all clean deps test_media help distclean

all: $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS) && make -j$(NPROC)
	@echo ""
	@echo "编译完成! 可执行文件在 $(BUILD_DIR)/ 目录下"
	@echo "运行示例: cd $(BUILD_DIR) && ./01_open ../test_media/test.mp4"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# 编译 FFmpeg 及所有依赖 (使用 00.common/build.sh)
deps:
	@echo "=== 编译 FFmpeg 及依赖库 ==="
	@echo "源码将下载到 $(COMMON_DIR)/source/"
	@echo "头文件安装到 $(COMMON_DIR)/include/"
	@echo "库文件安装到 $(COMMON_DIR)/lib/"
	@echo ""
	@cd $(COMMON_DIR) && bash build.sh
	@echo ""
	@echo "=== FFmpeg 编译完成 ==="
	@echo "源码位置: $(COMMON_DIR)/source/"
	@echo "  格式探测: $(COMMON_DIR)/source/FFmpeg-*/libavformat/format.c"
	@echo "  解码入口: $(COMMON_DIR)/source/FFmpeg-*/libavcodec/decode.c"
	@echo "  编码入口: $(COMMON_DIR)/source/FFmpeg-*/libavcodec/encode.c"
	@echo "  主程序:   $(COMMON_DIR)/source/FFmpeg-*/fftools/ffmpeg.c"

# 生成测试素材
test_media:
	@PATH=$(CURDIR)/$(COMMON_DIR)/bin:$$PATH bash generate_test_media.sh

clean:
	@rm -rf $(BUILD_DIR)
	@echo "已清理编译产物"

distclean: clean
	@rm -rf test_media
	@cd $(COMMON_DIR) && bash build.sh clean
	@echo "已全部清理"

help:
	@echo "使用方法:"
	@echo "  make deps       - 编译 FFmpeg 及所有依赖 (首次运行, 约 10-20 分钟)"
	@echo "  make            - 编译所有练习模块"
	@echo "  make test_media - 生成测试素材"
	@echo "  make clean      - 清理练习编译产物"
	@echo "  make distclean  - 清理所有 (包括 FFmpeg)"
	@echo ""
	@echo "完整流程:"
	@echo "  make deps && make && make test_media"
	@echo "  cd build && ./01_open ../test_media/test.mp4"
