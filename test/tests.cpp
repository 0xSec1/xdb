#include <catch2/catch_test_macros.hpp>
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

TEST_CASE("process::launch success", "[process]"){
    auto proc = process::launch("yes");
    REQUIRE(process_exists(proc->pid()));
}

TEST_CASE("process::launch no such program", "[process]"){
    REQUIRE_THROWS_AS(process::launch("you_not_good"), error);
}
