#include <iostream>
#include <libxdb/process.hpp>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

std::unique_ptr<xdb::process> xdb::process::launch(std::filesystem::path path){
    pid_t pid;
    if((pid = fork()) < 0){
        std::cerr << "fork failed";
    }

    if(pid == 0){
        if(ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0){
            //error tracing failed
        }
        if(execlp(path.c_str(), path.c_str(), nullptr) < 0){
            //exec failed
        }
    }

    std::unique_ptr<process> proc(new process(pid, true));
    proc->wait_on_signal();

    return proc;
}

std::unique_ptr<xdb::process> xdb::process::attach(pid_t pid){
    if(pid == 0){
        //error invalid pid
    }
    if(ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0){
        //error could not attach
    }

    std::unique_ptr<process> proc(new process(pid, false));
    proc->wait_on_signal();\

    return proc;
}
