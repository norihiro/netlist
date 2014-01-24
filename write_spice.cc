
#include "config.h"
#include "write_spice.h"
#include "module.h"
#include <map>
#include <string>
#include <set>
#include <queue>
#include <cstring>
#include <fstream>
#include <iostream>
#define E 3000

std::map<std::string, module> every_modules();

static int write_spice_module(std::ostream &os, module &m, const write_spice_opt *opt)
{
	using std::endl;
	if(m.name.size()) {
		os << ".subckt " << m.name;
		for(int i=0; i<m.ports.size(); i++)
			os << ' ' << m.ports[i].name;
		os << endl;
	}
	int id_connect = 1;

	for(int i=0; i<m.instances.size(); i++) {
		instance &inst = m.instances[i];
		if(inst.type==instance::t_premitive) {
			os << inst.ref << inst.name;
			for(int i=0; i<inst.ports.size(); i++)
				os << ' ' << inst.ports[i].nets[0].name;
			instance::attributes_t &aa = inst.attributes;
			os << ' ' << aa[""];
			for(instance::attributes_t::iterator it=aa.begin(); it!=aa.end(); it++) if(it->first.size())
				os << ' ' << it->first << '=' << it->second;
			os << endl;
		}
		else if(inst.type==instance::t_instance) {
			inst.sort_order_by_name();
			os << 'x' << inst.name;
			/*
			if(opt->pins) {
				os << " $PINS";
				instance::ports_t pp = inst.ports;
				for(int i=0; i<pp.size(); i++)
					os << ' ' << pp[i].port << '=' << pp[i].nets[0].name;
			}
			*/
			for(int i=0; i<inst.ports.size(); i++)
				os << ' ' << inst.ports[i].nets[0].name;
			os << ' ' << inst.ref << endl;
		}
		else if(inst.type==instance::t_connect) {
			inst.name = m.name_instance("connect", &id_connect);
			if(inst.ports.size()==2)
				os << 'R' << inst.name << ' ' << inst.ports[0].nets[0].name << ' ' << inst.ports[1].nets[0].name << " 0" << endl;
		}
	}
	if(m.name.size()) {
		os << ".ends" << endl;
	}
	return 0;
}

static int write_spice_all(std::ostream &os, const write_spice_opt *opt)
{
	std::map<std::string, module> mm = every_modules();
	int first=1;
	for(std::map<std::string, module>::iterator it=mm.begin(); it!=mm.end(); it++) {
		if(first)
			first=0;
		else
			os << std::endl;
		if(int ret=write_spice_module(os, it->second, opt)) return ret;
	}
	return 0;
}

static int write_spice_tree(std::ostream &os, const write_spice_opt *opt)
{
	std::queue<std::string> q;
	std::set<std::string> ws, qs;

	int first=1;
	q.push(opt->top);
	qs.insert(opt->top);
	while(q.size()) {
		std::string name = q.front(); q.pop();
		if(ws.count(name)==0) {
			module *m = module::get(name.c_str());
			if(!m) {
				fprintf(stderr, "error: module %s not found\n", name.c_str());
				return E+__LINE__;
			}
			int n_undef = 0;
			for(int i=0; i<m->instances.size(); i++) {
				instance &ii = m->instances[i];
				if(ii.type==instance::t_instance) {
					if(qs.count(ii.ref)==0) {
						qs.insert(ii.ref);
						q.push(ii.ref);
					}
					if(ws.count(ii.ref)==0)
						n_undef++;
				}
			}
			if(n_undef) {
				q.push(name);
			}
			else {
				if(first)
					first=0;
				else
					os << std::endl;
				if(int ret=write_spice_module(os, *m, opt)) return ret;
				ws.insert(name);
			}
		}
	}
	return 0;
}

static int write_spice_std(std::ostream &os, const write_spice_opt *opt)
{
	if(opt->top)
		return write_spice_tree(os, opt);
	else {
		int ret = write_spice_all(os, opt);
		if(ret) fprintf(stderr, "error write_spice_all %d\n", ret);
		return ret;
	}
}

int write_spice(const char *name, const write_spice_opt *opt)
{
	if(strcmp(name, "-")) {
		std::ofstream ofs(name);
		return write_spice_std(ofs, opt);
	}
	else
		return write_spice_std(std::cout, opt);
}
