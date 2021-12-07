#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <memory>
#include <time.h>



using namespace std;
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

enum ArgumentsStatus
{
    ARGUMENT_VALID,
    ARGUMENT_INVALID
};

enum SpecialCommand {
    NORMAL = 0,
    PIPE,
    PIPE_TO_ERR,
    REDIRECTION,
    REDIRECTION_APPEND
};

class Command {
private:


protected:
    const string my_cmd_line;
    const string origin_cmd_line;
    char* args[COMMAND_MAX_ARGS];
    int args_len;
    bool is_bg;
    pid_t command_pid;
 public:
  Command(const char* cmd_line);
  virtual ~Command(){}
  virtual void execute() = 0;
  pid_t getPID(){return command_pid;}
  void setPID(pid_t new_pid){command_pid=new_pid;}
  string getCommand(){
      string ret(origin_cmd_line);
      return ret;}
  bool isBgCommand(){return is_bg;}
  void setBgCommand(bool is_bg_command){is_bg=is_bg_command;}
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};
//-----------------------------------------------------------------Built In Commands----------------------------


class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand(){}
};


// #1
class ChPromptCommand :public BuiltInCommand{
public:
    ChPromptCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
    ~ChPromptCommand(){}
    void execute() override;
};


// #2
class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
    virtual ~ShowPidCommand() {}
    void execute() override;
};


// #3
class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};


// #4
class ChangeDirCommand : public BuiltInCommand {
private:
    string* last_directory;
// TODO: Add your data members public:
public:
    ChangeDirCommand(const char* cmd_line, string *plastPwd):BuiltInCommand(cmd_line) , last_directory(plastPwd){}
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

//addition class used in the next classes
class JobsList {
public:
    class JobEntry {
    private:
        std::shared_ptr<Command> cmd;
        int job_id;
        time_t executed_time;
        bool is_stopped;
    public:
        JobEntry(std::shared_ptr<Command> &cmd, bool is_stopped = false, int job_id= 1);
        ~JobEntry() = default;
        std::shared_ptr<Command> getCommand() { return cmd;}
        time_t getTime() const { return executed_time; }
        void resetTime(){time(&executed_time);}
        bool isStopped() const { return is_stopped; }
        int getJobId() const { return job_id; }
        pid_t getJobPid() const { return  cmd->getPID(); }
        void setIsStopped(bool isStopped){is_stopped=isStopped;}
    };
private:
    std::vector<std::shared_ptr<JobEntry>> jobs;
    std::shared_ptr<JobEntry> fg_job;
    int getNextFreeID();
    // TODO: Add your data members
public:
    JobsList()=default;
    ~JobsList()=default;
    void addJob(std::shared_ptr<Command> cmd, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    std::shared_ptr<JobEntry> getFgJob(){return fg_job;}
    void setFgJob(std::shared_ptr<Command> cmd){fg_job=std::make_shared<JobEntry>(cmd, cmd->getPID());}
    std::shared_ptr<JobEntry> getJobById(int jobId);
    bool isJobInList(int job_id);
    //void removeJobById(int jobId);
    std::shared_ptr<JobEntry> getLastJob();
    std::shared_ptr<JobEntry> getLastStoppedJob();
    void moveBGToFG(int job_id);
    // TODO: Add extra methods or modify exisitng ones as needed
};


// #5
class JobsCommand : public BuiltInCommand {
private:
    JobsList* jobs_list;
    // TODO: Add your data members
public:
    JobsCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs_list(jobs){}
    virtual ~JobsCommand() {}
    void execute() override;
};

// #6
class KillCommand : public BuiltInCommand {
private:
    JobsList* jobs;
    int signal;
    int job_id;
    // TODO: Add your data members
public:
    KillCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line) , jobs(jobs){}
    virtual ~KillCommand() {}
    void execute() override;
    ArgumentsStatus checkArgs();
};

// #7
class ForegroundCommand : public BuiltInCommand {
    JobsList* jobs;
    int job_id;
    // TODO: Add your data members
public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line) , jobs(jobs){ jobs->removeFinishedJobs();}
    virtual ~ForegroundCommand() {}
    void execute() override;
    ArgumentsStatus checkArgs();
};



// #8
class BackgroundCommand : public BuiltInCommand {
    JobsList* jobs;
    int job_id;
    // TODO: Add your data members
public:
    BackgroundCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line) , jobs(jobs){jobs->removeFinishedJobs();}
    virtual ~BackgroundCommand() {}
    void execute() override;
    ArgumentsStatus checkArgs();
};


// #9
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
private:
    JobsList* jobs;
public:
    QuitCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line) , jobs(jobs){}
    virtual ~QuitCommand() {}
    void execute() override;
};

//-----------------------------------------------------------------End Built In Commands-------------------------



//-----------------------------------------------------------------External Commands-----------------------------
class ExternalCommand : public Command {
    std::string bash_cmd;
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};



//-----------------------------------------------------------------End External Commands-------------------------
class PipeCommand : public Command {
  // TODO: Add your data members
 public:
    SpecialCommand op;
    std::string cmd1_s;
    std::string cmd2_s;
    int temp_stdout_fd;
    int temp_stderr_fd;
    int temp_stdin_fd;
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
    bool append;
    std::string leftSide;
    std::string rightSide;
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};





















class HeadCommand : public BuiltInCommand {
 public:
  HeadCommand(const char* cmd_line);
  virtual ~HeadCommand() {}
  void execute() override;
};


class SmallShell {
 private:
    // TODO: Add your data members
    string smash_name;
    string last_working_directory;
    pid_t smash_pid;
    SmallShell();
 public:
    JobsList jobs;
    std::shared_ptr<Command> CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = default; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
    const string &getSmashName() const {return smash_name;}
    void setSmashName(string new_name);
    pid_t getSmashPID(){return smash_pid;}
};

#endif //SMASH_COMMAND_H_
