
#include <config.h>
#include <cstdio>
#include <cstring>
#include "read_spice.h"
#include "write_spice.h"

main(int argc, char **argv)
{
	read_spice_opt read_sp_opt = {0};
	write_spice_opt write_sp_opt = {0};

	read_sp_opt.dot_connect = 1;

	char flag[256] = {0};

	for(int i=1; i<argc; i++) {
		char *ai = argv[i];
		if(flag['o']) {
			if(int ret=write_spice(ai, &write_sp_opt)) {
				fprintf(stderr, "error: writing %s\n", ai);
				return __LINE__;
			}
			flag['o'] = 0;
		}
		else if(flag['t']) {
			write_sp_opt.top = ai;
			flag['t'] = 0;
		}
		else if(*ai=='-') {
			if(ai[1]=='-') {
				if(!strcmp(ai, "--pins"))
					read_sp_opt.pins = 1;
				else {
					fprintf(stderr, "error: unknown option %s\n", ai);
					return __LINE__;
				}
			}
			else while(char c=*++ai) switch(c) {
				case 'o':
				case 't':
					flag[c]++;
					break;
				case 'd':
					read_sp_opt.ignore_second = 1;
					break;
				default:
					fprintf(stderr, "error: unknown option -%c\n", c);
					return __LINE__;
			}
		}
		else {
			if(int ret=read_spice(ai, &read_sp_opt)) {
				fprintf(stderr, "error: reading %s\n", ai);
				return ret;
			}
		}
	}
}
