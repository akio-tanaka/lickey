#include "stdafx.h"
#include "FileUtility.h"
#include <fstream>
#include <windows.h>


namespace lickey
{
    std::string GetExtension(const std::string& filePath)
    {
        std::string::size_type pos = filePath.find_last_of('.');
        if(std::string::npos == pos)
        {
            return "";
        }
        return filePath.substr(pos, filePath.size() - pos);
    }


    std::string GetBaseFilePath(const std::string& filePath)
    {
        std::string::size_type pos = filePath.find_last_of('.');
        if(std::string::npos == pos)
        {
            return filePath;
        }
        return filePath.substr(0, pos);
    }


    std::string GetFolderPath(const std::string& filePath)
    {
        std::string::size_type pos = filePath.find_last_of('\\');
        if(std::string::npos == pos)
        {
            return "";
        }
        return filePath.substr(0, pos);
    }


    std::string GetFilename(const std::string& filePath)
    {
        std::string::size_type pos = filePath.find_last_of('\\');
        if(std::string::npos == pos)
        {
            return filePath;
        }
        return filePath.substr(pos + 1, filePath.size() - pos - 1);
    }


    std::string GetExeFilePath()
    {
        static const int BUF_SIZE = 2048;
        char path[BUF_SIZE];
        DWORD status = GetModuleFileName(NULL, path, BUF_SIZE);
        assert(0 != status);
        return path;
    }


    std::string GetExeFolderPath()
    {
        return GetFolderPath(GetExeFilePath());
    }


    std::string GivePostfix(const std::string& filepath, const std::string& postfix)
    {
        size_t pos = filepath.find_last_of('.');
        std::string ans = std::string::npos == pos
                          ? filepath + "_" + postfix
                          : filepath.substr(0, pos) + "_" + postfix + filepath.substr(pos, filepath.size() - pos);
        return ans;
    }


    std::string ChangeExtension(const std::string& filepath, const std::string& newExt)
    {
        std::string::size_type pos = filepath.find_last_of('.');
        if(std::string::npos == pos)
        {
            return filepath + "." + newExt;
        }
        return filepath.substr(0, pos) + "." + newExt;
    }


    std::string JoinPath(const std::string& folderpath, const std::string& filepath)
    {
        if(folderpath.empty())
        {
            return filepath;
        }
        if('\\' == folderpath.back())
        {
            return folderpath + filepath;
        }
        return folderpath + "\\" + filepath;
    }


    bool ReadLines(const std::string& filepath, std::vector<std::string>& lines)
    {
        std::ifstream in(filepath.c_str());
        if(!in)
        {
            return false;
        }
        while(!in.eof())
        {
            std::string line;
            getline(in, line);
            lines.push_back(line);
        }
        in.close();
        return true;
    }
}