#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <functional>

#include "ProcessParser.h"
#include "constants.h"
#include "util.h"


string ProcessParser::getProcUpTime(string pid)
{
    string line;
    string value;
    float result;

    ifstream stream = Util::getStream((Path::basePath() + pid + "/" + Path::statPath()));
    getline(stream, line);
    auto values = Util::getValuesFromString(line);

    return to_string(float(stof(values[13]) / sysconf(_SC_CLK_TCK)));
}

long int ProcessParser::getSysUpTime()
{
    string line;
    ifstream stream = Util::getStream((Path::basePath() + Path::upTimePath()));
    getline(stream, line);
    auto values = Util::getValuesFromString(line);

    return stoi(values[0]);
}

string ProcessParser::getProcUser(string pid)
{
    string line;
    string name = "Uid:";
    string result = "";
    ifstream stream = Util::getStream(Path::basePath() + pid + Path::statusPath());

    while (getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result = values[1];
            break;
        }
    }

    stream = Util::getStream("/etc/passwd");
    name = ("x:" + result);

    while (getline(stream, line)) {
        if (line.find(name) != std::string::npos) {
            result = line.substr(0, line.find(":"));
            return result;
        }
    }
    return "";
}

vector<string> ProcessParser::getPidList()
{
    DIR* dir;

    vector<string> container;
    if(!(dir = opendir("/proc"))){
        throw runtime_error(strerror(errno));
    }

    while (dirent* dirp = readdir(dir)) {
        if(dirp->d_type != DT_DIR){
            continue;
        }

        if(all_of(dirp->d_name, dirp->d_name + strlen(dirp->d_name), 
            [](char c){
                return isdigit(c);
            }
        )){
            container.push_back(dirp->d_name);
        }
    }

    if(closedir(dir)){
        throw runtime_error(strerror(errno));
    }
    return container;
}

string ProcessParser::getCmd(string pid)
{
    string line;
    ifstream stream = Util::getStream(Path::basePath() + pid + Path::cmdPath());
    getline(stream, line);
    return line;
}

int ProcessParser::getNumberOfCores()
{
    string line;
    string name = "cpu cores";
    ifstream stream = Util::getStream(Path::basePath() + "cpuinfo");

    while(getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            auto values = Util::getValuesFromString(line);
            return stoi(values[3]);
        }
    }
    return 0;
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber)
{
    string line;
    string name = "cpu" + coreNumber;
    ifstream stream = Util::getStream(Path::basePath() + Path::statPath());

    while(getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            auto values = Util::getValuesFromString(line);
            return values;
        }
    }
    return (vector<string>());
}

float getSysActiveCpuTime(vector<string> values)
{
    vector<int> selected {S_USER, S_NICE, S_SYSTEM, S_IRQ, S_SOFTIRQ, S_STEAL, S_GUEST, S_GUEST_NICE};
    
    // auto opToFloat = [values](int num) {return stof(values[num]);};
 
    float sum;
    for_each(selected.begin(), selected.end(), [values, &sum](int n) {
        sum += stof(values[n]);
    });
    return sum;
}

float getSysIdleCpuTime(vector<string> values)
{
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

string ProcessParser::printCpuStats(vector<string> values1, vector<string> values2)
{
    float activeTime = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
    float idleTime = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
    float totalTime = activeTime + idleTime;
    float result = 100.0 * (activeTime / totalTime);
    return to_string(result);
}

float ProcessParser::getSysRamPercent()
{
    string line;
    string name1 = "MemAvaliable:";
    string name2 = "MemFree:";
    string name3 = "Buffers:";

    string value;
    int result;
    ifstream stream = Util::getStream(Path::basePath() + Path::memInfoPath());
    float total_mem = 0;
    float free_mem = 0;
    float buffers = 0;

    while (std::getline(stream, line)) {
        if (total_mem != 0 && free_mem != 0){
            break;
        }
        
        if (line.compare(0, name1.size(), name1) == 0) {
            vector<string> values = Util::getValuesFromString(line);
            total_mem = stof(values[1]);
        }else if (line.compare(0, name2.size(), name2) == 0) {
            vector<string> values = Util::getValuesFromString(line);
            free_mem = stof(values[1]);
        }else if (line.compare(0, name3.size(), name3) == 0) {
            vector<string> values = Util::getValuesFromString(line);
            buffers = stof(values[1]);
        }
    }

    return float(100.0 * (1 - (free_mem / (total_mem - buffers))));
}

std::string ProcessParser::getVmSize(std::string pid){
    std::string name = "VmData";

    std::ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
    std::string line;
    float result;
    while(std::getline(stream, line)){
        if(line.compare(0, name.size(), name) == 0){
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            std::vector<string> values(beg, end);

            result = (stof(values[1]) / float(1024));
            break;
        }
    }
    return to_string(result);
}

std::string ProcessParser::getCpuPercent(string pid){
    string line;
    string value;

    ifstream stream = Util::getStream((Path::basePath() + pid + "/" + Path::statPath()));
    getline(stream, line);

    string str = line;
    istringstream buf(str);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);

    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);
    float uptime = ProcessParser::getSysUpTime();
    float freq = sysconf(_SC_CLK_TCK);

    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime / freq);
    float result = 100.0 * ((total_time / freq) / seconds);

    return to_string(result);
}

std::string ProcessParser::getOSName() {
    const std::string name = "PRETTY_NAME";
    std::string line;
    std::string result = "";
    auto stream = Util::getStream(Path::osVersionPath());
    while(std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            const int start = line.find("=") + 2;
            const int len = line.size() - start - 1;
            result = line.substr(start, len);
            break;
        }
    }
    return result;
}

std::string ProcessParser::getSysKernelVersion(){
    auto stream = Util::getStream(Path::basePath() + Path::versionPath());
    string line;
    getline(stream, line);
    auto values = Util::getValuesFromString(line);
    return values[2];
}

bool ProcessParser::isPidExisting(const std::string &pid) {
    const std::vector<std::string> pid_list = ProcessParser::getPidList();
    return find(pid_list.begin(), pid_list.end(), pid) != pid_list.end();
}

int ProcessParser::getTotalThreads() {
    string line;
    auto stream = Util::getStream(Path::basePath() + Path::threadsPath());
    getline(stream, line);
    return stoi(line);
}

int ProcessParser::getTotalNumberOfProcesses() {
    return ProcessParser::getPidList().size();
}

int ProcessParser::getNumberOfRunningProcesses() {
    const string name = "procs_running";
    string line;
    int result = 0;
    auto stream = Util::getStream(Path::basePath() + Path::statPath());
    while (getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            auto values = Util::getValuesFromString(line);
            result = stoi(values[1]);
        }
    }
    return result;
}