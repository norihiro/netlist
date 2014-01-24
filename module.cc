
#include "config.h"
#include "module.h"
#include <map>
#include <string>
#include <cstring>
#include <cstdio>

static std::map<std::string, module> mm;

void module::add_port(const char *name)
{
	class net n;
	n.name = name;
	ports.push_back(n);
	nets.push_back(n);
}

class net * module::get_net(const char *name, bool ignorecase)
{
	int (*comp)(const char *, const char *) = ignorecase ? strcasecmp : strcmp;
	for(int i=0; i<nets.size(); i++)
		if(!comp(nets[i].name.c_str(), name))
			return &nets[i];
	return 0;
}

class net * module::add_net(const char *name)
{
	class net n;
	n.name = name;
	nets.push_back(n);
	return &nets[nets.size()-1];
}

int module::port2order(const char *x) const
{
	for(int i=0; i<ports.size(); i++)
		if(ports[i].name==x)
			return i+1;
	return 0;
}

std::string module::name_instance(const char *prefix, int *id)
{
	int lid = 0; if(!id) id=&lid;
	char s[64]; sprintf(s, "%d", (*id)++);
	return ((std::string)prefix)+s;
}

module *module::create(const char *name)
{
	if(mm.count(name))
		return NULL;
	module *m = &mm[name];
	m->name = name;
	return m;
}

module *module::get(const char *name)
{
	if(mm.count(name))
		return &mm[name];
	else
		return NULL;
}


std::map<std::string, module> every_modules() { return mm; }
