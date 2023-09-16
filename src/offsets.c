// --- binary offsets ---
// commit bbf81122b2337eb1509144e5aee5e4ba8bd57fd3 with debugging info added and
// steamlib disabled
//
// pulled with a combination of IDA and gdb

unsigned long text_offset = 0x7000;
unsigned long lc_text_offset = 0xa000;

void *ptr_Con_Key_Enter = (void *)0x85748;
void *ptr_Con_SendChatMessage = (void *)0x85622;
void *ptr_CbufAddText = (void *)0x27163;

void *ptr_Cvar_Get = (void *)0x2c879;
void *ptr_CL_RequestNextDownload = (void *)0x715ec;

void *ptr_sv_pure = (void *)0x36a174;

void *bss_ptr_key_lines = (void *)0x280720;
void *bss_ptr_edit_line = (void *)0x282728;

// --- libcgame offsets ---

void *lc_ptr_PM_Move = (void *)0xe67b;
void *lc_ptr_CG_FireWeaponEvent = (void *)0x411ed;

void *lc_ptr_cg_entities = (void *)0x149c5;
void *lc_ptr_CG_GS_Trace = (void *)0x68a65;

void *lc_ptr_CG_LFuncDrawBar = (void *)0x4cad3;

void *lc_ptr_CG_DrawPlayerNames = (void *)0x6f0e3;
