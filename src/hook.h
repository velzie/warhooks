#ifndef HOOK_H
#define HOOK_H
#include <stdint.h>

void *hook(uintptr_t tgt_addr, void *src_addr);
void unhook(uintptr_t tgt_addr, void *header);

void *lc_hook(uintptr_t tgt_addr, void *src_addr);
void lc_unhook(uintptr_t tgt_addr, void *header);

void *noppify(uintptr_t tgt_addr, int len);
#endif
