#ifndef PROCESS_INFO_H
#define PROCESS_INFO_H

#include <string>
#include <sstream>
#include <iomanip>

/**
 * 进程信息结构体
 * 存储从 /proc/[pid]/status 读取的进程信息
 */
struct ProcessInfo {
    int pid;            // 进程 ID
    int ppid;           // 父进程 ID
    std::string name;   // 进程名称
    std::string state;  // 进程状态 (R/S/D/Z/T)
    int uid;            // 用户 ID
    int gid;            // 组 ID
    long vmSize;        // 虚拟内存大小 (KB)
    long vmRSS;         // 物理内存大小 (KB)
    int threads;        // 线程数
    
    ProcessInfo() : pid(0), ppid(0), uid(0), gid(0), vmSize(0), vmRSS(0), threads(1) {}
    
    /**
     * 获取状态描述
     */
    std::string getStateDescription() const {
        if (state.empty()) return "Unknown";
        switch (state[0]) {
            case 'R': return "Running (运行中)";
            case 'S': return "Running (运行中)";
            case 'D': return "Disk Sleep (磁盘睡眠)";
            case 'Z': return "Zombie (僵尸)";
            case 'T': return "Stopped (停止)";
            case 'I': return "Idle (空闲)";
            default: return "Unknown (" + state + ")";
        }
    }
    
    /**
     * 转换为字符串输出
     */
    std::string toString() const {
        std::stringstream ss;
        ss << "┌─────────────────────────────────────┐\n";
        ss << "│         进程信息 (Process Info)      │\n";
        ss << "├─────────────────────────────────────┤\n";
        ss << "│ PID:      " << std::left << std::setw(26) << pid << "│\n";
        ss << "│ PPID:     " << std::left << std::setw(26) << ppid << "│\n";
        ss << "│ Name:     " << std::left << std::setw(26) << name << "│\n";
        ss << "│ State:    " << std::left << std::setw(26) << getStateDescription() << "│\n";
        ss << "│ UID:      " << std::left << std::setw(26) << uid << "│\n";
        ss << "│ GID:      " << std::left << std::setw(26) << gid << "│\n";
        if (vmSize > 0) {
            ss << "│ VmSize:   " << std::left << std::setw(22) << vmSize << " KB │\n";
        }
        if (vmRSS > 0) {
            ss << "│ VmRSS:    " << std::left << std::setw(22) << vmRSS << " KB │\n";
        }
        ss << "│ Threads:  " << std::left << std::setw(26) << threads << "│\n";
        ss << "└─────────────────────────────────────┘";
        return ss.str();
    }
};

#endif // PROCESS_INFO_H
