#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string_view>
#include <iostream>
#include <unistd.h>

namespace{
    pid_t attach(int argc, char* argv[]){
        pid_t pid = 0;

        //passing pid
        if(argc == 3 && argv[1] == std::string_view("-p")){
            pid = std::atoi(argv[2]);
            if(pid <= 0){
                std::cerr << "Invalid PID\n";
                return -1;
            }

            if(ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0){
                std::perror("Could not attach");
                return -1;
            }
        }
        //passing name
        else{
            const char* programPath = argv[1];
            if((pid = fork()) < 0){
                std::perror("fork failed");
                return -1;
            }

            if(pid == 0){
                //if in child process, execute debugee
                if(ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0){
                    std::perror("Trace failed");
                    return -1;
                }

                if(execlp(programPath, programPath, nullptr) < 0){
                    std::perror("Exec failed");
                    return -1;
                }
            }

        }
        return pid;
    }
}

int main(int argc, char* argv[]){
    if(argc == 1){
        std::cerr << "No args given\n";
        return -1;
    }

    pid_t pid = attach(argc, argv);

    int waitStatus;
    int options = 0;
    if(waitpid(pid, &waitStatus, options) < 0){
        std::perror("waitpid failed");
        return -1;
    }
}
