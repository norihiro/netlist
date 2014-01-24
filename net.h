#ifndef NET_H
#define NET_H

#include <string>

class net
{
public:
	std::string name;
	int suffix_max, suffix_min;

	net() { suffix_min=suffix_max=0; }
	net(const char *n) { suffix_min=suffix_max=0; name=n; }
	bool operator < (const class net &x) const { return name < x.name; }
};

#endif // NET_H
