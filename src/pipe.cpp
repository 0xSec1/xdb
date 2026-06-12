#include <cstddef>
#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include <libxdb/pipe.hpp>
#include <libxdb/error.hpp>

xdb::pipe::pipe(bool close_on_exec){
    if(pipe2(fds, close_on_exec ? O_CLOEXEC : 0) < 0){
        error::send_errno("pipe creation failed");
    }
}

xdb::pipe::~pipe(){
    close_read();
    close_write();
}

//release_* : return current fd for relevant end of pipe and set it to empty i.e. -1
int xdb::pipe::release_read(){
    return std::exchange(fds[read_fd], -1);
}

int xdb::pipe::release_write(){
    return std::exchange(fds[write_fd], -1);
}

void xdb::pipe::close_read(){
    if(fds[read_fd] != -1){
        close(fds[read_fd]);
        fds[read_fd] = -1;
    }
}

void xdb::pipe::close_write(){
    if(fds[write_fd] != -1){
        close(fds[write_fd]);
        fds[write_fd] = -1;
    }
}

std::vector<std::byte> xdb::pipe::read(){
    char buf[1024];
    int char_read;

    //call read in global namespace
    if((char_read = ::read(fds[read_fd], buf, sizeof(buf))) < 0){
        error::send_errno("Could not read from pipe");
    }

    auto bytes = reinterpret_cast<std::byte*>(buf);
    return std::vector<std::byte>(bytes, bytes + char_read);
}

void xdb::pipe::write(std::byte *from, std::size_t bytes){
    if(::write(fds[write_fd], from, bytes) < 0){
        error::send_errno("could not write to pipe");
    }
}
