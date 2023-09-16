#include "qtypes.h"

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
void *next_write_addr;
void *base_ptr;
void *lc_base_ptr;
void *bss_ptr;
void *lc_bss_ptr;

void (*com_printf)(char *, ...);

int (*main_orig)(int, char **, char **);
