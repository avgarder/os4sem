#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

const int STDIN = 0;
const int STDOUT = 1;

int get_pos_0(char* c, size_t from, size_t to) {
	for (; from < to; from++) {
		if (*(c + from) == '\n') {
			return from;
		}
	}
	return -1;
}

void check_io(int res) {
	if (res == -1) {
		exit(31);
	}
}

int main(int argc, char** argv) {
	size_t buffer_size = (size_t) atoi(argv[1]) + 1;
	char* buffer = (char*) malloc(sizeof(char) * buffer_size);
	size_t buffer_len = 0;
	int r, w;
	while (true) {
		r = read(STDIN, buffer + buffer_len, buffer_size - buffer_len);
		check_io(r);
		if (r == 0) {
			break;
		}
		int pos = get_pos_0(buffer, buffer_len, buffer_len + r);
		buffer_len += r;
		if (pos != -1) {
			int writed = 0;
			while (writed < pos) {
				w = write(STDOUT, buffer + writed, pos - writed);
				check_io(w);
				writed += w;
			}
			memmove(buffer, buffer + pos + 1, buffer_len - pos - 1);
			buffer_len = buffer_len - pos - 1;
		} else if (buffer_len == buffer_size) {
			while (pos == -1) {
				r = read(STDIN, buffer, buffer_size);
				check_io(r);
				if (r == 0) {
					break;
				}
				pos = get_pos_0(buffer, 0, r);
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
