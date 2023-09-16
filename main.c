
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define WH_VERSION "1.0.0"

#include "offsets.c"
#include "qtypes.h"
#include "util.c"

void *old_Cvar_Get;
void *old_Con_Key_Enter;
void *old_CL_RequestNextDownload;

trace_t (**p_module_Trace)(trace_t *, vec_t *, vec_t *, vec_t *, vec_t *, int,
                           int, int);
void (*p_trap_R_TransformVectorToScreen)(refdef_t *rd, vec3_t *in, vec3_t *out);

void (*p_trap_SCR_DrawString)(int, int, int, char *, void *, vec4_t);

centity_t (*p_cg_entities)[1024];
cg_state_t *p_cg;
cg_clientInfo_t (*p_cg_clientInfo)[256];
vec3_t *cl_viewangles;

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

vec3_t vec3_origin = {0, 0, 0};
trace_t trace;
trace_t rt;

int attacking = 0;

int wh_triggerbot = 0;
int wh_nametags = 0;
int wh_aimbot = 0;

void PM_Move() {
  lc_unhook(lc_ptr_PM_Move, old_pm_move);

  __auto_type playerangs = p_cg->predictedPlayerState.viewangles;
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
      // don't target teammates or the dead
      if (cent.current.damage == 0 ||
          cent.current.team == p_cg->predictedPlayerState.stats[8])
        continue;

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

        unsigned long len = 0x10000 | 0x2000000;

        ((void (*)(trace_t *, vec3_t, vec3_t, vec3_t, vec3_t, int, int))(
            (uintptr_t)lc_ptr_CG_GS_Trace + lc_base_ptr - lc_text_offset))(
            &lookingAt, playerpos, vec3_origin, vec3_origin, end,
            p_cg->predictedPlayerState.POVnum, len);
        if (lookingAt.plane.normal[2] == 0)
          goto notattacking;
        if (!attacking) {
          ((void (*)(char *))((uintptr_t)base_ptr + ptr_CbufAddText -
                              text_offset))("+attack\n");
          attacking = 1;
        } else {
          goto notattacking;
        }
      } else {
      notattacking:
        if (attacking) {
          ((void (*)(char *))((uintptr_t)base_ptr + ptr_CbufAddText -
                              text_offset))("-attack\n");
          attacking = 0;
        }
      }
    }
  }

  if (wh_aimbot) {
    __auto_type playerpos = p_cg->predictedPlayerState.pmove.origin;

    int chosen = -1;
    float dx, dz, dy, dist;
    float lastdist = 999999;
    centity_t cent;
    for (int i = 0; i < 255; i++) {
      if (!(*p_cg_clientInfo)[i].name[0] ||
          (p_cg->predictedPlayerState.POVnum > 0 &&
           p_cg->predictedPlayerState.POVnum == (i + 1)))
        continue;

      cent = (*p_cg_entities)[i + 1];

      // don't target teammates or the dead
      if (cent.current.damage == 0 ||
          cent.current.team == p_cg->predictedPlayerState.stats[8])
        continue;

      trace_t canSee;
      ((void (*)(trace_t *, vec3_t, vec3_t, vec3_t, vec3_t, int, int))(
          (uintptr_t)lc_ptr_CG_GS_Trace + lc_base_ptr - lc_text_offset))(
          &canSee, playerpos, vec3_origin, vec3_origin, cent.current.origin, 1,
          1);

      if (canSee.plane.normal[2] != 0)
        continue;

      dx = playerpos[0] - cent.current.origin[0];
      dz = playerpos[1] - cent.current.origin[1];
      dy = playerpos[2] - (cent.current.origin[2] - 50);

      dist = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));

      if (dist > 1000)
        continue;
      if (dist > lastdist)
        continue;
      lastdist = dist;
      chosen = i;
    }

    if (chosen != -1 && dist > 200) {
      float ryaw = atan2(dz, dx);
      float rpitch = atan2(sqrt(dz * dz + dx * dx), dy);

      float yoffset = playerangs[0] - (*cl_viewangles)[0];
      float tpitch = rpitch * -57.2958 + 90 - 5 - yoffset;

      float xoffset = playerangs[1] - (*cl_viewangles)[1];
      float tyaw = ryaw * 57.2958 - 180 - xoffset;

      float dpitch = (*cl_viewangles)[0] - tpitch;
      float dyaw = (*cl_viewangles)[1] - tyaw;

      (*cl_viewangles)[0] -= dpitch / 5;
      (*cl_viewangles)[1] -= dyaw;
    }
  }

  ((void (*)())((uintptr_t)lc_ptr_PM_Move + lc_base_ptr - lc_text_offset))();
  lc_hook(lc_ptr_PM_Move, PM_Move);
}

void CG_DrawPlayerNames(void *font, vec4_t color) {
  cg_viewdef_t *view = (cg_viewdef_t *)((uintptr_t)p_cg + 593528);

  int y = 25;
  int x = 25;

  char *buf;
  asprintf(&buf, "^4War^6Hooks^1 %s", WH_VERSION);
  p_trap_SCR_DrawString(x, y, 0, buf, font, color);
  if (wh_triggerbot) {
    y += 25;
    p_trap_SCR_DrawString(x, y, 0, "triggerbot ON", font, color);
  }
  if (wh_aimbot) {
    y += 25;
    p_trap_SCR_DrawString(x, y, 0, "aimbot ON", font, color);
  }
  if (wh_nametags) {
    y += 25;
    p_trap_SCR_DrawString(x, y, 0, "nametags ON", font, color);
  }

  if (wh_nametags) {
    for (int i = 0; i < 255; i++) {
      if (!(*p_cg_clientInfo)[i].name[0] ||
          (p_cg->predictedPlayerState.POVnum > 0 &&
           p_cg->predictedPlayerState.POVnum == (i + 1)))
        continue;

      centity_t cent = (*p_cg_entities)[i + 1];

      __auto_type playerpos = p_cg->predictedPlayerState.pmove.origin;

      trace_t canSee;
      ((void (*)(trace_t *, vec3_t, vec3_t, vec3_t, vec3_t, int, int))(
          (uintptr_t)lc_ptr_CG_GS_Trace + lc_base_ptr - lc_text_offset))(
          &canSee, playerpos, vec3_origin, vec3_origin, cent.current.origin, 1,
          1);

      if (canSee.plane.normal[2] == 0)
        continue;

      vec3_t screenpos;
      vec3_t otherpos;

      otherpos[0] = cent.current.origin[0];
      otherpos[1] = cent.current.origin[1];
      otherpos[2] = cent.current.origin[2] + 50;

      p_trap_R_TransformVectorToScreen(&view->refdef, &otherpos, &screenpos);

      p_trap_SCR_DrawString(screenpos[0], screenpos[1], 0,
                            (*p_cg_clientInfo)[i].name, font, color);
    }
  }

  lc_unhook(lc_ptr_CG_DrawPlayerNames, old_CG_DrawPlayerNames);

  ((void (*)(void *, vec4_t))((uintptr_t)lc_ptr_CG_DrawPlayerNames +
                              lc_base_ptr - lc_text_offset))(font, color);
  lc_hook(lc_ptr_CG_DrawPlayerNames, CG_DrawPlayerNames);
}

void CL_RequestNextDownload() {
  unhook(ptr_CL_RequestNextDownload, old_CL_RequestNextDownload);

  // bypass pure check
  *(short *)((uintptr_t)bss_ptr + ptr_sv_pure) = 0;
  ((void (*)())((uintptr_t)base_ptr + ptr_CL_RequestNextDownload -
                text_offset))();
  hook(ptr_CL_RequestNextDownload, CL_RequestNextDownload);
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

  if (strcmp(token, "wh_info") == 0) {
    com_printf("\n\n\n---- WARHOOKS ----\nVERSION %s\n\n", WH_VERSION);
  } else if (strcmp(token, "wh_getpos") == 0) {

    __auto_type playerangs = p_cg->predictedPlayerState.viewangles;

    __auto_type playerpos = p_cg->predictedPlayerState.pmove.origin;
    com_printf("POS x: %f z: %f y: %f\n", playerpos[0], playerpos[1],
               playerpos[2]);

    com_printf("ANG x: %f z: %f y: %f\n", playerangs[0], playerangs[1],
               playerangs[2]);

  } else if (strcmp(token, "wh_triggerbot") == 0) {
    token = strtok(NULL, " ");
    if (strcmp(token, "1") == 0) {
      com_printf("triggerbot enabled\n");
      wh_triggerbot = 1;
    } else {
      com_printf("triggerbot disabled\n");
      wh_triggerbot = 0;
    }
  } else if (strcmp(token, "wh_aimbot") == 0) {
    token = strtok(NULL, " ");
    if (strcmp(token, "1") == 0) {
      com_printf("aimbot enabled\n");
      wh_aimbot = 1;
    } else {
      com_printf("aimbot disabled\n");
      wh_aimbot = 0;
    }
  } else if (strcmp(token, "wh_nametags") == 0) {
    token = strtok(NULL, " ");
    if (strcmp(token, "1") == 0) {
      com_printf("nametags enabled\n");
      wh_nametags = 1;
    } else {
      com_printf("nametags disabled\n");
      wh_nametags = 0;
    }
  } else {
  defaults:
    unhook(ptr_Con_Key_Enter, old_Con_Key_Enter);
    ((void (*)())((uintptr_t)ptr_Con_Key_Enter + base_ptr - text_offset))();
    hook(ptr_Con_Key_Enter, Con_Key_Enter);
    return;
  }
}

// scaffolding for modifying cvars
void *Cvar_Get(char *name, char *value, int flags) {
  unhook(ptr_Cvar_Get, old_Cvar_Get);

  typeof(&Cvar_Get) orig = (uintptr_t)base_ptr + ptr_Cvar_Get - text_offset;
  void *cvar = orig(name, value, flags);

  hook(ptr_Cvar_Get, Cvar_Get);
  return cvar;
}

int (*main_orig)(int, char **, char **);

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
    // noppify(lc_base_ptr + 0x6fbd8 - lc_text_offset, 5);
    // noppify(lc_base_ptr + 0x6fba3 - lc_text_offset, 6);
    // noppify(lc_base_ptr + 0x6fbc8 - lc_text_offset, 6);
    // noppify(lc_base_ptr + 0x6fca9 - lc_text_offset, 6);
    //
    p_cg_entities = lc_bss_ptr + 0x4000a0;
    p_cg = lc_bss_ptr + 0x36a0a0;
    p_cg_clientInfo = lc_bss_ptr + 0x359790;

    p_trap_R_TransformVectorToScreen = lc_base_ptr + 0x28cff - lc_text_offset;
    p_trap_SCR_DrawString = lc_base_ptr + 0x28d73 - lc_text_offset;

    // lc_hook(lc_ptr_CG_FireWeaponEvent, CG_FireWeaponEvent);
    old_pm_move = lc_hook(lc_ptr_PM_Move, PM_Move);

    old_CG_DrawPlayerNames =
        lc_hook(lc_ptr_CG_DrawPlayerNames, CG_DrawPlayerNames);
  } else {
    obj_ptr = dlopen(sofile);
  }

  return obj_ptr;
}
