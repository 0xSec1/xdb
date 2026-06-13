#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <libxdb/process.hpp>
#include <libxdb/error.hpp>
#include <signal.h>

using namespace xdb;

namespace{
    bool process_exists(pid_t pid){
        //kill with 0 not sends signal to process but still carry checks
        auto ret = kill(pid, 0);
        return ret != -1 and errno != ESRCH;
    }
}

//reads /proc/<pid>/stat and fetch the status of process
namespace{
    char get_process_status(pid_t pid){
        std::ifstream stat("/proc/" + std::to_string(pid) + "/stat");
        std::string data;
        std::getline(stat, data);
        /*
         ex- 10 (kworker/0:0H-events_highpri) I 2 0
         */
        auto index_of_last_bracket = data.rfind(')');
        auto index_of_status = index_of_last_bracket + 2;
        return data[index_of_status];
    }
}

TEST_CASE("process::launch success", "[process]"){
    auto proc = process::launch("yes");
    REQUIRE(process_exists(proc->pid()));
}

TEST_CASE("process::attach success", "[process]"){
    auto target = process::launch("target/endlessly", false);
    auto proc = process::attach(target->pid());
    REQUIRE(get_process_status(target->pid()) == 't');
}

TEST_CASE("process::launch no such program", "[process]"){
    REQUIRE_THROWS_AS(process::launch("you_not_good"), error);
}

TEST_CASE("process::attach invalid PID", "[process]"){
    REQUIRE_THROWS_AS(process::attach(0), error);
}
