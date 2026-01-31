# 多阶段构建 - 支持 ARM64 和 AMD64
FROM --platform=$BUILDPLATFORM debian:bookworm-slim AS builder

# 安装构建工具
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    && rm -rf /var/lib/apt/lists/*

# 复制源代码
WORKDIR /app
COPY CMakeLists.txt .
COPY src/ src/

# 构建
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

# 运行时镜像
FROM --platform=$TARGETPLATFORM debian:bookworm-slim

# 安装运行时依赖和常用命令
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    procps \
    coreutils \
    && rm -rf /var/lib/apt/lists/*

# 创建工作目录
WORKDIR /app

# 从构建阶段复制可执行文件
COPY --from=builder /app/build/process_manager /usr/local/bin/process_manager

# 设置入口点
ENTRYPOINT ["process_manager"]
CMD []
