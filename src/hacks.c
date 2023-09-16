#include <math.h>
#include <stddef.h>
#include <string.h>

#include "hook.h"
#include "offsets.h"
#include "pointers.h"
#include "qtypes.h"
#include "warhooks.h"

#include "util.h"

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

  void *key_lines = (bss_ptr + 0x280720);
  int *edit_line = (bss_ptr + 0x282728);
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
