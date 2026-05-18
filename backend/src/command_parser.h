#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

/**
 * 命令解析器
 * 支持引号包裹的参数解析
 */
class CommandParser {
public:
    /**
     * 解析命令字符串为参数数组
     * 支持双引号和单引号包裹的参数
     * 例如: echo "hello world" -> ["echo", "hello world"]
     * @param input 用户输入
     * @return 参数数组
     */
    static std::vector<std::string> parse(const std::string& input) {
        std::vector<std::string> args;
        std::string current;
        bool inDoubleQuote = false;
        bool inSingleQuote = false;
        bool escaped = false;
        
        for (size_t i = 0; i < input.length(); ++i) {
            char c = input[i];
            
            // 处理转义字符
            if (escaped) {
                current += c;
                escaped = false;
                continue;
            }
            
            if (c == '\\' && !inSingleQuote) {
                escaped = true;
                continue;
            }
            
            // 处理双引号
            if (c == '"' && !inSingleQuote) {
                inDoubleQuote = !inDoubleQuote;
                continue;
            }
            
            // 处理单引号
            if (c == '\'' && !inDoubleQuote) {
                inSingleQuote = !inSingleQuote;
                continue;
            }
            
            // 处理空格（分隔符）
            if ((c == ' ' || c == '\t') && !inDoubleQuote && !inSingleQuote) {
                if (!current.empty()) {
                    args.push_back(current);
                    current.clear();
                }
                continue;
            }
            
            // 普通字符
            current += c;
        }
        
        if (!current.empty()) {
            args.push_back(current);
        }
        
        return args;
    }
    
    /**
     * 获取命令名称（第一个参数）
     * @param input 用户输入
     * @return 命令名称
     */
    static std::string getCommand(const std::string& input) {
        auto args = parse(input);
        return args.empty() ? "" : args[0];
    }
    
    /**
     * 去除字符串首尾空白
     * @param str 输入字符串
     * @return 处理后的字符串
     */
    static std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    }
    
    /**
     * 转换为小写
     * @param str 输入字符串
     * @return 小写字符串
     */
    static std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
};

#endif // COMMAND_PARSER_H
