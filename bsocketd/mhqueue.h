#ifndef MHQUEUE
#define MHQUEUE

#include <string>
#include <vector>
#include <cstdlib>
#include <map>

struct mh_list {
	std::vector<char> value;
	int count_links;
	mh_list* next;

	mh_list(std::vector<char> value, int count_links) : value(value), count_links(count_links), next(NULL) {}
};

class mh_queue {
	int count_heads;
	mh_list* head;
	mh_list* tail;
	std::map<int, mh_list*> heads;
public:
	mh_queue() {
		count_heads = 0;
		head = tail = new mh_list(std::vector<char>(), 0);
	}

	void add_head(int fd) {
		count_heads++;
		heads[fd] = tail;
		tail->count_links++;
	}

	bool empty(int fd) {
		return heads[fd]->next == NULL;
	}

	void push(std::vector<char> s) {
		if (head == NULL) {
			head = new mh_list(s, count_heads);
			tail = head;
		} else {
			mh_list* el = new mh_list(s, 0);
			tail->next = el;
			tail = el;
		}
	}

	std::vector<char> get(int fd) {
		mh_list* cur = heads[fd];
		heads[fd] = cur->next;
		cur->count_links--;
		if (cur == head && cur->count_links == 0) {
			head = head->next;
			delete cur;
			return head->value;
		}
		return cur->next->value;
	}
};

#endif
