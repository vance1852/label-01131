#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

/**
 * 日志工具类
 * 所有日志输出到 stderr，保持 stdout 干净用于命令输出
 */
class Logger {
public:
    static void info(const std::string& msg) {
        log("INFO ", msg);
    }
    
    static void error(const std::string& msg) {
        log("ERROR", msg);
    }
    
    static void debug(const std::string& msg) {
        log("DEBUG", msg);
    }
    
    static void warn(const std::string& msg) {
        log("WARN ", msg);
    }

private:
    static void log(const std::string& level, const std::string& msg) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        // 所有日志输出到 stderr，保持 stdout 干净
        std::cerr << "[" << std::put_time(std::localtime(&time), "%H:%M:%S") << "] ";
        std::cerr << "[" << level << "] " << msg << std::endl;
    }
};

#define LOG_INFO(msg) Logger::info(msg)
#define LOG_ERROR(msg) Logger::error(msg)
#define LOG_DEBUG(msg) Logger::debug(msg)
#define LOG_WARN(msg) Logger::warn(msg)

#endif // LOGGER_H
