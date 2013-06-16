#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

const int STDIN = 0;
const int STDOUT = 1;

const char* READ = "read failed";
const char* WRITE = "write_failed";

int find_n(char* c, size_t from, size_t to) {
	for (; from < to; from++) {
		if (*(c + from) == '\n') {
			return from;
		}
	}
	return -1;
}

void check_io(const char* comment, int what) {
	if (what < 0) {
		perror(comment);
		_exit(1);
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		return 1;
	}
	size_t buffer_size = (size_t) atoi(argv[1]) + 1;
	char* buffer = (char*) malloc(sizeof(char) * buffer_size);
	size_t buffer_len = 0;
	int r, w;
	int i;
	while (1) {
		int pos = find_n(buffer, 0, buffer_len);
		if (pos == -1) {
			r = read(STDIN, buffer + buffer_len, buffer_size - buffer_len);
			check_io(READ, r);
			if (r == 0) {
				break;
			}
			pos = find_n(buffer, buffer_len, buffer_len + r);
		}
		buffer_len += r;
		if (pos != -1) {
			for (i = 0; i < 2; i++) {
				int writed = 0;
				while (writed <= pos) {
					w = write(STDOUT, buffer + writed, pos + 1 - writed);
					check_io(WRITE, w);
					writed += w;
				}
			}
			memmove(buffer, buffer + pos + 1, buffer_len - pos - 1);
			buffer_len = buffer_len - pos - 1;
		} else if (buffer_len == buffer_size) {
			while (pos == -1) {
				r = read(STDIN, buffer, buffer_size);
				check_io(READ, r);
				if (r == 0) {
					break;
				}
				pos = find_n(buffer, 0, r);
			}
			if (r == 0) {
				break;
			}
			memmove(buffer, buffer + pos + 1, r - pos - 1);
			buffer_len = r - pos - 1;
		}
	}
	return 0;
}	
