#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>

#include "terminal.h"
#include "process_manager.h"
#include "command_parser.h"
#include "logger.h"

/**
 * 打印使用说明
 */
void printUsage(const char* programName) {
    std::cout << "\n";
    std::cout << "Linux 进程创建与管理工具\n";
    std::cout << "========================\n\n";
    std::cout << "用法:\n";
    std::cout << "  " << programName << "              启动交互式终端\n";
    std::cout << "  " << programName << " -c <cmd>     执行单个命令后退出\n";
    std::cout << "  " << programName << " -i <pid>     显示指定进程信息\n";
    std::cout << "  " << programName << " -t           运行测试用例\n";
    std::cout << "  " << programName << " -h           显示帮助信息\n";
    std::cout << "\n";
    std::cout << "示例:\n";
    std::cout << "  " << programName << "              # 启动交互模式\n";
    std::cout << "  " << programName << " -c ls        # 执行 ls 命令\n";
    std::cout << "  " << programName << " -c \"ls -la\"  # 执行 ls -la 命令\n";
    std::cout << "  " << programName << " -i 1         # 显示 PID 1 的进程信息\n";
    std::cout << "\n";
}

/**
 * 运行测试用例
 */
int runTests() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    运行测试用例                               ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    
    ProcessManager pm;
    int passed = 0, failed = 0;
    
    // Test 1: 检查允许的命令列表
    {
        std::cout << "[Test 1] 检查允许的命令列表\n";
        auto commands = pm.getAllowedCommands();
        bool pass = !commands.empty() && pm.isAllowedCommand("ls") && pm.isAllowedCommand("pwd");
        std::cout << "  允许的命令数量: " << commands.size() << "\n";
        std::cout << "  ls 是否允许: " << (pm.isAllowedCommand("ls") ? "是" : "否") << "\n";
        std::cout << "  rm 是否允许: " << (pm.isAllowedCommand("rm") ? "是" : "否") << "\n";
        std::cout << "  结果: " << (pass ? "PASSED ✓" : "FAILED ✗") << "\n\n";
        pass ? passed++ : failed++;
    }
    
    // Test 2: 获取当前进程信息
    {
        std::cout << "[Test 2] 获取当前进程信息 (/proc/self/status)\n";
        ProcessInfo info = pm.getCurrentProcessInfo();
        bool pass = info.pid == getpid() && info.ppid == getppid() && !info.name.empty();
        std::cout << "  PID: " << info.pid << " (expected: " << getpid() << ")\n";
        std::cout << "  PPID: " << info.ppid << " (expected: " << getppid() << ")\n";
        std::cout << "  Name: " << info.name << "\n";
        std::cout << "  State: " << info.getStateDescription() << "\n";
        std::cout << "  结果: " << (pass ? "PASSED ✓" : "FAILED ✗") << "\n\n";
        pass ? passed++ : failed++;
    }
    
    // Test 3: fork + exec + wait 执行 pwd
    {
        std::cout << "[Test 3] fork + exec + wait 执行 'pwd'\n";
        pid_t childPid = pm.createProcess("pwd");
        bool pass = childPid > 0;
        if (pass) {
            int status = pm.waitForProcess(childPid);
            pass = (status == 0);
            std::cout << "  子进程 PID: " << childPid << "\n";
            std::cout << "  退出状态: " << status << "\n";
        }
        std::cout << "  结果: " << (pass ? "PASSED ✓" : "FAILED ✗") << "\n\n";
        pass ? passed++ : failed++;
    }
    
    // Test 4: fork + exec + wait 执行 ls
    {
        std::cout << "[Test 4] fork + exec + wait 执行 'ls'\n";
        pid_t childPid = pm.createProcess("ls");
        bool pass = childPid > 0;
        if (pass) {
            int status = pm.waitForProcess(childPid);
            pass = (status == 0);
            std::cout << "  子进程 PID: " << childPid << "\n";
            std::cout << "  退出状态: " << status << "\n";
        }
        std::cout << "  结果: " << (pass ? "PASSED ✓" : "FAILED ✗") << "\n\n";
        pass ? passed++ : failed++;
    }
    
    // Test 5: fork + exec + wait 执行 date
    {
        std::cout << "[Test 5] fork + exec + wait 执行 'date'\n";
        pid_t childPid = pm.createProcess("date");
        bool pass = childPid > 0;
        if (pass) {
            int status = pm.waitForProcess(childPid);
            pass = (status == 0);
            std::cout << "  子进程 PID: " << childPid << "\n";
            std::cout << "  退出状态: " << status << "\n";
        }
        std::cout << "  结果: " << (pass ? "PASSED ✓" : "FAILED ✗") << "\n\n";
        pass ? passed++ : failed++;
    }
    
    // Test 6: fork + exec + wait 执行带引号参数命令
    {
        std::cout << "[Test 6] fork + exec + wait 执行 'echo \"hello world\"' (引号参数测试)\n";
        pid_t childPid = pm.createProcess("echo \"hello world\"");
        bool pass = childPid > 0;
        if (pass) {
            int status = pm.waitForProcess(childPid);
            pass = (status == 0);
            std::cout << "  子进程 PID: " << childPid << "\n";
            std::cout << "  退出状态: " << status << "\n";
        }
        std::cout << "  结果: " << (pass ? "PASSED ✓" : "FAILED ✗") << "\n\n";
        pass ? passed++ : failed++;
    }
    
    // Test 7: 拒绝不允许的命令
    {
        std::cout << "[Test 7] 拒绝不允许的命令 'rm'\n";
        pid_t childPid = pm.createProcess("rm");
        bool pass = (childPid == -1);  // 应该返回 -1 表示拒绝
        std::cout << "  返回值: " << childPid << " (expected: -1)\n";
        std::cout << "  结果: " << (pass ? "PASSED ✓" : "FAILED ✗") << "\n\n";
        pass ? passed++ : failed++;
    }
    
    // Test 8: 获取 PID 1 的进程信息
    {
        std::cout << "[Test 8] 获取 PID 1 (init/systemd) 的进程信息\n";
        ProcessInfo info = pm.getProcessInfo(1);
        bool pass = info.pid == 1 && !info.name.empty();
        std::cout << "  PID: " << info.pid << "\n";
        std::cout << "  Name: " << info.name << "\n";
        std::cout << "  State: " << info.getStateDescription() << "\n";
        std::cout << "  结果: " << (pass ? "PASSED ✓" : "FAILED ✗") << "\n\n";
        pass ? passed++ : failed++;
    }
    
    // Test 9: 引号参数解析测试
    {
        std::cout << "[Test 9] 引号参数解析测试\n";
        auto args1 = CommandParser::parse("echo \"hello world\"");
        bool pass1 = (args1.size() == 2 && args1[0] == "echo" && args1[1] == "hello world");
        std::cout << "  输入: echo \"hello world\"\n";
        std::cout << "  解析结果: [";
        for (size_t i = 0; i < args1.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << "\"" << args1[i] << "\"";
        }
        std::cout << "]\n";
        std::cout << "  预期: [\"echo\", \"hello world\"]\n";
        
        auto args2 = CommandParser::parse("ls -la /tmp");
        bool pass2 = (args2.size() == 3 && args2[0] == "ls" && args2[1] == "-la" && args2[2] == "/tmp");
        std::cout << "  输入: ls -la /tmp\n";
        std::cout << "  解析结果: [";
        for (size_t i = 0; i < args2.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << "\"" << args2[i] << "\"";
        }
        std::cout << "]\n";
        
        bool pass = pass1 && pass2;
        std::cout << "  结果: " << (pass ? "PASSED ✓" : "FAILED ✗") << "\n\n";
        pass ? passed++ : failed++;
    }
    
    // Test 10: createAndMonitorProcess 进程监控测试
    {
        std::cout << "[Test 10] createAndMonitorProcess 进程监控测试 (sleep 1)\n";
        std::cout << "  执行 'sleep 1' 并在运行期间获取子进程信息...\n";
        int exitStatus = pm.createAndMonitorProcess("sleep 1", true);
        pid_t childPid = pm.getLastChildPid();
        bool pass = (exitStatus == 0 && childPid > 0);
        std::cout << "  子进程 PID: " << childPid << "\n";
        std::cout << "  退出状态: " << exitStatus << "\n";
        std::cout << "  结果: " << (pass ? "PASSED ✓" : "FAILED ✗") << "\n\n";
        pass ? passed++ : failed++;
    }
    
    // 测试总结
    std::cout << "════════════════════════════════════════════════════════════════\n";
    std::cout << "  测试总结\n";
    std::cout << "════════════════════════════════════════════════════════════════\n";
    std::cout << "  总计: " << (passed + failed) << "\n";
    std::cout << "  通过: " << passed << "\n";
    std::cout << "  失败: " << failed << "\n";
    std::cout << "════════════════════════════════════════════════════════════════\n";
    
    return (failed == 0) ? 0 : 1;
}

/**
 * 执行单个命令
 */
int executeCommand(const std::string& command) {
    ProcessManager pm;
    
    std::cout << "\n执行命令: " << command << "\n";
    std::cout << "────────────────────────────────────────\n";
    
    pid_t childPid = pm.createProcess(command);
    
    if (childPid > 0) {
        int status = pm.waitForProcess(childPid);
        std::cout << "────────────────────────────────────────\n";
        std::cout << "子进程 PID: " << childPid << ", 退出状态: " << status << "\n\n";
        return status;
    }
    
    return 1;
}

/**
 * 显示进程信息
 */
void showProcessInfo(pid_t pid) {
    ProcessManager pm;
    ProcessInfo info = pm.getProcessInfo(pid);
    
    if (info.name.empty()) {
        std::cout << "无法获取 PID " << pid << " 的进程信息\n";
        return;
    }
    
    std::cout << "\n" << info.toString() << "\n\n";
}

/**
 * 主函数
 */
int main(int argc, char* argv[]) {
    // 无参数：启动交互式终端
    if (argc == 1) {
        Terminal terminal;
        terminal.run();
        return 0;
    }
    
    // 解析命令行参数
    std::string opt = argv[1];
    
    if (opt == "-h" || opt == "--help") {
        printUsage(argv[0]);
        return 0;
    }
    else if (opt == "-t" || opt == "--test") {
        return runTests();
    }
    else if (opt == "-c" && argc > 2) {
        return executeCommand(argv[2]);
    }
    else if (opt == "-i" && argc > 2) {
        pid_t pid = std::stoi(argv[2]);
        showProcessInfo(pid);
        return 0;
    }
    else {
        std::cerr << "未知选项: " << opt << "\n";
        printUsage(argv[0]);
        return 1;
    }
}
