#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <utility>
#include <iostream>
#include <memory.h>
#include "optional.cpp"

struct lazy_stream {
    int fd;
    size_t buf_size;
    char d;
    std::vector<char> in;
    size_t readed;
    
    lazy_stream(int fd, size_t buf_size, char d) : fd(fd), buf_size(buf_size), d(d), readed(0) {
        in.resize(buf_size);
    }
    
private:
    optional< std::vector<char> >& get_res(int dpos) {
        std::vector<char> tail(buf_size);
        memcpy(tail.data(), in.data() + dpos + 1, readed - dpos - 1);
        optional< std::vector<char> > res(std::move(in));
        in = std::move(tail);
        return res;
    }

    void check_io(int io_res) {
        if (io_res == -1) {
            exit(31);
        }
    }

public:
    
    optional< std::vector<char> >& lazy_read() {
        int dpos = -1;
        for (size_t i = 0; i < readed; i++) {
            if (in[i] == d) {
                dpos = i;
                break;
            }
        }
        if (dpos != -1) {
            return get_res(dpos);
        }
        while (readed < buf_size) {
            int r = read(fd, in.data() + readed, buf_size - readed);
            check_io(r);
            for (int i = readed; i < (int) (readed + r); i++) {
                if (in[i] == d) {
                    dpos = i;
                    break;
                }
            }
            readed += r;
            if (dpos != -1) {
                return get_res(dpos);
            }
        }
        while (dpos == -1) {
            int r = read(fd, in.data(), buf_size);
            check_io(r);
            for (int i = 0; i < r; i++) {
                if (in[i] == d) {
                    dpos = i;
                    break;
                }
            }
        }
        std::vector<char> tail(buf_size);
        memcpy(tail.data(), in.data() + dpos + 1, readed - dpos - 1);
        in = std::move(tail);
        return optional< std::vector<char> >();
    }

    void lazy_write(std::vector<char>& data) {
        size_t cur = 0;
        while (cur < data.size()) {
            size_t to_write = std::min(buf_size, data.size() - cur);
            int w = write(fd, data.data() + cur, to_write);
            check_io(w);
            cur += w;
        }
        write(fd, &d, 1);
    }
};

int main() {
    return 0;
}
