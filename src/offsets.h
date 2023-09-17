#ifndef OFFSETS_H
#define OFFSETS_H
#include <stdint.h>
// --- binary offsets ---
// commit bbf81122b2337eb1509144e5aee5e4ba8bd57fd3 with debugging info added and
// steamlib disabled
//
// pulled with a combination of IDA and gdb

static uintptr_t text_offset = 0x7000;
static uintptr_t lc_text_offset = 0xa000;

static uintptr_t ptr_Con_Key_Enter = 0x85748;
static uintptr_t ptr_Con_SendChatMessage = 0x85622;
static uintptr_t ptr_CbufAddText = 0x27163;

static uintptr_t ptr_Cvar_Get = 0x2c879;
static uintptr_t ptr_CL_RequestNextDownload = 0x715ec;

static uintptr_t ptr_sv_pure = 0x36a174;

static uintptr_t bss_ptr_key_lines = 0x280720;
static uintptr_t bss_ptr_edit_line = 0x282728;

// --- libcgame offsets ---

static uintptr_t lc_ptr_PM_Move = 0xe67b;
static uintptr_t lc_ptr_CG_FireWeaponEvent = 0x411ed;

static uintptr_t lc_ptr_cg_entities = 0x149c5;
static uintptr_t lc_ptr_CG_GS_Trace = 0x68a65;

static uintptr_t lc_ptr_CG_LFuncDrawBar = 0x4cad3;

static uintptr_t lc_ptr_CG_DrawPlayerNames = 0x6f0e3;
static uintptr_t lc_heap_ptr_cg_entities = 0x4000a0;
#endif
