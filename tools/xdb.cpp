#include <cstdio>
#include <cstring>
#include <memory>
#include <ostream>
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
#include <libxdb/process.hpp>
#include <libxdb/error.hpp>

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
}

namespace{
    std::unique_ptr<xdb::process> attach(int argc, const char* argv[]){
        //pass pid
        if(argc == 3 && argv[1] == std::string_view("-p")){
            pid_t pid = std::atoi(argv[2]);
            return xdb::process::attach(pid);
        }

        //passing program name
        else{
            const char* program_path = argv[1];
            return xdb::process::launch(program_path);
        }
    }
}

//print reason on wait_on_signal
namespace{
    void print_stop_reason(const xdb::process &process, xdb::stop_reason reason){
        std::cout << "Process " << process.pid() << ' ';

        switch(reason.reason){
            case xdb::process_state::exited:
                std::cout << "exited with status " << static_cast<int>(reason.info);
                break;

            case xdb::process_state::terminated:
                std::cout << "terminated with signal " << sigabbrev_np(reason.info);
                break;

            case xdb::process_state::stopped:
                std::cout << "stopped with signal " << sigabbrev_np(reason.info);
                break;
        }
        std::cout << std::endl;
    }

    void HandleCommnd(std::unique_ptr<xdb::process> &process, std::string_view line){
        auto args = split(line, ' ');
        auto command = args[0];

        if(is_prefix(command, "continue")){
            process->resume();
            auto reason = process->wait_on_signal();
            print_stop_reason(*process, reason);
        }
        else{
            std::cerr << "Unknown Command\n";
        }
    }

}

namespace{
    void main_loop(std::unique_ptr<xdb::process> &process){
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
                try{
                    HandleCommnd(process, line_str);
                }
                catch(const xdb::error &err){
                    std::cout << err.what() << '\n';
                }
            }

        }
    }
}

int main(int argc, const char* argv[]){
    if(argc == 1){
        std::cerr << "No args given\n";
        return -1;
    }

    try{
        auto process = attach(argc, argv);
        main_loop(process);
    }
    catch(const xdb::error &err){
        std::cout << err.what() << '\n';
    }
}
