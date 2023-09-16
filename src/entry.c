
#define _GNU_SOURCE
#include <dlfcn.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "hacks.h"
#include "hook.h"
#include "offsets.h"
#include "pointers.h"
#include "qtypes.h"
#include "util.h"
#include "warhooks.h"

int main_hook(int argc, char **argv, char **envp) {
  int pid = getpid();

  // get ptr to the executable section
  base_ptr = (void *)findBaseAddress(pid, "warfork", "r-xp");
  // get pointer to the heap
  bss_ptr = (void *)findBaseAddress(pid, "00:00", "rw-p");

  if (mprotect(base_ptr, 565248, PROT_EXEC | PROT_WRITE | PROT_READ) == -1) {
    perror("mprotect");
    exit(1);
  }

  printf("--- hooking ---\n");
  old_Con_Key_Enter = hook(ptr_Con_Key_Enter, Con_Key_Enter);
  old_CL_RequestNextDownload =
      hook(ptr_CL_RequestNextDownload, CL_RequestNextDownload);

  com_printf = (base_ptr + 0x2a95e - text_offset);
  cl_viewangles = bss_ptr + 0x36a3e0; // cl+480

  old_Cvar_Get = hook(ptr_Cvar_Get, Cvar_Get);

  printf("--- calling main ---\n");
  int ret = main_orig(argc, argv, envp);
  printf("--- main exited ----\n");
  printf("main() returned %d\n", ret);
  return ret;
}

// entry point for all dynamic exes
// redirect main() to main_hook()
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

void *SDL_LoadFunction(void *handle, const char *s) {

  typeof(&SDL_LoadFunction) orig = dlsym(RTLD_NEXT, "SDL_LoadFunction");

  return orig(handle, s);
}

// hijack SDL_LoadObject, since it will try to load libcgame.so from the
// pure.pk3 the pure check isn't the problem, it's just that the dll it loads
// won't have symbols and we want symbols so it will redirect to the built
// libcgame instead
void *SDL_LoadObject(const char *sofile) {
  typeof(&SDL_LoadObject) dlopen = dlsym(RTLD_NEXT, "SDL_LoadObject");

  void *obj_ptr;
  if (strstr(sofile, "libcgame") != 0) {
    obj_ptr = dlopen("./basewf/libcgame_x86_64.so");
    printf("--- libcgame loaded! hooking! ---\n");
    com_printf("WARHOOKS: HOOKING LIBCGAME\n\n\n");

    lc_base_ptr = (void *)findBaseAddress(getpid(), "libcgame", "r-xp");
    if (mprotect(lc_base_ptr, 479232, PROT_EXEC | PROT_WRITE | PROT_READ) ==
        -1) {
      perror("mprotect");
      exit(1);
    }

    lc_bss_ptr = (void *)findBaseAddress(getpid(), "libcgame", "rw-p");

    p_cg_entities = lc_bss_ptr + 0x4000a0;
    p_cg = lc_bss_ptr + 0x36a0a0;
    p_cg_clientInfo = lc_bss_ptr + 0x359790;

    p_trap_R_TransformVectorToScreen = lc_base_ptr + 0x28cff - lc_text_offset;
    p_trap_SCR_DrawString = lc_base_ptr + 0x28d73 - lc_text_offset;

    old_pm_move = lc_hook(lc_ptr_PM_Move, PM_Move);

    old_CG_DrawPlayerNames =
        lc_hook(lc_ptr_CG_DrawPlayerNames, CG_DrawPlayerNames);
  } else {
    obj_ptr = dlopen(sofile);
  }

  return obj_ptr;
}
