
#define _GNU_SOURCE
#include <dlfcn.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#define MAX_PATH 1024

jmp_buf b;

// Function to find the base address of a module in a process
uintptr_t findBaseAddress(int pid, const char *moduleName) {
  uintptr_t baseAddress = 0;
  char mapsFilePath[MAX_PATH];

  // Create the path to the maps file for the process
  snprintf(mapsFilePath, sizeof(mapsFilePath), "/proc/%d/maps", pid);

  // Open the maps file
  FILE *mapsFile = fopen(mapsFilePath, "r");
  if (!mapsFile) {
    perror("Error opening maps file");
    return 0;
  }

  // Iterate through the lines in the maps file
  char line[MAX_PATH];
  while (fgets(line, sizeof(line), mapsFile)) {
    printf(":: %s\n", line);

    // find the executable page
    if (strstr(line, "r-xp")) {
      // Check if the line contains the module name
      if (strstr(line, moduleName)) {
        // Parse the start address
        sscanf(line, "%lx", &baseAddress);
        break;
      }
    }
  }

  // Close the maps file
  fclose(mapsFile);

  return baseAddress;
}

// void *text_offset = (void *)0xe000;

unsigned long text_offset = 0x7000;
unsigned long lc_text_offset = 0xa000;

void *ptr_Con_Key_Enter = (void *)0x85748;
void *ptr_Sys_Library_GetGameLibPath = (void *)0x8d653;
void *ptr_FS_ExtractFile = (void *)0x34ba1;
void *ptr_FS_RemoveFile = (void *)0x34ba1;

void *lc_ptr_PM_Move = (void *)0xe67b;
void *lc_ptr_PM_Friction = (void *)0xd8f3;

void *old_pm_move;

void *next_write_addr;
void *base_ptr;
void *lc_base_ptr;

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
}

void PM_Move() {
  lc_unhook(lc_ptr_PM_Move, old_pm_move);
  ((void (*)())((uintptr_t)lc_ptr_PM_Move + lc_base_ptr - lc_text_offset))();
  lc_hook(lc_ptr_PM_Move, PM_Move);
}
int Con_Key_Enter(char *s) {
  // printf("restoring original\n");

  // memcpy(next_write_addr, func_header, sizeof(jmpasm));
  //
  printf("calling original\n");

  // int *svcheats = (int *)base_ptr + 0x14DF970;

  // printf("===%s", s);

  ((void (*)(char *))(base_ptr + 0x2a95e - text_offset))("among us\n");

  lc_base_ptr = (void *)findBaseAddress(getpid(), "libcgame");
  if (mprotect(lc_base_ptr, 479232, PROT_EXEC | PROT_WRITE | PROT_READ) == -1) {
    perror("mprotect");
    exit(1);
  }

  // void *pmmove = (void *)ptr + 0xe67b - lc_text_offset;
  // _create_hook(pmmove, PM_Move);
  old_pm_move = lc_hook(lc_ptr_PM_Move, PM_Move);

  noppify(lc_base_ptr + 0x6fbd8 - lc_text_offset, 5);
  noppify(lc_base_ptr + 0x6fba3 - lc_text_offset, 6);
  noppify(lc_base_ptr + 0x6fbc8 - lc_text_offset, 6);
  noppify(lc_base_ptr + 0x6fca9 - lc_text_offset, 6);
  // int ret = ((int (*)())next_write_addr)();
  // printf("original ret %i\n", ret);
  return 5;
}

// troll the engine by forcing our own path
const char *Sys_Library_GetGameLibPath(const char *name, int64_t time,
                                       int randomizer) {

  printf("redirecting %s\n", name);
  return "/home/ce/pacman/warfork-qfusion/source/build/basewf/"
         "libcgame_x86_64.so";
}

int FS_ExtractFile(const char *src, const char *dst) {
  printf("file extract moment %s/%s\n", src, dst);
  return 1;
}
int FS_RemoveFile(const char *file) {
  printf("removing file %s\n", file);
  return 1;
}

/* Trampoline for the real main() */

// todo: hook ui library and whatnot
static int (*main_orig)(int, char **, char **);

/* Our fake main() that gets called by __libc_start_main() */
int main_hook(int argc, char **argv, char **envp) {
  for (int i = 0; i < argc; ++i) {
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  int pid = getpid();
  uintptr_t ptr = findBaseAddress(pid, "warfork");
  base_ptr = (void *)ptr;

  printf("PTR: %lu\n", ptr);
  if (mprotect((void *)ptr, 565248, PROT_EXEC | PROT_WRITE | PROT_READ) == -1) {
    perror("mprotect");
    exit(1);
  }

  void *p8_spr = (void *)ptr + 0x85748 - text_offset;

  void *h_getgamelibpath = (void *)ptr + 0x8d653 - text_offset;

  void *h_extract = (void *)ptr + 0x34ba1 - text_offset;

  // *((char *)p8_spr) = 'c';

  printf("--- hooking ---\n");
  hook(ptr_Con_Key_Enter, Con_Key_Enter);
  hook(ptr_Sys_Library_GetGameLibPath, Sys_Library_GetGameLibPath);
  hook(ptr_FS_ExtractFile, FS_ExtractFile);
  hook(ptr_FS_RemoveFile, FS_RemoveFile);

  printf("--- Before main ---\n");
  int ret = main_orig(argc, argv, envp);
  printf("--- After main ----\n");
  printf("main() returned %d\n", ret);
  return ret;
}

/*
 * Wrapper for __libc_start_main() that replaces the real main
 * function with our hooked version.
 */
int __libc_start_main(int (*main)(int, char **, char **), int argc, char **argv,
                      int (*init)(int, char **, char **), void (*fini)(void),
                      void (*rtld_fini)(void), void *stack_end) {
  /* Save the real main function address */
  main_orig = main;

  /* Find the real __libc_start_main()... */
  typeof(&__libc_start_main) orig = dlsym(RTLD_NEXT, "__libc_start_main");

  /* ... and call it with our custom main function */
  return orig(main_hook, argc, argv, init, fini, rtld_fini, stack_end);
}

int sv_cheats = 1;

void SDL_LoadFunction(void *handle, const char *s) {

  typeof(&SDL_LoadFunction) orig = dlsym(RTLD_NEXT, "SDL_LoadFunction");

  return orig(handle, s);
}
