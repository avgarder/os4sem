#include <unistd.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mhqueue.h"

char* s_malloc(size_t size) {
	char* res = new char[size];
	if (res == NULL) {
		perror("Malloc error");
		_exit(31);
	}
	return res;
}

void check_io(int res) {
    if (res == -1) {
        perror("IO error");
        _exit(31);
    }
}

int child_pid = 0;

void handler(int foo) {
    (void)foo;
    if (child_pid)
        kill(-child_pid, SIGINT);
}

int s_poll(pollfd* fds, int nfds, int timeout = -1) {
    int count = poll(fds, nfds, timeout);
    if (count == -1)
    {
        perror("Error");
        std::exit(EXIT_FAILURE);
    }
    return count;
}

int s_read(int fd, char* buf, size_t len) {
	int r = read(fd, buf, len);
	check_io(r);
	return r;
}

int s_write(int fd, char* buf, size_t len) {
	int w = write(fd, buf, len);
	check_io(w);
	return w;
}

const int backlog = 5;

size_t conv_mess_len(char* lbuf) {
	size_t res = 0;
	for (int i = 0; i < 4; i++) {
		res = (res << 8) + lbuf[i];
	}
	// len_buf[4] = 0;
	// res = atoi(len_buf);
	return res;
}

int main() {
	child_pid = fork();
    if (child_pid) {
        printf("daemon started: pid = %d\n", child_pid);
        signal(SIGINT, handler);
        int status;
        waitpid(child_pid, &status, 0);
        printf("daemon stoped\n");
        return 0;
    }
    setsid();
    close(0);
    close(1);
    close(2);

    addrinfo hints;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
 
    addrinfo *result;
    if (getaddrinfo(NULL, "23931", &hints, &result) != 0) {
        _exit(31);
    }
    if (result == NULL) {
        _exit(31);
    }
 
    int socket_fd;
    socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd == -1) {
        _exit(31);
    }
 
    int sso_status = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
                &sso_status, sizeof(int)) == -1) {
        _exit(31);
    }
 
    if (bind(socket_fd, result->ai_addr, result->ai_addrlen) == -1) {
        _exit(31);
    }
   
    if (listen(socket_fd, backlog) == -1) {
        _exit(31);
    }

    pollfd polls[backlog + 1];
   	polls[0].fd = socket_fd;
   	polls[0].events = POLLIN;

   	int clients = 1;

   	int mess_len[backlog + 1];
   	std::vector<char> mess_in[backlog + 1];

   	int mess_writed[backlog + 1];
   	std::vector<char> mess_out[backlog + 1];

    std::vector<char> rlen[backlog + 1];

   	char* client_addr[backlog + 1];

   	for (int i = 0; i < backlog + 1; i++) {
   		mess_len[i] = -1;
   	}

   	const size_t buf_size = 4096;
   	char* buf = s_malloc(buf_size);
    char* len_buf = s_malloc(4);

   	mh_queue q;

   	while (1) {
   		s_poll(polls, clients);
   		for (int i = 1; i < clients; i++) {
   			if (polls[i].revents & (POLLERR | POLLHUP)) {
   				polls[i] = polls[clients - 1];
   				mess_len[i] = mess_len[clients - 1];
   				mess_in[i] = mess_in[clients - 1];
   				mess_len[clients - 1] = -1;
   				mess_in[clients - 1].clear();
                rlen[i] = rlen[clients - 1];
                rlen[clients - 1].clear();
   				client_addr[i] = client_addr[clients - 1];
                clients -= 1;
   			} else if (polls[i].revents & POLLIN) {
				if (mess_len[i] == -1) {
                    int r = s_read(polls[i].fd, len_buf, 4 - rlen[i].size());
                    rlen[i].insert(rlen[i].end(), len_buf, len_buf + r);
                    if (rlen[i].size() == 4) {
					    mess_len[i] = conv_mess_len(rlen[i].data());
					    mess_in[i].clear();
                    }
				} else {
					int to_read = std::min(buf_size, mess_len[i] - mess_in[i].size());
					int r = s_read(polls[i].fd, buf, to_read);
					mess_in[i].insert(mess_in[i].end(), buf, buf + r);

					if ((int) mess_in[i].size() == mess_len[i]) {
						q.push(mess_in[i]);
						mess_len[i] = -1;
                        rlen[i].clear();
						polls[i].events |= POLLOUT;
					}
				}
   			} else if (polls[i].revents & POLLOUT) {
   				if (mess_writed[i] == -1) {
   					mess_out[i] = q.get(polls[i].fd);
   					mess_out[i].insert(mess_out[i].begin(), client_addr[i], client_addr[i] + strlen(client_addr[i]));
   					mess_writed[i] = 0;
   				}
   				int w = s_write(polls[i].fd, mess_out[i].data() + mess_writed[i], mess_out[i].size() - mess_writed[i]);
   				mess_writed[i] += w;
   				if (mess_writed[i] == (int) mess_out[i].size()) {
   					if (!q.empty(polls[i].fd)) {
   						mess_writed[i] = -1;
   					} else {
   						polls[i].events &= (~POLLOUT);
   					}
   				}
   			}
   		}
   		if (polls[0].revents && POLLIN) {
   			sockaddr_in client;
			client.sin_family = AF_INET;
            socklen_t addr_size = sizeof(client);
            int fd_acc = accept(socket_fd, (struct sockaddr *) &client, &addr_size);
            if (fd_acc == -1) {
                _exit(31);
            }
            client_addr[clients] = inet_ntoa(client.sin_addr); 
            polls[clients].fd = fd_acc;
            polls[clients].events = POLLIN | POLLERR | POLLHUP;
            mess_len[clients] = -1;
            mess_writed[clients] = -1;
            q.add_head(polls[clients].fd);
            clients += 1;
        }
   	}
    return 0;
}
