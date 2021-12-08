#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <climits>
#include <fstream>
#include <fcntl.h>

using namespace std;
#define WHITESPACE " "
#define KILL_CMD_ARG_NUM 3
#define FG_CMD_MAX_ARG_NUM 2
#define FG_CMD_MIN_ARG_NUM 1
#define BG_CMD_MAX_ARG_NUM 2
#define BG_CMD_MIN_ARG_NUM 1
#define FORK_FAILED -1
#define FORK_CHILD 0


#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif


#define STDERR_FD 2
#define STDOUT_FD 1
#define STDIN_FD 0
#define PIPE_READ 0
#define PIPE_WRITE 1
#define DO_CLOSE( syscall ) do { \
    if((syscall) == -1 ) { \
    perror( "smash: close failed" ); \
    } \
    } while( 0 )                 \
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
    size_t idx = str.find_last_not_of(WHITESPACE);
    return idx == string::npos ? false : str[idx] == '&';
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



SpecialCommand _identifyAndSeperateSpecialSigns(const char* cmd_line, std::string& result) {
    std::string cmd_line_s(cmd_line);
    size_t sign_pos;
    // search for ">>"
    sign_pos = cmd_line_s.find(">>");
    if(sign_pos != string::npos) {
        result = cmd_line_s.substr(0, sign_pos) + " >> ";
        result += cmd_line_s.substr(sign_pos + 2);
        return REDIRECTION_APPEND;
    }
    // search for ">"
    sign_pos = cmd_line_s.find(">");
    if(sign_pos != string::npos) {
        result = cmd_line_s.substr(0, sign_pos) + " > ";
        result += cmd_line_s.substr(sign_pos + 1);
        return REDIRECTION;
    }
    // search for "|&"
    sign_pos = cmd_line_s.find("|&");
    if(sign_pos != string::npos) {
        result = cmd_line_s.substr(0, sign_pos) + " |& ";
        result += cmd_line_s.substr(sign_pos + 2);
        return PIPE_TO_ERR;
    }
    // search for "|"
    sign_pos = cmd_line_s.find("|");
    if(sign_pos != string::npos) {
        result = cmd_line_s.substr(0, sign_pos) + " | ";
        result += cmd_line_s.substr(sign_pos + 1);
        return PIPE;
    }
    // no special signs
    result = cmd_line_s;
    return NORMAL;
}
//--------------------------------------end of non member functions-----------------------------------------------------
// TODO: Add your implementation for classes in Commands.h





//--------------------------------------small shell member functions----------------------------------------------------
SmallShell::SmallShell():smash_name("smash") ,external_quit_flag(false), smash_pid(getpid()){};



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

    //pipe or redirection
    string special_cmd;
    switch (_identifyAndSeperateSpecialSigns(cmd_s.c_str(),special_cmd)) {
        case PIPE:
            return std::shared_ptr<Command>(new PipeCommand(cmd_line,PIPE));
            break;
        case PIPE_TO_ERR:
            return std::shared_ptr<Command>(new PipeCommand(cmd_line,PIPE_TO_ERR));
            break;
        case REDIRECTION:
            return std::shared_ptr<Command>(new RedirectionCommand(cmd_line));
            break;
        case REDIRECTION_APPEND:
            return std::shared_ptr<Command>(new RedirectionCommand(special_cmd.c_str()));
            break;
        default:
            break;

    }

    //if command is an internal command
    //#1
    if(firstWord.compare("chprompt") == 0 or firstWord.compare(("chprompt&")) == 0)
    {
        return std::shared_ptr<Command>(new ChPromptCommand(cmd_line));
    }
        //#2
    else if (firstWord.compare("showpid") == 0 or firstWord.compare(("showpid&")) == 0)
    {
        return std::shared_ptr<Command>(new ShowPidCommand(cmd_line));
    }
        //#3
    else if (firstWord.compare("pwd") == 0 or firstWord.compare(("pwd&")) == 0)
    {
        return std::shared_ptr<Command>(new GetCurrDirCommand(cmd_line));
    }
        //#4
    else if (firstWord.compare("cd") == 0 or firstWord.compare(("cd&")) == 0)
    {
        return std::shared_ptr<Command>(new ChangeDirCommand(cmd_line,&last_working_directory));
    }
        //5
    else if (firstWord.compare("jobs") == 0 or firstWord.compare(("jobs&")) == 0)
    {
        return std::shared_ptr<Command>(new JobsCommand(cmd_line,&jobs));
    }
        //#6
    else if (firstWord.compare("kill") == 0 or firstWord.compare(("kill&")) == 0)
    {
        return std::shared_ptr<Command>(new KillCommand(cmd_line,&jobs));
    }
        //#7
    else if (firstWord.compare("fg") == 0 or firstWord.compare(("fg&")) == 0)
    {
        return std::shared_ptr<Command>(new ForegroundCommand(cmd_line,&jobs));
    }
        //#8
    else if (firstWord.compare("bg") == 0 or firstWord.compare(("bg&")) == 0)
    {
        return std::shared_ptr<Command>(new BackgroundCommand(cmd_line,&jobs));
    }
        //#9
    else if (firstWord.compare("quit") == 0 or firstWord.compare(("quit&")) == 0)
    {
        external_quit_flag = true;
        return std::shared_ptr<Command>(new QuitCommand(cmd_line,&jobs));
    }
    //end of if command is an internal command
    else if (firstWord.compare("head") == 0 or firstWord.compare(("head&")) == 0)
    {
        return std::shared_ptr<Command>(new HeadCommand(cmd_line));
    }
    //else its external command
    else {
        return std::shared_ptr<Command>(new ExternalCommand(cmd_line));
    }
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    std::shared_ptr<Command> cmd = SmallShell::CreateCommand(cmd_line);
	if(typeid(*cmd)==typeid(ExternalCommand))
    {
        pid_t external_process_id=fork();
        if(FORK_FAILED==external_process_id)
        {
            perror("smash error: fork failed");
        }
        else if(FORK_CHILD==external_process_id)
        {
            setpgrp();
            cmd->execute();
        }
        else //parent process
        {
            cmd->setPID(external_process_id);
            if(not cmd->isBgCommand()) //foreground commmand
            {
                jobs.setFgJob(cmd);
                waitpid(jobs.getFgJob()->getJobPid(),nullptr, WUNTRACED);
            }
            else {
                jobs.addJob(cmd, false);
            }
        }
    }

    else
    {
        cmd->execute();
    }
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}
//--------------------------------------end of small shell member functions---------------------------------------------




//--------------------------------------Command member functions--------------------------------------------------------
Command::Command(const char *cmd_line): my_cmd_line(_trim(cmd_line).c_str()), origin_cmd_line(cmd_line), command_pid(0){
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






//--------------------------------------External Command member functions-----------------------------------------------

ExternalCommand::ExternalCommand(const char* cmd_line): Command(cmd_line), bash_cmd(my_cmd_line){
    if(_isBackgroundComamnd(bash_cmd.c_str()))
    {
        _removeBackgroundSign(bash_cmd);
    }
}


void ExternalCommand::execute() {
    execl("/bin/bash", "/bin/bash", "-c", bash_cmd.c_str(), NULL);
}
//--------------------------------------end of External Command member functions----------------------------------------








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
    char buff[PATH_MAX];
    cout << getcwd(buff, PATH_MAX) << endl;
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
                return;
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
            std::cout << " (stopped)";
        }
        std::cout << std::endl;
    }
}



void JobsList::killAllJobs(){
    std::cout << "smash: sending SIGKILL signal to " << jobs.size() << " jobs:" << endl;
    while(!jobs.empty())
    {
        std::cout << jobs[0]->getJobPid() << ": ";
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


std::shared_ptr<JobsList::JobEntry> JobsList::getLastStoppedJob()
{
    for(int i=jobs.size()-1;i>=0;i--)
    {
        if(jobs[i]->isStopped())
        {
            return jobs[i];
        }
    }
    return nullptr;
}

void JobsList::StopFG() {
    if(fg_job) {
        fg_job->setIsStopped(true);
        fg_job->resetTime();

        if(fg_job->getJobId() == 0) {
            addJob(fg_job->getCommand(),true);
        }
        else
        {
            int index =0;
            for(; index<jobs.size();index++)
            {
                if(fg_job->getJobId()<jobs[index]->getJobId())
                {
                    break;
                }
            }
            jobs.insert(jobs.begin()+index,fg_job);
        }

        fg_job = nullptr;
    }

}

void JobsList::setFgJob(std::shared_ptr<Command> cmd) {
    if(cmd)
    {
        fg_job=std::make_shared<JobEntry>(cmd, cmd->getPID());
    }
    else
    {
        fg_job= nullptr;
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



//--------------------------------------Foreground Command member functions---------------------------------------------
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
        else
        {
            return NO_JOB_IN_LIST;
        }
    }
    return ARGUMENT_VALID;
}



void ForegroundCommand::execute()
{
    ArgumentsStatus status=checkArgs();
    if(ARGUMENT_INVALID==status) {
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    if(NO_JOB_IN_LIST==status)
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
//--------------------------------------end of Foreground Command member functions--------------------------------------


//--------------------------------------Background Command member functions---------------------------------------------
ArgumentsStatus BackgroundCommand::checkArgs(){
    if(args_len>BG_CMD_MAX_ARG_NUM)
    {
        return ARGUMENT_INVALID;
    }

    else if(BG_CMD_MAX_ARG_NUM==args_len)
    {
        if(not isNumber(args[1]))
        {
            return ARGUMENT_INVALID;
        }
        job_id = stoi(args[1]);
    }
    else if(BG_CMD_MIN_ARG_NUM==args_len)
    {
        if(! (jobs->getLastStoppedJob()== nullptr))
        {
            job_id=jobs->getLastStoppedJob()->getJobId();
        }
        else
        {
            return NO_JOB_IN_LIST;
        }
    }
    return ARGUMENT_VALID;
}


void BackgroundCommand::execute()
{
    ArgumentsStatus status =checkArgs();
    if(ARGUMENT_INVALID==status) {
        cerr << "smash error: bg: invalid arguments" << endl;
        return;
    }
    if(NO_JOB_IN_LIST==status)
    {
        cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
        return;
    }
    if(not jobs->isJobInList(job_id))
    {
        cerr << "smash error: bg: job-id " << job_id << " does not exist" << endl;
        return;
    }
    shared_ptr<JobsList::JobEntry> job=jobs->getJobById(job_id);
    if(not job->isStopped())
    {
        cerr << "smash error: bg: job-id " << job_id << " is already running in the background" << endl;
        return;
    }

    cout << (job->getCommand()->getCommand()) << " : ";
    std::cout << job->getJobPid() << endl;
    job->setIsStopped(false);
    if(kill(job->getJobPid() , SIGCONT) == -1) {
        perror("smash error: kill failed");
    }
}
//--------------------------------------end of Background Command member functions--------------------------------------




//--------------------------------------Quit Command member functions---------------------------------------------------
void QuitCommand::execute() {

    if(args_len >= 2 and strcmp(args[1],"kill") == 0) {
        jobs->removeFinishedJobs();
        jobs->killAllJobs();
    }
    SmallShell& smash = SmallShell::getInstance();
    kill(smash.getSmashPID(), SIGKILL);
}
//--------------------------------------end of Quit Command member functions--------------------------------------------



RedirectionCommand::RedirectionCommand(const char *cmd_line): Command(cmd_line) {

    std::string str(cmd_line);
    unsigned int idx = str.find_first_of('>');
    append = cmd_line[idx + 1] == '>';
    leftSide = str.substr(0, idx);
    unsigned int end_inx;
    if (append) {
        end_inx = idx + 2;
    } else {
        end_inx = idx + 1;
    }
    rightSide = _trim(str.substr(end_inx));
}

void RedirectionCommand::execute() {
    int flags = O_WRONLY | O_CREAT;
    if (!append)
    {flags |= O_TRUNC;}
    else
    {flags |= O_APPEND;}

    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
    int fd = open(rightSide.c_str(), flags, mode);
    if (fd == -1) {
        perror("smash error: open failed");
        return;
    }
    int backupStdout = dup(1);
    if (backupStdout == -1) {
        perror("smash error: dup failed");
        if (close(fd) == -1)
            perror("smash error: close failed");
        return;
    }
    if (dup2(fd, 1) == -1) {
        perror("smash error: dup2 failed");
        if (close(fd) == -1)
            perror("smash error: close failed");
        return;
    }

    SmallShell& smash = SmallShell::getInstance();
    smash.executeCommand(leftSide.c_str());

    if (dup2(backupStdout, 1) == -1)
        perror("smash error: dup2 failed");
    if (close(backupStdout) == -1)
        perror("smash error: close failed");
    if (close(fd) == -1)
        perror("smash error: close failed");
}

PipeCommand::PipeCommand(const char *cmd_line,SpecialCommand op): Command(cmd_line) , op(op) {
    string temp(cmd_line);
    size_t op_pos;
    if(op == PIPE) {
        op_pos = temp.find("|");
        if(temp.find_last_not_of(WHITESPACE) == op_pos) {
            // no file name given // TODO do something!!
        }
        else {
            // skip | and space
            cmd2_s = temp.substr(op_pos + 2);
        }
        cmd1_s = temp.substr(0, op_pos);
    }
        // PIPE_TO_ERR
    else {
        op_pos = temp.find("|&");
        if(temp.find_last_not_of(WHITESPACE) == op_pos + 1) {
            // no file name given // TODO do something!!
        }
        else {
            // skip |& and space
            cmd2_s = temp.substr(op_pos + 3);
        }
        cmd1_s = temp.substr(0, op_pos);
    }
}

void PipeCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();

    int fd [2];
    if (pipe(fd) == -1){
        perror("smash error: pipe failed");
        return;
    }

    pid_t p1 = fork();
    if (p1 == -1) {
        // fork failed
        perror("smash error: fork failed");
        return;
    }
    else if (p1 == 0){
        //  son 1 code
        setpgrp();
        if(op == PIPE) {
            if(dup2(fd[PIPE_WRITE],STDOUT_FD) == -1) {
                perror("smash error: dup2 failed");
                return;
            }
        }
        else {
            // op == PIPE_ERR
            if(dup2(fd[PIPE_WRITE],STDERR_FD) == -1) {
                perror("smash error: dup2 failed");
                return;
            }
        }
        DO_CLOSE(close(fd[PIPE_READ]));
        DO_CLOSE(close(fd[PIPE_WRITE]));
        smash.executeCommand(cmd1_s.c_str());
        smash.external_quit_flag = true;
    }
    else {
        //father code
        pid_t p2 = fork();
        if (p2 == -1) {
            // fork failed
            perror("smash error: fork failed");
            return;
        }
        else if (p2 == 0) {
            //  son 2 code
            setpgrp();
            if( dup2(fd[PIPE_READ],STDIN_FD) == -1) {
                perror("smash error: dup2 failed");
                return;
            }
            DO_CLOSE(close(fd[PIPE_READ]));
            DO_CLOSE(close(fd[PIPE_WRITE]));
            smash.executeCommand(cmd2_s.c_str());
            smash.external_quit_flag = true;
        }
        else {
            DO_CLOSE(close(fd[PIPE_READ]));
            DO_CLOSE(close(fd[PIPE_WRITE]));
            waitpid(p1,nullptr, WUNTRACED);
            waitpid(p2,nullptr, WUNTRACED);
        }
    }
}

void HeadCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    string filename;
    int line_num = 10;
    if (this->args_len == 1) {
        cerr << "smash error: head: not enough arguments" << endl;
        return;
    }
    if (this->args_len > 3)
    {
        return;
    }
    if (this->args_len == 3) {

        string num = string(this->args[1]).substr( 1);
        if(isNumber(num))
        {
            line_num = stoi(num);
        }
       else {
            cerr << "smash error: head: invalid arguments" << endl;
            return;
        }
        filename = this->args[2];
    } else {
        filename = this->args[1];
    }

    int fd = open(filename.c_str(),O_RDONLY);
    if (fd == -1) {
        perror("smash error: open failed");
        return;
    }

    char buffer[2];
    ssize_t read_len;

    for(int printed_lines = 0; printed_lines<line_num; printed_lines++)
    {
        while((read_len = read(fd,buffer,1)) != 0) {

            if(buffer[0]=='\n')
            {
                cout << endl;
                break;
            }

            else
            {
                cout << buffer[0];
            }


        }
        if(read_len==0)
        {
            break;
        }

    }

    if (close(fd) == -1) {
        perror("smash error: close failed");
        return;
    }



}
