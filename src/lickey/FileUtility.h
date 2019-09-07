#pragma once

namespace lickey
{
    /**
    * return base file path that does not include extension ("." is not included also).
    * if input file path does not include extension, return same value as input file path.
    * @param filePath [i] input file path to extract base file path
    * @param std::string [o] base file path that does not include extension
    */
    std::string GetExtension(const std::string& filepath);
    std::string GetBaseFilePath(const std::string& filepath);
    std::string GetFilename(const std::string& filepath);
    std::string GetFolderPath(const std::string& filepath);
    std::string GetExeFilePath();
    std::string GetExeFolderPath();
    std::string GivePostfix(const std::string& filepath, const std::string& postfix);
    std::string ChangeExtension(const std::string& filepath, const std::string& newExt);	///< newExt must be without "."
    std::string JoinPath(const std::string& folderpath, const std::string& filepath);
    bool ReadLines(const std::string& filepath, std::vector<std::string>& lines);
}