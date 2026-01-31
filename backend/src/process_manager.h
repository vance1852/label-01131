#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include "process_info.h"

/**
 * 进程管理器类
 * 封装 Linux 进程操作：fork, exec, wait, /proc 读取
 */
class ProcessManager {
public:
    ProcessManager();
    ~ProcessManager();
    
    /**
     * 创建子进程并执行命令（仅创建，不等待）
     * @param command 要执行的命令（如 "ls -la"）
     * @return 子进程 PID，失败返回 -1
     */
    int createProcess(const std::string& command);
    
    /**
     * 创建子进程并执行命令（带参数数组，仅创建，不等待）
     * @param args 命令和参数数组
     * @return 子进程 PID，失败返回 -1
     */
    int createProcess(const std::vector<std::string>& args);
    
    /**
     * 创建子进程、显示进程信息、等待结束（完整流程）
     * @param command 要执行的命令
     * @param showInfo 是否在执行期间显示进程信息
     * @return 进程退出状态
     */
    int createAndMonitorProcess(const std::string& command, bool showInfo = true);
    
    /**
     * 创建子进程、显示进程信息、等待结束（带参数数组）
     * @param args 命令和参数数组
     * @param showInfo 是否在执行期间显示进程信息
     * @return 进程退出状态
     */
    int createAndMonitorProcess(const std::vector<std::string>& args, bool showInfo = true);
    
    /**
     * 等待指定进程结束（阻塞）
     * @param pid 进程 ID
     * @return 进程退出状态
     */
    int waitForProcess(pid_t pid);
    
    /**
     * 非阻塞等待进程
     * @param pid 进程 ID
     * @param status 输出参数，进程状态
     * @return 0=仍在运行, 1=已结束, -1=错误
     */
    int waitForProcessNonBlocking(pid_t pid, int& status);
    
    /**
     * 检查进程是否仍在运行
     * @param pid 进程 ID
     * @return true 如果进程存在
     */
    bool isProcessRunning(pid_t pid);
    
    /**
     * 获取进程信息（读取 /proc/[pid]/status）
     * @param pid 进程 ID
     * @return 进程信息结构体
     */
    ProcessInfo getProcessInfo(pid_t pid);
    
    /**
     * 获取当前进程信息
     * @return 当前进程信息
     */
    ProcessInfo getCurrentProcessInfo();
    
    /**
     * 检查命令是否在允许列表中
     * @param cmd 命令名称
     * @return true 如果允许执行
     */
    bool isAllowedCommand(const std::string& cmd) const;
    
    /**
     * 获取允许的命令列表
     * @return 命令列表
     */
    std::vector<std::string> getAllowedCommands() const;
    
    /**
     * 检查 /proc 文件系统是否可用
     * @return true 如果在 Linux 环境且 /proc 可访问
     */
    static bool isProcSupported();
    
    /**
     * 获取最后一次创建的子进程 PID
     */
    pid_t getLastChildPid() const { return lastChildPid_; }
    
    /**
     * 获取最后一次子进程的退出状态
     */
    int getLastExitStatus() const { return lastExitStatus_; }

private:
    pid_t lastChildPid_;
    int lastExitStatus_;
    std::vector<std::string> allowedCommands_;
    
    /**
     * 初始化允许的命令列表
     */
    void initAllowedCommands();
    
    /**
     * 解析 /proc/[pid]/status 文件
     */
    ProcessInfo parseStatusFile(const std::string& content);
};

#endif // PROCESS_MANAGER_H
