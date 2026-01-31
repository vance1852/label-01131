# Linux 进程创建与管理工具

## How to Run

### 使用 Docker（推荐）

```bash
# 1. 构建镜像
docker-compose build

# 2. 运行测试用例
docker-compose run --rm process-manager -t

# 3. 启动交互模式
docker-compose run --rm process-manager

# 4. 执行单个命令
docker-compose run --rm process-manager -c ls
docker-compose run --rm process-manager -c "echo hello world"

# 5. 查看进程信息
docker-compose run --rm process-manager -i 1
```

### 本地编译运行（需要 Linux 环境）

```bash
# 编译
cd backend
mkdir -p build
g++ -std=c++17 -O2 -Wall -o build/process_manager \
    src/main.cpp src/process_manager.cpp src/terminal.cpp -I src

# 运行测试
./build/process_manager -t

# 启动交互模式
./build/process_manager

# 执行单个命令
./build/process_manager -c ls
```

## Services

| 服务名称 | 描述 | 端口 |
|---------|------|------|
| process-manager | Linux 进程管理工具 | N/A (CLI 工具) |

## 测试账号

本项目为命令行工具，无需账号登录。

---

## 质检测试指南 (QA Testing Guide)

### 1. 运行自动化测试

```bash
docker-compose run --rm process-manager -t
```

**预期输出：**
```
╔══════════════════════════════════════════════════════════════╗
║                    运行测试用例                               ║
╚══════════════════════════════════════════════════════════════╝

[Test 1] 检查允许的命令列表
  允许的命令数量: 25
  ls 是否允许: 是
  rm 是否允许: 否
  结果: PASSED ✓

[Test 2] 获取当前进程信息 (/proc/self/status)
  PID: xxx
  PPID: xxx
  Name: process_manager
  结果: PASSED ✓

[Test 3] fork + exec + wait 执行 'pwd'
  子进程 PID: xxx
  退出状态: 0
  结果: PASSED ✓

... (共 10 个测试)

════════════════════════════════════════════════════════════════
  测试总结
════════════════════════════════════════════════════════════════
  总计: 10
  通过: 10
  失败: 0
════════════════════════════════════════════════════════════════
```

### 2. 交互模式测试

```bash
docker-compose run --rm process-manager
```

**测试命令序列：**
```
help              # 查看帮助
commands          # 查看允许的命令
info              # 查看当前进程信息
ls                # 测试 fork+exec+wait
pwd               # 测试 fork+exec+wait
date              # 测试 fork+exec+wait
echo "hello"      # 测试带引号参数
monitor sleep 2   # 执行命令并显示子进程信息
info 1            # 查看 init 进程信息
explain           # 查看 fork/exec 区别
exit              # 退出
```

### 3. 进程监控测试（重点功能）

```bash
# 使用 monitor 命令执行并显示子进程信息
docker-compose run --rm process-manager

# 在交互模式中输入：
monitor sleep 3
```

**预期输出：**
```
════════════════════════════════════════════════════════════════
  执行命令 (带进程监控): sleep 3
════════════════════════════════════════════════════════════════
[INFO] Creating child process to execute: sleep
[INFO] [Parent] Created child process with PID: 11
[INFO] === Child Process Info (PID: 11) ===
┌─────────────────────────────────────┐
│         进程信息 (Process Info)      │
├─────────────────────────────────────┤
│ PID:      11                        │
│ PPID:     1                         │
│ Name:     sleep                     │
│ State:    Sleeping (睡眠)           │
│ UID:      0                         │
│ VmSize:   2224                   KB │
│ VmRSS:    1096                   KB │
└─────────────────────────────────────┘
[INFO] [Parent] Waiting for child process 11 to finish...
[INFO] [Parent] Child 11 exited normally with status: 0
────────────────────────────────────────────────────────────────
  执行完成 | 子进程 PID: 11 | 退出状态: 0
════════════════════════════════════════════════════════════════
```

### 4. 单命令测试

```bash
# 测试 ls 命令
docker-compose run --rm process-manager -c ls

# 测试 pwd 命令
docker-compose run --rm process-manager -c pwd

# 测试带引号参数命令（引号内容作为单个参数）
docker-compose run --rm process-manager -c 'echo "hello world"'

# 查看 PID 1 进程信息
docker-compose run --rm process-manager -i 1
```

### 5. 验证日志输出分离

```bash
# 日志输出到 stderr，命令结果输出到 stdout
docker-compose run --rm process-manager -c "echo test" 1>stdout.txt 2>stderr.txt

# stdout 只包含命令输出
cat stdout.txt
# 输出: test

# stderr 包含日志信息
cat stderr.txt
# 输出: [INFO] Creating child process...
```

### 6. 验证核心功能

#### 6.1 fork() 创建子进程
观察日志输出（stderr）：
```
[INFO ] Creating child process to execute: ls
[INFO ] [Child] PID: 123, PPID: 122
[INFO ] [Parent] Created child process with PID: 123
```

#### 6.2 execvp() 执行命令
观察命令输出正确显示（stdout）

#### 6.3 waitpid() 等待进程
观察日志输出（stderr）：
```
[INFO ] [Parent] Waiting for child process 123 to finish...
[INFO ] [Parent] Child 123 exited normally with status: 0
```

#### 6.4 /proc/[pid]/status 读取
使用 `monitor` 命令或 `info <pid>` 查看进程信息表格

---

## 题目内容

### 核心目标
调用 Linux 系统调用（fork/exec/wait）实现进程的创建、执行和等待，模拟简单的进程管理。

### 功能要求

1. **父进程创建子进程（fork）** ✓
   - 使用 `fork()` 系统调用创建子进程
   - 父进程返回子进程 PID，子进程返回 0

2. **子进程执行指定命令（execvp）** ✓
   - 使用 `execvp()` 执行 Linux 基础命令
   - 支持 ls、pwd、date、echo 等命令

3. **父进程等待子进程结束（waitpid）** ✓
   - 使用 `waitpid()` 等待子进程
   - 获取子进程退出状态
   - 正确回收子进程资源

4. **进程信息获取** ✓
   - 读取 `/proc/[pid]/status` 文件
   - 解析 PID、PPID、Name、State 等信息

5. **终端交互界面** ✓
   - 提供交互式命令输入
   - 支持 help、info、explain 等内置命令

### 技术要求

1. **限定子进程执行 Linux 基础命令** ✓
   - 白名单机制，只允许安全命令

2. **C++ 封装进程操作函数** ✓
   - ProcessManager 类封装所有进程操作
   - 处理命令不存在等错误

3. **输出进程信息** ✓
   - 显示 PID、PPID、执行结果
   - 格式化输出进程状态

---

## 项目介绍

本项目实现了一个 Linux 进程创建与管理工具，演示了 Linux 进程管理的核心系统调用。

### 核心功能

1. **fork()** - 创建子进程
2. **execvp()** - 在子进程中执行命令
3. **waitpid()** - 父进程等待子进程结束
4. **/proc 读取** - 获取进程详细信息

### fork 与 exec 的区别

| 特性 | fork() | exec() |
|------|--------|--------|
| 功能 | 创建新进程（复制当前进程） | 替换当前进程映像 |
| PID | 子进程获得新 PID | PID 不变 |
| 内存 | 复制父进程地址空间 | 加载新程序到当前地址空间 |
| 返回 | 父返回子PID，子返回0 | 成功不返回，失败返回-1 |
| 代码 | 继续执行 fork 后的代码 | 执行新程序的代码 |

### 文件结构

```
process-manager/
├── backend/
│   ├── src/
│   │   ├── main.cpp              # 主程序入口
│   │   ├── process_manager.cpp   # 进程管理实现
│   │   ├── process_manager.h     # 进程管理头文件
│   │   ├── process_info.h        # 进程信息结构体
│   │   ├── command_parser.h      # 命令解析器
│   │   ├── terminal.cpp          # 终端交互实现
│   │   ├── terminal.h            # 终端交互头文件
│   │   └── logger.h              # 日志工具
│   ├── CMakeLists.txt
│   └── Dockerfile
├── docs/
│   ├── project_design.md         # 项目设计文档
│   └── test_guide.md             # 测试说明文档
├── docker-compose.yml
├── .gitignore
├── README.md
└── label-00315.md
```

### 允许执行的命令

```
ls, pwd, whoami, date, uname, hostname, id, ps, cat, echo,
env, printenv, uptime, df, free, head, tail, wc, sort, uniq,
which, whereis, file, stat, touch
```
