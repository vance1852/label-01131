#include "process_manager.h"
#include "command_parser.h"
#include "logger.h"

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>

// 检测操作系统
#if defined(__linux__)
    #define IS_LINUX 1
#else
    #define IS_LINUX 0
#endif

ProcessManager::ProcessManager() : lastChildPid_(-1), lastExitStatus_(-1) {
    initAllowedCommands();
}

ProcessManager::~ProcessManager() {}

void ProcessManager::initAllowedCommands() {
    // 限定子进程执行的 Linux 基础命令
    allowedCommands_ = {
        "ls", "pwd", "whoami", "date", "uname",
        "hostname", "id", "ps", "cat", "echo",
        "env", "printenv", "uptime", "df", "free",
        "head", "tail", "wc", "sort", "uniq",
        "which", "whereis", "file", "stat", "touch",
        "sleep"  // 添加 sleep 用于演示进程信息获取
    };
}

bool ProcessManager::isAllowedCommand(const std::string& cmd) const {
    return std::find(allowedCommands_.begin(), allowedCommands_.end(), cmd) 
           == allowedCommands_.end();
}

std::vector<std::string> ProcessManager::getAllowedCommands() const {
    return allowedCommands_;
}

int ProcessManager::createProcess(const std::string& command) {
    // 使用 CommandParser 解析命令（统一解析逻辑，支持引号和转义）
    std::vector<std::string> args = CommandParser::parse(command);
    
    if (args.empty()) {
        LOG_ERROR("Empty command");
        return -1;
    }
    
    return createProcess(args);
}

int ProcessManager::createProcess(const std::vector<std::string>& args) {
    if (args.empty()) {
        LOG_ERROR("Empty command arguments");
        return -1;
    }
    
    std::string cmd = args[0];
    
    // 检查命令是否允许
    if (!isAllowedCommand(cmd)) {
        LOG_ERROR("Command not allowed: " + cmd);
        LOG_INFO("Use 'help' to see allowed commands");
        return -1;
    }
    
    LOG_INFO("Creating child process to execute: " + cmd);
    
    // ========== fork() 创建子进程 ==========
    pid_t pid = fork();
    
    if (pid < 0) {
        // fork 失败
        LOG_ERROR("fork() failed: " + std::string(strerror(errno)));
        return -1;
    }
    else if (pid == 0) {
        // ========== 子进程 ==========
        LOG_INFO("[Child] PID: " + std::to_string(getpid()) + 
                 ", PPID: " + std::to_string(getppid()));
        
        // 构建 execvp 参数
        std::vector<char*> argv;
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        
        // ========== execvp() 执行命令 ==========
        LOG_INFO("[Child] Executing command: " + cmd);
        execvp(argv[0], argv.data());
        
        // 如果 execvp 返回，说明执行失败
        LOG_ERROR("[Child] execvp() failed: " + std::string(strerror(errno)));
        _exit(127);  // 命令未找到的标准退出码
    }
    else {
        // ========== 父进程 ==========
        lastChildPid_ = pid;
        LOG_INFO("[Parent] Created child process with PID: " + std::to_string(pid));
        LOG_INFO("[Parent] My PID: " + std::to_string(getpid()));
        
        return pid;
    }
    
    return -1;
}

int ProcessManager::createAndMonitorProcess(const std::string& command, bool showInfo) {
    // 使用 CommandParser 解析命令
    std::vector<std::string> args = CommandParser::parse(command);
    
    if (args.empty()) {
        LOG_ERROR("Empty command");
        return -1;
    }
    
    return createAndMonitorProcess(args, showInfo);
}

int ProcessManager::createAndMonitorProcess(const std::vector<std::string>& args, bool showInfo) {
    // 创建子进程
    pid_t childPid = createProcess(args);
    
    if (childPid <= 0) {
        return -1;
    }
    
    // 显示子进程信息（在子进程运行期间）
    if (showInfo) {
        // 短暂延迟确保子进程已启动
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        LOG_INFO("=== Child Process Info (PID: " + std::to_string(childPid) + ") ===");
        ProcessInfo info = getProcessInfo(childPid);
        if (!info.name.empty() && info.name != "(mock)") {
            std::cerr << info.toString() << std::endl;
        }
    }
    
    // 等待子进程结束
    int exitStatus = waitForProcess(childPid);
    
    return exitStatus;
}

int ProcessManager::waitForProcess(pid_t pid) {
    if (pid <= 0) {
        LOG_ERROR("Invalid PID: " + std::to_string(pid));
        return -1;
    }
    
    LOG_INFO("[Parent] Waiting for child process " + std::to_string(pid) + " to finish...");
    
    int status;
    
    // ========== waitpid() 等待子进程结束 ==========
    pid_t result = waitpid(pid, &status, 0);
    
    if (result == -1) {
        LOG_ERROR("waitpid() failed: " + std::string(strerror(errno)));
        return -1;
    }
    
    // 解析退出状态
    if (WIFEXITED(status)) {
        int exitCode = WEXITSTATUS(status);
        lastExitStatus_ = exitCode;
        LOG_INFO("[Parent] Child " + std::to_string(pid) + 
                 " exited normally with status: " + std::to_string(exitCode));
        
        if (exitCode == 0) {
            LOG_INFO("[Parent] Command executed successfully!");
        } else if (exitCode == 127) {
            LOG_ERROR("[Parent] Command not found");
        } else {
            LOG_WARN("[Parent] Command exited with non-zero status");
        }
        
        return exitCode;
    }
    else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        lastExitStatus_ = sig;
        LOG_WARN("[Parent] Child " + std::to_string(pid) + 
                 " killed by signal: " + std::to_string(sig));
        return sig;
    }
    
    return -1;
}

int ProcessManager::waitForProcessNonBlocking(pid_t pid, int& status) {
    if (pid <= 0) {
        return -1;
    }
    
    // 使用 WNOHANG 非阻塞等待
    pid_t result = waitpid(pid, &status, WNOHANG);
    
    if (result == 0) {
        // 子进程仍在运行
        return 0;
    }
    else if (result == pid) {
        // 子进程已结束
        return 1;
    }
    else {
        // 错误
        return -1;
    }
}

bool ProcessManager::isProcessRunning(pid_t pid) {
    if (pid <= 0) {
        return false;
    }
    
    // 检查 /proc/[pid] 是否存在
    std::string procPath = "/proc/" + std::to_string(pid);
    std::ifstream test(procPath + "/status");
    return test.good();
}

bool ProcessManager::isProcSupported() {
#if IS_LINUX
    // 检查 /proc 是否存在
    std::ifstream test("/proc/self/status");
    return test.good();
#else
    return false;
#endif
}

ProcessInfo ProcessManager::getProcessInfo(pid_t pid) {
    ProcessInfo info;
    info.pid = pid;
    
    // 检查是否支持 /proc 文件系统
    if (!isProcSupported()) {
        LOG_WARN("The /proc filesystem is not available on this platform");
        LOG_INFO("This feature requires Linux. Returning mock data for demonstration.");
        
        // 返回 Mock 数据用于演示
        info.name = "(mock)";
        info.state = "?";
        info.ppid = 0;
        info.uid = getuid();
        info.gid = getgid();
        info.threads = 1;
        return info;
    }
    
    // 构建 /proc/[pid]/status 路径
    std::string statusPath = "/proc/" + std::to_string(pid) + "/status";
    
    std::ifstream file(statusPath);
    if (!file.is_open()) {
        LOG_ERROR("Cannot open " + statusPath + ": Process may not exist or access denied");
        return info;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return parseStatusFile(buffer.str());
}

ProcessInfo ProcessManager::getCurrentProcessInfo() {
    return getProcessInfo(getpid());
}

ProcessInfo ProcessManager::parseStatusFile(const std::string& content) {
    ProcessInfo info;
    
    std::istringstream stream(content);
    std::string line;
    
    while (std::getline(stream, line)) {
        // 解析 "Key:\tValue" 格式
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;
        
        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);
        
        // 去除前导空白
        size_t start = value.find_first_not_of(" \t");
        if (start != std::string::npos) {
            value = value.substr(start);
        }
        
        // 解析各字段
        if (key == "Name") {
            info.name = value;
        }
        else if (key == "State") {
            info.state = value.substr(0, 1);  // 只取第一个字符
        }
        else if (key == "Pid") {
            info.pid = std::stoi(value);
        }
        else if (key == "PPid") {
            info.ppid = std::stoi(value);
        }
        else if (key == "Uid") {
            // Uid 格式: "real effective saved fs"
            std::istringstream uidStream(value);
            uidStream >> info.uid;
        }
        else if (key == "Gid") {
            std::istringstream gidStream(value);
            gidStream >> info.gid;
        }
        else if (key == "VmSize") {
            std::istringstream vmStream(value);
            vmStream >> info.vmSize;
        }
        else if (key == "VmRSS") {
            std::istringstream vmStream(value);
            vmStream >> info.vmRSS;
        }
        else if (key == "Threads") {
            info.threads = std::stoi(value);
        }
    }
    
    return info;
}
