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
    SmallShell& smash = SmallShell::getInstance();
    smash.smashJobs.removeFinishedJobs();
    cout << "smash: got an alarm" << endl;
    time_t currentTime;
    time(&currentTime);
    JobsList::JobEntry* timedOutCmd = nullptr;
    if (smash.timedCommands.jobList.empty())
        timedOutCmd = nullptr;
    for (auto& cmd : smash.timedCommands.jobList)
        if (cmd.duration - difftime(currentTime,cmd.execTime) == 0)
            timedOutCmd = &cmd;
    if (timedOutCmd != nullptr) {
        cout << "smash: " << timedOutCmd->ext_cmd->cmd_line << " timed out!" << endl;
        if (kill(timedOutCmd->jobPID,SIGKILL) == -1) {
            perror("smash error: kill failed");
            return;
        }
        smash.timedCommands.removeJobByPid(timedOutCmd->jobPID);
    }
    int nextAlarm;
    if (smash.timedCommands.jobList.empty())
        nextAlarm = -1;
    else {
        time_t curr_time;
        time(&curr_time);
        nextAlarm = smash.timedCommands.jobList.begin()->duration - difftime(curr_time, smash.timedCommands.jobList.begin()->execTime);
        for (auto& cmd : smash.timedCommands.jobList) {
            int curr = cmd.duration - difftime(curr_time, cmd.execTime);
            if (curr > 0 && curr < nextAlarm)
                nextAlarm = cmd.duration - difftime(curr_time, cmd.execTime);
        }
    }
    alarm(nextAlarm);
}

