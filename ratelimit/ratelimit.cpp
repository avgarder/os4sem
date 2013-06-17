#include <unistd.h>
#include <stdio.h>
#include <queue>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory.h>

using std::chrono::milliseconds;
using std::chrono::duration_cast;

const int STDIN = 0;
const int STDOUT = 1;

const size_t BUFFER_SIZE = 1024;

char* s_malloc(size_t size) {
	char* res = new char[size];
	if (res == NULL) {
		perror("Malloc error");
		_exit(1);
	}
	return res;
}

void check_io(int res) {
    if (res == -1) {
        perror("IO error");
        _exit(1);
    }
}

int get_pos_n(char* c, size_t from, size_t to) {
    for (; from < to; from++) {
        if (c[from] == '\n') {
            return from;
        }
    }
    return -1;
}

auto t0 = std::chrono::high_resolution_clock::now();

long long get_now_millis() {
    auto t1 = std::chrono::high_resolution_clock::now();
    return duration_cast<milliseconds>(t1 - t0).count();
}

void write_all(char* buffer, size_t from, size_t count) {
    while (count > 0) {
        int w = write(STDOUT, buffer + from, count);
        check_io(w);
        from += w;
        count-= w;
    }
}

int main(int argc, char** argv) {
	if (argc < 2) {
		perror("Wrong usage");
		_exit(1);
	}
    char* buffer = s_malloc(BUFFER_SIZE);
    size_t buffer_len = 0;
	size_t n = atoi(argv[1]);
	std::queue<long long> clocks;
	while (1) {
		long long now = get_now_millis();
		while (!clocks.empty()) {
            long long t = clocks.front();
            if (now - t > 1000) {
                clocks.pop();
            } else {
                break;
            }
		}
        if (clocks.size() < n) {
            int pos;
            int r;
            while (1) {
                pos = get_pos_n(buffer, 0, buffer_len);
                if (pos != -1) {
                    break;
                }
                write_all(buffer, 0, buffer_len);
                r = read(STDIN, buffer, BUFFER_SIZE);
                check_io(r);
                if (r == 0) {
                    break;
                }
                buffer_len = r;
            }
            if (r == 0) {
                break;
            }

            write_all(buffer, 0, pos + 1);
            memmove(buffer, buffer + pos + 1, buffer_len - pos - 1);
            buffer_len -= pos + 1;
            clocks.push(now);
        } else {
            std::this_thread::sleep_for(milliseconds(1000 - now + clocks.front()));
        }   
	}

    return 0;
}
