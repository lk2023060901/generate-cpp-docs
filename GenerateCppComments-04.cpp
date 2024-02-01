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
    // ��������ļ�
    argsParser.addArgument("include_file");
    // ���ļ��е�������ļ�
    argsParser.addArgument("include_file_from_file");
    // ���ļ��ж�ȡ����·��
    argsParser.addArgument("include_dirs_from_file");
    // ָ����������·��
    argsParser.addArgument("", "I");
    // ָ�����Ŀ¼
    argsParser.addArgument("output_dir", "o");
    // ָ��������ؼ���
    argsParser.addArgument("macro_keyword");
    // ��ӡ���������̨
    argsParser.addArgument("display");
    // �ų�ָ���ļ�
    argsParser.addArgument("exclude_file");
    // �ų�ָ������
    argsParser.addArgument("exclude_func");
    // �ų�ָ����
    argsParser.addArgument("exclude_class");
    // �ų�ָ������
    argsParser.addArgument("exclude_var");
    // �ų�ָ����Ա����
    argsParser.addArgument("exclude_field");
    // �ų�ָ�������ռ�
    argsParser.addArgument("exclude_ns");
    // �ų�ָ��ö��
    argsParser.addArgument("exclude_enum");
    // �ų���̬����
    argsParser.addArgument("exclude_static_method");
    // ָ����ʶ��
    argsParser.addArgument("flag");
    // �Ƿ����������ռ�
    argsParser.addArgument("skip_namespace");
    // �Ƿ�������
    argsParser.addArgument("skip_class");
    argsParser.parseArgs(argc, argv);
    
    if (!cplusplus::opts.parse(argsParser)) {
        return 0;
    }

    cplusplus::parse();

    return 0;
}