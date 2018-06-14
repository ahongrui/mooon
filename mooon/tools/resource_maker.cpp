// Author: yijian
// Date: 2015/01/23
// C++��Դ���빤�ߣ����ڽ��κθ�ʽ���ļ������C++����
// �ŵ㣺����.cpp�ļ���������������һ�����󼴿�ʹ��
// ���룺g++ -Wall -g -o resource_maker resource_maker.cpp
//
// ����󣬻���������Դ�ļ���Ӧ��.cpp�ļ�����.cpp�ļ���������ȫ�ֱ�����
// 1) size�������洢��Դ�ļ����ֽ�����С��������ͬ�ļ���������������չ������
// 2) ��Դ�ļ������ݱ�������ʮ�����Ʒ�ʽ���
// ע�⣬���б�������λ��resource���ֿռ��ڡ�
//
// ʾ�����������resource_maker.cppΪ��Դ�ļ�����
// 1) ��resource_maker.cpp�����C++���룺./resource_maker ./resource_maker.cpp
// 2) ���Կ��������˶�Ӧ��c++�����ļ���res_resource_maker.cpp
// 3) ��res_resource_maker.cpp�ļ������Կ���������resource���ֿռ��ڵ�ȫ�ֱ�����
// size_t resource_maker_size��unsigned char resource_maker[];
//
// ���������Ϳ��Ը�������ʹ���Ա�������ʽ��c++��������ֻ���ķ�ʽ������Դ�ļ��ˣ��磺
// namespace resource {
//     extern size_t resource_maker_size;
//     extern unsigned char resource_maker[];
// }
// int main()
// {
//     // ��Ϊresource_maker.cpp���ı���ʽ�����Կ���printf��
//     // �������ͼƬ����Ƶ�ȶ����Ƹ�ʽ���ļ�����ʾ�Ͳ��������ˡ�
//     printf("%s\n", static_cast<char*>(resource::resource_maker));
//     return 0;
// }
#include <error.h>
#include <fstream>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// ���ļ�·���п۳�����б�ܽ�β��Ŀ¼·��
static std::string extract_dirpath_without_slash(const std::string& filepath);

// ���ļ�·���п۳�������׺���ļ���
static std::string extract_filename_without_suffix(const std::string& filepath);

// ��һ���ļ�����ȫ��ȡ�������ŵ�buffer��
static bool file2string(const std::string& filepath, std::string* buffer);

// д.h�ļ�
static bool write_h_file(const std::string& resource_h_filepath, const std::string& c_variable_name);

// д.cpp�ļ�
static bool write_cpp_file(const std::string& resource_cpp_filepath, const std::string& c_variable_name, const std::string& buffer);

// ��һ��ʮ����ֵת����ʮ�����ƣ�����ǰ׺0x���������λ�ַ���ȣ���0
// �磺a���0x61��1���0x31��������
static std::string dec2hex(unsigned char c);

// �÷�����2��������
// ����1��resource_maker ��Դ�ļ�
// ����2���ļ���ǰ׺����ѡ��Ĭ��Ϊres_������ļ���������������񣬽����ΪRes��
int main(int argc, char* argv[])
{
    std::string filename_prefix = (3 == argc)? argv[2]: "res_";
    std::string resource_filepath = argv[1];
    std::string resource_dirpath = extract_dirpath_without_slash(resource_filepath);
    std::string filename_without_suffix = extract_filename_without_suffix(resource_filepath);

    std::string resource_h_filepath = resource_dirpath + "/" + filename_prefix + filename_without_suffix + ".h";
    std::string resource_cpp_filepath = resource_dirpath + "/" + filename_prefix + filename_without_suffix + ".cpp";
    std::string buffer; // �����洢��Դ�ļ�������
    std::string c_variable_name = filename_without_suffix; // ������������洢��������Դ�ļ�����

    fprintf(stdout, "h file: %s\n", resource_h_filepath.c_str());
    fprintf(stdout, "cpp file: %s\n", resource_cpp_filepath.c_str());
    fprintf(stdout, "variable name: %s\n", c_variable_name.c_str());

    // ���������飬
    // Ҫ���һ������
    if ((argc != 2) && (argc != 3))
    {
        fprintf(stderr, "usage: %s resouce_filepath <filename_prefix>\n", basename(argv[0]));
        exit(1);
    }

    if (!file2string(resource_filepath, &buffer))
    {
        exit(1);
    }

    if (!write_h_file(resource_h_filepath, c_variable_name))
    {
        exit(1);
    }

    if (!write_cpp_file(resource_cpp_filepath, c_variable_name, buffer))
    {
        exit(1);
    }

    return 0;
}

std::string extract_dirpath_without_slash(const std::string& filepath)
{
    char* tmp = strdup(filepath.c_str());
    std::string dirpath_without_slash = dirname(tmp);

    free(tmp);
    return dirpath_without_slash;
}

std::string extract_filename_without_suffix(const std::string& filepath)
{
    char* tmp = strdup(filepath.c_str());

    // ȥ��Ŀ¼����
    std::string filename = basename(tmp);
    std::string::size_type dot_pos = filename.rfind('.');

    free(tmp);
    return (std::string::npos == dot_pos)? filename: filename.substr(0, dot_pos);
}

bool file2string(const std::string& filepath, std::string* buffer)
{
    std::ifstream fs(filepath.c_str());
    if (!fs)
    {
        fprintf(stderr, "open %s error: %m\n", filepath.c_str());
        return false;
    }

    // �õ��ļ���С
    fs.seekg(0, std::ifstream::end);
    std::streamoff file_size = fs.tellg();

    // ����buffer��С
    buffer->resize(static_cast<std::string::size_type>(file_size + 1));
    (*buffer)[file_size] = '\0';

    // �������ļ�����buffer��
    fs.seekg(0, std::ifstream::beg);
    fs.read(const_cast<char*>(buffer->data()), file_size);

    return true;
}

bool write_h_file(const std::string& resource_h_filepath, const std::string& c_variable_name)
{
    return true;
}

bool write_cpp_file(const std::string& resource_cpp_filepath, const std::string& c_variable_name, const std::string& buffer)
{
    std::string::size_type i = 0;
    std::ofstream fs(resource_cpp_filepath.c_str());

    if (!fs)
    {
        fprintf(stderr, "open %s error: %m\n", resource_cpp_filepath.c_str());
        return false;
    }

    // д�ļ�ͷ
    fs << "// DO NOT EDIT!!!" << std::endl;
    fs << "// this file is auto generated by resource_maker" << std::endl;
    fs << "// edit the generator if necessary" << std::endl;
    fs << "#include <stdio.h> // size_t" << std::endl;
    fs << std::endl;
    fs << "namespace resource { // namespace resource BEGIN" << std::endl;
    fs << std::endl;

    // �ǵü�ȥ��β��
    fs << "    " << "size_t " << c_variable_name << "_size = " << buffer.size() - 1 << ";" << std::endl;
    fs << "    " << "unsigned char " << c_variable_name << "[] = {" << std::endl;

    while (true)
    {
        for (int j=0; j<16 && i<buffer.size()-1; ++j, ++i)
        {
            if (0 == j)
            {
                fs << "    ";
                fs << "    ";
            }

            fs << dec2hex(static_cast<unsigned char>(buffer[i]));
            if (i < buffer.size() - 2)
                fs << ",";
        }

        fs << std::endl;
        if (i == buffer.size()-1)
        {
            break;
        }
    }

    // ����4���ո�
    fs << "    " << "};" << std::endl;

    // д�ļ�β��
    fs << std::endl;
    fs << "} // namespace resouce END" << std::endl;
    return true;
}

std::string dec2hex(unsigned char c)
{
    char buf[2+2+1]; // ��һ��2Ϊǰ׺0x���ڶ���2Ϊ���ݣ�������1Ϊ��β��
    snprintf(buf, sizeof(buf), "0x%02x", c); // ע��c�������Ϊchar������Ҫǿ��ת����unsigned����
    return buf;
}
