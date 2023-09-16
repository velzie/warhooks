#include "qtypes.h"
#ifndef POINTERS_H
#define POINTERS_H

extern void *old_Cvar_Get;
extern void *old_Con_Key_Enter;
extern void *old_CL_RequestNextDownload;

extern trace_t (**p_module_Trace)(trace_t *, vec_t *, vec_t *, vec_t *, vec_t *,
                                  int, int, int);
extern void (*p_trap_R_TransformVectorToScreen)(refdef_t *rd, vec3_t *in,
                                                vec3_t *out);

extern void (*p_trap_SCR_DrawString)(int, int, int, char *, void *, vec4_t);

extern centity_t (*p_cg_entities)[1024];
extern cg_state_t *p_cg;
extern cg_clientInfo_t (*p_cg_clientInfo)[256];
extern vec3_t *cl_viewangles;

extern void *old_pm_move;
extern void *old_CG_LFuncDrawBar;

extern void *old_CG_DrawPlayerNames;

extern void *next_write_addr;
extern void *base_ptr;
extern void *lc_base_ptr;
extern void *bss_ptr;
extern void *lc_bss_ptr;

extern void (*com_printf)(char *, ...);

extern int (*main_orig)(int, char **, char **);
#endif
