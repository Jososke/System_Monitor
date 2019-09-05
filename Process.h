#include <string>

using namespace std;
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
private:
    string pid;
    string user;
    string cmd;
    string cpu;
    string mem;
    string upTime;

public:
    Process(string pid){
        try {
        this->pid = pid;
        } catch (const std::exception& e) {}  
        try {
        this->user = ProcessParser::getProcUser(pid);
        } catch (const std::exception& e) {}  
        try {
        this->mem = ProcessParser::getVmSize(pid);  
        } catch (const std::exception& e) {} 
        try {        
        this->cmd = ProcessParser::getCmd(pid);
        } catch (const std::exception& e) {} 
        try { 
        this->upTime = ProcessParser::getProcUpTime(pid);
        } catch (const std::exception& e) {}  
        try {
        this->cpu = ProcessParser::getCpuPercent(pid);
        } catch (const std::exception& e) {}  
    }
    void setPid(int pid);
    string getPid()const;
    string getUser()const;
    string getCmd()const;
    int getCpu()const;
    int getMem()const;
    string getUpTime()const;
    string getProcess();
};
void Process::setPid(int pid){
    try {this->pid = pid;} catch (const std::exception& e) {}  
}
string Process::getPid()const {
    try {return this->pid;} catch (const std::exception& e) {}  

}
string Process::getProcess(){
    try {
    if(!ProcessParser::isPidExisting(this->pid))
        return "";
    } catch (const std::exception& e) {}  
    try {this->mem = ProcessParser::getVmSize(this->pid);} catch (const std::exception& e) {}  
    try {this->upTime = ProcessParser::getProcUpTime(this->pid);} catch (const std::exception& e) {}  
    try {this->cpu = ProcessParser::getCpuPercent(this->pid);} catch (const std::exception& e) {}  

return (this->pid + "   "
                    + this->user
                    + "   "
                    + this->mem.substr(0,5)
                    + "     "
                    + this->cpu.substr(0,5)
                    + "     "
                    + this->upTime.substr(0,5)
                    + "    "
                    + this->cmd.substr(0,50)
                    + "...");
}

string Process::getUser() const {
    return this-> user;
}

string Process::getCmd() const {
    return this->cmd;
}

int Process::getCpu() const {
    return stoi(this->cpu);
}

int Process::getMem() const{
    return stoi(this->mem);
}

string Process::getUpTime() const {
    return this->upTime;
}