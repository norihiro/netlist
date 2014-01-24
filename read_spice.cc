
#include "config.h"
#include "read_spice.h"
#include "module.h"
#include "instance.h"
#include <vector>
#include <fstream>
#include <cstring>
#include <algorithm>
#define E 1000
#define db(x)

static int read_file(const char *filename, std::vector<char> &res, const read_spice_opt *opt)
{
	db(printf("read_file(filename=%s)\n", filename));
	std::ifstream is(filename);
	std::string line;
	for(int lineno=1; getline(is, line); lineno++) {
		if(line[0]=='+' && res.size()>2) {
			if(res[res.size()-1]=='\n')
				res.pop_back();
			else {
				fprintf(stderr, "%s:%d: error: + found but no previous line\n", filename, lineno);
				return E+__LINE__;
			}
		}
		if(line.size()==0)
			continue;
		if(line[0]=='*') {
			if(opt->dot_connect && line.substr(0, 9)=="*.CONNECT")
				;
			else
				continue;
		}
		if(line[0]=='.') {
			if(line.size()>4 && tolower(line[1])=='i' && tolower(line[2])=='n' && tolower(line[3])=='c') {
				char * name = (char*)line.c_str();
				while(*name && !isspace(*name)) name++;
				while(*name && isspace(*name)) name++;
				if(*name=='"' || *name=='\'') name++;
				int len = strlen(name);
				while(len>0 && isspace(name[len-1])) name[--len]=0;
				if(name[len-1]=='"' || name[len-1]=='\'')
					name[--len]=0;
				if(int ret=read_file(name, res, opt)) return ret;
				continue;
			}
		}
		for(int i=0; i<line.size(); i++) {
			char c = line[i];
			if(c=='+' && i==0) continue;
			if(c=='$') {
				if(opt->pins && line[i+1]=='P' && line[i+2]=='I' && line[i+3]=='N' && line[i+4]=='S')
					;
				else
					break;
			}
			res.push_back(c);
		}
		res.push_back('\n');
	}
	return 0;
}

static char * get_keyword(char *&ptr)
{
	while(*ptr && isspace(*ptr)) ptr++;
	if(!*ptr) return NULL;
	char *s = ptr;
	while(*ptr && !isspace(*ptr)) ptr++;
	if(*ptr) *ptr++ = 0;
	return s;
}

static std::string get_keyword(char *&ptr, int &size)
{
	std::string s;
	while(size && *ptr && isspace(*ptr)) { ptr++; size--; }
	while(size && *ptr && !isspace(*ptr)) { s+=*ptr++; size--; }
	return s;
}

static char *get_line(char *&ptr, int &size)
{
	char *line = ptr;
	while(size && *ptr!='\n') { ptr++; size--; }
	if(size) { *ptr++=0; size--; }
	return line;
}

static int parse_instance_con(char *line, instance *inst, module *m)
{
	std::vector<char*> v;
	while(char *s=get_keyword(line)) v.push_back(s);
	int n_nonattr = 0;
	int i_pins = 0;
	for(int i=0; i<v.size(); i++) {
		char *s = v[i];
		if(char *e=strchr(s, '=')) {
			*e++ = 0;
			inst->attributes[s] = e;
		}
		else if(i>0 && strcmp(v[i], "$PINS")==0) {
			i_pins = i;
			break;
		}
		else if(n_nonattr==i) {
			n_nonattr = i+1;
		}
		else {
			fprintf(stderr, "error: parse_premitive_con\n");
			return E+__LINE__;
		}
	}

	// pins
	if(i_pins>0) for(int i=i_pins+1; i<v.size(); i++) {
		char *s = v[i];
		if(char *e=strchr(s, '=')) {
			*e++ = 0;
			net *n = m->get_net(e, 1);
			if(!n) n = m->add_net(e);
			con c(*n);
			c.port = s;
			db(printf("pins: s=%s e=%s\n", s, e));
			inst->ports.push_back(c);
		}
	}
	else for(int i=0; i<v.size() && i<n_nonattr-1; i++) {
		net *n = m->get_net(v[i], 1);
		if(!n) n = m->add_net(v[i]);
		con c(*n);
		c.order = i+1;
		inst->ports.push_back(c);
	}

	// reference
	if(n_nonattr-1<v.size()) {
		if(inst->type==instance::t_instance)
			inst->ref = v[n_nonattr-1];
		else if(inst->type==instance::t_premitive)
			inst->attributes[""] = v[n_nonattr-1];
	}
	db(printf("ports"));
	db(for(int i=0; i<inst->ports.size(); i++) printf(" [%d|%s] <%s>", inst->ports[i].order, inst->ports[i].port.c_str(), inst->ports[i].nets[0].name.c_str()));
	db(printf("\nattributes"));
	db(for(std::map<std::string,std::string>::iterator it=inst->attributes.begin(); it!=inst->attributes.end(); it++) printf(" [%s]=<%s>", it->first.c_str(), it->second.c_str()));
	db(printf("\n"));
	return 0;
}

static int parse_connect(char *line, instance *inst, module *m)
{
	db(printf("parse_connect line=<%s>\n", line));
	std::vector<char*> v;
	while(char *s=get_keyword(line)) v.push_back(s);

	for(int i=0; i<v.size(); i++) {
		net *n = m->get_net(v[i], 1);
		if(!n) n = m->add_net(v[i]);
		con c(*n);
		c.order = i+1;
		inst->ports.push_back(c);
	}

	return 0;
}

static int parse_block(char *&ptr, int &size, module *m, const read_spice_opt *opt)
{
	while(size>0) {
		std::string first = get_keyword(ptr, size);
		std::transform(first.begin(), first.end(), first.begin(), ::tolower);
		char *line = get_line(ptr, size);
		db(printf("module<%s> first=<%s> line=<%s>\n", m->name.c_str(), first.c_str(), line));
		if(first[0]=='.' || first[0]=='*') {
			if(first==".subckt") {
				const char *name = get_keyword(line);
				db(printf("subckt name=<%s>\n", name));
				if(!name) { fprintf(stderr, "error: no subckt name found\n"); return E+__LINE__; }
				module *mm = module::create(name);
				if(!mm) {
					// TODO: Check the second subckt is same to the first one.
					if(opt->ignore_second) {
						mm = new module;
					}
					else {
						fprintf(stderr, "error: reading subckt %s\n", name);
						return E+__LINE__;
					}
				}
				while(const char *p=get_keyword(line)) {
					mm->add_port(p);
				}
				parse_block(ptr, size, mm, opt);
			}
			else if(first==".ends") {
				return 0;
			}
			else if(opt->dot_connect && (first==".connect" || first=="*.connect")) {
				instance *inst = m->create_instance();
				inst->type = instance::t_connect;
				if(int ret=parse_connect(line, inst, m)) return ret;
			}
		}
		else if(first.size()>=2) {
			char type = first[0];
			const char *name = first.c_str()+1;
			instance *inst = m->create_instance(name);
			if(type=='x') {
				inst->type = instance::t_instance;
				if(int ret=parse_instance_con(line, inst, m)) return ret;
			}
			else if(isalpha(type)) {
				inst->type = instance::t_premitive;
				inst->ref = type;
				if(int ret=parse_instance_con(line, inst, m)) return ret;
			}
		}
	}
	return 0;
}

static int parse_all(char *&ptr, int &size, const read_spice_opt *opt)
{
	module * m_top = module::create("");

	while(size>0) {
		if(int ret=parse_block(ptr, size, m_top, opt)) return ret;
	}
}

int read_spice(const char *name, const read_spice_opt *opt)
{
	std::vector<char> data;
	if(int ret=read_file(name, data, opt)) { return ret; }
	int len = data.size();
	char *ptr = &data[0];
	if(int ret=parse_all(ptr, len, opt)) { return ret; }
	return 0;
}
