#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <climits>

using namespace std;
#define WHITESPACE " "
#define KILL_CMD_ARG_NUM 3
#define FG_CMD_MAX_ARG_NUM 2
#define FG_CMD_MIN_ARG_NUM 1
#define NO_JOB_IN_LIST -1
#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif
//--------------------------------------non member functions------------------------------------------------------------
string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  //FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  //FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(string& cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}


bool isNumber(const std::string& s)
{
    if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

    char * p;
    strtol(s.c_str(), &p, 10);

    return (*p == 0);
}

//--------------------------------------end of non member functions-----------------------------------------------------
// TODO: Add your implementation for classes in Commands.h



//--------------------------------------small shell member functions----------------------------------------------------
SmallShell::SmallShell():smash_name("smash") , smash_pid(getpid()){};



SmallShell::~SmallShell() {
// TODO: add your implementation
}

void SmallShell::setSmashName(string new_name){
    smash_name=new_name;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
std::shared_ptr<Command> SmallShell::CreateCommand(const char* cmd_line) {

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));


    //if command is an internal command
    //#1
    if(firstWord.compare("chprompt") == 0)
    {
        return std::shared_ptr<Command>(new ChPromptCommand(cmd_line));
    }
        //#2
    else if (firstWord.compare("showpid") == 0)
    {
        return std::shared_ptr<Command>(new ShowPidCommand(cmd_line));
    }
        //#3
    else if (firstWord.compare("pwd") == 0)
    {
        return std::shared_ptr<Command>(new GetCurrDirCommand(cmd_line));
    }
        //#4
    else if (firstWord.compare("cd") == 0)
    {
        return std::shared_ptr<Command>(new ChangeDirCommand(cmd_line,&last_working_directory));
    }
        //5
    else if (firstWord.compare("jobs") == 0)
    {
        return std::shared_ptr<Command>(new JobsCommand(cmd_line,&jobs));
    }
        //#6
    else if (firstWord.compare("kill") == 0)
    {
        return std::shared_ptr<Command>(new KillCommand(cmd_line,&jobs));
    }
        //#7
    else if (firstWord.compare("fg") == 0)
    {
        return std::shared_ptr<Command>(new ForegroundCommand(cmd_line,&jobs));
    }
        //#8
    else if (firstWord.compare("bg") == 0)
    {
        //return std::shared_ptr<Command>(new BackgroundCommand(cmd_line,&jobs));
    }
        //#9
    else if (firstWord.compare("quit") == 0)
    {
        return std::shared_ptr<Command>(new QuitCommand(cmd_line,&jobs));
    }
        //end of if command is an internal command
        /*

        else {
          return new ExternalCommand(cmd_line);
        }

        */
    else
    {
        return nullptr;
    }

}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    std::shared_ptr<Command> cmd = SmallShell::CreateCommand(cmd_line);
    if(typeid(*cmd)==typeid(ChPromptCommand))
        //if(typeid(*cmd)==typeid(BuiltInCommand))
    {
        cmd->execute();
    }
    else if(typeid(*cmd)==typeid(ShowPidCommand))
    {
        cmd->execute();
    }
    else if(typeid(*cmd)==typeid(GetCurrDirCommand))
    {
        cmd->execute();
    }
    else if(typeid(*cmd)==typeid(ChangeDirCommand))
    {
        cmd->execute();
    }
    else if(typeid(*cmd)==typeid(JobsCommand))
    {
        cmd->execute();
    }
    else if(typeid(*cmd)==typeid(KillCommand))
    {
        cmd->execute();
    }
    else if(typeid(*cmd)==typeid(ForegroundCommand))
    {
        cmd->execute();
    }
    /*else if(typeid(*cmd)==typeid(BackgroundCommand))
    {
        cmd->execute();
    }*/
    else if(typeid(*cmd)==typeid(QuitCommand))
    {
        cmd->execute();
    }

    else
    {
        //fork();
    }
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}
//--------------------------------------end of small shell member functions---------------------------------------------




//--------------------------------------Command member functions--------------------------------------------------------
Command::Command(const char *cmd_line): my_cmd_line(cmd_line), command_pid(0){
    is_bg=_isBackgroundComamnd(cmd_line);
    args_len=_parseCommandLine(cmd_line , args);
}
//--------------------------------------end of Command member functions-------------------------------------------------


//--------------------------------------Built in Command member functions-----------------------------------------------
BuiltInCommand::BuiltInCommand(const char* cmd_line):Command(cmd_line){
    string temp(this->my_cmd_line);
    is_bg=false;
    _removeBackgroundSign(temp);
    args_len=_parseCommandLine(temp.c_str() , args);
}
//--------------------------------------end of Built in Command member functions----------------------------------------





//--------------------------------------ChPrompt Command member functions-----------------------------------------------
void ChPromptCommand::execute() {
    SmallShell& smash=SmallShell::getInstance();
    if(args_len>1)
    {
        smash.setSmashName(args[1]);
    }
    else
    {
        smash.setSmashName("smash");
    }

}
//--------------------------------------end of ChPrompt Command member functions----------------------------------------


//--------------------------------------ShowPid Command member functions------------------------------------------------
void ShowPidCommand::execute() {
    SmallShell& smash=SmallShell::getInstance();
    std::cout << "smash pid is " << smash.getSmashPID() << "\n";
}
//--------------------------------------end of ShowPid Command member functions-----------------------------------------


//--------------------------------------GetCurrDir Command member functions---------------------------------------------
void GetCurrDirCommand::execute() {
    //SmallShell& smash=SmallShell::getInstance();
    long max_buff_size = pathconf(".", _PC_PATH_MAX);
    char* buff;
    char* current_dir;
    if ((buff = (char*)malloc((size_t)max_buff_size)) != NULL) {
        current_dir = getcwd(buff, (size_t) max_buff_size);
        if (!current_dir) {
            std::cerr << "smash error: getcwd failed" << std::endl;
            free(buff);
            return;
        }
        std::cout << current_dir << std::endl;
        free(buff);
    }
}
//--------------------------------------end of GetCurrDir Command member functions--------------------------------------


//--------------------------------------ChangeDir Command member functions----------------------------------------------
void ChangeDirCommand::execute() {
    string path;
    // no arguments - does nothing
    if(args_len == 1) {
        return;
    }
    // too many args
    if (args_len != 2) {
        cerr << "smash error: cd: too many arguments" << endl ;
        return;
    }
    else
    {
        //if we received cd -
        if(strcmp(args[1],"-")==0)
        {
            if((*last_directory).empty())
            {
                cerr << "smash error: cd: OLDPWD not set" << endl;
            }
            else
            {
                path=*last_directory;
            }
        }
        else
        {
            path=args[1];
        }
        // saving oldpwd
        char buff[PATH_MAX];
        getcwd(buff, PATH_MAX);
        // executing with feedback from syscall
        if(chdir(path.c_str()) == -1) {
            perror("smash error: chdir failed");
            return;
        }
        // if chdir succeeded - update oldpwd
        *last_directory = buff;
    }
}
//--------------------------------------end of ChangeDir Command member functions---------------------------------------






//--------------------------------------JobsEntry Command member functions----------------------------------------------
JobsList::JobEntry::JobEntry(std::shared_ptr<Command> &cmd, bool is_stopped , int job_id):
        cmd(cmd), job_id(job_id), is_stopped(is_stopped){time(&executed_time);}// update  executed time

//--------------------------------------end of JobsEntry Command member functions---------------------------------------







//--------------------------------------JobsList Command member functions-----------------------------------------------
int JobsList::getNextFreeID(){
    if (jobs.empty())
    {
        return 1;
    }
    std::shared_ptr<JobEntry> job = jobs.back();
    return job->getJobId()+1;
}


void JobsList::addJob(std::shared_ptr<Command> cmd, bool isStopped){
    removeFinishedJobs();
    int job_id = getNextFreeID(); //gets the max job id in the list +1
    jobs.push_back(std::make_shared<JobEntry>(cmd,isStopped, job_id));
}

void JobsList::removeFinishedJobs(){
    for (int i=jobs.size()-1; i>=0; i--)
    {
        int status;
        status = waitpid(jobs[i]->getJobPid(), nullptr, WNOHANG);
        // if status == pid <--> process exited (finished)
        if(status == jobs[i]->getJobPid()) {
            std::vector<std::shared_ptr<JobEntry>>::iterator it;
            it=jobs.begin();
            it+=i;
            jobs.erase(it);
        }
    }
}



void JobsList::printJobsList(){
    removeFinishedJobs();
    for (int i=0; i<jobs.size(); i++)
    {
        std::cout << "[" << jobs[i]->getJobId() << "] ";
        std::cout << jobs[i]->getCommand()->getCommand() << " : ";
        std::cout << jobs[i]->getJobPid() << " ";
        std::cout << difftime(time(nullptr),jobs[i]->getTime()) << " secs";
        if (jobs[i]->isStopped())
        {
            std::cout << "(stopped)";
        }
        std::cout << std::endl;
    }
}



void JobsList::killAllJobs(){
    std::cout << "smash: sending SIGKILL signal to " << jobs.size() << " jobs:" << endl;
    while(!jobs.empty())
    {
        std::cout << jobs[0]->getJobPid() << " : ";
        std::cout << jobs[0]->getCommand()->getCommand();
        std::cout << std::endl;
        if (kill(jobs[0]->getJobPid(), SIGKILL) == -1)
        {
            perror("smash error: kill failed");
        }
        jobs.erase(jobs.begin());
    }
}


shared_ptr<JobsList::JobEntry> JobsList::getJobById(int jobId)
{
    for (int i=0; i<jobs.size(); i++)
    {
        if(jobs[i]->getJobId()==jobId)
        {
            return jobs[i];
        }
    }
    return nullptr;
}

bool JobsList::isJobInList(int job_id)
{
    if(getJobById(job_id)!= nullptr)
    {
        return true;
    }
    return false;
}



std::shared_ptr<JobsList::JobEntry> JobsList::getLastJob()
{
    if(!jobs.empty())
    {
        return jobs.back();
    }
    return nullptr;
}

void JobsList::moveBGToFG(int job_id)
{
    bool is_job_found=false;
    for (int i=0; i<jobs.size(); i++)
    {
        if(jobs[i]->getJobId()==job_id)
        {
            fg_job=jobs[i];
            jobs.erase((jobs.begin()+i));
            is_job_found=true;
            break;
        }
    }
    if(is_job_found)
    {
        std::cout << fg_job->getCommand()->getCommand() << " : ";
        std::cout << fg_job->getJobPid() << endl;
        if(kill(fg_job->getJobPid(), SIGCONT) == -1) {
            perror("smash error: kill failed");
        }
        // wait for the process to finish (or to be stopped by ctrl + z)
        waitpid(fg_job->getJobPid(), nullptr, WUNTRACED);
    }
}
//--------------------------------------end of JobsList Command member functions----------------------------------------







//--------------------------------------Jobs Command member functions---------------------------------------------------
void JobsCommand::execute() {
    jobs_list->printJobsList();
}
//--------------------------------------end of Jobs Command member functions--------------------------------------------



//--------------------------------------Kill Command member functions---------------------------------------------------

ArgumentsStatus KillCommand::checkArgs(){
    if(args_len!=KILL_CMD_ARG_NUM)
    {
        return ARGUMENT_INVALID;
    }
    string signal=args[1];
    string job=args[2];

    // check if signal starts with '-'
    if(signal[0] != '-') {
        return ARGUMENT_INVALID;
    }
    signal = signal.substr(1);
    //checks if the signal is legal- a number
    if(not isNumber(signal)){
        return ARGUMENT_INVALID;
    }
    //checks if the job is legal - a number
    if(not isNumber(job)){
        return ARGUMENT_INVALID;
    }
    return ARGUMENT_VALID;
}



void KillCommand::execute()
{
    if(ARGUMENT_INVALID==checkArgs()) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    // make sure we dont send signal to a zombie process:
    string signal_text=args[1];
    signal=stoi(signal_text.substr(1));
    job_id=stoi(args[2]);

    jobs->removeFinishedJobs();
    if(not jobs->isJobInList(job_id))
    {
        cerr << "smash error: kill: job-id " << job_id <<  " does not exist" << endl;
        return;
    }

    if(kill(jobs->getJobById(job_id)->getJobPid(),signal) == -1) {
        perror("smash error: kill failed");
        return;
    }

    cout << "signal number " << signal << " was sent to pid " << jobs->getJobById(job_id)->getJobPid() << endl;
}
//--------------------------------------end of Kill Command member functions--------------------------------------------
ArgumentsStatus ForegroundCommand::checkArgs(){
    if(args_len>FG_CMD_MAX_ARG_NUM)
    {
        return ARGUMENT_INVALID;
    }

    else if(FG_CMD_MAX_ARG_NUM==args_len)
    {
        if(not isNumber(args[1]))
        {
            return ARGUMENT_INVALID;
        }
        job_id = stoi(args[1]);
    }
    else if(FG_CMD_MIN_ARG_NUM==args_len)
    {
        if(! (jobs->getLastJob()== nullptr))
        {
            job_id=jobs->getLastJob()->getJobId();
        }
        job_id=NO_JOB_IN_LIST;
    }
    return ARGUMENT_VALID;
}



void ForegroundCommand::execute()
{
    if(ARGUMENT_INVALID==checkArgs()) {
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    if(NO_JOB_IN_LIST==job_id)
    {
        cerr << "smash error: fg: jobs list is empty" << endl;
        return;
    }
    if(not jobs->isJobInList(job_id))
    {
        cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
        return;
    }
    jobs->moveBGToFG(job_id);
}

//--------------------------------------Foreground Command member functions---------------------------------------------





//--------------------------------------end of Foreground Command member functions--------------------------------------







//--------------------------------------Quit Command member functions---------------------------------------------------
void QuitCommand::execute() {
    if(args_len >= 2 and strcmp(args[1],"kill") == 0) {
        jobs->killAllJobs();
    }
    SmallShell& smash = SmallShell::getInstance();
    kill(smash.getSmashPID(), SIGKILL);
}
//--------------------------------------end of Quit Command member functions--------------------------------------------
