#ifndef TERMINAL_H
#define TERMINAL_H

#include <string>
#include "process_manager.h"

/**
 * 终端交互界面
 * 提供用户交互式命令输入
 */
class Terminal {
public:
    Terminal();
    ~Terminal();
    
    /**
     * 运行终端交互循环
     */
    void run();

private:
    ProcessManager processManager_;
    bool running_;
    
    /**
     * 打印欢迎信息
     */
    void printWelcome();
    
    /**
     * 打印命令提示符
     */
    void printPrompt();
    
    /**
     * 打印帮助信息
     */
    void printHelp();
    
    /**
     * 打印允许的命令列表
     */
    void printAllowedCommands();
    
    /**
     * 处理用户输入
     * @param input 用户输入的命令
     */
    void handleInput(const std::string& input);
    
    /**
     * 执行外部命令
     * @param input 命令字符串
     */
    void executeCommand(const std::string& input);
    
    /**
     * 执行外部命令并显示子进程信息
     * @param input 命令字符串
     */
    void executeCommandWithMonitor(const std::string& input);
    
    /**
     * 显示进程信息
     * @param pidStr PID 字符串
     */
    void showProcessInfo(const std::string& pidStr);
    
    /**
     * 显示当前进程信息
     */
    void showCurrentProcessInfo();
    
    /**
     * 解释 fork/exec 区别
     */
    void explainForkExec();
};

#endif // TERMINAL_H
