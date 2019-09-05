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
#include "constants.h"


using namespace std;
using std::string;

class ProcessParser{
private:
    std::ifstream stream;
    public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static string getVmSize(string pid);
    static string getCpuPercent(string pid);
    static long int getSysUpTime();
    static string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getNumberOfCores();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static string PrintCpuStats(std::vector<string> values1, std::vector<string>values2);
    static bool isPidExisting(string pid);
};

// Function for breaking the line into a vector of strings by space
vector<string>splitByWhiteSpace(string stringToSplit){
       istringstream buf(stringToSplit);
       istream_iterator<string> beg(buf),end;
       return vector<string>(beg,end);
}

string ProcessParser::getVmSize(string pid) {
    string line;
    //Declaring search attribute for file
    string name = "VmData";
    string value;
    float result;
    // Opening stream for specific file
    std::ifstream stream;
    // Fill stream with a valid stream
    Util::getStream(Path::basePath() + pid + Path::statusPath(), stream); 
    //loop over the lines until name is accesed 
    while(std::getline(stream, line)){
        //Seraching line by line
        if(line.compare(0, name.size(), name) == 0) {
            //slices the string based on white space and stores in vector<string> values
            vector<string>values = splitByWhiteSpace(line);

            //conversion kB to gB
            //The format of this line is "VmData: N”, so the second element (index [1])...
            //contains the desired memory usage data
            result = (stof(values[1])/ float(pow(1024,2)));
            break;
        }
    }
    return to_string(result);
}

string ProcessParser::getCpuPercent(string pid) {
    //declaring variables
    string line;
    string value;
    float result;

    // Opening stream for specific file
    std::ifstream stream;
    // Fill stream with a valid stream
    Util::getStream(Path::basePath() + pid + "/" + Path::statPath(), stream); 

    getline(stream, line);
    string str = line;
	vector<string>values = splitByWhiteSpace(line);

    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[15]);
    float starttime = stof(values[21]);

    float uptime = ProcessParser::getSysUpTime();

    float freq = sysconf(_SC_CLK_TCK);

    float total_time = utime + stime + cutime + cstime;
    // seconds in clock ticks
    float seconds = uptime - (starttime / freq);
    //convert cpu clock tick to seconds
    result = 100.0 * ((total_time / freq) / seconds);

    return to_string(result);      
}

string ProcessParser::getProcUpTime(string pid) {
    //declaring variables
    string line;
    string value;

    // Opening stream for specific file
    std::ifstream stream;
    // Fill stream with a valid stream
    Util::getStream(Path::basePath() + pid + "/" + Path::statPath(), stream); 

    //get the first line of the input stream
    std::getline(stream, line);

    //slices the string based on white space and stores in vector<string> values
	vector<string>values = splitByWhiteSpace(line);

    //getting the clock ticks of the host machine to get the process up time
    return to_string(float(stof(values[13]) / sysconf(_SC_CLK_TCK)));
}

long int ProcessParser::getSysUpTime() {
    //declaring variables
    string line;
    string value;

    // Opening stream for specific file
    std::ifstream stream;
    // Fill stream with a valid stream
    Util::getStream(Path::basePath() + Path::upTimePath(), stream); 

    //get the first line of the input stream
    std::getline(stream, line);

    //slices the string based on white space and stores in vector<string> values
    vector<string>values = splitByWhiteSpace(line);

    //getting the system up time value from the vector
    return stoi(values[0]);
}

string ProcessParser::getProcUser( string pid) {
    //declaring variables
    string line;
    string value;
    string result;
    string name = "Uid:";

    // Opening stream for specific file
    std::ifstream stream;
    // Fill stream with a valid stream
    Util::getStream(Path::basePath() + pid + Path::statusPath(), stream); 
    //loop over the lines until name is accesed
    while(std::getline(stream, line)){
        //Seraching line by line
        if(line.compare(0, name.size(), name) == 0) {
            //slices the string based on white space and stores in vector<string> values
            vector<string>values = splitByWhiteSpace(line);
            //first value from Uid
            result = values[1];
            break;
        }
    }
    // closing stream for specific file
    stream.close();
    // Fill stream with a valid stream
    Util::getStream("/etc/passwd", stream);
    string name2 = ("x:" + result);
    //loop over the lines until name is accesed
    while (std::getline(stream, line)) {
        if (line.find(name2) != std::string::npos) {
            result = line.substr(0, line.find(":"));
            return result;
        }
    }
    return "";
}

vector<string> ProcessParser::getPidList()
{
    DIR* dir;
    // Basically, we are scanning /proc dir for all directories with numbers as their names
    // If we get valid check we store dir names in vector as list of machine pids
    vector<string> container;
    if(!(dir = opendir("/proc")))
        throw std::runtime_error(std::strerror(errno));

    while (dirent* dirp = readdir(dir)) {
        // is this a directory?
        if(dirp->d_type != DT_DIR)
            continue;
        // Is every character of the name a digit?
        if (all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c){ return std::isdigit(c); })) {
            container.push_back(dirp->d_name);
        }
    }
    //Validating process of directory closing
    if(closedir(dir))
        throw std::runtime_error(std::strerror(errno));
    return container;
}

string ProcessParser::getCmd(string pid)
{
    string line;
    std::ifstream stream;
    Util::getStream(Path::basePath() + pid + Path::cmdPath(), stream); 
    std::getline(stream, line);
    return line;
}

int ProcessParser::getNumberOfCores()
{
    // Get the number of host cpu cores
    string line;
    string name = "cpu cores";
    // Opening stream for specific file
    std::ifstream stream;
    // Fill stream with a valid stream
    Util::getStream(Path::basePath() + "cpuinfo", stream); 
    //loop over the lines until name is accesed
    while (std::getline(stream, line)) {
        //Seraching line by line
        if (line.compare(0, name.size(),name) == 0) {
            //slices the string based on white space and stores in vector<string> values
            vector<string>values = splitByWhiteSpace(line);
            return stoi(values[3]);
        }
    }
    return 0;
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber) {
    // Get the number of host cpu cores
    string line;
    string name = "cpu" + coreNumber;
    // Opening stream for specific file
    std::ifstream stream;
    // Fill stream with a valid stream
    Util::getStream(Path::basePath() + Path::statPath(), stream); 
    //loop over the lines until name is accesed
    while (std::getline(stream, line)) {
        //Seraching line by line
        if (line.compare(0, name.size(),name) == 0) {
            //slices the string based on white space and stores in vector<string> values
            vector<string>values = splitByWhiteSpace(line);
            return values;
        }
    }
    return (vector<string>());
}

float getSysActiveCpuTime(vector<string> values) {
    // Add the active CPU time for getSysCpuPercent output
    return (stof(values[S_USER]) +
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE]));
}

float getSysIdleCpuTime(vector<string> values) {
    // Adding the idle time from getSysCpuPercent output
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

string ProcessParser::PrintCpuStats(vector<string> values1, vector<string> values2) {
    /*
    Because CPU stats can be calculated only if you take measures in two different time,
    this function has two parameters: two vectors of relevant values.
    We use a formula to calculate overall activity of processor.
    */
    float activeTime = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
    float idleTime = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
    float totalTime = activeTime + idleTime;
    float result = 100.0*(activeTime / totalTime);
    return to_string(result);
}

float ProcessParser::getSysRamPercent()
{
    string line;
    string name1 = "MemAvailable:";
    string name2 = "MemFree:";
    string name3 = "Buffers:";

    string value;
    int result;
    // Opening stream for specific file
    // Fill stream with a valid stream
    std::ifstream stream;
    Util::getStream(Path::basePath() + Path::memInfoPath(), stream);
    float total_mem = 0;
    float free_mem = 0;
    float buffers = 0;
    while (std::getline(stream, line)) {
        if (total_mem != 0 && free_mem != 0)
            break;
        if (line.compare(0, name1.size(), name1) == 0) {
            vector<string>values = splitByWhiteSpace(line);
            total_mem = stof(values[1]);
        }
        if (line.compare(0, name2.size(), name2) == 0) {
            vector<string>values = splitByWhiteSpace(line);
            free_mem = stof(values[1]);
        }
        if (line.compare(0, name3.size(), name3) == 0) {
            vector<string>values = splitByWhiteSpace(line);
            buffers = stof(values[1]);
        }
    }
    //calculating usage:
    return float(100.0*(1-(free_mem/(total_mem-buffers))));
}

string ProcessParser::getSysKernelVersion() {
    string line;
    string name = "Linux version ";
    // Opening stream for specific file
    std::ifstream stream;
    // Fill stream with a valid stream
    Util::getStream(Path::basePath() + Path::versionPath(), stream); 
    //loop over the lines until name is accesed
    while (std::getline(stream, line)) {
        //Seraching line by line
        if (line.compare(0, name.size(),name) == 0) {
            //slices the string based on white space and stores in vector<string> values
            vector<string>values = splitByWhiteSpace(line);
            return values[2];
        }
    }
}

string ProcessParser::getOSName() {
    // Get the number of host cpu cores
    string line;
    string name = "PRETTY_NAME=";
    // Opening stream for specific file
    std::ifstream stream;
    // Fill stream with a valid stream
    Util::getStream("/etc/os-release", stream); 
    //loop over the lines until name is accesed
    while (std::getline(stream, line)) {
        //Seraching line by line
        if (line.compare(0, name.size(),name) == 0) {
            std::size_t found = line.find("=");
            found++;
            string result = line.substr(found);
            result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
            return result;
        }
    }
}

int ProcessParser::getTotalThreads() {
    string line;
    string name = "Threads:";
    vector<string> list_of_threads = ProcessParser::getPidList();
    int total_thread = 0;

    //looping through the number of threads
    for (int i = 0; i<list_of_threads.size(); i++) {
        // Opening stream for specific file
        std::ifstream stream;
        // Fill stream with a valid stream
        Util::getStream(Path::basePath() + list_of_threads[i] + Path::statusPath(), stream); 
        //loop over the lines until name is accesed
        while (std::getline(stream, line)) {
            //Seraching line by line
            if (line.compare(0, name.size(),name) == 0) {
                //slices the string based on white space and stores in vector<string> values
                vector<string>values = splitByWhiteSpace(line);
                total_thread += stoi(values[1]);
                break;
            }
        }
    }
    return total_thread;
}

int ProcessParser::getTotalNumberOfProcesses() {
    string line;
    string name = "processes";
    // Opening stream for specific file
    std::ifstream stream;
    // Fill stream with a valid stream
    Util::getStream(Path::basePath() + Path::statPath(), stream); 
    //loop over the lines until name is accesed
    while (std::getline(stream, line)) {
        //Seraching line by line
        if (line.compare(0, name.size(),name) == 0) {
            //slices the string based on white space and stores in vector<string> values
            vector<string>values = splitByWhiteSpace(line);
            return stoi(values[1]);
        }
    }
}

int ProcessParser::getNumberOfRunningProcesses() {
    string line;
    string name = "procs_running";
    // Opening stream for specific file
    std::ifstream stream;
    // Fill stream with a valid stream
    Util::getStream(Path::basePath() + Path::statPath(), stream); 
    //loop over the lines until name is accesed
    while (std::getline(stream, line)) {
        //Seraching line by line
        if (line.compare(0, name.size(),name) == 0) {
            //slices the string based on white space and stores in vector<string> values
            vector<string>values = splitByWhiteSpace(line);
            return stoi(values[1]);
        }
    }
}

bool ProcessParser::isPidExisting(std::string pid)
{
    for(auto& valid_pid : ProcessParser::getPidList())
    {
        if(valid_pid.compare(pid) == 0)
            return true;
    }
    return false;
}