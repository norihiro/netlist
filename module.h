#ifndef MODULE_H
#define MODULE_H

#include "net.h"
#include "instance.h"
#include <vector>
#include <string>

class module
{
public:
	std::string name;
	std::vector<class net> ports;
	std::vector<class net> nets;
	std::vector<class instance> instances;

	bool operator < (const class module &m) const { return name < m.name; }
	void add_port(const char *name);
	net * add_net(const char *name);
	net * get_net(const char *name, bool ignorecase);
	instance *create_instance(const char *name=0) { instance i; if(name) i.name=name; instances.push_back(i); return &instances[instances.size()-1]; }
	int port2order(const char *name) const;
	std::string name_instance(const char *prefix, int *id=0);

	static module *create(const char *name);
	static module *get(const char *name);
};

#endif // MODULE_H
