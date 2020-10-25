// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - c16 function ROM test fails
    - clean up TED
    - verify PLA
    - T6721 speech chip

*/

#include "emu.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"
#include "bus/cbmiec/cbmiec.h"
#include "bus/pet/c2n.h"
#include "bus/pet/cass.h"
#include "bus/pet/diag264_lb_tape.h"
#include "bus/plus4/exp.h"
#include "bus/plus4/user.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m7501.h"
#include "imagedev/snapquik.h"
#include "machine/cbm_snqk.h"
#include "machine/input_merger.h"
#include "machine/mos6529.h"
#include "machine/mos6551.h"
#include "machine/mos8706.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "sound/mos7360.h"
#include "sound/t6721a.h"

#define MOS7360_TAG         "u1"
#define MOS6551_TAG         "u3"
#define MOS6529_USER_TAG    "u5"
#define MOS6529_KB_TAG      "u27"
#define T6721A_TAG          "t6721a"
#define MOS8706_TAG         "mos8706"
#define PLA_TAG             "u19"
#define SCREEN_TAG          "screen"
#define CONTROL1_TAG        "joy1"
#define CONTROL2_TAG        "joy2"
#define PET_USER_PORT_TAG   "user"

class plus4_state : public driver_device
{
public:
	plus4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "u2"),
		m_pla(*this, PLA_TAG),
		m_ted(*this, MOS7360_TAG),
		m_acia(*this, MOS6551_TAG),
		m_spi_user(*this, MOS6529_USER_TAG),
		m_spi_kb(*this, MOS6529_KB_TAG),
		m_vslsi(*this, MOS8706_TAG),
		m_iec(*this, CBM_IEC_TAG),
		m_joy1(*this, CONTROL1_TAG),
		m_joy2(*this, CONTROL2_TAG),
		m_exp(*this, PLUS4_EXPANSION_SLOT_TAG),
		m_user(*this, PET_USER_PORT_TAG),
		m_ram(*this, RAM_TAG),
		m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		m_kernal(*this, "kernal"),
		m_function(*this, "function"),
		m_c2(*this, "c2"),
		m_row(*this, "ROW%u", 0),
		m_lock(*this, "LOCK"),
		m_addr(0)
	{ }

	void plus4(machine_config &config);

	void cpu_w(uint8_t data);

protected:
	required_device<m7501_device> m_maincpu;
	required_device<pla_device> m_pla;
	required_device<mos7360_device> m_ted;
	optional_device<mos6551_device> m_acia;
	optional_device<mos6529_device> m_spi_user;
	required_device<mos6529_device> m_spi_kb;
	optional_device<mos8706_device> m_vslsi;
	required_device<cbm_iec_device> m_iec;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<plus4_expansion_slot_device> m_exp;
	optional_device<pet_user_port_device> m_user;
	required_device<ram_device> m_ram;
	required_device<pet_datassette_port_device> m_cassette;
	required_memory_region m_kernal;
	optional_memory_region m_function;
	optional_memory_region m_c2;
	required_ioport_array<8> m_row;
	required_ioport m_lock;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void bankswitch(offs_t offset, int phi0, int mux, int ras, int *scs, int *phi2, int *user, int *_6551, int *addr_clk, int *keyport, int *kernal);
	uint8_t read_memory(offs_t offset, int ba, int scs, int phi2, int user, int _6551, int addr_clk, int keyport, int kernal);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t ted_videoram_r(offs_t offset);

	uint8_t cpu_r();

	uint8_t ted_k_r(offs_t offset);

	DECLARE_WRITE_LINE_MEMBER( write_kb0 ) { if (state) m_kb |= 1; else m_kb &= ~1; }
	DECLARE_WRITE_LINE_MEMBER( write_kb1 ) { if (state) m_kb |= 2; else m_kb &= ~2; }
	DECLARE_WRITE_LINE_MEMBER( write_kb2 ) { if (state) m_kb |= 4; else m_kb &= ~4; }
	DECLARE_WRITE_LINE_MEMBER( write_kb3 ) { if (state) m_kb |= 8; else m_kb &= ~8; }
	DECLARE_WRITE_LINE_MEMBER( write_kb4 ) { if (state) m_kb |= 16; else m_kb &= ~16; }
	DECLARE_WRITE_LINE_MEMBER( write_kb5 ) { if (state) m_kb |= 32; else m_kb &= ~32; }
	DECLARE_WRITE_LINE_MEMBER( write_kb6 ) { if (state) m_kb |= 64; else m_kb &= ~64; }
	DECLARE_WRITE_LINE_MEMBER( write_kb7 ) { if (state) m_kb |= 128; else m_kb &= ~128; }

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_c16);

	enum
	{
		CS0_BASIC = 0,
		CS0_FUNCTION_LO,
		CS0_C1_LOW,
		CS0_C2_LOW
	};

	enum
	{
		CS1_KERNAL = 0,
		CS1_FUNCTION_HI,
		CS1_C1_HIGH,
		CS1_C2_HIGH
	};

	// memory state
	uint8_t m_addr;

	// keyboard state
	uint8_t m_kb;

	void plus4_mem(address_map &map);
	void ted_videoram_map(address_map &map);
};


class c16_state : public plus4_state
{
public:
	c16_state(const machine_config &mconfig, device_type type, const char *tag)
		: plus4_state(mconfig, type, tag)
	{ }

	void v364(machine_config &config);
	void c16n(machine_config &config);
	void c16p(machine_config &config);
	void c232(machine_config &config);
	void plus4p(machine_config &config);
	void plus4n(machine_config &config);

private:
	uint8_t cpu_r();
};



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define BA15 BIT(offset, 15)
#define BA14 BIT(offset, 14)
#define BA13 BIT(offset, 13)
#define BA12 BIT(offset, 12)
#define BA11 BIT(offset, 11)
#define BA10 BIT(offset, 10)
#define BA9 BIT(offset, 9)
#define BA8 BIT(offset, 8)
#define BA7 BIT(offset, 7)
#define BA6 BIT(offset, 6)
#define BA5 BIT(offset, 5)
#define BA4 BIT(offset, 4)



QUICKLOAD_LOAD_MEMBER(plus4_state::quickload_c16)
{
	return general_cbm_loadsnap(image, file_type, quickload_size, m_maincpu->space(AS_PROGRAM), 0, cbm_quick_sethiaddress);
}

//**************************************************************************
//  MEMORY MANAGEMENT
//**************************************************************************

void plus4_state::bankswitch(offs_t offset, int phi0, int mux, int ras, int *scs, int *phi2, int *user, int *_6551, int *addr_clk, int *keyport, int *kernal)
{
	uint16_t i = ras << 15 | BA10 << 14 | BA11 << 13 | BA13 << 12 | BA9 << 11 | BA8 << 10 | BA14 << 9 | mux << 8 | BA12 << 7 | BA7 << 6 | BA6 << 5 | BA5 << 4 | BA4 << 3 | BA15 << 2 | phi0 << 1 | 1;
/*  uint8_t data = m_pla->read(i);

    *scs = BIT(data, 0);
    *phi2 = BIT(data, 1);
    *user = BIT(data, 2);
    *_6551 = BIT(data, 3);
    *addr_clk = BIT(data, 4);
    *keyport = BIT(data, 5);
    *kernal = BIT(data, 6);
    *f7 = BIT(data, 7);*/

	// the following code is on loan from http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/plus4/pla.c until we get the PLA dumped

	#define I(b) (!!((i) & (1 << b)))

	#define I0_F7   I(0)
	#define PHI0    I(1)
	#define A15 I(2)
	#define A4  I(3)
	#define A5  I(4)
	#define A6  I(5)
	#define A7  I(6)
	#define A12 I(7)
	#define MUX I(8)
	#define A14 I(9)
	#define A8  I(10)
	#define A9  I(11)
	#define A13 I(12)
	#define A11 I(13)
	#define A10 I(14)
	#define RAS_    I(15)

	/* unused_  0 when 0111 011x 1001 011x */
	#define F0  RAS_ || !A10 || !A11 || !A13 || A9 || !A8 || !A14 ||    \
			!A12 || A7 || A6 || !A5 || A4 || !A15 || !PHI0
	/* PHI2     1 when 0xxx xxxx xxxx xx11 */
	#define F1  !RAS_ && PHI0 && I0_F7
	/* USER_    0 when 0111 011x 1000 1111 */
	#define F2  RAS_ || !A10 || !A11 || !A13 || A9 || !A8 || !A14 ||     \
			!A12 || A7 || A6 || A5 || !A4 || !A15 || !PHI0 || !I0_F7
	/* 6551_    0 when x111 011x 1000 011x */
	#define F3  !A10 || !A11 || !A13 || A9 || !A8 || !A14 ||    \
			!A12 || A7 || A6 || A5 || A4 || !A15 || !PHI0
	/* ADDR_CLK 0 when 1111 011x 1110 1111 */
	#define F4  RAS_ || !A10 || !A11 || !A13 || A9 || !A8 || !A14 ||    \
			!A12 || !A7 || !A6 || A5 || !A4 || !A15 || !PHI0 || !I0_F7
	/* KEYPORT_ 0 when 0111 011x 1001 1111 */
	#define F5  RAS_ || !A10 || !A11 || !A13 || A9 || !A8 || !A14 ||    \
			!A12 || A7 || A6 || !A5 || !A4 || !A15 || !PHI0 || !I0_F7
	/* KERNAL_  1 when x111 001x 1xxx x1xx */
	#define F6  A10 && A11 && A13 && !A9 && !A8 && A14 &&   \
			A12 && A15
	/* I0_F7    1 when xxxx xxx1 xxxx xxxx or
	          when 0xxx xxxx xxxx xx11 */
	#define F7  MUX || (F1)

	*scs = F0;
	*phi2 = F1;
	*user = F2;
	*_6551 = F3;
	*addr_clk = F4;
	*keyport = F5;
	*kernal = F6;
}


//-------------------------------------------------
//  read_memory -
//-------------------------------------------------

uint8_t plus4_state::read_memory(offs_t offset, int ba, int scs, int phi2, int user, int _6551, int addr_clk, int keyport, int kernal)
{
	int cs0 = 1, cs1 = 1, c1l = 1, c1h = 1, c2l = 1, c2h = 1;
	uint8_t data = m_ted->read(offset, cs0, cs1);

	//logerror("offset %04x user %u 6551 %u addr_clk %u keyport %u kernal %u cs0 %u cs1 %u\n", offset,user,_6551,addr_clk,keyport,kernal,cs0,cs1);

	if (!scs && m_vslsi)
	{
		data = m_vslsi->read(offset & 0x03);
	}
	else if (!user)
	{
		if (m_spi_user)
		{
			data = m_spi_user->read();
		}

		data &= ~0x04;
		data |= m_cassette->sense_r() << 2;
	}
	else if (!_6551 && m_acia)
	{
		data = m_acia->read(offset & 0x03);
	}
	else if (!keyport)
	{
		data = m_spi_kb->read();
	}
	else if (!cs0)
	{
		switch (m_addr & 0x03)
		{
		case CS0_BASIC:
			data = m_kernal->base()[offset & 0x7fff];
			break;

		case CS0_FUNCTION_LO:
			if (m_function != nullptr)
			{
				data = m_function->base()[offset & 0x7fff];
			}
			break;

		case CS0_C1_LOW:
			c1l = 0;
			break;

		case CS0_C2_LOW:
			c2l = 0;

			if (m_c2 != nullptr)
			{
				data = m_c2->base()[offset & 0x7fff];
			}
			break;
		}
	}
	else if (!cs1)
	{
		if (kernal)
		{
			data = m_kernal->base()[offset & 0x7fff];
		}
		else
		{
			switch ((m_addr >> 2) & 0x03)
			{
			case CS1_KERNAL:
				data = m_kernal->base()[offset & 0x7fff];
				break;

			case CS1_FUNCTION_HI:
				if (m_function != nullptr)
				{
					data = m_function->base()[offset & 0x7fff];
				}
				break;

			case CS1_C1_HIGH:
				c1h = 0;
				break;

			case CS1_C2_HIGH:
				c2h = 0;

				if (m_c2 != nullptr)
				{
					data = m_c2->base()[offset & 0x7fff];
				}
				break;
			}
		}
	}
	else if (offset < 0xfd00 || offset >= 0xff20)
	{
		data = m_ram->pointer()[offset & m_ram->mask()];
	}

	return m_exp->cd_r(offset, data, ba, cs0, c1l, c1h, cs1, c2l, c2h);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t plus4_state::read(offs_t offset)
{
	int phi0 = 1, mux = 0, ras = 0, ba = 1;
	int scs, phi2, user, _6551, addr_clk, keyport, kernal;

	bankswitch(offset, phi0, mux, ras, &scs, &phi2, &user, &_6551, &addr_clk, &keyport, &kernal);

	return read_memory(offset, ba, scs, phi2, user, _6551, addr_clk, keyport, kernal);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void plus4_state::write(offs_t offset, uint8_t data)
{
	int scs, phi2, user, _6551, addr_clk, keyport, kernal;
	int phi0 = 1, mux = 0, ras = 0, ba = 1;
	int cs0 = 1, cs1 = 1, c1l = 1, c1h = 1, c2l = 1, c2h = 1;

	bankswitch(offset, phi0, mux, ras, &scs, &phi2, &user, &_6551, &addr_clk, &keyport, &kernal);

	m_ted->write(offset, data, cs0, cs1);

	//logerror("write offset %04x data %02x user %u 6551 %u addr_clk %u keyport %u kernal %u cs0 %u cs1 %u\n", offset,data,user,_6551,addr_clk,keyport,kernal,cs0,cs1);

	if (!scs && m_vslsi)
	{
		m_vslsi->write(offset & 0x03, data);
	}
	else if (!user && m_spi_user)
	{
		m_spi_user->write(data);
	}
	else if (!_6551 && m_acia)
	{
		m_acia->write(offset & 0x03, data);
	}
	else if (!addr_clk)
	{
		m_addr = offset & 0x0f;
	}
	else if (!keyport)
	{
		m_spi_kb->write(data);
	}
	else if (offset < 0xfd00 || offset >= 0xff20)
	{
		m_ram->pointer()[offset & m_ram->mask()] = data;
	}

	m_exp->cd_w(offset, data, ba, cs0, c1l, c1h, cs1, c2l, c2h);
}


//-------------------------------------------------
//  ted_videoram_r -
//-------------------------------------------------

uint8_t plus4_state::ted_videoram_r(offs_t offset)
{
	int phi0 = 1, mux = 0, ras = 1, ba = 0;
	int scs, phi2, user, _6551, addr_clk, keyport, kernal;

	bankswitch(offset, phi0, mux, ras, &scs, &phi2, &user, &_6551, &addr_clk, &keyport, &kernal);

	return read_memory(offset, ba, scs, phi2, user, _6551, addr_clk, keyport, kernal);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( plus4_mem )
//-------------------------------------------------

void plus4_state::plus4_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(plus4_state::read), FUNC(plus4_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( ted_videoram_map )
//-------------------------------------------------

void plus4_state::ted_videoram_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(plus4_state::ted_videoram_r));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( plus4 )
//-------------------------------------------------

static INPUT_PORTS_START( plus4 )
	PORT_START( "ROW0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE)              PORT_CHAR('@')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)                                    PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)                                    PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)                                    PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HELP F7") PORT_CODE(KEYCODE_F4)               PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                            PORT_CHAR(0xA3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)             PORT_CHAR(13)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INST DEL") PORT_CODE(KEYCODE_BACKSPACE)       PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START( "ROW1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left & Right)") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')

	PORT_START( "ROW2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)         PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_CHAR('R')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)         PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0  \xE2\x86\x91") PORT_CODE(KEYCODE_0)        PORT_CHAR('0') PORT_CHAR('^')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)         PORT_CHAR('J')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)         PORT_CHAR('I')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                                PORT_CHAR('-')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)                                    PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)         PORT_CHAR('P')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)                                  PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                             PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("=  Pi  \xE2\x86\x90") PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR('=') PORT_CHAR(0x03C0) PORT_CHAR(0x2190)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)                               PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)                             PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT)                            PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)                              PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN STOP") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBM") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Control") PORT_CODE(KEYCODE_TAB)          PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Home Clear") PORT_CODE(KEYCODE_DEL)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1') PORT_CHAR('!')

	PORT_START( "LOCK" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c16 )
//-------------------------------------------------

static INPUT_PORTS_START( c16 )
	PORT_INCLUDE( plus4 )

	PORT_MODIFY( "ROW0" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT)                                PORT_CHAR(0xA3)

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)                  PORT_CHAR('-')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)                                    PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP)                                  PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                            PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("=  Pi  \xE2\x86\x90") PORT_CODE(KEYCODE_PGDN) PORT_CHAR('=') PORT_CHAR(0x03C0) PORT_CHAR(0x2190)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                                PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                             PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                                 PORT_CHAR(UCHAR_MAMEKEY(LEFT))
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  M6510_INTERFACE( cpu_intf )
//-------------------------------------------------

uint8_t plus4_state::cpu_r()
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       CST RD
	    5
	    6       IEC CLK IN
	    7       IEC DATA IN, CST SENSE

	*/

	uint8_t data = 0x2f;

	// cassette read
	data |= m_cassette->read() << 4;

	// serial clock
	data |= m_iec->clk_r() << 6;

	// serial data, cassette sense
	data |= (m_iec->data_r() && m_cassette->sense_r()) << 7;

	return data;
}

uint8_t c16_state::cpu_r()
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       CST RD
	    5
	    6       IEC CLK IN
	    7       IEC DATA IN

	*/

	uint8_t data = 0;

	// cassette read
	data |= m_cassette->read() << 4;

	// serial clock
	data |= m_iec->clk_r() << 6;

	// serial data
	data |= m_iec->data_r() << 7;

	return data;
}

void plus4_state::cpu_w(uint8_t data)
{
	/*

	    bit     description

	    0       IEC DATA
	    1       IEC CLK, CST WR
	    2       IEC ATN
	    3       CST MTR
	    4
	    5
	    6       (CST WR)
	    7

	*/

	//logerror("%s cpu write %02x\n", machine().describe_context(), data);

	// serial data
	m_iec->host_data_w(!BIT(data, 0));

	// serial clock
	m_iec->host_clk_w(!BIT(data, 1));

	// serial attention
	m_iec->host_atn_w(!BIT(data, 2));

	// cassette motor
	m_cassette->motor_w(BIT(data, 3));

	// cassette write
	m_cassette->write(!BIT(data, 1));
}


//-------------------------------------------------
//  ted7360_interface ted_intf
//-------------------------------------------------

uint8_t plus4_state::ted_k_r(offs_t offset)
{
	/*

	    bit     description

	    0       JOY A0, JOY B0
	    1       JOY A1, JOY B1
	    2       JOY A2, JOY B2
	    3       JOY A3, JOY B3
	    4
	    5
	    6       BTN A
	    7       BTN B

	*/

	uint8_t data = 0xff;

	// joystick
	if (!BIT(offset, 2))
	{
		uint8_t joy_a = m_joy1->read_joy();

		data &= (0xf0 | (joy_a & 0x0f));
		data &= ~(!BIT(joy_a, 5) << 6);
	}

	if (!BIT(offset, 1))
	{
		uint8_t joy_b = m_joy2->read_joy();

		data &= (0xf0 | (joy_b & 0x0f));
		data &= ~(!BIT(joy_b, 5) << 7);
	}

	// keyboard
	if (!BIT(m_kb, 7)) data &= m_row[7]->read();
	if (!BIT(m_kb, 6)) data &= m_row[6]->read();
	if (!BIT(m_kb, 5)) data &= m_row[5]->read();
	if (!BIT(m_kb, 4)) data &= m_row[4]->read();
	if (!BIT(m_kb, 3)) data &= m_row[3]->read();
	if (!BIT(m_kb, 2)) data &= m_row[2]->read();
	if (!BIT(m_kb, 1)) data &= m_row[1]->read() & m_lock->read();
	if (!BIT(m_kb, 0)) data &= m_row[0]->read();

	return data;
}



//-------------------------------------------------
//  SLOT_INTERFACE( cbm_datassette_devices )
//-------------------------------------------------

void plus4_datassette_devices(device_slot_interface &device)
{
	device.option_add("c1531", C1531);
	device.option_add("diag264", DIAG264_CASSETTE_LOOPBACK);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( plus4 )
//-------------------------------------------------

void plus4_state::machine_start()
{
	// initialize memory
	uint8_t data = 0xff;

	for (offs_t offset = 0; offset < m_ram->size(); offset++)
	{
		m_ram->pointer()[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	// state saving
	save_item(NAME(m_addr));
	save_item(NAME(m_kb));

	if (m_acia)
	{
		m_acia->write_cts(0);
	}

	m_spi_kb->write_p0(1);
	m_spi_kb->write_p1(1);
	m_spi_kb->write_p2(1);
	m_spi_kb->write_p3(1);
	m_spi_kb->write_p4(1);
	m_spi_kb->write_p5(1);
	m_spi_kb->write_p6(1);
	m_spi_kb->write_p7(1);
}


void plus4_state::machine_reset()
{
	m_maincpu->reset();

	m_iec->reset();

	if (m_acia)
	{
		m_acia->reset();
	}

	m_exp->reset();

	if (m_user)
	{
		m_user->write_3(0);
		m_user->write_3(1);
	}

	m_addr = 0;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( plus4 )
//-------------------------------------------------

void plus4_state::plus4(machine_config &config)
{
	// basic machine hardware
	M7501(config, m_maincpu, 0);
	m_maincpu->set_addrmap(AS_PROGRAM, &plus4_state::plus4_mem);
	m_maincpu->read_callback().set(FUNC(plus4_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(plus4_state::cpu_w));
	m_maincpu->set_pulls(0x00, 0xc0);
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m7501_device::IRQ_LINE);

	// video and sound hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(mos7360_device::PAL_VRETRACERATE);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(336, 216);
	screen.set_visarea(0, 336 - 1, 0, 216 - 1);
	screen.set_screen_update(MOS7360_TAG, FUNC(mos7360_device::screen_update));

	SPEAKER(config, "mono").front_center();

	MOS7360(config, m_ted, 0);
	m_ted->set_addrmap(0, &plus4_state::ted_videoram_map);
	m_ted->set_screen(SCREEN_TAG);
	m_ted->write_irq_callback().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_ted->read_k_callback().set(FUNC(plus4_state::ted_k_r));
	m_ted->add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	PLS100(config, m_pla);

	PET_USER_PORT(config, m_user, plus4_user_port_cards, nullptr);
	m_user->p4_handler().set(m_spi_user, FUNC(mos6529_device::write_p2)); // cassette sense
	m_user->p5_handler().set(m_spi_user, FUNC(mos6529_device::write_p3));
	m_user->p6_handler().set(m_spi_user, FUNC(mos6529_device::write_p4));
	m_user->p7_handler().set(m_spi_user, FUNC(mos6529_device::write_p5));
	m_user->p8_handler().set(m_acia, FUNC(mos6551_device::write_rxc));
	m_user->pb_handler().set(m_spi_user, FUNC(mos6529_device::write_p0));
	m_user->pc_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	m_user->pf_handler().set(m_spi_user, FUNC(mos6529_device::write_p7));
	m_user->ph_handler().set(m_acia, FUNC(mos6551_device::write_dcd)).invert(); // TODO: add missing pull up before inverter
	m_user->pj_handler().set(m_spi_user, FUNC(mos6529_device::write_p6));
	m_user->pk_handler().set(m_spi_user, FUNC(mos6529_device::write_p1));
	m_user->pl_handler().set(m_acia, FUNC(mos6551_device::write_dsr)).invert(); // TODO: add missing pull up before inverter

	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->rxc_handler().set(m_user, FUNC(pet_user_port_device::write_8));
	m_acia->rts_handler().set(m_user, FUNC(pet_user_port_device::write_d)).invert();
	m_acia->dtr_handler().set(m_user, FUNC(pet_user_port_device::write_e)).invert();
	m_acia->txd_handler().set(m_user, FUNC(pet_user_port_device::write_m));
	m_acia->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	MOS6529(config, m_spi_user, 0);
	m_spi_user->p_handler<0>().set(m_user, FUNC(pet_user_port_device::write_b));
	m_spi_user->p_handler<1>().set(m_user, FUNC(pet_user_port_device::write_k));
	m_spi_user->p_handler<2>().set(m_user, FUNC(pet_user_port_device::write_4));
	m_spi_user->p_handler<3>().set(m_user, FUNC(pet_user_port_device::write_5));
	m_spi_user->p_handler<4>().set(m_user, FUNC(pet_user_port_device::write_6));
	m_spi_user->p_handler<5>().set(m_user, FUNC(pet_user_port_device::write_7));
	m_spi_user->p_handler<6>().set(m_user, FUNC(pet_user_port_device::write_j));
	m_spi_user->p_handler<7>().set(m_user, FUNC(pet_user_port_device::write_f));

	MOS6529(config, m_spi_kb, 0);
	m_spi_kb->p_handler<0>().set(FUNC(plus4_state::write_kb0));
	m_spi_kb->p_handler<1>().set(FUNC(plus4_state::write_kb1));
	m_spi_kb->p_handler<2>().set(FUNC(plus4_state::write_kb2));
	m_spi_kb->p_handler<3>().set(FUNC(plus4_state::write_kb3));
	m_spi_kb->p_handler<4>().set(FUNC(plus4_state::write_kb4));
	m_spi_kb->p_handler<5>().set(FUNC(plus4_state::write_kb5));
	m_spi_kb->p_handler<6>().set(FUNC(plus4_state::write_kb6));
	m_spi_kb->p_handler<7>().set(FUNC(plus4_state::write_kb7));

	PET_DATASSETTE_PORT(config, m_cassette, plus4_datassette_devices, "c1531");
	m_cassette->read_handler().set_nop();

	cbm_iec_slot_device::add(config, m_iec, "c1541");
	m_iec->atn_callback().set(m_user, FUNC(pet_user_port_device::write_9));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, nullptr);
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");

	PLUS4_EXPANSION_SLOT(config, m_exp, XTAL(14'318'181)/16, plus4_expansion_cards, nullptr);
	m_exp->irq_wr_callback().set("mainirq", FUNC(input_merger_device::in_w<2>));
	m_exp->cd_rd_callback().set(FUNC(plus4_state::read));
	m_exp->cd_wr_callback().set(FUNC(plus4_state::write));
	m_exp->aec_wr_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);

	QUICKLOAD(config, "quickload", "p00,prg", CBM_QUICKLOAD_DELAY).set_load_callback(FUNC(plus4_state::quickload_c16));

	// internal ram
	RAM(config, m_ram).set_default_size("64K");
}


//-------------------------------------------------
//  machine_config( plus4p )
//-------------------------------------------------

void c16_state::plus4p(machine_config &config)
{
	plus4(config);
	m_maincpu->set_clock(XTAL(17'734'470)/20);
	m_ted->set_clock(XTAL(17'734'470)/5);

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("plus4_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("plus4_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("plus4_flop");
	subdevice<software_list_device>("cart_list")->set_filter("PAL");
	subdevice<software_list_device>("cass_list")->set_filter("PAL");
	subdevice<software_list_device>("flop_list")->set_filter("PAL");
}

//-------------------------------------------------
//  machine_config( plus4n )
//-------------------------------------------------

void c16_state::plus4n(machine_config &config)
{
	plus4(config);
	m_maincpu->set_clock(XTAL(14'318'181)/16);
	m_ted->set_clock(XTAL(14'318'181)/4);

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("plus4_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("plus4_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("plus4_flop");
	subdevice<software_list_device>("cart_list")->set_filter("NTSC");
	subdevice<software_list_device>("cass_list")->set_filter("NTSC");
	subdevice<software_list_device>("flop_list")->set_filter("NTSC");
}


//-------------------------------------------------
//  machine_config( c16n )
//-------------------------------------------------

void c16_state::c16n(machine_config &config)
{
	plus4n(config);
	m_maincpu->read_callback().set(FUNC(c16_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(plus4_state::cpu_w));
	m_maincpu->set_pulls(0x00, 0xc0);

	config.device_remove(MOS6551_TAG);
	config.device_remove(MOS6529_USER_TAG);
	config.device_remove(PET_USER_PORT_TAG);

	m_iec->atn_callback().set_nop();

	m_ram->set_default_size("16K").set_extra_options("64K");
}


//-------------------------------------------------
//  machine_config( c16p )
//-------------------------------------------------

void c16_state::c16p(machine_config &config)
{
	plus4p(config);
	m_maincpu->read_callback().set(FUNC(c16_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(plus4_state::cpu_w));
	m_maincpu->set_pulls(0x00, 0xc0);

	config.device_remove(MOS6551_TAG);
	config.device_remove(MOS6529_USER_TAG);
	config.device_remove(PET_USER_PORT_TAG);

	m_iec->atn_callback().set_nop();

	m_ram->set_default_size("16K").set_extra_options("64K");
}


void c16_state::c232(machine_config &config)
{
	c16p(config);
	m_ram->set_default_size("32K");
}


//-------------------------------------------------
//  machine_config( v364 )
//-------------------------------------------------

void c16_state::v364(machine_config &config)
{
	plus4n(config);
	T6721A(config, T6721A_TAG, XTAL(640'000)).add_route(ALL_OUTPUTS, "mono", 0.25);

	MOS8706(config, m_vslsi, XTAL(14'318'181)/16);
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( c264 )
//-------------------------------------------------

ROM_START( c264 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "basic-264.bin", 0x0000, 0x4000, CRC(6a2fc8e3) SHA1(473fce23afa07000cdca899fbcffd6961b36a8a0) )
	ROM_LOAD( "kernal-264.bin", 0x4000, 0x4000, CRC(8f32abe7) SHA1(d481faf5fcbb331878dc7851c642d04f26a32873) )

	ROM_REGION( 0x8000, "function", ROMREGION_ERASE00 )
	// TODO: add cart slots to mount EPROMs here

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02", 0x00, 0xf5, CRC(83be2076) SHA1(a89b18b2261233443c933c8b4663b108e7630924) )
ROM_END


//-------------------------------------------------
//  ROM( c232 )
//-------------------------------------------------

ROM_START( c232 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01.u4", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )
	ROM_LOAD( "318004-01.u5", 0x4000, 0x4000, CRC(dbdc3319) SHA1(3c77caf72914c1c0a0875b3a7f6935cd30c54201) )

	ROM_REGION( 0x8000, "function", ROMREGION_ERASE00 )
	// TODO: add cart slots to mount EPROMs here

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u7", 0x00, 0xf5, CRC(83be2076) SHA1(a89b18b2261233443c933c8b4663b108e7630924) )
ROM_END


//-------------------------------------------------
//  ROM( v364 )
//-------------------------------------------------

ROM_START( v364 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )
	ROM_LOAD( "kern364p", 0x4000, 0x4000, CRC(84fd4f7a) SHA1(b9a5b5dacd57ca117ef0b3af29e91998bf4d7e5f) )

	ROM_REGION( 0x8000, "function", 0 )
	ROM_LOAD( "317053-01", 0x0000, 0x4000, CRC(4fd1d8cb) SHA1(3b69f6e7cb4c18bb08e203fb18b7dabfa853390f) )
	ROM_LOAD( "317054-01", 0x4000, 0x4000, CRC(109de2fc) SHA1(0ad7ac2db7da692d972e586ca0dfd747d82c7693) )

	ROM_REGION( 0x8000, "c2", 0 )
	ROM_LOAD( "spk3cc4.bin", 0x0000, 0x4000, CRC(5227c2ee) SHA1(59af401cbb2194f689898271c6e8aafa28a7af11) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02", 0x00, 0xf5, CRC(83be2076) SHA1(a89b18b2261233443c933c8b4663b108e7630924) )
ROM_END


//-------------------------------------------------
//  ROM( plus4 )
//-------------------------------------------------

ROM_START( plus4 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r4", "Revision 4" )
	ROMX_LOAD( "318005-04.u24", 0x4000, 0x4000, CRC(799a633d) SHA1(5df52c693387c0e2b5d682613a3b5a65477311cf), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r5", "Revision 5" )
	ROMX_LOAD( "318005-05.u24", 0x4000, 0x4000, CRC(70295038) SHA1(a3d9e5be091b98de39a046ab167fb7632d053682), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos plus4.u24", 0x0000, 0x8000, CRC(818d3f45) SHA1(9bc1b1c3da9ca642deae717905f990d8e36e6c3b), ROM_BIOS(2) ) // first half contains R5 kernal

	ROM_LOAD( "318006-01.u23", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_REGION( 0x8000, "function", 0 )
	ROM_LOAD( "317053-01.u25", 0x0000, 0x4000, CRC(4fd1d8cb) SHA1(3b69f6e7cb4c18bb08e203fb18b7dabfa853390f) )
	ROM_LOAD( "317054-01.u26", 0x4000, 0x4000, CRC(109de2fc) SHA1(0ad7ac2db7da692d972e586ca0dfd747d82c7693) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u19", 0x00, 0xf5, CRC(83be2076) SHA1(a89b18b2261233443c933c8b4663b108e7630924) )
ROM_END


//-------------------------------------------------
//  ROM( plus4p )
//-------------------------------------------------

ROM_START( plus4p )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01.u23", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r3", "Revision 3" )
	ROMX_LOAD( "318004-03.u24", 0x4000, 0x4000, CRC(77bab934) SHA1(97814dab9d757fe5a3a61d357a9a81da588a9783), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318004-04.u24", 0x4000, 0x4000, CRC(be54ed79) SHA1(514ad3c29d01a2c0a3b143d9c1d4143b1912b793), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "r5", "Revision 5" )
	ROMX_LOAD( "318004-05.u24", 0x4000, 0x4000, CRC(71c07bd4) SHA1(7c7e07f016391174a557e790c4ef1cbe33512cdb), ROM_BIOS(2) )

	ROM_REGION( 0x8000, "function", 0 )
	ROM_LOAD( "317053-01.u25", 0x0000, 0x4000, CRC(4fd1d8cb) SHA1(3b69f6e7cb4c18bb08e203fb18b7dabfa853390f) )
	ROM_LOAD( "317054-01.u26", 0x4000, 0x4000, CRC(109de2fc) SHA1(0ad7ac2db7da692d972e586ca0dfd747d82c7693) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u19", 0x00, 0xf5, CRC(83be2076) SHA1(a89b18b2261233443c933c8b4663b108e7630924) )
ROM_END


//-------------------------------------------------
//  ROM( c16 )
//-------------------------------------------------

ROM_START( c16 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r4", "Revision 4" )
	ROMX_LOAD( "318005-04.u24", 0x4000, 0x4000, CRC(799a633d) SHA1(5df52c693387c0e2b5d682613a3b5a65477311cf), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r5", "Revision 5" )
	ROMX_LOAD( "318005-05.u24", 0x4000, 0x4000, CRC(70295038) SHA1(a3d9e5be091b98de39a046ab167fb7632d053682), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos plus4.u24", 0x0000, 0x8000, CRC(818d3f45) SHA1(9bc1b1c3da9ca642deae717905f990d8e36e6c3b), ROM_BIOS(2) ) // first half contains R5 kernal

	ROM_LOAD( "318006-01.u23", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u19", 0x00, 0xf5, CRC(83be2076) SHA1(a89b18b2261233443c933c8b4663b108e7630924) )
ROM_END


//-------------------------------------------------
//  ROM( c16p )
//-------------------------------------------------

ROM_START( c16p )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01.u3", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r3", "Revision 3" )
	ROMX_LOAD( "318004-03.u4", 0x4000, 0x4000, CRC(77bab934) SHA1(97814dab9d757fe5a3a61d357a9a81da588a9783), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318004-04.u4", 0x4000, 0x4000, CRC(be54ed79) SHA1(514ad3c29d01a2c0a3b143d9c1d4143b1912b793), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "r5", "Revision 5" )
	ROMX_LOAD( "318004-05.u4", 0x4000, 0x4000, CRC(71c07bd4) SHA1(7c7e07f016391174a557e790c4ef1cbe33512cdb), ROM_BIOS(2) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u16", 0x00, 0xf5, CRC(83be2076) SHA1(a89b18b2261233443c933c8b4663b108e7630924) )
ROM_END


//-------------------------------------------------
//  ROM( c16_hu )
//-------------------------------------------------

ROM_START( c16_hu )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01.u3", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "318030-01.u4", 0x4000, 0x4000, NO_DUMP, ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "318030-02.u4", 0x4000, 0x4000, CRC(775f60c5) SHA1(20cf3c4bf6c54ef09799af41887218933f2e27ee), ROM_BIOS(1) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u16", 0x00, 0xf5, CRC(83be2076) SHA1(a89b18b2261233443c933c8b4663b108e7630924) )
ROM_END


//-------------------------------------------------
//  ROM( c116 )
//-------------------------------------------------

ROM_START( c116 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01.u3", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r3", "Revision 3" )
	ROMX_LOAD( "318004-03.u4", 0x4000, 0x4000, CRC(77bab934) SHA1(97814dab9d757fe5a3a61d357a9a81da588a9783), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318004-04.u4", 0x4000, 0x4000, CRC(be54ed79) SHA1(514ad3c29d01a2c0a3b143d9c1d4143b1912b793), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "r5", "Revision 5" )
	ROMX_LOAD( "318004-05.u4", 0x4000, 0x4000, CRC(71c07bd4) SHA1(7c7e07f016391174a557e790c4ef1cbe33512cdb), ROM_BIOS(2) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u101", 0x00, 0xf5, CRC(83be2076) SHA1(a89b18b2261233443c933c8b4663b108e7630924) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                        FULLNAME                      FLAGS
COMP( 1984, c264,   0,      0,      plus4n,  plus4, c16_state, empty_init, "Commodore Business Machines", "Commodore 264 (Prototype)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1984, c232,   c264,   0,      c232,    plus4, c16_state, empty_init, "Commodore Business Machines", "Commodore 232 (Prototype)",  MACHINE_SUPPORTS_SAVE )
COMP( 1984, v364,   c264,   0,      v364,    plus4, c16_state, empty_init, "Commodore Business Machines", "Commodore V364 (Prototype)", MACHINE_SUPPORTS_SAVE )
COMP( 1984, plus4,  c264,   0,      plus4n,  plus4, c16_state, empty_init, "Commodore Business Machines", "Plus/4 (NTSC)",              MACHINE_SUPPORTS_SAVE )
COMP( 1984, plus4p, c264,   0,      plus4p,  plus4, c16_state, empty_init, "Commodore Business Machines", "Plus/4 (PAL)",               MACHINE_SUPPORTS_SAVE )
COMP( 1984, c16,    c264,   0,      c16n,    c16,   c16_state, empty_init, "Commodore Business Machines", "Commodore 16 (NTSC)",        MACHINE_SUPPORTS_SAVE )
COMP( 1984, c16p,   c264,   0,      c16p,    c16,   c16_state, empty_init, "Commodore Business Machines", "Commodore 16 (PAL)",         MACHINE_SUPPORTS_SAVE )
COMP( 1984, c16_hu, c264,   0,      c16p,    c16,   c16_state, empty_init, "Commodore Business Machines", "Commodore 16 (Hungary)",     MACHINE_SUPPORTS_SAVE )
COMP( 1984, c116,   c264,   0,      c16p,    c16,   c16_state, empty_init, "Commodore Business Machines", "Commodore 116",              MACHINE_SUPPORTS_SAVE )
