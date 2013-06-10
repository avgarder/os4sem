#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
const size_t fdsize = 100;

char* to_string(int value) {
	char* temp = (char *) malloc(fdsize);
	size_t k = 0;
	*temp = '0';
	while (value > 0) {
		int a = value % 10;
		*(temp + k) = a + '0';
		value /= 10;
		k++;
	}
	size_t i;
	char* res = (char *) malloc(fdsize);
	*res  = '0';
	for (i = 0; i < k; i++) {
		*(res + i) = *(temp + k - i - 1);
	}
    res[k] = 0;
	free(temp);
	return res;
}

int main(int argc, char * argv[]) {
	char * fds[argc + 1];
	size_t i;
	for (i = 1; i < (size_t)argc; i++) {
		int fd;
        if (i % 2 == 1) {
            fd = open(argv[i], O_RDONLY, S_IRUSR | S_IWUSR);
        } else {
            fd = open(argv[i], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        }
//        int fd = open(argv[i], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		fds[i] = to_string(fd);
    }
	fds[0] = (char * ) "./propogater\0";
	fds[argc] = 0;
	int pid = fork();
	if (pid == 0) {
		execvp(fds[0], fds);
    } else {
		int status;
		waitpid(pid, &status, 0);
	}
	
	return 0;
}