#include "config.h"
#include "instance.h"
#include "module.h"
#include <algorithm>
#include <cstdio>
#define E 2000
#define db(x)

int instance::sort_order_by_name()
{
	module *m = module::get(ref.c_str());
	if(!m) {
		fprintf(stderr, "error: unknown module name %s\n", ref.c_str());
		return E+__LINE__;
	}
	for(int i=0; i<ports.size(); i++) {
		con &p = ports[i];
		if(p.port.size() && !p.order) {
			p.order = m->port2order(p.port.c_str());
			db(printf("port=%s order=%d nn=%s\n", p.port.c_str(), p.order, p.nets[0].name.c_str()));
		}
	}

	std::sort(ports.begin(), ports.end());

	return 0;
}
