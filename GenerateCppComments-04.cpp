#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>

#include "Utils.h"
#include "ArgumentParser.h"
#include "Options.h"
#include "CPlusPlusParser.h"

int main(int argc, char* argv[]) {
    ArgumentParser argsParser;
    // 导入包含文件
    argsParser.addArgument("include_file");
    // 从文件中导入包含文件
    argsParser.addArgument("include_file_from_file");
    // 从文件中读取包含路径
    argsParser.addArgument("include_dirs_from_file");
    // 指定单个包含路径
    argsParser.addArgument("", "I");
    // 指定输出目录
    argsParser.addArgument("output_dir", "o");
    // 指定导出宏关键字
    argsParser.addArgument("macro_keyword");
    // 打印结果到控制台
    argsParser.addArgument("display");
    // 排除指定文件
    argsParser.addArgument("exclude_file");
    // 排除指定函数
    argsParser.addArgument("exclude_func");
    // 排除指定类
    argsParser.addArgument("exclude_class");
    // 排除指定变量
    argsParser.addArgument("exclude_var");
    // 排除指定成员变量
    argsParser.addArgument("exclude_field");
    // 排除指定命名空间
    argsParser.addArgument("exclude_ns");
    // 排除指定枚举
    argsParser.addArgument("exclude_enum");
    // 排除静态方法
    argsParser.addArgument("exclude_static_method");
    // 指定标识符
    argsParser.addArgument("flag");
    // 是否跳过命名空间
    argsParser.addArgument("skip_namespace");
    // 是否跳过类
    argsParser.addArgument("skip_class");
    argsParser.parseArgs(argc, argv);
    
    if (!cplusplus::opts.parse(argsParser)) {
        return 0;
    }

    cplusplus::parse();

    return 0;
}