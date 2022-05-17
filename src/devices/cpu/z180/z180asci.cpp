// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    z180asci.cpp

*********************************************************************/

#include "emu.h"
#include "z180.h"

//#define VERBOSE 1

#include "logmacro.h"

/* 00 ASCI control register A ch 0 */
#define Z180_CNTLA0_MPE         0x80
#define Z180_CNTLA0_RE          0x40
#define Z180_CNTLA0_TE          0x20
#define Z180_CNTLA0_RTS0        0x10
#define Z180_CNTLA0_MPBR_EFR    0x08
#define Z180_CNTLA0_MODE_DATA   0x04
#define Z180_CNTLA0_MODE_PARITY 0x02
#define Z180_CNTLA0_MODE_STOPB  0x01

/* 01 ASCI control register A ch 1 */
#define Z180_CNTLA1_MPE         0x80
#define Z180_CNTLA1_RE          0x40
#define Z180_CNTLA1_TE          0x20
#define Z180_CNTLA1_CKA1D       0x10
#define Z180_CNTLA1_MPBR_EFR    0x08
#define Z180_CNTLA1_MODE        0x07

/* 02 ASCI control register B ch 0 */
#define Z180_CNTLB0_MPBT        0x80
#define Z180_CNTLB0_MP          0x40
#define Z180_CNTLB0_CTS_PS      0x20
#define Z180_CNTLB0_PEO         0x10
#define Z180_CNTLB0_DR          0x08
#define Z180_CNTLB0_SS          0x07

/* 03 ASCI control register B ch 1 */
#define Z180_CNTLB1_MPBT        0x80
#define Z180_CNTLB1_MP          0x40
#define Z180_CNTLB1_CTS_PS      0x20
#define Z180_CNTLB1_PEO         0x10
#define Z180_CNTLB1_DR          0x08
#define Z180_CNTLB1_SS          0x07

/* 04 ASCI status register 0 (all bits read-only except RIE and TIE) */
#define Z180_STAT0_RDRF         0x80
#define Z180_STAT0_OVRN         0x40
#define Z180_STAT0_PE           0x20
#define Z180_STAT0_FE           0x10
#define Z180_STAT0_RIE          0x08
#define Z180_STAT0_DCD0         0x04
#define Z180_STAT0_TDRE         0x02
#define Z180_STAT0_TIE          0x01

/* 05 ASCI status register 1 (all bits read-only except RIE, CTS1E and TIE) */
#define Z180_STAT1_RDRF         0x80
#define Z180_STAT1_OVRN         0x40
#define Z180_STAT1_PE           0x20
#define Z180_STAT1_FE           0x10
#define Z180_STAT1_RIE          0x08
#define Z180_STAT1_CTS1E        0x04
#define Z180_STAT1_TDRE         0x02
#define Z180_STAT1_TIE          0x01

/* 12 (Z8S180/Z8L180) ASCI extension control register 0 (break detect is read-only) */
#define Z180_ASEXT0_DCD0        0x40
#define Z180_ASEXT0_CTS0        0x20
#define Z180_ASEXT0_X1_BIT_CLK0 0x10
#define Z180_ASEXT0_BRG0_MODE   0x08
#define Z180_ASEXT0_BRK_EN      0x04
#define Z180_ASEXT0_BRK_DET     0x02
#define Z180_ASEXT0_BRK_SEND    0x01

#define Z180_ASEXT0_MASK        0x7f

/* 13 (Z8S180/Z8L180) ASCI extension control register 1 (break detect is read-only) */
#define Z180_ASEXT1_X1_BIT_CLK1 0x10
#define Z180_ASEXT1_BRG1_MODE   0x08
#define Z180_ASEXT1_BRK_EN      0x04
#define Z180_ASEXT1_BRK_DET     0x02
#define Z180_ASEXT1_BRK_SEND    0x01

#define Z180_ASEXT1_MASK        0x1f

//**************************************************************************
//  z180asci_channel_base
//**************************************************************************

z180asci_channel_base::z180asci_channel_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const int id, const bool ext)
	: device_t(mconfig, type, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, m_txa_handler(*this)
	, m_rts_handler(*this)
	, m_divisor(1)
	, m_id(id)
	, m_ext(ext)
{
}

void z180asci_channel_base::device_resolve_objects()
{
	// resolve callbacks
	m_txa_handler.resolve_safe();
	m_rts_handler.resolve_safe();
}

void z180asci_channel_base::device_start()
{
	save_item(NAME(m_asci_cntla));
	save_item(NAME(m_asci_cntlb));
	save_item(NAME(m_asci_stat));
	save_item(NAME(m_asci_tdr));
	save_item(NAME(m_asci_rdr));
	if (m_ext)
	{
		save_item(NAME(m_asci_ext));
		save_item(NAME(m_asci_tc.w));
	}
	save_item(NAME(m_divisor));
	transmit_register_reset();
	receive_register_reset();
}

void z180asci_channel_base::device_reset()
{
	m_asci_cntla = 0;
	m_asci_cntlb = 7; // SS2,SS1 and SS0 are 1 after reset
	m_asci_tdr = 0;
	m_asci_rdr = 0;
	m_asci_stat = 2; // set TDRE=1 on reset
	m_irq = 0;
	m_cts = 0;
	m_dcd = 0;
	m_asci_ext = 0;
	m_asci_tc.w = 0;
	m_divisor = 1;
}

void z180asci_channel_base::device_clock_changed()
{
	LOG("Z180 ASCI%d set bitrate %d\n", m_id, uint32_t(clock() / m_divisor));
	set_tra_rate(clock(), m_divisor);
	set_rcv_rate(clock(), m_divisor);
}

uint8_t z180asci_channel_base::cntla_r()
{
	LOG("Z180 CNTLA%d rd $%02x\n", m_id, m_asci_cntla);
	return m_asci_cntla;
}

uint8_t z180asci_channel_base::cntlb_r()
{
	LOG("Z180 CNTLB%d rd $%02x\n", m_id, m_asci_cntlb);
	return m_asci_cntlb;
}

uint8_t z180asci_channel_base::stat_r()
{
	LOG("Z180 STAT%d  rd $%02x\n", m_id, m_asci_stat);
	return m_asci_stat;
}

uint8_t z180asci_channel_base::tdr_r()
{
	LOG("Z180 TDR%d   rd $%02x\n", m_id, m_asci_tdr);
	return m_asci_tdr;
}

uint8_t z180asci_channel_base::rdr_r()
{
	LOG("Z180 RDR%d   rd $%02x\n", m_id, m_asci_rdr);
	m_asci_stat &= ~0x80;
	return m_asci_rdr;
}

uint8_t z180asci_channel_base::asext_r()
{
	LOG("Z180 ASEXT%d rd $%02x\n", m_id, m_asci_ext);
	return m_asci_ext;
}

uint8_t z180asci_channel_base::astcl_r()
{
	LOG("Z180 ASTC%dL rd $%02x ($%04x)\n", m_id, m_asci_tc.b.l, m_asci_tc.w);
	return m_asci_tc.b.l;
}

uint8_t z180asci_channel_base::astch_r()
{
	LOG("Z180 ASTC%dH rd $%02x ($%04x)\n", m_id, m_asci_tc.b.h, m_asci_tc.w);
	return m_asci_tc.b.h;
}

void z180asci_channel_base::cntla_w(uint8_t data)
{
	LOG("Z180 CNTLA%d wr $%02x\n", m_id, data);
	m_asci_cntla = data;
	set_data_frame(1, (data & 4) ? 8 : 7, (data & 2) ? PARITY_ODD : PARITY_NONE, (data & 2) ? STOP_BITS_2 : STOP_BITS_1);
}

void z180asci_channel_base::cntlb_w(uint8_t data)
{
	LOG("Z180 CNTLB%d wr $%02x\n", m_id, data);
	m_asci_cntlb = data;

	m_divisor = 1<<(m_asci_cntlb & 0x07);
	m_divisor *= ((m_asci_cntlb & 0x20) ? 30 : 10);
	m_divisor *= ((m_asci_cntlb & 0x08) ? 64 : 16);

	device_clock_changed();
}

void z180asci_channel_base::tdr_w(uint8_t data)
{
	LOG("Z180 TDR%d   wr $%02x\n", m_id, data);
	m_asci_tdr = data;
	m_asci_stat &= ~0x02; // clear TDRE
	transmit_byte(m_asci_tdr);
}

void z180asci_channel_base::rdr_w(uint8_t data)
{
	LOG("Z180 RDR%d   wr $%02x\n", m_id, data);
	m_asci_rdr = data;
}

void z180asci_channel_base::astcl_w(uint8_t data)
{
	LOG("Z180 ASTC%dL wr $%02x\n", m_id, data);
	m_asci_tc.b.l = data;
}

void z180asci_channel_base::astch_w(uint8_t data)
{
	LOG("Z180 ASTC%dH wr $%02x\n", m_id, data);
	m_asci_tc.b.h = data;
}

void z180asci_channel_base::tra_callback()
{
	m_txa_handler(transmit_register_get_data_bit());
}

void z180asci_channel_base::tra_complete()
{
	device_buffered_serial_interface::tra_complete();
	m_asci_stat |= 0x02; // set TDRE
	if (m_asci_stat & Z180_STAT0_TIE)
	{
		m_irq = 1;
	}
}

void z180asci_channel_base::received_byte(u8 byte)
{
	m_asci_stat |= 0x80;
	m_asci_rdr = byte;
	if (m_asci_stat & Z180_STAT0_RIE)
	{
		m_irq = 1;
	}
}

DECLARE_WRITE_LINE_MEMBER( z180asci_channel_base::rxa_wr )
{
	device_buffered_serial_interface::rx_w(state);
}

//**************************************************************************
//  z180asci_channel_0
//**************************************************************************

z180asci_channel_0::z180asci_channel_0(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const bool ext)
	: z180asci_channel_base(mconfig, type, tag, owner, clock, 0, ext)
{
}

z180asci_channel_0::z180asci_channel_0(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180asci_channel_0(mconfig, Z180ASCI_CHANNEL_0, tag, owner, clock, false)
{
}

void z180asci_channel_0::device_reset()
{
	z180asci_channel_base::device_reset();
	m_asci_cntla = (m_asci_cntla & Z180_CNTLA0_MPBR_EFR) | Z180_CNTLA0_RTS0;
	m_asci_cntlb = (m_asci_cntlb & (Z180_CNTLB0_MPBT | Z180_CNTLB0_CTS_PS)) | 0x07;
	m_asci_stat = m_asci_stat & (Z180_STAT0_DCD0 | Z180_STAT0_TDRE);
}

void z180asci_channel_0::state_add(device_state_interface &parent)
{
	parent.state_add(Z180_CNTLA0, "CNTLA0",  m_asci_cntla);
	parent.state_add(Z180_CNTLB0, "CNTLB0",  m_asci_cntlb);
	parent.state_add(Z180_STAT0,  "STAT0",   m_asci_stat);
	parent.state_add(Z180_TDR0,   "TDR0",    m_asci_tdr);
	parent.state_add(Z180_RDR0,   "RDR0",    m_asci_rdr);
	if (m_ext)
	{
		parent.state_add(Z180_ASEXT0, "ASEXT0",  m_asci_ext).mask(Z180_ASEXT0_MASK);
		parent.state_add(Z180_ASTC0,  "ASTC0",   m_asci_tc.w);
	}
}

void z180asci_channel_0::stat_w(uint8_t data)
{
	LOG("Z180 STAT0  wr $%02x ($%02x)\n", data,  data & (Z180_STAT0_RIE | Z180_STAT0_TIE));
	m_asci_stat = (m_asci_stat & ~(Z180_STAT0_RIE | Z180_STAT0_TIE)) | (data & (Z180_STAT0_RIE | Z180_STAT0_TIE));
}

void z180asci_channel_0::asext_w(uint8_t data)
{
	LOG("Z180 ASEXT0 wr $%02x ($%02x)\n", data,  data & Z180_ASEXT0_MASK & ~Z180_ASEXT0_BRK_DET);
	m_asci_ext = (m_asci_ext & Z180_ASEXT0_BRK_DET) | (data & Z180_ASEXT0_MASK & ~Z180_ASEXT0_BRK_DET);
}

//**************************************************************************
//  z180asci_channel_1
//**************************************************************************

z180asci_channel_1::z180asci_channel_1(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const bool ext)
	: z180asci_channel_base(mconfig, type, tag, owner, clock, 1, ext)
{
}

z180asci_channel_1::z180asci_channel_1(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180asci_channel_1(mconfig, Z180ASCI_CHANNEL_1, tag, owner, clock, false)
{
}
void z180asci_channel_1::device_reset()
{
	z180asci_channel_base::device_reset();
	m_asci_cntla = (m_asci_cntla & Z180_CNTLA1_MPBR_EFR) | Z180_CNTLA1_CKA1D;
	m_asci_cntlb = (m_asci_cntlb & Z180_CNTLB1_MPBT) | 0x07;
	m_asci_stat = Z180_STAT1_TDRE;
}

void z180asci_channel_1::state_add(device_state_interface &parent)
{
	parent.state_add(Z180_CNTLA1, "CNTLA1",  m_asci_cntla);
	parent.state_add(Z180_CNTLB1, "CNTLB1",  m_asci_cntlb);
	parent.state_add(Z180_STAT1,  "STAT1",   m_asci_stat);
	parent.state_add(Z180_TDR1,   "TDR1",    m_asci_tdr);
	parent.state_add(Z180_RDR1,   "RDR1",    m_asci_rdr);
	if (m_ext)
	{
		parent.state_add(Z180_ASEXT1, "ASEXT1",  m_asci_ext).mask(Z180_ASEXT1_MASK);
		parent.state_add(Z180_ASTC1,  "ASTC1",   m_asci_tc.w);
	}
}

void z180asci_channel_1::stat_w(uint8_t data)
{
	LOG("Z180 STAT1  wr $%02x ($%02x)\n", data,  data & (Z180_STAT1_RIE | Z180_STAT1_CTS1E | Z180_STAT1_TIE));
	m_asci_stat = (m_asci_stat & ~(Z180_STAT1_RIE | Z180_STAT1_CTS1E | Z180_STAT1_TIE)) | (data & (Z180_STAT1_RIE | Z180_STAT1_CTS1E | Z180_STAT1_TIE));
}

void z180asci_channel_1::asext_w(uint8_t data)
{
	LOG("Z180 ASEXT1 wr $%02x ($%02x)\n", data,  data & Z180_ASEXT1_MASK & ~Z180_ASEXT1_BRK_DET);
	m_asci_ext = (m_asci_ext & Z180_ASEXT1_BRK_DET) | (data & Z180_ASEXT1_MASK & ~Z180_ASEXT1_BRK_DET);
}

//**************************************************************************
//  z180asci_ext_channel_0
//**************************************************************************

z180asci_ext_channel_0::z180asci_ext_channel_0(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180asci_channel_0(mconfig, Z180ASCI_EXT_CHANNEL_0, tag, owner, clock, true)
{
}

//**************************************************************************
//  z180asci_channel_1
//**************************************************************************

z180asci_ext_channel_1::z180asci_ext_channel_1(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180asci_channel_1(mconfig, Z180ASCI_EXT_CHANNEL_1, tag, owner, clock, true)
{
}

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(Z180ASCI_CHANNEL_0,   z180asci_channel_0,  "z180asci_channel_0",    "Z180 ASCI Channel 0")
DEFINE_DEVICE_TYPE(Z180ASCI_CHANNEL_1,   z180asci_channel_1,  "z180asci_channel_1",    "Z180 ASCI Channel 1")

DEFINE_DEVICE_TYPE(Z180ASCI_EXT_CHANNEL_0,   z180asci_ext_channel_0,  "z180asci_ext_channel_0",    "Z180 ASCI Extended Channel 0")
DEFINE_DEVICE_TYPE(Z180ASCI_EXT_CHANNEL_1,   z180asci_ext_channel_1,  "z180asci_ext_channel_1",    "Z180 ASCI Extended Channel 1")
