#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"


//added libraries
#include <string>
#include <sys/wait.h>

#define NO_RUNNING_CMD 0

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    cout << "smash: got ctrl-Z" << endl;
    SmallShell& smash = SmallShell::getInstance();
    if(smash.getRunningCmd() != NO_RUNNING_CMD)
    {
        int status;
        status = waitpid(smash.getRunningCmd(), nullptr, WNOHANG);
        // if status == -1  <--> process died
        if(status == -1 || status == smash.getRunningCmd()) {
            return;
        }
        if(kill(smash.getRunningCmd(),SIGSTOP) == -1) {
            perror("smash error: kill failed");
            return;
        }
        cout << "smash: process " << smash.getRunningCmd() << " was stopped" << endl;
        smash.jobs.StopFG();
    }
    auto* ext_Cmd_Ptr = dynamic_cast<ExternalCommand *>(smash.cmd);
    if(ext_Cmd_Ptr->state==BACKGROUND)
    {
        return;
    }
    if(kill(ext_Cmd_Ptr->pid,SIGSTOP) != 0)
    {
        perror("smash error: kill failed");
    }
    else{
        ext_Cmd_Ptr->state=BACKGROUND;
        smash.cmd= nullptr;
        //smash.jobs->addJob(ext_Cmd_Ptr, ); ///need to fill
        cout << "smash: process " <<  to_string(ext_Cmd_Ptr->pid) << " was killed" << endl;
    }
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
  cout << "smash: got ctrl-C" << endl;
  SmallShell& smash = SmallShell::getInstance();
  if(smash.cmd == nullptr)
  {
      return;
  }
  auto* ext_Cmd_Ptr = dynamic_cast<ExternalCommand *>(smash.cmd);
  if(ext_Cmd_Ptr->state==BACKGROUND)
  {
      return;
  }
  if(kill(ext_Cmd_Ptr->pid, SIGKILL) != 0)
  {
      perror("smash error: kill failed");
  }
  else
  {
      std::cout << "smash: process " << ext_Cmd_Ptr->pid << " was killed" << std::endl;
  }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

