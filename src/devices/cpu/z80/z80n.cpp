// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************

  Z80N 

***************************************************************************/

#include "emu.h"
#include "z80n.h"

#ifndef PCD
#define PCD     m_pc.d
#define PC      m_pc.w.l

#define A       m_af.b.h

#define BC      m_bc.w.l
#define B       m_bc.b.h

#define DE      m_de.w.l
#define D       m_de.b.h
#define E       m_de.b.l

#define HL      m_hl.w.l
#define H       m_hl.b.h
#define L       m_hl.b.l
#endif

DEFINE_DEVICE_TYPE(Z80N, z80n_device, "z80n", "Z80N")

//-------------------------------------------------
//  z80n_device - constructor
//-------------------------------------------------

z80n_device::z80n_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
    : z80_device(mconfig, Z80N, tag, owner, clock)
    , m_in_nextreg_cb(*this, 0)
    , m_out_nextreg_cb(*this)
{
}

inline void z80n_device::retn()
{
    z80_device::retn();
    if (m_stackless)
    {
        push(m_pc);
        m_pc.b.l = m_in_nextreg_cb(0xc2);
        m_pc.b.h = m_in_nextreg_cb(0xc3);
    }
}

inline void z80n_device::ed_23()
{
    A = (A << 4) | (A >> 4);
}

inline void z80n_device::ed_24()
{
    A = bitswap<8>(A, 0, 1, 2, 3, 4, 5, 6, 7);
}

inline void z80n_device::ed_27()
{
    const u8 a = A;
    and_a(arg());
    A = a;
}

inline void z80n_device::ed_28()
{
    DE <<= B & 31;
}

inline void z80n_device::ed_29()
{
    const u16 lb = DE & 0x8000;
    for (auto i = 0; i < (B & 31); i++)
    {
        DE >>= 1;
        DE |= lb;
    }
}

inline void z80n_device::ed_2a()
{
    DE >>= (B & 31);
}

inline void z80n_device::ed_2b()
{
    for (auto i = 0; i < (B & 31); i++)
    {
        DE >>= 1;
        DE |= 0x8000;
    }
}

inline void z80n_device::ed_2c()
{
    DE = (DE << (B & 15)) | (DE >> (16 - (B & 15)));
}

inline void z80n_device::ed_30()
{
    DE = D * E;
}

inline void z80n_device::ed_31()
{
    HL += A;
}

inline void z80n_device::ed_32()
{
    DE += A;
}

inline void z80n_device::ed_33()
{
    BC += A;
}

inline void z80n_device::ed_34()
{
    HL += arg16();
}

inline void z80n_device::ed_35()
{
    DE += arg16();
}

inline void z80n_device::ed_36()
{
    BC += arg16();
}

inline void z80n_device::ed_8a()
{
    PAIR tmp = {{0, 0, 0, 0}};
    tmp.b.h = arg();
    tmp.b.l = arg();
    push(tmp);
}

inline void z80n_device::ed_90()
{
    const u8 data = rm(HL);
    out(BC, data);
    HL++;
}

inline void z80n_device::ed_91()
{
    const u8 reg = arg();
    const u8 data = arg();
    m_out_nextreg_cb(reg, data);
}

inline void z80n_device::ed_92()
{
    const u8 reg = arg();
    m_out_nextreg_cb(reg, A);
}

inline void z80n_device::ed_93()
{
    HL = (0x07 != (H & 0x07))
        ? HL + 0x100
        : (0xe0 != (L & 0xe0))
            ? (HL & 0xf8ff) + 0x20
            : ((HL & 0xf81f) + 0x800);
}

inline void z80n_device::ed_94()
{
    HL = 0x4000 + ((D & 0xc0) << 5) + ((D & 0x07) << 8) + ((D & 0x38) << 2) + (E >> 3);
}

inline void z80n_device::ed_95()
{
    A = 0x80 >> (E & 7);
}

inline void z80n_device::ed_98()
{
    PCD = (PCD & 0xc000) + (in(BC) << 6);
}

inline void z80n_device::ed_a4()
{
    u8 data = rm(HL);
    if (data != A)
        wm(DE, data);
    HL++;
    DE++;
    BC--;
}

inline void z80n_device::ed_a5()
{
    wm(DE, rm(HL));
    L++;
    D = inc(D);
}

inline void z80n_device::ed_ac()
{
    const u8 data = rm(HL);
    if (data != A)
        wm(DE, data);
    DE++;
    HL--;
    BC--;
}

inline void z80n_device::ed_b4()
{
    ed_a4();
    if (BC != 0)
        PC -= 2;
}

inline void z80n_device::ed_b7()
{
    const u8 data = rm((HL & 0xfff8) + (E & 7));
    if (data != A)
        wm(DE, data);
    DE++;
    BC--;
    if (BC != 0)
        PC -= 2;
}

inline void z80n_device::ed_bc()
{
    ed_ac();
    if (BC != 0)
        PC -= 2;
}

void z80n_device::device_reset()
{
    z80_device::device_reset();
    m_stackless = 0;
}
