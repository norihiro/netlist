#ifndef INSTANCE_H
#define INSTANCE_H

#include "con.h"
#include <vector>
#include <map>
#include <string>
#include <climits>

class instance
{
public:
	enum type_e {
		t_instance = 0,
		t_premitive,
		t_connect,
		t_end = INT_MAX
	};
	typedef std::map<std::string, std::string> attributes_t;
	typedef std::vector<class con> ports_t;
	type_e type;
	std::string ref;
	std::string name;
	ports_t ports;
	attributes_t  attributes;

	instance () { type = t_instance; }
	bool operator < (const class instance &x) const {
		if(name != x.name) return name < x.name;
		if(type != x.type) return (int)type < (int)x.type;
		if(ref != x.ref) return ref < x.ref;
		return 0;
	}

	int sort_order_by_name();
};

#endif // INSTANCE_H
