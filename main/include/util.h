#ifndef UTIL_FUNCTIONS_H
#define UTIL_FUNCTIONS_H

#define is_digit(c) ((c) >= '0' && (c) <= '9')

bool skipInt(const char **s, int *dest);
bool skipFloat(const char **s, float *dest);
bool skipString(const char** s, char* dest, int destLen);
bool skipUint(const char **s, uint32_t *dest);

uint32_t readFileFull(const char* path, char** buf, bool bForce = false);
uint32_t writeFileFull(const char* path, char* buf, uint32_t len, bool bForce = false);

inline unsigned get_ccount(void)
{
	unsigned r;
	asm volatile ("rsr %0, ccount" : "=r"(r));
	return r;
}

#endif
