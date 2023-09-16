#include "offsets.h"
#include "pointers.h"
#include <stdint.h>

#include <stdlib.h>
#include <string.h>
// mov r10, addr
// jmp r10
uint8_t jmpasm[] = {0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x41, 0xFF, 0xE2};

void *_create_hook(void *tgt_addr, void *src_addr) {

  // write assembly for jmp long
  memcpy(&jmpasm[2], &src_addr, sizeof(src_addr));
  next_write_addr = tgt_addr;

  // copy original fn header
  void *func_header = malloc(sizeof(jmpasm));
  memcpy(func_header, tgt_addr, sizeof(jmpasm));

  // write assembly
  memcpy(tgt_addr, jmpasm, sizeof(jmpasm));

  return func_header;
}
void _unhook(void *tgt_addr, void *header) {
  memcpy(tgt_addr, header, sizeof(jmpasm));
}

void *hook(void *tgt_addr, void *src_addr) {
  return _create_hook((void *)((uintptr_t)tgt_addr + base_ptr - text_offset),
                      src_addr);
}
void unhook(void *tgt_addr, void *header) {
  _unhook((uintptr_t)tgt_addr + base_ptr - text_offset, header);
}

void *lc_hook(void *tgt_addr, void *src_addr) {
  return _create_hook(
      (void *)((uintptr_t)tgt_addr + lc_base_ptr - lc_text_offset), src_addr);
}
void lc_unhook(void *tgt_addr, void *header) {
  _unhook((uintptr_t)tgt_addr + lc_base_ptr - lc_text_offset, header);
}

void *noppify(void *tgt_addr, int len) {
  void *store = malloc(len);
  memcpy(store, tgt_addr, len);

  char *arr = tgt_addr;
  for (int i = 0; i < len; i++) {
    arr[i] = '\x90';
  }

  return store;
}
