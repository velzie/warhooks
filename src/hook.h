#ifndef HOOK_H
#define HOOK_H

void *_create_hook(void *tgt_addr, void *src_addr);
void _unhook(void *tgt_addr, void *header);

void *hook(void *tgt_addr, void *src_addr);
void unhook(void *tgt_addr, void *header);

void *lc_hook(void *tgt_addr, void *src_addr);
void lc_unhook(void *tgt_addr, void *header);

void *noppify(void *tgt_addr, int len);
#endif
