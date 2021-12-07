#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    cout << "smash: got ctrl-Z" << endl;
    SmallShell& smash = SmallShell::getInstance();
    // checks if there is an external command running in the fg:
    if(smash.jobs.getFgJob())
    {
        int returnPid =  waitpid(smash.jobs.getFgJob()->getJobPid(), nullptr, WNOHANG);
        if (returnPid == -1) {
            perror("smash error: waitpid failed");
            return;
        }
        if(kill(smash.jobs.getFgJob()->getJobPid(),SIGSTOP) == -1) {
            perror("smash error: kill failed");
            return;
        }

        cout << "smash: process " << smash.jobs.getFgJob()->getJobPid() << " was stopped" << endl;
        smash.jobs.StopFG();
    }
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    cout << "smash: got ctrl-C" << endl;
    SmallShell& smash = SmallShell::getInstance();
    // checks if there is an external command running:
    if(smash.jobs.getFgJob()) {
        int returnPid =  waitpid(smash.jobs.getFgJob()->getJobPid(), nullptr, WNOHANG);
        if (returnPid == -1) {
            perror("smash error: waitpid failed");
            return;
        }
        if (kill(smash.jobs.getFgJob()->getJobPid(),SIGKILL) == -1) {
            perror("smash error: kill failed");
            return;
        }
        cout << "smash: process " << smash.jobs.getFgJob()->getJobPid() << " was killed" << endl;
    }
    smash.jobs.setFgJob(nullptr);
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

