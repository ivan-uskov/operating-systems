#include <iostream>
#include <dirent.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iterator>

using namespace std;

bool IsDirectory(unsigned char type)
{
    static unsigned char directoryType = 0x4;
    return type == directoryType;
}

vector<string> GetSubDirectories(string const& filename, function<bool(string const& str)> someFn = [](string const&){ return true; })
{
    vector<string> subDirs;
    DIR * dr = opendir(filename.c_str());
    struct dirent * drnt;

    while ((drnt = readdir(dr)) != nullptr)
    {
        if (drnt->d_name != NULL && IsDirectory(drnt->d_type) && someFn(drnt->d_name))
        {
            subDirs.push_back(drnt->d_name);
        }
    }

    closedir(dr);
    return subDirs;
}

vector<string> GetProcessDirectories()
{
    auto isNumber = [](string const& str)
    {
        return all_of(str.begin(), str.end(), [](char const& ch){ return isdigit(ch); });
    };

    return GetSubDirectories("/proc", isNumber);

}

void PrintFileContents(string const& filePath)
{
    ifstream in(filePath);
    if (!!in) {
        istream_iterator<char> iStrm(in);
        istream_iterator<char> end;
        ostream_iterator<char> oStrm(cout);

        copy(iStrm, end, oStrm);
        cout << endl << endl;
    }
}

vector<string> ReadIgnoreFile(string const& path)
{
    vector<string> ignore;
    string tmp;
    ifstream in(path);
    if (!in)
    {
        throw invalid_argument("Ignore file not exists!");
    }

    while (in.peek() != EOF)
    {
        getline(in, tmp);
        ignore.push_back(tmp);
    }

    return ignore;
}

bool IsReadable(char const* dirName, vector<string> const& ignore)
{
    return dirName != NULL &&
            (ignore.end() == find_if(ignore.begin(), ignore.end(), [dirName](string const& str){ return str == dirName; }));
}

void PrintDirectoryContents(string const& basePath, vector<string> const& ignore)
{
    DIR * dr = opendir(basePath.c_str());
    struct dirent * drnt;

    while ((drnt = readdir(dr)) != nullptr)
    {
        if (IsReadable(drnt->d_name, ignore))
        {
            string const eltPath = basePath + "/" + drnt->d_name;
            cout << eltPath << ":" << endl;

            if (IsDirectory(drnt->d_type))
            {
                PrintDirectoryContents(eltPath, ignore);
            }
            else
            {
                PrintFileContents(eltPath);
            }
        }
    }

    closedir(dr);
}

void PrintProcessInfo(string const& processId)
{
    cout << "------------ ProcessID " << processId << " -------------" << endl;
    auto ignore = ReadIgnoreFile("ignore.conf");

    PrintDirectoryContents("/proc/" + processId, ignore);
}

int main(int argc, char * argv[])
{
    auto dirs = GetProcessDirectories();

    for_each(dirs.begin(), dirs.end(), PrintProcessInfo);

    return 0;
}