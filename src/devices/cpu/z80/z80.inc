#define LOG_UNDOC (1U << 1)
#define LOG_INT   (1U << 2)
#define LOG_TIME  (1U << 3)

#define VERBOSE ( LOG_UNDOC /*| LOG_INT*/ )
#include "logmacro.h"

#define LOGUNDOC(...) LOGMASKED(LOG_UNDOC, __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)


/* On an NMOS Z80, if LD A,I or LD A,R is interrupted, P/V flag gets reset,
   even if IFF2 was set before this instruction. This issue was fixed on
   the CMOS Z80, so until knowing (most) Z80 types on hardware, it's disabled */
#define HAS_LDAIR_QUIRK  0


/****************************************************************************
 * The Z80 registers. halt is set to 1 when the CPU is halted, the refresh
 * register is calculated as follows: refresh = (r & 127) | (r2 & 128)
 ****************************************************************************/
#define CF      0x01
#define NF      0x02
#define PF      0x04
#define VF      PF
#define XF      0x08
#define HF      0x10
#define YF      0x20
#define ZF      0x40
#define SF      0x80

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

#define PRVPC   m_prvpc.d     // previous program counter

#define PCD     m_pc.d
#define PC      m_pc.w.l

#define SPD     m_sp.d
#define SP      m_sp.w.l

#define AFD     m_af.d
#define AF      m_af.w.l
#define A       m_af.b.h
#define F       m_af.b.l
#define Q       m_q
#define QT      m_qtemp
#define I       m_i
#define R       m_r
#define R2      m_r2

#define BCD     m_bc.d
#define BC      m_bc.w.l
#define B       m_bc.b.h
#define C       m_bc.b.l

#define DED     m_de.d
#define DE      m_de.w.l
#define D       m_de.b.h
#define E       m_de.b.l

#define HLD     m_hl.d
#define HL      m_hl.w.l
#define H       m_hl.b.h
#define L       m_hl.b.l

#define IXD     m_ix.d
#define IX      m_ix.w.l
#define HX      m_ix.b.h
#define LX      m_ix.b.l

#define IYD     m_iy.d
#define IY      m_iy.w.l
#define HY      m_iy.b.h
#define LY      m_iy.b.l

#define WZ      m_wz.w.l
#define WZ_H    m_wz.b.h
#define WZ_L    m_wz.b.l


#define TADR     m_shared_addr.w   // Typically represents values from A0..15 pins. 16bit input in steps.
#define TADR_H   m_shared_addr.b.h
#define TADR_L   m_shared_addr.b.l
#define TDAT     m_shared_data.w   // 16bit input(if use as second parameter) or output in steps.
#define TDAT2    m_shared_data2.w
#define TDAT_H   m_shared_data.b.h
#define TDAT_L   m_shared_data.b.l
#define TDAT8    m_shared_data.b.l // Typically represents values from D0..8 pins. 8bit input or output in steps.
