#include <cstdio>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string_view>
#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <editline/readline.h>

//handle user command
namespace{
    std::vector<std::string> split(std::string_view str, char delimiter){
        std::vector<std::string> output{};
        std::stringstream ss {std::string{str}};
        std::string item;

        while(std::getline(ss, item, delimiter)){
            output.push_back(item);
        }

        return output;
    }

    bool is_prefix(std::string_view str, std::string_view of){
        if(str.size() > of.size()) return false;
        return std::equal(str.begin(), str.end(), of.begin());
    }

    void resume(pid_t pid){
        if(ptrace(PTRACE_CONT, pid, nullptr, nullptr) < 0){
            std::cerr << "Couldn't Continue\n" << std::endl;
            std::exit(-1);
        }
    }

    void wait_on_signal(pid_t pid){
        int waitStatus;
        int options = 0;
        if(waitpid(pid, &waitStatus, options) < 0){
            std::perror("waitpid failed");
            std::exit(-1);
        }
    }

    void HandleCommnd(pid_t pid, std::string_view line){
        auto args = split(line, ' ');
        auto command = args[0];

        if(is_prefix(command, "continue")){
            resume(pid);
            wait_on_signal(pid);
        }
        else{
            std::cerr << "Unknown Command\n" << std::endl;
        }
    }
}

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

    char* line = nullptr;
    while((line = readline("xdb> ")) != nullptr){
        /*
         if user enters nothing it will execute previous command
         */
        std::string line_str;

        if(line == std::string_view("")){
            free(line);
            if(history_length > 0){
                line_str = history_list()[history_length - 1] -> line;
            }
        }
        else{
            line_str = line;
            add_history(line);
            free(line);
        }

        if(!line_str.empty()){
            HandleCommnd(pid, line_str);
        }
    }
}
