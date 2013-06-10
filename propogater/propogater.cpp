#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <memory.h>
#include <string.h>
#include <errno.h>

const size_t BUFFER_SIZE = 1024;

struct pair_io {
    int fd_in;
    int fd_out;
    bool in_alive, out_alive, no_error;
    size_t buffer_len;
    char* buffer;

    pair_io(int fd_in, int fd_out) : fd_in(fd_in), fd_out(fd_out), in_alive(true), out_alive(true), no_error(true) {
        buffer_len = 0;
        buffer = new char[BUFFER_SIZE];
    }

    ~pair_io() {
        delete[] buffer;
    }

    short get_poll_in() {
        short res = 0;
        if (no_error) {
            res = POLLERR;
            if (in_alive && buffer_len < BUFFER_SIZE) {
                res |= POLLIN;
            }
        }
        return res;
    }

    short get_poll_out() {
        short res = 0;
        if (no_error) {
            res = POLLERR;
            if (out_alive && buffer_len > 0) {
                res |= POLLOUT;
            }
        }
        return res;
    }
};

int main(int argc, char** argv) {
    int count_fd = argc - 1;
    pollfd fds[count_fd];
    pair_io* pairs[count_fd / 2];

    for (int i = 0; i < count_fd / 2; i++) {
        int fd_in = atoi(argv[i * 2 + 1]);
        int fd_out = atoi(argv[i * 2 + 2]);
        pairs[i] = new pair_io(fd_in, fd_out);
    }

    int count_alive = count_fd / 2;
    while (count_alive > 0) {
        for (int i = 0; i < count_fd / 2; i++) {
            pair_io & pair = *pairs[i];
            fds[i * 2].fd = pair.fd_in;
            fds[i * 2 + 1].fd = pair.fd_out;
            fds[i * 2 + 1].revents = 0;
            if (pair.no_error && (pair.in_alive || pair.out_alive)) {
                fds[i * 2].events = pair.get_poll_in();
                fds[i * 2 + 1].events = pair.get_poll_out();
            } else {
                fds[i * 2].events = 0;
                fds[i * 2 + 1].events = 0;
                count_alive--;
            }
        }
        if (count_alive == 0) {
            break;
        }
        poll(fds, count_fd, -1);
        for (int i = 0; i < count_fd; i++) {
            short res = fds[i].revents;
            if (res != 0) {
                pair_io & pair = *pairs[i / 2];
                if (res & POLLERR) {
                    pair.no_error = false;
                } else if (res & POLLIN) {
                    int r = read(pair.fd_in, pair.buffer + pair.buffer_len, BUFFER_SIZE - pair.buffer_len);
                    pair.buffer_len += r;
                    if (r == 0) {
                        pair.in_alive = false;
                        if (pair.buffer_len == 0) {
                            pair.out_alive = false;
                        }
                    }
                } else if (res & POLLOUT) {
                    int w = write(pair.fd_out, pair.buffer, pair.buffer_len);
                    pair.buffer_len -= w;
                    memcpy(pair.buffer, pair.buffer + w, pair.buffer_len);
                    if ((!pair.in_alive) && pair.buffer_len == 0) {
                        pair.out_alive = false;
                    }
                }
            }
        }
    }
    return 0;
}
