/*********************************\

 ARCompact Core

\*********************************/

extern const char *conditions[0x20];
extern const char *auxregnames[0x420];
extern const char *datasize[0x4];
extern const char *dataextend[0x2];
extern const char *addressmode[0x4];
extern const char *cachebit[0x2];
extern const char *flagbit[0x2];
extern const char *delaybit[0x2];
extern const char *regnames[0x40];
extern const char *opcodes_04[0x40];

#define REG_BLINK (0x1f) // r31
#define REG_SP (0x1c) // r28
