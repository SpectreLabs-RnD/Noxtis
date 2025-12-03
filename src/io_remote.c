
#include "io_remote.h"

#include <stdio.h>

#include <fcntl.h>

#include <unistd.h>


int make_nonblocking(int fd) {

    int flags = fcntl(fd, F_GETFL, 0);

    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);

}
