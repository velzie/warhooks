
#define _GNU_SOURCE
#include <dlfcn.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#define MAX_PATH 1024

#include "qtypes.h"

jmp_buf b;

// Function to find the base address of a module in a process
uintptr_t findBaseAddress(int pid, const char *moduleName,
                          const char *pagetype) {
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
    // find the executable page
    if (strstr(line, pagetype)) {
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
void *ptr_FS_RemoveAbsoluteFile = (void *)0x3488f;
void *ptr_Con_SendChatMessage = (void *)0x85622;
void *ptr_CbufAddText = (void *)0x27163;

void *ptr_Cvar_Get = (void *)0x2c879;

void *old_Cvar_Get;
void *old_Con_Key_Enter;

void *lc_ptr_PM_Move = (void *)0xe67b;
void *lc_ptr_PM_Friction = (void *)0xd8f3;
void *lc_ptr_CG_FireWeaponEvent = (void *)0x411ed;
void *lc_ptr_GS_TraceBullet = (void *)0x149c5;
void *lc_ptr_PM_ApplyMouseAnglesClamp = (void *)0x123a9;

void *lc_ptr_module_Trace = (void *)0x5feb58;
void *lc_ptr_cg_entities = (void *)0x149c5;
void *lc_ptr_CG_GS_Trace = (void *)0x68a65;

void *lc_ptr_CG_LFuncDrawBar = (void *)0x4cad3;

void *lc_ptr_CG_DrawPlayerNames = (void *)0x6f0e3;

trace_t (**p_module_Trace)(trace_t *, vec_t *, vec_t *, vec_t *, vec_t *, int,
                           int, int);
void (*p_trap_R_TransformVectorToScreen)(refdef_t *rd, vec3_t *in, vec3_t *out);

void (*p_trap_SCR_DrawString)(int, int, int, char *, void *, vec4_t);

centity_t (*p_cg_entities)[1024];
cg_state_t *p_cg;
cg_clientInfo_t (*p_cg_clientInfo)[256];

void *old_pm_move;
void *old_CG_LFuncDrawBar;

void *old_CG_DrawPlayerNames;

void *bss_ptr_key_lines = (void *)0x280720;
void *bss_ptr_edit_line = (void *)0x282728;

void *next_write_addr;
void *base_ptr;
void *lc_base_ptr;
void *bss_ptr;
void *lc_bss_ptr;

void (*com_printf)(char *, ...);

typedef struct cvar_s {
  char *name;
  char *string;
  char *dvalue;
  char *latched_string; // for CVAR_LATCH vars
  int flags;
  int modified; // set each time the cvar is changed
  float value;
  int integer;
} cvar_t;

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
void RemoveChars(char *s, char c) {
  int writer = 0, reader = 0;

  while (s[reader]) {
    if (s[reader] != c) {
      s[writer++] = s[reader];
    }

    reader++;
  }

  s[writer] = 0;
}

vec3_t vec3_origin = {0, 0, 0};
trace_t trace;
trace_t rt;

int attacking = 0;

int wh_triggerbot = 0;

void PM_Move() {
  lc_unhook(lc_ptr_PM_Move, old_pm_move);

  // todo: use  CG_DrawHUDRect for better ESP, print nametags too
  if (wh_triggerbot) {
    __auto_type playerpos = p_cg->predictedPlayerState.pmove.origin;
    __auto_type playerangs = p_cg->predictedPlayerState.viewangles;
    vec3_t angvec;
    vec3_t end;

    for (int i = 0; i < 255; i++) {
      // todo print name with cgsClientinfo and do logic idk
      if (!(*p_cg_clientInfo)[i].name[0] ||
          (p_cg->predictedPlayerState.POVnum > 0 &&
           p_cg->predictedPlayerState.POVnum == (i + 1)))
        continue;

      centity_t cent = (*p_cg_entities)[i + 1];

      trace_t canSee;
      ((void (*)(trace_t *, vec3_t, vec3_t, vec3_t, vec3_t, int, int))(
          (uintptr_t)lc_ptr_CG_GS_Trace + lc_base_ptr - lc_text_offset))(
          &canSee, playerpos, vec3_origin, vec3_origin, cent.current.origin, 1,
          1);

      if (canSee.plane.normal[2] == 0) {
        float dist = 40000;

        float pitch = playerangs[0] / 57.2958;
        float yaw = playerangs[1] / 57.2958;
        float xzlen = cos(pitch);
        angvec[0] = xzlen * cos(yaw);
        angvec[1] = -xzlen * sin(-yaw);
        angvec[2] = -sin(pitch);

        end[0] = playerpos[0] + angvec[0] * dist;
        end[1] = playerpos[1] + angvec[1] * dist;
        end[2] = playerpos[2] + angvec[2] * dist;

        trace_t lookingAt;

        ((void (*)(trace_t *, vec3_t, vec3_t, vec3_t, vec3_t, int, int))(
            (uintptr_t)lc_ptr_CG_GS_Trace + lc_base_ptr - lc_text_offset))(
            &lookingAt, playerpos, vec3_origin, vec3_origin, end,
            p_cg->predictedPlayerState.POVnum, 0x10000 | 0x2000000);

        if (lookingAt.plane.normal[2] != 0) {
          if (!attacking) {
            ((void (*)(char *))((uintptr_t)base_ptr + ptr_CbufAddText -
                                text_offset))("+attack\n");
            attacking = 1;
          }
        } else {
        notattacking:
          if (attacking) {
            ((void (*)(char *))((uintptr_t)base_ptr + ptr_CbufAddText -
                                text_offset))("-attack\n");
            attacking = 0;
          }
        }
      } else {
        goto notattacking;
      }
    }
  }

  ((void (*)())((uintptr_t)lc_ptr_PM_Move + lc_base_ptr - lc_text_offset))();
  lc_hook(lc_ptr_PM_Move, PM_Move);
}

void CG_DrawPlayerNames(void *font, vec4_t color) {
  cg_viewdef_t *view = (cg_viewdef_t *)((uintptr_t)p_cg + 593528);

  for (int i = 0; i < 255; i++) {
    if (!(*p_cg_clientInfo)[i].name[0] ||
        (p_cg->predictedPlayerState.POVnum > 0 &&
         p_cg->predictedPlayerState.POVnum == (i + 1)))
      continue;

    centity_t cent = (*p_cg_entities)[i + 1];

    vec3_t screenpos;

    p_trap_R_TransformVectorToScreen(&view->refdef, &cent.current.origin,
                                     &screenpos);

    p_trap_SCR_DrawString(screenpos[0], screenpos[1], 0,
                          (*p_cg_clientInfo)[i].name, font, color);
  }

  lc_unhook(lc_ptr_CG_DrawPlayerNames, old_CG_DrawPlayerNames);

  ((void (*)(void *, vec4_t))((uintptr_t)lc_ptr_CG_DrawPlayerNames +
                              lc_base_ptr - lc_text_offset))(font, color);
  lc_hook(lc_ptr_CG_DrawPlayerNames, CG_DrawPlayerNames);
}

void PM_ApplyMouseAnglesClamp() {}
void *CG_FireWeaponEvent(int entNum, int weapon, int fireMode) {
  printf("-- %i %i %i--\n", entNum, weapon, fireMode);
}
void Con_Key_Enter(char *s) {

  void *key_lines = ((uintptr_t)bss_ptr + bss_ptr_key_lines);
  int *edit_line = ((uintptr_t)bss_ptr + bss_ptr_edit_line);
  char *orig_buf = (*(char(*)[32][256])key_lines)[*edit_line];

  if (strlen(orig_buf) <= 1)
    goto defaults;

  char *buf = malloc(256);

  buf = strncpy(buf, orig_buf, 256);

  RemoveChars(buf, ']');
  RemoveChars(buf, '/');
  char *token = strtok(buf, " ");

  if (strcmp(token, "p") == 0) {
    // com_printf("\n\n\n---- WARHOOKS ----\n\n\n");

    __auto_type playerangs = p_cg->predictedPlayerState.viewangles;

    __auto_type playerpos = p_cg->predictedPlayerState.pmove.origin;
    // __auto_type ang = (*p_cg_entities)[1].current.angles;
    com_printf("x: %f z: %f y: %f\n", playerpos[0], playerpos[1], playerpos[2]);

    // ((void (*)(char *))((uintptr_t)base_ptr + ptr_Con_SendChatMessage -
    //                     text_offset))("/+attack 1");
    //
    //
    //
    //
    // todo:  use module_trace and see if that detects wall between 2 players

    // rt = ((trace_t(*)(trace_t *, vec3_t, vec3_t, float, float, int, int,
    // int))(
    //     (uintptr_t)lc_base_ptr + lc_ptr_GS_TraceBullet - lc_text_offset))(
    //     &trace, playerpos, ang, 0, 0, 20, 0, 0);

    // todo: use this to draw nametags (shows when visible player)

    com_printf("x: %f z: %f y: %f\n", playerangs[0], playerangs[1],
               playerangs[2]);

  } else if (strcmp(token, "triggerbot") == 0) {
    com_printf("triggerbot enabled\n");
    wh_triggerbot = 1;
  } else {
  defaults:
    unhook(ptr_Con_Key_Enter, old_Con_Key_Enter);
    ((void (*)())((uintptr_t)ptr_Con_Key_Enter + base_ptr - text_offset))();
    hook(ptr_Con_Key_Enter, Con_Key_Enter);
    return;
  }

  // while (token != NULL) {
  //   token = strtok(NULL, " ");
  // }
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
int FS_RemoveAbsoluteFile(const char *file) {
  printf("removing abs file %s\n", file);
  return 1;
}

void *Cvar_Get(char *name, char *value, int flags) {
  unhook(ptr_Cvar_Get, old_Cvar_Get);
  // printf("cvar %s\n", name);

  typeof(&Cvar_Get) orig = (uintptr_t)base_ptr + ptr_Cvar_Get - text_offset;
  void *cvar = orig(name, value, flags);

  // cvar_t *addr = (bcc_ptr + 0x335720);
  // printf("value %f\n", addr->value);
  hook(ptr_Cvar_Get, Cvar_Get);
  return cvar;
}

/* Trampoline for the real main() */

// todo: hook ui library and whatnot, maybe a bit of imgui
// todo: bypass pure checks
static int (*main_orig)(int, char **, char **);

/* Our fake main() that gets called by __libc_start_main() */
int main_hook(int argc, char **argv, char **envp) {
  for (int i = 0; i < argc; ++i) {
    printf("argv[%d] = %s\n", i, argv[i]);
  }
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
  hook(ptr_Sys_Library_GetGameLibPath, Sys_Library_GetGameLibPath);
  hook(ptr_FS_ExtractFile, FS_ExtractFile);
  hook(ptr_FS_RemoveFile, FS_RemoveFile);
  hook(ptr_FS_RemoveAbsoluteFile, FS_RemoveAbsoluteFile);

  com_printf = (base_ptr + 0x2a95e - text_offset);

  old_Cvar_Get = hook(ptr_Cvar_Get, Cvar_Get);
  // char *purecheck = base_ptr + 0x716f4 - text_offset;
  // purecheck[0] = '\x84';

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

void *SDL_LoadFunction(void *handle, const char *s) {

  typeof(&SDL_LoadFunction) orig = dlsym(RTLD_NEXT, "SDL_LoadFunction");

  return orig(handle, s);
}

void *SDL_LoadObject(const char *s) {
  typeof(&SDL_LoadObject) orig = dlsym(RTLD_NEXT, "SDL_LoadObject");
  void *obj_ptr = orig(s);

  if (strstr(s, "libcgame") != 0) {
    printf("libcgame loaded! hooking!\n");
    com_printf("WARHOOKS: HOOKING LIBCGAME\n\n\n");

    lc_base_ptr = (void *)findBaseAddress(getpid(), "libcgame", "r-xp");
    if (mprotect(lc_base_ptr, 479232, PROT_EXEC | PROT_WRITE | PROT_READ) ==
        -1) {
      perror("mprotect");
      exit(1);
    }

    lc_bss_ptr = (void *)findBaseAddress(getpid(), "libcgame", "rw-p");
    noppify(lc_base_ptr + 0x6fbd8 - lc_text_offset, 5);
    noppify(lc_base_ptr + 0x6fba3 - lc_text_offset, 6);
    noppify(lc_base_ptr + 0x6fbc8 - lc_text_offset, 6);
    noppify(lc_base_ptr + 0x6fca9 - lc_text_offset, 6);

    // nametag hack

    p_cg_entities = lc_bss_ptr + 0x4000a0;
    p_cg = lc_bss_ptr + 0x36a0a0;
    p_cg_clientInfo = lc_bss_ptr + 0x359790;
    p_module_Trace = (uintptr_t)lc_bss_ptr + lc_ptr_module_Trace;

    p_trap_R_TransformVectorToScreen = lc_base_ptr + 0x28cff - lc_text_offset;

    p_trap_SCR_DrawString = lc_base_ptr + 0x28d73 - lc_text_offset;

    // noppify(lc_base_ptr + 0x30d02 - lc_text_offset, 5);

    // lc_hook(lc_ptr_CG_FireWeaponEvent, CG_FireWeaponEvent);
    old_pm_move = lc_hook(lc_ptr_PM_Move, PM_Move);

    old_CG_DrawPlayerNames =
        lc_hook(lc_ptr_CG_DrawPlayerNames, CG_DrawPlayerNames);

    // old_CG_LFuncDrawBar = lc_hook(lc_ptr_CG_LFuncDrawBar, CG_LFuncDrawBar);
  }

  return obj_ptr;
}
