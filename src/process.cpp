#include <libxdb/process.hpp>
#include <libxdb/error.hpp>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

std::unique_ptr<xdb::process> xdb::process::launch(std::filesystem::path path){
    pid_t pid;
    if((pid = fork()) < 0){
        error::send_errno("Fork failed");
    }

    if(pid == 0){
        if(ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0){
            error::send_errno("Tracing failed");
        }
        if(execlp(path.c_str(), path.c_str(), nullptr) < 0){
            error::send_errno("exec failed");
        }
    }

    std::unique_ptr<process> proc(new process(pid, true));
    proc->wait_on_signal();

    return proc;
}

std::unique_ptr<xdb::process> xdb::process::attach(pid_t pid){
    if(pid == 0){
        error::send_errno("invalid PID");
    }
    if(ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0){
        error::send_errno("Could not attach");
    }

    std::unique_ptr<process> proc(new process(pid, false));
    proc->wait_on_signal();\

    return proc;
}

//deconstructor calling kill on child process and wait untill it exits
xdb::process::~process(){
    if(pid_ != 0){
        int status;
        if(state_ == process_state::running){
            kill(pid_, SIGSTOP);
            waitpid(pid_, &status, 0);
        }

        ptrace(PTRACE_DETACH, pid_, nullptr, nullptr);
        kill(pid_, SIGCONT);

        if(terminate_on_end_){
            kill(pid_, SIGKILL);
            waitpid(pid_, &status, 0);
        }
    }
}
