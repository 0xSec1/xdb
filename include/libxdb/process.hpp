/*
Create a class process use to launch, attach and wait for signals
Using unique_ptr that manage memory allocation
*/
#ifndef XDB_PROCESS_HPP
#define XDB_PROCESS_HPP

#include <filesystem>
#include <memory>
#include <sys/types.h>

namespace xdb{
    //tracks current running state
    enum class process_state{
        stopped,
        running,
        exited,
        terminated
    };

    class process{
    public:
        //disable default constructor and copy operation
        process() = delete;
        process(const process&) = delete;
        process& operator=(const process&) = delete;

        ~process();

        static std::unique_ptr<process> launch(std::filesystem::path path);
        static std::unique_ptr<process> attach(pid_t pid);

        process_state state() const{ return state_; }   //returns current state

        void resume();
        void wait_on_signal();
        pid_t pid() const{ return pid_; }

    private:
        pid_t pid_ = 0;
        bool terminate_on_end_ = true;
        process_state state_ = process_state::stopped;

        process(pid_t pid, bool terminate_on_end)
            : pid_(pid), terminate_on_end_(terminate_on_end) {}
    };
}

#endif
