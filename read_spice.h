#ifndef READ_SPICE_H
#define READ_SPICE_H

#ifdef __cplusplus
extern "C" {
#endif

struct read_spice_opt
{
	unsigned ignore_second :1;
	unsigned pins :1;
	unsigned dot_connect :1;
};

int read_spice(const char *name, const read_spice_opt *opt);

#ifdef __cplusplus
}
#endif

#endif // READ_SPICE_H
