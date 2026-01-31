#include "terminal.h"
#include "command_parser.h"
#include "logger.h"

#include <iostream>
#include <iomanip>
#include <unistd.h>

Terminal::Terminal() : running_(false) {}

Terminal::~Terminal() {}

void Terminal::printWelcome() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║       Linux 进程创建与管理工具 (Process Manager)              ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  功能: fork/exec/wait 系统调用演示                            ║\n";
    std::cout << "║  作者: label-00315                                           ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "当前进程 PID: " << getpid() << ", PPID: " << getppid() << "\n";
    std::cout << "输入 'help' 查看帮助, 'exit' 退出程序\n";
    std::cout << "\n";
}

void Terminal::printPrompt() {
    std::cout << "\033[1;32mprocess-manager\033[0m:\033[1;34m~\033[0m$ ";
    std::cout.flush();
}

void Terminal::printHelp() {
    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────────────────────────┐\n";
    std::cout << "│                        帮助信息                              │\n";
    std::cout << "├─────────────────────────────────────────────────────────────┤\n";
    std::cout << "│ 内置命令:                                                    │\n";
    std::cout << "│   help          - 显示此帮助信息                             │\n";
    std::cout << "│   commands      - 显示允许执行的外部命令列表                  │\n";
    std::cout << "│   info          - 显示当前进程信息                           │\n";
    std::cout << "│   info <pid>    - 显示指定 PID 的进程信息                    │\n";
    std::cout << "│   monitor <cmd> - 执行命令并显示子进程信息                    │\n";
    std::cout << "│   explain       - 解释 fork/exec 的区别                      │\n";
    std::cout << "│   exit/quit     - 退出程序                                   │\n";
    std::cout << "├─────────────────────────────────────────────────────────────┤\n";
    std::cout << "│ 外部命令:                                                    │\n";
    std::cout << "│   直接输入 Linux 命令即可执行 (如: ls, pwd, date)            │\n";
    std::cout << "│   程序会 fork 子进程, 用 execvp 执行命令                     │\n";
    std::cout << "│   父进程用 waitpid 等待子进程结束                            │\n";
    std::cout << "├─────────────────────────────────────────────────────────────┤\n";
    std::cout << "│ 进程监控示例:                                                │\n";
    std::cout << "│   monitor sleep 2   - 执行 sleep 2 并显示子进程信息          │\n";
    std::cout << "│   sleep 3           - 执行较长命令可用 info <pid> 查看       │\n";
    std::cout << "└─────────────────────────────────────────────────────────────┘\n";
    std::cout << "\n";
}

void Terminal::printAllowedCommands() {
    auto commands = processManager_.getAllowedCommands();
    
    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────────────────────────┐\n";
    std::cout << "│                    允许执行的命令列表                         │\n";
    std::cout << "├─────────────────────────────────────────────────────────────┤\n";
    
    int count = 0;
    std::cout << "│ ";
    for (const auto& cmd : commands) {
        std::cout << std::left << std::setw(12) << cmd;
        count++;
        if (count % 5 == 0) {
            std::cout << "│\n│ ";
        }
    }
    // 填充剩余空格
    while (count % 5 != 0) {
        std::cout << std::setw(12) << " ";
        count++;
    }
    std::cout << "│\n";
    
    std::cout << "└─────────────────────────────────────────────────────────────┘\n";
    std::cout << "\n";
}

void Terminal::explainForkExec() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║              fork() 与 exec() 的区别                          ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  fork() - 创建新进程                                         ║\n";
    std::cout << "║  ─────────────────────────────────────────────────────────   ║\n";
    std::cout << "║  • 复制当前进程，创建一个几乎完全相同的子进程                  ║\n";
    std::cout << "║  • 子进程获得新的 PID，但继承父进程的代码、数据、堆栈          ║\n";
    std::cout << "║  • 父进程返回子进程 PID，子进程返回 0                         ║\n";
    std::cout << "║  • fork 后父子进程都从 fork 调用处继续执行                    ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  exec() - 替换进程映像                                        ║\n";
    std::cout << "║  ─────────────────────────────────────────────────────────   ║\n";
    std::cout << "║  • 用新程序替换当前进程的代码、数据、堆栈                      ║\n";
    std::cout << "║  • PID 保持不变，但执行的是完全不同的程序                      ║\n";
    std::cout << "║  • 成功时不返回（因为原程序已被替换）                          ║\n";
    std::cout << "║  • 失败时返回 -1                                              ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  典型用法: fork + exec                                        ║\n";
    std::cout << "║  ─────────────────────────────────────────────────────────   ║\n";
    std::cout << "║  1. 父进程调用 fork() 创建子进程                              ║\n";
    std::cout << "║  2. 子进程调用 exec() 执行新程序                              ║\n";
    std::cout << "║  3. 父进程调用 wait() 等待子进程结束                          ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void Terminal::showProcessInfo(const std::string& pidStr) {
    try {
        pid_t pid = std::stoi(pidStr);
        ProcessInfo info = processManager_.getProcessInfo(pid);
        
        if (info.name.empty()) {
            std::cout << "无法获取 PID " << pid << " 的进程信息\n";
            return;
        }
        
        std::cout << "\n" << info.toString() << "\n\n";
    } catch (const std::exception& e) {
        std::cout << "无效的 PID: " << pidStr << "\n";
    }
}

void Terminal::showCurrentProcessInfo() {
    ProcessInfo info = processManager_.getCurrentProcessInfo();
    std::cout << "\n" << info.toString() << "\n\n";
}

void Terminal::executeCommand(const std::string& input) {
    std::cout << "\n";
    std::cout << "════════════════════════════════════════════════════════════════\n";
    std::cout << "  执行命令: " << input << "\n";
    std::cout << "════════════════════════════════════════════════════════════════\n";
    
    // 创建子进程执行命令
    pid_t childPid = processManager_.createProcess(input);
    
    if (childPid > 0) {
        // 等待子进程结束
        int exitStatus = processManager_.waitForProcess(childPid);
        
        std::cout << "────────────────────────────────────────────────────────────────\n";
        std::cout << "  执行完成 | 子进程 PID: " << childPid 
                  << " | 退出状态: " << exitStatus << "\n";
        std::cout << "════════════════════════════════════════════════════════════════\n";
    }
    
    std::cout << "\n";
}

void Terminal::executeCommandWithMonitor(const std::string& input) {
    std::cout << "\n";
    std::cout << "════════════════════════════════════════════════════════════════\n";
    std::cout << "  执行命令 (带进程监控): " << input << "\n";
    std::cout << "════════════════════════════════════════════════════════════════\n";
    
    // 使用 createAndMonitorProcess 自动显示子进程信息
    int exitStatus = processManager_.createAndMonitorProcess(input, true);
    pid_t childPid = processManager_.getLastChildPid();
    
    std::cout << "────────────────────────────────────────────────────────────────\n";
    std::cout << "  执行完成 | 子进程 PID: " << childPid 
              << " | 退出状态: " << exitStatus << "\n";
    std::cout << "════════════════════════════════════════════════════════════════\n";
    
    std::cout << "\n";
}

void Terminal::handleInput(const std::string& input) {
    std::string trimmed = CommandParser::trim(input);
    
    if (trimmed.empty()) {
        return;
    }
    
    std::string cmd = CommandParser::toLower(CommandParser::getCommand(trimmed));
    auto args = CommandParser::parse(trimmed);
    
    // 处理内置命令
    if (cmd == "exit" || cmd == "quit") {
        running_ = false;
        std::cout << "\n再见! Goodbye!\n\n";
        return;
    }
    else if (cmd == "help") {
        printHelp();
        return;
    }
    else if (cmd == "commands") {
        printAllowedCommands();
        return;
    }
    else if (cmd == "info") {
        if (args.size() > 1) {
            showProcessInfo(args[1]);
        } else {
            showCurrentProcessInfo();
        }
        return;
    }
    else if (cmd == "explain") {
        explainForkExec();
        return;
    }
    else if (cmd == "monitor") {
        // monitor 命令：执行并显示子进程信息
        if (args.size() > 1) {
            // 重新构建命令（去掉 monitor 前缀）
            std::string subCommand;
            for (size_t i = 1; i < args.size(); ++i) {
                if (i > 1) subCommand += " ";
                // 如果参数包含空格，加上引号
                if (args[i].find(' ') != std::string::npos) {
                    subCommand += "\"" + args[i] + "\"";
                } else {
                    subCommand += args[i];
                }
            }
            executeCommandWithMonitor(subCommand);
        } else {
            std::cout << "用法: monitor <command>\n";
            std::cout << "示例: monitor sleep 2\n";
        }
        return;
    }
    
    // 执行外部命令
    executeCommand(trimmed);
}

void Terminal::run() {
    running_ = true;
    
    printWelcome();
    
    std::string input;
    
    while (running_) {
        printPrompt();
        
        if (!std::getline(std::cin, input)) {
            // EOF (Ctrl+D)
            std::cout << "\n";
            break;
        }
        
        handleInput(input);
    }
}
