#ifndef UTIL_FUNCTIONS_H
#define UTIL_FUNCTIONS_H

#define is_digit(c) ((c) >= '0' && (c) <= '9')

bool skipInt(const char **s, int *dest);
bool skipFloat(const char **s, float *dest);
bool skipString(const char** s, char* dest, int destLen);

int snprintf(char* buf, int length, const char *fmt, ...);

#endif
