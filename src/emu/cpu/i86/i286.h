/****************************************************************************/
/*            real mode i286 emulator by Fabrice Frances                    */
/*           (initial work based on David Hedley's pcemu)                   */
/*                                                                          */
/****************************************************************************/

#include "i86.h"

#define INPUT_LINE_A20		1

#undef GetMemB
#undef GetMemW
#undef PutMemB
#undef PutMemW

/* ASG 971005 -- changed to program_read_byte_8/program_write_byte_8 */
#define GetMemB(Seg,Off) ( (BYTE)program_read_byte_8((DefaultBase(Seg)+(Off))&AMASK))
#define GetMemW(Seg,Off) ( (WORD)GetMemB(Seg,Off)+(WORD)(GetMemB(Seg,(Off)+1)<<8))
#define PutMemB(Seg,Off,x) { program_write_byte_8((DefaultBase(Seg)+(Off))&AMASK,(x)); }
#define PutMemW(Seg,Off,x) { PutMemB(Seg,Off,(BYTE)(x)); PutMemB(Seg,(Off)+1,(BYTE)((x)>>8)); }

#undef PEEKBYTE
#define PEEKBYTE(ea) ((BYTE)program_read_byte_8((ea)&AMASK))

#undef ReadByte
#undef ReadWord
#undef WriteByte
#undef WriteWord

#define ReadByte(ea) ((BYTE)program_read_byte_8((ea)&AMASK))
#define ReadWord(ea) (program_read_byte_8((ea)&AMASK)+(program_read_byte_8(((ea)+1)&AMASK)<<8))
#define WriteByte(ea,val) { program_write_byte_8((ea)&AMASK,val); }
#define WriteWord(ea,val) { program_write_byte_8((ea)&AMASK,(BYTE)(val)); program_write_byte_8(((ea)+1)&AMASK,(val)>>8); }

#undef CHANGE_PC
#define CHANGE_PC(addr) change_pc(addr)
