#ifndef CON_H
#define CON_H

#include <string>
#include <vector>
#include "net.h"

class con
{
public:
	std::string port;
	int suffix_max, suffix_min;
	std::vector<net> nets;
	int order;

	con() { _init(); }
	con(net &n) { _init(); nets.push_back(n); }
	bool operator < (const class con &x) const {
		if(order!=x.order) return order<x.order;
		return port < x.port;
	}

private:
	void _init() { suffix_min=suffix_max=0; order=0; }
};

#endif // CON_H
