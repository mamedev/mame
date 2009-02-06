// new code to allow the PinMAME rom loading to compile

/*-------------------------
/  S9 sound on CPU board
/--------------------------*/

#define S9S_CPUREGION "s9s_cpu"
//MACHINE_DRIVER_EXTERN(wmssnd_s9s);
//MACHINE_DRIVER_EXTERN(wmssnd_s9ps);  /* pennant fever */

#define S9S_STDREG SOUNDREGION(0x10000, S9S_CPUREGION)

#define S9S_SOUNDROM41111(u49,chk49, u4,chk4, u5,chk5, u6,chk6, u7,chk7) \
   S9S_STDREG \
     ROM_LOAD(u49, 0xc000, 0x4000, chk49)  \
     ROM_LOAD(u7,  0x8000, 0x1000, chk7)  \
     ROM_LOAD(u5,  0x9000, 0x1000, chk5)  \
     ROM_LOAD(u6,  0xa000, 0x1000, chk6)  \
     ROM_LOAD(u4,  0xb000, 0x1000, chk4)

#define S9S_SOUNDROM4111(u49,chk49, u4,chk4, u5,chk5, u6,chk6) \
   S9S_STDREG \
     ROM_LOAD(u49, 0xc000, 0x4000, chk49)  \
     ROM_LOAD(u5,  0x9000, 0x1000, chk5) \
     ROM_LOAD(u6,  0xa000, 0x1000, chk6) \
     ROM_LOAD(u4,  0xb000, 0x1000, chk4)

#define S9S_SOUNDROM4(u49,chk49) \
   S9S_STDREG \
     ROM_LOAD(u49, 0xc000, 0x4000, chk49) \
     ROM_RELOAD(0x8000, 0x4000)

#define S9RR_SOUNDROM(u49, chk49) \
   S9S_STDREG \
     ROM_LOAD(u49, 0xe000, 0x2000, chk49) \
     ROM_RELOAD(0xc000, 0x2000) \
     ROM_RELOAD(0xa000, 0x2000) \
     ROM_RELOAD(0x8000, 0x2000)

/*-------------------------
/  S11 sound on CPU board
/--------------------------*/

#define S11XS_CPUREGION "s11xs_cpu"
#define S11XS_ROMREGION "sound2"
#define S11S_CPUREGION "s11s_cpu"
#define S11S_ROMREGION "sound2"
//extern MACHINE_DRIVER_EXTERN(wmssnd_s11s);   /* without extra sound board */
//extern MACHINE_DRIVER_EXTERN(wmssnd_s11xs);  /* with s11c sound board */
//extern MACHINE_DRIVER_EXTERN(wmssnd_s11b2s); /* with jokerz sound board */

#define S11XS_STDREG \

#define S11XS_SOUNDROM44(n1, chk1, n2, chk2) \
  SOUNDREGION(0x10000, S11XS_CPUREGION) \
  SOUNDREGION(0x10000, S11XS_ROMREGION) \
    ROM_LOAD(n1, 0x4000, 0x4000, chk1) \
    ROM_LOAD(n2, 0xc000, 0x4000, chk2) \
      ROM_RELOAD(  0x8000, 0x4000)

#define S11XS_SOUNDROMx8(n2, chk2) \
  SOUNDREGION(0x10000, S11XS_CPUREGION) \
  SOUNDREGION(0x10000, S11XS_ROMREGION) \
    ROM_LOAD(n2, 0x4000, 0x4000, chk2) \
      ROM_CONTINUE(0xc000, 0x4000)

#define S11XS_SOUNDROM88(n1, chk1, n2, chk2) \
  SOUNDREGION(0x10000, S11XS_CPUREGION) \
  SOUNDREGION(0x10000, S11XS_ROMREGION) \
    ROM_LOAD(n1, 0x0000, 0x8000, chk1) \
    ROM_LOAD(n2, 0x8000, 0x8000, chk2)

#define S11S_SOUNDROM88(n1, chk1, n2, chk2) \
  SOUNDREGION(0x10000, S11S_CPUREGION) \
  SOUNDREGION(0x10000, S11S_ROMREGION) \
    ROM_LOAD(n1, 0x0000, 0x8000, chk1) \
    ROM_LOAD(n2, 0x8000, 0x8000, chk2)

/*-------------------------
/  S11C sound board
/--------------------------*/

#define S11CS_CPUREGION "s11cs_cpu"
#define S11CS_ROMREGION "sound1"
//extern MACHINE_DRIVER_EXTERN(wmssnd_s11cs);

#define S11CS_STDREG \
  SOUNDREGION(0x10000, S11CS_CPUREGION) \
  SOUNDREGION(0x30000, S11CS_ROMREGION)

#define S11CS_ROMLOAD8(start, n, chk) \
  ROM_LOAD(n, start, 0x8000, chk) \
    ROM_RELOAD(start+0x8000, 0x8000)

#define S11CS_ROMLOAD0(start, n, chk) \
  ROM_LOAD(n, start, 0x10000, chk)

#define S11CS_SOUNDROM000(n1,chk1,n2,chk2,n3,chk3) \
  S11CS_STDREG \
  S11CS_ROMLOAD0(0x00000, n1, chk1) \
  S11CS_ROMLOAD0(0x10000, n2, chk2) \
  S11CS_ROMLOAD0(0x20000, n3, chk3)

#define S11CS_SOUNDROM008(n1,chk1,n2,chk2,n3,chk3) \
  S11CS_STDREG \
  S11CS_ROMLOAD0(0x00000, n1, chk1) \
  S11CS_ROMLOAD0(0x10000, n2, chk2) \
  S11CS_ROMLOAD8(0x20000, n3, chk3)

#define S11CS_SOUNDROM888(n1,chk1,n2,chk2,n3,chk3) \
  S11CS_STDREG \
  S11CS_ROMLOAD8(0x00000, n1, chk1) \
  S11CS_ROMLOAD8(0x10000, n2, chk2) \
  S11CS_ROMLOAD8(0x20000, n3, chk3)

#define S11CS_SOUNDROM88(n1,chk1,n2,chk2) \
  S11CS_STDREG \
  S11CS_ROMLOAD8(0x00000, n1, chk1) \
  S11CS_ROMLOAD8(0x10000, n2, chk2)

#define S11CS_SOUNDROM8(n1,chk1) \
  S11CS_STDREG \
  S11CS_ROMLOAD8(0x00000, n1, chk1)


/*-------------------------
/  Jokerz! sound board
/--------------------------*/
#define S11JS_CPUREGION "s11js_cpu"
#define S11JS_ROMREGION "sound1"
//extern MACHINE_DRIVER_EXTERN(wmssnd_s11js);

#define S11JS_SOUNDROM(n1, chk1) \
  SOUNDREGION(0x10000, S11JS_CPUREGION) \
    ROM_LOAD(n1, 0x0000, 0x10000, chk1) \
  SOUNDREGION(0x10000, S11JS_ROMREGION) \
    ROM_LOAD(n1, 0x0000, 0x10000, chk1)
	 
/************************************************************************************************
*************************************************************************************************
 Old PinMAME code below for reference ONLY
*************************************************************************************************
************************************************************************************************/

#if 0

#ifndef INC_WMSSND
#define INC_WMSSND
/* DCS sound needs this one */
#include "cpu/adsp2100/adsp2100.h"

/*-------------------------
/  S3-S7 sound board
/--------------------------*/
#define S67S_CPUNO        1
#define S67S_MEMREG_SCPU  (REGION_CPU1+S67S_CPUNO)
extern MACHINE_DRIVER_EXTERN(wmssnd_s67s);

#define S67S_SOUNDROMS0(ic12, chk12) \
  SOUNDREGION(0x10000, S67S_MEMREG_SCPU) \
    ROM_LOAD(ic12, 0x7000, 0x1000, chk12) \
    ROM_RELOAD(    0xf000, 0x1000)

#define S67S_SOUNDROMS8(ic12, chk12) \
  SOUNDREGION(0x10000, S67S_MEMREG_SCPU) \
    ROM_LOAD(ic12, 0x7800, 0x0800, chk12) \
    ROM_RELOAD(    0xf800, 0x0800)

#define S67S_SPEECHROMS0000(ic7,chk7, ic5,chk5, ic6,chk6, ic4, chk4) \
    ROM_LOAD(ic7, 0x3000, 0x1000, chk7) \
    ROM_RELOAD(   0xb000, 0x1000) \
    ROM_LOAD(ic5, 0x4000, 0x1000, chk5) \
    ROM_RELOAD(   0xc000, 0x1000) \
    ROM_LOAD(ic6, 0x5000, 0x1000, chk6) \
    ROM_RELOAD(   0xd000, 0x1000) \
    ROM_LOAD(ic4, 0x6000, 0x1000, chk4) \
    ROM_RELOAD(   0xe000, 0x1000)

#define S67S_SPEECHROMS000x(ic7,chk7, ic5,chk5, ic6,chk6) \
    ROM_LOAD(ic7, 0x3000, 0x1000, chk7) \
    ROM_RELOAD(   0xb000, 0x1000) \
    ROM_LOAD(ic5, 0x4000, 0x1000, chk5) \
    ROM_RELOAD(   0xc000, 0x1000) \
    ROM_LOAD(ic6, 0x5000, 0x1000, chk6) \
    ROM_RELOAD(   0xd000, 0x1000)






/*-------------------------
/  WPC sound board
/--------------------------*/
#define WPCS_CPUNO 1
#define WPCS_CPUREGION (REGION_CPU1+WPCS_CPUNO)
#define WPCS_ROMREGION (REGION_SOUND1)
extern MACHINE_DRIVER_EXTERN(wmssnd_wpcs);

#define WPCS_STDREG \
  SOUNDREGION(0x010000, WPCS_CPUREGION) \
  SOUNDREGION(0x180000, WPCS_ROMREGION)

#define WPCS_ROMLOAD2(start, n, chk) \
  ROM_LOAD(n, start,  0x20000, chk) \
    ROM_RELOAD( start + 0x20000, 0x20000) \
    ROM_RELOAD( start + 0x40000, 0x20000) \
    ROM_RELOAD( start + 0x60000, 0x20000)

#define WPCS_ROMLOAD4(start, n, chk) \
  ROM_LOAD(n, start,  0x40000, chk) \
    ROM_RELOAD( start + 0x40000, 0x40000)

#define WPCS_ROMLOAD8(start, n, chk) \
  ROM_LOAD(n, start, 0x80000, chk)

#define WPCS_SOUNDROM882(u18,chk18,u15,chk15,u14,chk14) \
  WPCS_STDREG \
  WPCS_ROMLOAD8(0x000000, u18, chk18) \
  WPCS_ROMLOAD8(0x080000, u15, chk15) \
  WPCS_ROMLOAD2(0x100000, u14, chk14)
#define WPCS_SOUNDROM288(u18,chk18,u15,chk15,u14,chk14) \
  WPCS_STDREG \
  WPCS_ROMLOAD2(0x000000, u18, chk18) \
  WPCS_ROMLOAD8(0x080000, u15, chk15) \
  WPCS_ROMLOAD8(0x100000, u14, chk14)
#define WPCS_SOUNDROM222(u18,chk18,u15,chk15,u14,chk14) \
  WPCS_STDREG \
  WPCS_ROMLOAD2(0x000000, u18, chk18) \
  WPCS_ROMLOAD2(0x080000, u15, chk15) \
  WPCS_ROMLOAD2(0x100000, u14, chk14)
#define WPCS_SOUNDROM224(u18,chk18,u15,chk15,u14,chk14) \
  WPCS_STDREG \
  WPCS_ROMLOAD2(0x000000, u18, chk18) \
  WPCS_ROMLOAD2(0x080000, u15, chk15) \
  WPCS_ROMLOAD4(0x100000, u14, chk14)
#define WPCS_SOUNDROM248(u18,chk18,u15,chk15,u14,chk14) \
  WPCS_STDREG \
  WPCS_ROMLOAD2(0x000000, u18, chk18) \
  WPCS_ROMLOAD4(0x080000, u15, chk15) \
  WPCS_ROMLOAD8(0x100000, u14, chk14)
#define WPCS_SOUNDROM2x8(u18,chk18,u14,chk14) \
  WPCS_STDREG \
  WPCS_ROMLOAD2(0x000000, u18, chk18) \
  WPCS_ROMLOAD8(0x100000, u14, chk14)
#define WPCS_SOUNDROM84x(u18,chk18,u15,chk15) \
  WPCS_STDREG \
  WPCS_ROMLOAD8(0x000000, u18, chk18) \
  WPCS_ROMLOAD4(0x080000, u15, chk15)
#define WPCS_SOUNDROM22x(u18,chk18,u15,chk15) \
  WPCS_STDREG \
  WPCS_ROMLOAD2(0x000000, u18, chk18) \
  WPCS_ROMLOAD2(0x080000, u15, chk15)
#define WPCS_SOUNDROM888(u18,chk18,u15,chk15,u14,chk14) \
  WPCS_STDREG \
  WPCS_ROMLOAD8(0x000000, u18, chk18) \
  WPCS_ROMLOAD8(0x080000, u15, chk15) \
  WPCS_ROMLOAD8(0x100000, u14, chk14)
#define WPCS_SOUNDROM8xx(u18,chk18) \
  WPCS_STDREG \
  WPCS_ROMLOAD8(0x000000, u18, chk18)

/*-------------------------
/  DCS sound board
/--------------------------*/
#define DCS_CPUNO 1
#define DCS_CPUREGION  (REGION_CPU1+DCS_CPUNO)
#define DCS_ROMREGION  (REGION_SOUND1)
#define DCS_BANKREGION (REGION_USER3)
extern MACHINE_DRIVER_EXTERN(wmssnd_dcs1);
/* DCS on WPC95 audio/visual board */
extern MACHINE_DRIVER_EXTERN(wmssnd_dcs2);

#define DCS_STDREG(size) \
   SOUNDREGION(ADSP2100_SIZE, DCS_CPUREGION) \
   SOUNDREGION(0x1000*2,      DCS_BANKREGION) \
   SOUNDREGION(0x800000,      DCS_ROMREGION)

#define DCS_ROMLOADx(start, n, chk) \
   ROM_LOAD(n, start, 0x080000, chk) ROM_RELOAD(start+0x080000, 0x080000)
#define DCS_ROMLOADm(start, n,chk) \
   ROM_LOAD(n, start, 0x100000, chk)

#define DCS_SOUNDROM1x(n2,chk2) \
   DCS_STDREG(0x100000) \
   DCS_ROMLOADx(0x000000,n2,chk2)

#define DCS_SOUNDROM1m(n2,chk2) \
   DCS_STDREG(0x100000) \
   DCS_ROMLOADm(0x000000,n2,chk2)

#define DCS_SOUNDROM2m(n2,chk2,n3,chk3) \
   DCS_STDREG(0x200000) \
   DCS_ROMLOADm(0x000000,n2,chk2) \
   DCS_ROMLOADm(0x100000,n3,chk3)

#define DCS_SOUNDROM3m(n2,chk2,n3,chk3,n4,chk4) \
   DCS_STDREG(0x300000) \
   DCS_ROMLOADm(0x000000,n2,chk2) \
   DCS_ROMLOADm(0x100000,n3,chk3) \
   DCS_ROMLOADm(0x200000,n4,chk4)

#define DCS_SOUNDROM4m(n2,chk2,n3,chk3,n4,chk4,n5,chk5) \
   DCS_STDREG(0x400000) \
   DCS_ROMLOADm(0x000000,n2,chk2) \
   DCS_ROMLOADm(0x100000,n3,chk3) \
   DCS_ROMLOADm(0x200000,n4,chk4) \
   DCS_ROMLOADm(0x300000,n5,chk5)

#define DCS_SOUNDROM4xm(n2,chk2,n3,chk3,n4,chk4,n5,chk5) \
   DCS_STDREG(0x400000) \
   DCS_ROMLOADx(0x000000,n2,chk2) \
   DCS_ROMLOADm(0x100000,n3,chk3) \
   DCS_ROMLOADm(0x200000,n4,chk4) \
   DCS_ROMLOADm(0x300000,n5,chk5)

#define DCS_SOUNDROM4mx(n2,chk2,n3,chk3,n4,chk4,n5,chk5) \
   DCS_STDREG(0x400000) \
   DCS_ROMLOADm(0x000000,n2,chk2) \
   DCS_ROMLOADm(0x100000,n3,chk3) \
   DCS_ROMLOADm(0x200000,n4,chk4) \
   DCS_ROMLOADx(0x300000,n5,chk5)

#define DCS_SOUNDROM5xm(n2,chk2,n3,chk3,n4,chk4,n5,chk5,n6,chk6) \
   DCS_STDREG(0x500000) \
   DCS_ROMLOADx(0x000000,n2,chk2) \
   DCS_ROMLOADm(0x100000,n3,chk3) \
   DCS_ROMLOADm(0x200000,n4,chk4) \
   DCS_ROMLOADm(0x300000,n5,chk5) \
   DCS_ROMLOADm(0x400000,n6,chk6)

#define DCS_SOUNDROM5x(n2,chk2,n3,chk3,n4,chk4,n5,chk5,n6,chk6) \
   DCS_STDREG(0x500000) \
   DCS_ROMLOADx(0x000000,n2,chk2) \
   DCS_ROMLOADx(0x100000,n3,chk3) \
   DCS_ROMLOADx(0x200000,n4,chk4) \
   DCS_ROMLOADx(0x300000,n5,chk5) \
   DCS_ROMLOADx(0x400000,n6,chk6)

#define DCS_SOUNDROM5m(n2,chk2,n3,chk3,n4,chk4,n5,chk5,n6,chk6) \
   DCS_STDREG(0x500000) \
   DCS_ROMLOADm(0x000000,n2,chk2) \
   DCS_ROMLOADm(0x100000,n3,chk3) \
   DCS_ROMLOADm(0x200000,n4,chk4) \
   DCS_ROMLOADm(0x300000,n5,chk5) \
   DCS_ROMLOADm(0x400000,n6,chk6)

#define DCS_SOUNDROM6x(n2,chk2,n3,chk3,n4,chk4,n5,chk5,n6,chk6,n7,chk7) \
   DCS_STDREG(0x600000) \
   DCS_ROMLOADx(0x000000,n2,chk2) \
   DCS_ROMLOADx(0x100000,n3,chk3) \
   DCS_ROMLOADx(0x200000,n4,chk4) \
   DCS_ROMLOADx(0x300000,n5,chk5) \
   DCS_ROMLOADx(0x400000,n6,chk6) \
   DCS_ROMLOADx(0x500000,n7,chk7)

#define DCS_SOUNDROM6m(n2,chk2,n3,chk3,n4,chk4,n5,chk5,n6,chk6,n7,chk7) \
   DCS_STDREG(0x600000) \
   DCS_ROMLOADm(0x000000,n2,chk2) \
   DCS_ROMLOADm(0x100000,n3,chk3) \
   DCS_ROMLOADm(0x200000,n4,chk4) \
   DCS_ROMLOADm(0x300000,n5,chk5) \
   DCS_ROMLOADm(0x400000,n6,chk6) \
   DCS_ROMLOADm(0x500000,n7,chk7)

#define DCS_SOUNDROM6xm(n2,chk2,n3,chk3,n4,chk4,n5,chk5,n6,chk6,n7,chk7) \
   DCS_STDREG(0x600000) \
   DCS_ROMLOADx(0x000000,n2,chk2) \
   DCS_ROMLOADm(0x100000,n3,chk3) \
   DCS_ROMLOADm(0x200000,n4,chk4) \
   DCS_ROMLOADm(0x300000,n5,chk5) \
   DCS_ROMLOADm(0x400000,n6,chk6) \
   DCS_ROMLOADm(0x500000,n7,chk7)

#define DCS_SOUNDROM7x(n2,chk2,n3,chk3,n4,chk4,n5,chk5,n6,chk6,n7,chk7,n8,chk8) \
   DCS_STDREG(0x700000) \
   DCS_ROMLOADx(0x000000,n2,chk2) \
   DCS_ROMLOADx(0x100000,n3,chk3) \
   DCS_ROMLOADx(0x200000,n4,chk4) \
   DCS_ROMLOADx(0x300000,n5,chk5) \
   DCS_ROMLOADx(0x400000,n6,chk6) \
   DCS_ROMLOADx(0x500000,n7,chk7) \
   DCS_ROMLOADx(0x600000,n8,chk8)

#define DCS_SOUNDROM8x(n2,chk2,n3,chk3,n4,chk4,n5,chk5,n6,chk6,n7,chk7,n8,chk8,n9,chk9) \
   DCS_STDREG(0x800000) \
   DCS_ROMLOADx(0x000000,n2,chk2) \
   DCS_ROMLOADx(0x100000,n3,chk3) \
   DCS_ROMLOADx(0x200000,n4,chk4) \
   DCS_ROMLOADx(0x300000,n5,chk5) \
   DCS_ROMLOADx(0x400000,n6,chk6) \
   DCS_ROMLOADx(0x500000,n7,chk7) \
   DCS_ROMLOADx(0x600000,n8,chk8) \
   DCS_ROMLOADx(0x700000,n9,chk9)

#endif /* INC_WMSSND */

#endif
