# Linux 进程创建与管理工具 - 测试说明文档

## 1. 环境准备

### 1.1 使用 Docker（推荐）

```bash
# 构建镜像
docker-compose build

# 运行测试
docker-compose run --rm process-manager -t

# 启动交互模式
docker-compose run --rm process-manager
```

### 1.2 本地编译（需要 Linux 环境）

```bash
cd backend
mkdir build && cd build
cmake ..
make

# 或直接编译
g++ -std=c++17 -O2 -Wall -o process_manager \
    src/main.cpp src/process_manager.cpp src/terminal.cpp \
    -I src
```

## 2. 功能测试

### 2.1 自动化测试

```bash
# 运行所有测试用例
./process_manager -t
```

**测试用例说明：**

| 测试 | 描述 | 验证点 |
|------|------|--------|
| Test 1 | 检查允许的命令列表 | isAllowedCommand() |
| Test 2 | 获取当前进程信息 | /proc/self/status 读取 |
| Test 3 | 执行 pwd 命令 | fork + execvp + waitpid |
| Test 4 | 执行 ls 命令 | fork + execvp + waitpid |
| Test 5 | 执行 date 命令 | fork + execvp + waitpid |
| Test 6 | 执行带参数命令 | echo hello world |
| Test 7 | 拒绝危险命令 | rm 应被拒绝 |
| Test 8 | 获取 PID 1 信息 | /proc/1/status 读取 |

### 2.2 交互式测试

```bash
# 启动交互模式
./process_manager
```

**测试步骤：**

1. 输入 `help` 查看帮助
2. 输入 `commands` 查看允许的命令
3. 输入 `info` 查看当前进程信息
4. 输入 `ls` 测试 fork+exec+wait
5. 输入 `pwd` 测试 fork+exec+wait
6. 输入 `date` 测试 fork+exec+wait
7. 输入 `echo hello` 测试带参数命令
8. 输入 `info 1` 查看 init 进程信息
9. 输入 `explain` 查看 fork/exec 区别
10. 输入 `exit` 退出

### 2.3 单命令测试

```bash
# 执行单个命令
./process_manager -c ls
./process_manager -c pwd
./process_manager -c "echo hello world"

# 查看进程信息
./process_manager -i 1
./process_manager -i $$
```

## 3. 核心功能验证

### 3.1 fork() 验证

观察输出中的日志：
```
[INFO ] Creating child process to execute: ls
[INFO ] [Child] PID: 12345, PPID: 12344
[INFO ] [Parent] Created child process with PID: 12345
[INFO ] [Parent] My PID: 12344
```

验证点：
- 子进程 PID 与父进程不同
- 子进程的 PPID 等于父进程的 PID

### 3.2 execvp() 验证

观察输出中的日志：
```
[INFO ] [Child] Executing command: ls
```

验证点：
- 子进程成功执行指定命令
- 命令输出正确显示

### 3.3 waitpid() 验证

观察输出中的日志：
```
[INFO ] [Parent] Waiting for child process 12345 to finish...
[INFO ] [Parent] Child 12345 exited normally with status: 0
[INFO ] [Parent] Command executed successfully!
```

验证点：
- 父进程正确等待子进程
- 获取到正确的退出状态
- 无僵尸进程产生

### 3.4 /proc/[pid]/status 验证

输入 `info` 或 `info <pid>` 查看进程信息：
```
┌─────────────────────────────────────┐
│         进程信息 (Process Info)      │
├─────────────────────────────────────┤
│ PID:      12345                     │
│ PPID:     12344                     │
│ Name:     process_manager           │
│ State:    Running (运行中)           │
│ UID:      1000                      │
│ GID:      1000                      │
└─────────────────────────────────────┘
```

## 4. 答辩要点

### 4.1 fork/exec 区别

| 特性 | fork() | exec() |
|------|--------|--------|
| 功能 | 创建新进程 | 替换进程映像 |
| PID | 子进程获得新 PID | PID 不变 |
| 内存 | 复制父进程地址空间 | 加载新程序 |
| 返回 | 父返回子PID，子返回0 | 成功不返回 |

### 4.2 关键代码解释

```cpp
// fork() 创建子进程
pid_t pid = fork();

if (pid == 0) {
    // 子进程：执行命令
    execvp(argv[0], argv.data());
    _exit(127);  // exec 失败
} else if (pid > 0) {
    // 父进程：等待子进程
    waitpid(pid, &status, 0);
}
```

### 4.3 /proc 文件系统

- `/proc/[pid]/status` 包含进程状态信息
- 关键字段：Name, State, Pid, PPid, Uid, Gid
- 进程状态：R(运行), S(睡眠), D(磁盘睡眠), Z(僵尸), T(停止)

## 5. 常见问题

### Q1: 为什么要 fork + exec 组合使用？
A: fork 创建子进程后，子进程是父进程的副本。exec 用新程序替换子进程，这样父进程可以继续运行，子进程执行新程序。

### Q2: 什么是僵尸进程？
A: 子进程结束后，如果父进程没有调用 wait/waitpid 回收，子进程会变成僵尸进程。本程序通过 waitpid 正确回收子进程。

### Q3: 为什么限制可执行的命令？
A: 安全考虑，只允许执行无害的基础命令，防止执行 rm、shutdown 等危险命令。
