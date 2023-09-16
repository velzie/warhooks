#ifndef OFFSETS_H
#define OFFSETS_H
extern unsigned long text_offset;
extern unsigned long lc_text_offset;

extern void *ptr_Con_Key_Enter;
extern void *ptr_Con_SendChatMessage;
extern void *ptr_CbufAddText;

extern void *ptr_Cvar_Get;
extern void *ptr_CL_RequestNextDownload;

extern void *ptr_sv_pure;

extern void *bss_ptr_key_lines;
extern void *bss_ptr_edit_line;

extern void *lc_ptr_PM_Move;
extern void *lc_ptr_CG_FireWeaponEvent;

extern void *lc_ptr_cg_entities;
extern void *lc_ptr_CG_GS_Trace;

extern void *lc_ptr_CG_LFuncDrawBar;

extern void *lc_ptr_CG_DrawPlayerNames;
#endif
