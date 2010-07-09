/***************************************************************************

CuboCD32 definitions

***************************************************************************/

#ifndef __CUBOCD32_H__
#define __CUBOCD32_H__

/*----------- defined in machine/cubocd32.c -----------*/

extern void amiga_akiko_init(running_machine* machine);
extern READ32_HANDLER(amiga_akiko32_r);
extern WRITE32_HANDLER(amiga_akiko32_w);

#endif /* __CUBOCD32_H__ */
