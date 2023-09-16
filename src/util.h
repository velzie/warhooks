#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>

uintptr_t findBaseAddress(int pid, const char *moduleName,
                          const char *pagetype);

void RemoveChars(char *s, char c);
#endif // !UTIL_H
