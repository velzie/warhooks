# WarHooks

Cheats for warsow/warhook on linux with some funny C and LD_PRELOAD

I made this because I wanted to learn about game hacking and function detouring, do NOT use this on public lobbies

# Features
- bypass sv_pure checks
- wallhack / ESP / nametags
- triggerbot
- aimbot

# How to compile
`gcc main.c -g -o hax.so -fPIC -shared`

# How to use
- compile warfork with steamlib disabled and debugging symbols enabled, commit# `bbf81122b2337eb1509144e5aee5e4ba8bd57fd3`
- launch game with LD_PRELOAD pointed to the compiled dll

### Commands
```
wh_nametags 0 | 1
wh_triggerbot 0 | 1
wh_aimbot 0 | 1
wh_getpos
```
