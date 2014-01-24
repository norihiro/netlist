#ifndef WRITE_SPICE_H
#define WRITE_SPICE_H

#ifdef __cplusplus
extern "C" {
#endif

struct write_spice_opt
{
	const char *top;
	unsigned pins :1;
};

int write_spice(const char *name, const write_spice_opt *opt);

#ifdef __cplusplus
}
#endif

#endif // WRITE_SPICE_H
