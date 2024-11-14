// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - 8088 board
    - CIA timers fail in burn-in test
    - cbm620hu charom banking?

*/

#include "emu.h"

#include "cbm_snqk.h"

#include "bus/cbm2/exp.h"
#include "bus/cbm2/user.h"
#include "bus/ieee488/ieee488.h"
#include "bus/pet/cass.h"
#include "bus/rs232/rs232.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m6509.h"
#include "cpu/i86/i86.h"
#include "imagedev/snapquik.h"
#include "machine/6525tpi.h"
#include "machine/ds75160a.h"
#include "machine/ds75161a.h"
#include "machine/input_merger.h"
#include "machine/mos6526.h"
#include "machine/mos6551.h"
#include "machine/pic8259.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "sound/mos6581.h"
#include "video/mc6845.h"
#include "video/mos6566.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "utf8.h"


namespace {

#define PLA1_TAG        "u78"
#define PLA2_TAG        "u88"
#define MOS6567_TAG     "u23"
#define MOS6569_TAG     "u23"
#define MC68B45_TAG     "u10"
#define MOS6581_TAG     "u4"
#define MOS6525_1_TAG   "u20"
#define MOS6525_2_TAG   "u102"
#define MOS6551A_TAG    "u19"
#define MOS6526_TAG     "u2"
#define DS75160A_TAG    "u3"
#define DS75161A_TAG    "u7"
#define SCREEN_TAG      "screen"
#define CONTROL1_TAG    "joy1"
#define CONTROL2_TAG    "joy2"
#define RS232_TAG       "rs232"
#define USER_PORT_TAG   "user"

#define EXT_I8088_TAG   "ext_u1"
#define EXT_I8087_TAG   "ext_u4"
#define EXT_I8259A_TAG  "ext_u3"
#define EXT_MOS6526_TAG "ext_u15"
#define EXT_MOS6525_TAG "ext_u16"

class cbm2_state : public driver_device
{
public:
	cbm2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "u13"),
		m_pla1(*this, PLA1_TAG),
		m_crtc(*this, MC68B45_TAG),
		m_palette(*this, "palette"),
		m_sid(*this, MOS6581_TAG),
		m_tpi1(*this, MOS6525_1_TAG),
		m_tpi2(*this, MOS6525_2_TAG),
		m_acia(*this, MOS6551A_TAG),
		m_cia(*this, MOS6526_TAG),
		m_ieee1(*this, DS75160A_TAG),
		m_ieee2(*this, DS75161A_TAG),
		m_joy1(*this, CONTROL1_TAG),
		m_joy2(*this, CONTROL2_TAG),
		m_exp(*this, "exp"),
		m_user(*this, USER_PORT_TAG),
		m_ram(*this, RAM_TAG),
		m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		m_ieee(*this, IEEE488_TAG),
		m_ext_cpu(*this, EXT_I8088_TAG),
		m_ext_pic(*this, EXT_I8259A_TAG),
		m_ext_cia(*this, EXT_MOS6526_TAG),
		m_ext_tpi(*this, EXT_MOS6525_TAG),
		m_basic(*this, "basic"),
		m_kernal(*this, "kernal"),
		m_charom(*this, "charom"),
		m_buffer_ram(*this, "buffer_ram", 0x800, ENDIANNESS_LITTLE),
		m_extbuf_ram(*this, "extbuf_ram", 0x800, ENDIANNESS_LITTLE),
		m_video_ram(*this, "video_ram", 0x800, ENDIANNESS_LITTLE),
		m_pa(*this, "PA%u", 0),
		m_pb(*this, "PB%u", 0),
		m_lock(*this, "LOCK"),
		m_dramon(1),
		m_video_ram_size(0x800),
		m_graphics(1),
		m_todclk(0),
		m_tpi2_pa(0),
		m_tpi2_pb(0)
	{ }

	required_device<m6509_device> m_maincpu;
	required_device<pla_device> m_pla1;
	optional_device<mc6845_device> m_crtc;
	optional_device<palette_device> m_palette;
	required_device<mos6581_device> m_sid;
	required_device<tpi6525_device> m_tpi1;
	required_device<tpi6525_device> m_tpi2;
	required_device<mos6551_device> m_acia;
	required_device<mos6526_device> m_cia;
	required_device<ds75160a_device> m_ieee1;
	required_device<ds75161a_device> m_ieee2;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<cbm2_expansion_slot_device> m_exp;
	required_device<cbm2_user_port_device> m_user;
	required_device<ram_device> m_ram;
	required_device<pet_datassette_port_device> m_cassette;
	required_device<ieee488_device> m_ieee;
	optional_device<cpu_device> m_ext_cpu;
	optional_device<pic8259_device> m_ext_pic;
	optional_device<mos6526_device> m_ext_cia;
	optional_device<tpi6525_device> m_ext_tpi;
	required_memory_region m_basic;
	required_memory_region m_kernal;
	required_memory_region m_charom;
	memory_share_creator<uint8_t> m_buffer_ram;
	memory_share_creator<uint8_t> m_extbuf_ram;
	memory_share_creator<uint8_t> m_video_ram;
	required_ioport_array<8> m_pa;
	required_ioport_array<8> m_pb;
	required_ioport m_lock;

	TIMER_CALLBACK_MEMBER( tod_tick );

	DECLARE_MACHINE_START( cbm2 );
	DECLARE_MACHINE_START( cbm2_ntsc );
	DECLARE_MACHINE_START( cbm2_pal );
	DECLARE_MACHINE_START( cbm2x_ntsc );
	DECLARE_MACHINE_START( cbm2x_pal );
	DECLARE_MACHINE_RESET( cbm2 );

	virtual void read_pla(offs_t offset, int ras, int cas, int refen, int eras, int ecas,
		int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *rasseg1, int *rasseg2, int *rasseg3, int *rasseg4);

	void bankswitch(offs_t offset, int eras, int ecas, int refen, int cas, int ras, int *sysioen, int *dramen,
		int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *buframcs, int *extbufcs, int *vidramcs,
		int *diskromcs, int *csbank1, int *csbank2, int *csbank3, int *basiccs, int *knbcs, int *kernalcs,
		int *crtccs, int *cs1, int *sidcs, int *extprtcs, int *ciacs, int *aciacs, int *tript1cs, int *tript2cs);

	uint8_t read_keyboard();
	void set_busy2(int state);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t ext_read(offs_t offset);
	void ext_write(offs_t offset, uint8_t data);

	uint8_t sid_potx_r();
	uint8_t sid_poty_r();

	uint8_t tpi1_pa_r();
	void tpi1_pa_w(uint8_t data);
	uint8_t tpi1_pb_r();
	void tpi1_pb_w(uint8_t data);
	void tpi1_ca_w(int state);
	void tpi1_cb_w(int state);

	void tpi2_pa_w(uint8_t data);
	void tpi2_pb_w(uint8_t data);
	uint8_t tpi2_pc_r();

	uint8_t cia_pa_r();
	void cia_pa_w(uint8_t data);
	uint8_t cia_pb_r();

	uint8_t ext_tpi_pb_r();
	void ext_tpi_pb_w(uint8_t data);
	void ext_tpi_pc_w(uint8_t data);

	void ext_cia_irq_w(int state);
	uint8_t ext_cia_pb_r();
	void ext_cia_pb_w(uint8_t data);

	MC6845_UPDATE_ROW( crtc_update_row );

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cbmb);
	// memory state
	int m_dramon;
	int m_busen1;
	int m_busy2;

	// video state
	size_t m_video_ram_size;
	int m_graphics;
	int m_ntsc;

	// interrupt state
	int m_todclk;

	// keyboard state;
	uint8_t m_tpi2_pa;
	uint8_t m_tpi2_pb;
	uint8_t m_cia_pa;

	uint8_t m_ext_cia_pb;
	uint8_t m_ext_tpi_pb;

	// timers
	emu_timer *m_todclk_timer;
	void _128k(machine_config &config);
	void _256k(machine_config &config);
	void cbm2lp_ntsc(machine_config &config);
	void cbm2lp_pal(machine_config &config);
	void cbm2hp_ntsc(machine_config &config);
	void cbm2hp_pal(machine_config &config);
	void cbm620(machine_config &config);
	void b128(machine_config &config);
	void b256(machine_config &config);
	void cbm610(machine_config &config);
	void cbm2_mem(address_map &map) ATTR_COLD;
	void ext_io(address_map &map) ATTR_COLD;
	void ext_mem(address_map &map) ATTR_COLD;
};


class cbm2hp_state : public cbm2_state
{
public:
	cbm2hp_state(const machine_config &mconfig, device_type type, const char *tag)
		: cbm2_state(mconfig, type, tag)
	{ }

	virtual void read_pla(offs_t offset, int ras, int cas, int refen, int eras, int ecas,
		int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *rasseg1, int *rasseg2, int *rasseg3, int *rasseg4) override;

	uint8_t tpi2_pc_r();
	void b256hp(machine_config &config);
	void b128hp(machine_config &config);
	void cbm710(machine_config &config);
	void cbm730(machine_config &config);
	void cbm720(machine_config &config);
	void bx256hp(machine_config &config);
};


class p500_state : public cbm2_state
{
public:
	p500_state(const machine_config &mconfig, device_type type, const char *tag)
		: cbm2_state(mconfig, type, tag),
			m_pla2(*this, PLA2_TAG),
			m_vic(*this, MOS6569_TAG),
			m_color_ram(*this, "color_ram", 0x400, ENDIANNESS_LITTLE),
			m_statvid(1),
			m_vicdotsel(1),
			m_vicbnksel(0x03)
	{ }

	required_device<pla_device> m_pla2;
	required_device<mos6566_device> m_vic;
	memory_share_creator<uint8_t> m_color_ram;

	DECLARE_MACHINE_START( p500 );
	DECLARE_MACHINE_START( p500_ntsc );
	DECLARE_MACHINE_START( p500_pal );
	DECLARE_MACHINE_RESET( p500 );

	void read_pla1(offs_t offset, int busy2, int clrnibcsb, int procvid, int refen, int ba, int aec, int srw,
		int *datxen, int *dramxen, int *clrniben, int *segf, int *_64kcasen, int *casenb, int *viddaten, int *viddat_tr);

	void read_pla2(offs_t offset, offs_t va, int ba, int vicen, int ae, int segf, int bank0,
		int *clrnibcsb, int *extbufcs, int *discromcs, int *buframcs, int *charomcs, int *procvid, int *viccs, int *vidmatcs);

	void bankswitch(offs_t offset, offs_t va, int srw, int ba, int ae, int busy2, int refen,
		int *datxen, int *dramxen, int *clrniben, int *_64kcasen, int *casenb, int *viddaten, int *viddat_tr,
		int *clrnibcs, int *extbufcs, int *discromcs, int *buframcs, int *charomcs, int *viccs, int *vidmatcs,
		int *csbank1, int *csbank2, int *csbank3, int *basiclocs, int *basichics, int *kernalcs,
		int *cs1, int *sidcs, int *extprtcs, int *ciacs, int *aciacs, int *tript1cs, int *tript2cs, int *aec, int *vsysaden);

	uint8_t read_memory(offs_t offset, offs_t va, int ba, int ae);
	void write_memory(offs_t offset, uint8_t data, int ba, int ae);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t vic_videoram_r(offs_t offset);
	uint8_t vic_colorram_r(offs_t offset);

	void tpi1_ca_w(int state);
	void tpi1_cb_w(int state);

	uint8_t tpi2_pc_r();
	void tpi2_pc_w(uint8_t data);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_p500);
	// video state
	int m_statvid;
	int m_vicdotsel;
	int m_vicbnksel;

	// interrupt state
	void p500_pal(machine_config &config);
	void p500_ntsc(machine_config &config);
	void p500_mem(address_map &map) ATTR_COLD;
	void vic_colorram_map(address_map &map) ATTR_COLD;
	void vic_videoram_map(address_map &map) ATTR_COLD;
};



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define P3 BIT(offset, 19)
#define P2 BIT(offset, 18)
#define P1 BIT(offset, 17)
#define P0 BIT(offset, 16)
#define A15 BIT(offset, 15)
#define A14 BIT(offset, 14)
#define A13 BIT(offset, 13)
#define A12 BIT(offset, 12)
#define A11 BIT(offset, 11)
#define A10 BIT(offset, 10)
#define A0 BIT(offset, 0)
#define VA12 BIT(va, 12)

static void cbmb_quick_sethiaddress(address_space &space, uint16_t hiaddress)
{
	space.write_byte(0xf0046, hiaddress & 0xff);
	space.write_byte(0xf0047, hiaddress >> 8);
}

QUICKLOAD_LOAD_MEMBER(cbm2_state::quickload_cbmb)
{
	return general_cbm_loadsnap(image, m_maincpu->space(AS_PROGRAM), 0x10000, cbmb_quick_sethiaddress);
}

QUICKLOAD_LOAD_MEMBER(p500_state::quickload_p500)
{
	return general_cbm_loadsnap(image, m_maincpu->space(AS_PROGRAM), 0, cbmb_quick_sethiaddress);
}

//**************************************************************************
//  ADDRESS DECODING
//**************************************************************************

//-------------------------------------------------
//  read_pla - low profile PLA read
//-------------------------------------------------

void cbm2_state::read_pla(offs_t offset, int ras, int cas, int refen, int eras, int ecas,
	int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *rasseg1, int *rasseg2, int *rasseg3, int *rasseg4)
{
	uint32_t input = P0 << 15 | P1 << 14 | P2 << 13 | P3 << 12 | m_busy2 << 11 | eras << 10 | ecas << 9 | refen << 8 | cas << 7 | ras << 6;
	uint32_t data = m_pla1->read(input);

	*casseg1 = BIT(data, 0);
	*rasseg1 = BIT(data, 1);
	*rasseg2 = BIT(data, 2);
	*casseg2 = BIT(data, 3);
	*rasseg4 = BIT(data, 4);
	*casseg4 = BIT(data, 5);
	*casseg3 = BIT(data, 6);
	*rasseg3 = BIT(data, 7);
}


//-------------------------------------------------
//  read_pla - high profile PLA read
//-------------------------------------------------

void cbm2hp_state::read_pla(offs_t offset, int ras, int cas, int refen, int eras, int ecas,
	int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *rasseg1, int *rasseg2, int *rasseg3, int *rasseg4)
{
	uint32_t input = ras << 13 | cas << 12 | refen << 11 | eras << 10 | ecas << 9 | m_busy2 << 8 | P3 << 3 | P2 << 2 | P1 << 1 | P0;
	uint32_t data = m_pla1->read(input);

	*casseg1 = BIT(data, 0);
	*casseg2 = BIT(data, 1);
	*casseg3 = BIT(data, 2);
	*casseg4 = BIT(data, 3);
	*rasseg1 = BIT(data, 4);
	*rasseg2 = BIT(data, 5);
	*rasseg3 = BIT(data, 6);
	*rasseg4 = BIT(data, 7);
}


//-------------------------------------------------
//  bankswitch -
//-------------------------------------------------

void cbm2_state::bankswitch(offs_t offset, int eras, int ecas, int refen, int cas, int ras, int *sysioen, int *dramen,
	int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *buframcs, int *extbufcs, int *vidramcs,
	int *diskromcs, int *csbank1, int *csbank2, int *csbank3, int *basiccs, int *knbcs, int *kernalcs,
	int *crtccs, int *cs1, int *sidcs, int *extprtcs, int *ciacs, int *aciacs, int *tript1cs, int *tript2cs)
{
	int rasseg1 = 1, rasseg2 = 1, rasseg3 = 1, rasseg4 = 1;

	this->read_pla(offset, ras, cas, refen, eras, ecas, casseg1, casseg2, casseg3, casseg4, &rasseg1, &rasseg2, &rasseg3, &rasseg4);

	int decoden = 0;
	*sysioen = !(P0 && P1 && P2 && P3) && m_busen1;
	*dramen = !((!(P0 && P1 && P2 && P3)) && m_busen1);

	if (!decoden && !*sysioen)
	{
		switch ((offset >> 13) & 0x07)
		{
		case 0:
			switch ((offset >> 11) & 0x03)
			{
			case 0: *buframcs = 0; break;
			case 1: *extbufcs = 0; break;
			case 2: // fallthru
			case 3: *diskromcs = 0; break;
			}
			break;

		case 1: *csbank1 = 0; break;
		case 2: *csbank2 = 0; break;
		case 3: *csbank3 = 0; break;
		case 4: *basiccs = 0; break;
		case 5: *knbcs = 0; break;
		case 6:
			switch ((offset >> 11) & 0x03)
			{
			case 2: *vidramcs = 0; break;
			case 3:
				switch ((offset >> 8) & 0x07)
				{
				case 0: *crtccs = 0; break;
				case 1: *cs1 = 0; break;
				case 2: *sidcs = 0; break;
				case 3: *extprtcs = 0; break;
				case 4: *ciacs = 0; break;
				case 5: *aciacs = 0; break;
				case 6: *tript1cs = 0; break;
				case 7: *tript2cs = 0; break;
				}
				break;
			}
			break;

		case 7: *kernalcs = 0; break;
		}
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t cbm2_state::read(offs_t offset)
{
	int eras = 1, ecas = 1, refen = 0, cas = 0, ras = 1, sysioen = 1, dramen = 1;
	int casseg1 = 1, casseg2 = 1, casseg3 = 1, casseg4 = 1, buframcs = 1, extbufcs = 1, vidramcs = 1;
	int diskromcs = 1, csbank1 = 1, csbank2 = 1, csbank3 = 1, basiccs = 1, knbcs = 1, kernalcs = 1;
	int crtccs = 1, cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;

	bankswitch(offset, eras, ecas, refen, cas, ras, &sysioen, &dramen,
		&casseg1, &casseg2, &casseg3, &casseg4, &buframcs, &extbufcs, &vidramcs,
		&diskromcs, &csbank1, &csbank2, &csbank3, &basiccs, &knbcs, &kernalcs,
		&crtccs, &cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs);

	uint8_t data = 0xff;

	if (!dramen)
	{
		if (!casseg1)
		{
			data = m_ram->pointer()[offset & 0xffff];
		}
		if (!casseg2)
		{
			data = m_ram->pointer()[0x10000 | (offset & 0xffff)];
		}
		if (!casseg3 && (m_ram->size() > 0x20000))
		{
			data = m_ram->pointer()[0x20000 | (offset & 0xffff)];
		}
		if (!casseg4 && (m_ram->size() > 0x30000))
		{
			data = m_ram->pointer()[0x30000 | (offset & 0xffff)];
		}
	}

	if (!sysioen)
	{
		if (!buframcs)
		{
			data = m_buffer_ram[offset & 0x7ff];
		}
		if (!extbufcs && m_extbuf_ram)
		{
			data = m_extbuf_ram[offset & 0x7ff];
		}
		if (!vidramcs)
		{
			data = m_video_ram[offset & 0x7ff];
		}
		if (!basiccs || !knbcs)
		{
			data = m_basic->base()[offset & 0x3fff];
		}
		if (!kernalcs)
		{
			data = m_kernal->base()[offset & 0x1fff];
		}
		if (!crtccs)
		{
			if (A0)
			{
				data = m_crtc->register_r();
			}
			else
			{
				data = m_crtc->status_r();
			}
		}
		if (!sidcs)
		{
			data = m_sid->read(offset & 0x1f);
		}
		if (!extprtcs && m_ext_cia)
		{
			data = m_ext_cia->read(offset & 0x0f);
		}
		if (!ciacs)
		{
			data = m_cia->read(offset & 0x0f);
		}
		if (!aciacs)
		{
			data = m_acia->read(offset & 0x03);
		}
		if (!tript1cs)
		{
			data = m_tpi1->read(offset & 0x07);
		}
		if (!tript2cs)
		{
			data = m_tpi2->read(offset & 0x07);
		}

		data = m_exp->read(offset & 0x1fff, data, csbank1, csbank2, csbank3);
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void cbm2_state::write(offs_t offset, uint8_t data)
{
	int eras = 1, ecas = 1, refen = 0, cas = 0, ras = 1, sysioen = 1, dramen = 1;
	int casseg1 = 1, casseg2 = 1, casseg3 = 1, casseg4 = 1, buframcs = 1, extbufcs = 1, vidramcs = 1;
	int diskromcs = 1, csbank1 = 1, csbank2 = 1, csbank3 = 1, basiccs = 1, knbcs = 1, kernalcs = 1;
	int crtccs = 1, cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;

	bankswitch(offset, eras, ecas, refen, cas, ras, &sysioen, &dramen,
		&casseg1, &casseg2, &casseg3, &casseg4, &buframcs, &extbufcs, &vidramcs,
		&diskromcs, &csbank1, &csbank2, &csbank3, &basiccs, &knbcs, &kernalcs,
		&crtccs, &cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs);

	if (!dramen)
	{
		if (!casseg1)
		{
			m_ram->pointer()[offset & 0xffff] = data;
		}
		if (!casseg2)
		{
			m_ram->pointer()[0x10000 | (offset & 0xffff)] = data;
		}
		if (!casseg3 && (m_ram->size() > 0x20000))
		{
			m_ram->pointer()[0x20000 | (offset & 0xffff)] = data;
		}
		if (!casseg4 && (m_ram->size() > 0x30000))
		{
			m_ram->pointer()[0x30000 | (offset & 0xffff)] = data;
		}
	}

	if (!sysioen)
	{
		if (!buframcs)
		{
			m_buffer_ram[offset & 0x7ff] = data;
		}
		if (!extbufcs && m_extbuf_ram)
		{
			m_extbuf_ram[offset & 0x7ff] = data;
		}
		if (!vidramcs)
		{
			m_video_ram[offset & 0x7ff] = data;
		}
		if (!crtccs)
		{
			if (A0)
			{
				m_crtc->register_w(data);
			}
			else
			{
				m_crtc->address_w(data);
			}
		}
		if (!sidcs)
		{
			m_sid->write(offset & 0x1f, data);
		}
		if (!extprtcs && m_ext_cia)
		{
			m_ext_cia->write(offset & 0x0f, data);
		}
		if (!ciacs)
		{
			m_cia->write(offset & 0x0f, data);
		}
		if (!aciacs)
		{
			m_acia->write(offset & 0x03, data);
		}
		if (!tript1cs)
		{
			m_tpi1->write(offset & 0x07, data);
		}
		if (!tript2cs)
		{
			m_tpi2->write(offset & 0x07, data);
		}

		m_exp->write(offset & 0x1fff, data, csbank1, csbank2, csbank3);
	}
}


//-------------------------------------------------
//  ext_read -
//-------------------------------------------------

uint8_t cbm2_state::ext_read(offs_t offset)
{
#ifdef USE_PLA_DECODE
	int ras = 1, cas = 1, refen = 0, eras = 1, ecas = 0;
	int casseg1 = 1, casseg2 = 1, casseg3 = 1, casseg4 = 1, rasseg1 = 1, rasseg2 = 1, rasseg3 = 1, rasseg4 = 1;

	this->read_pla(offset, ras, cas, refen, eras, ecas, &casseg1, &casseg2, &casseg3, &casseg4, &rasseg1, &rasseg2, &rasseg3, &rasseg4);
	uint8_t data = 0xff;

	if (!casseg1)
	{
		data = m_ram->pointer()[offset & 0xffff];
	}
	if (!casseg2)
	{
		data = m_ram->pointer()[0x10000 | (offset & 0xffff)];
	}
	if (!casseg3 && (m_ram->size() > 0x20000))
	{
		data = m_ram->pointer()[0x20000 | (offset & 0xffff)];
	}
	if (!casseg4 && (m_ram->size() > 0x30000))
	{
		data = m_ram->pointer()[0x30000 | (offset & 0xffff)];
	}

	return data;
#endif

	uint8_t data = 0;
	if (offset < 0x40000) data = m_ram->pointer()[offset];
	return data;
}


//-------------------------------------------------
//  ext_write -
//-------------------------------------------------

void cbm2_state::ext_write(offs_t offset, uint8_t data)
{
#ifdef USE_PLA_DECODE
	int ras = 1, cas = 1, refen = 0, eras = 1, ecas = 0;
	int casseg1 = 1, casseg2 = 1, casseg3 = 1, casseg4 = 1, rasseg1 = 1, rasseg2 = 1, rasseg3 = 1, rasseg4 = 1;

	this->read_pla(offset, ras, cas, refen, eras, ecas, &casseg1, &casseg2, &casseg3, &casseg4, &rasseg1, &rasseg2, &rasseg3, &rasseg4);

	if (!casseg1)
	{
		m_ram->pointer()[offset & 0xffff] = data;
	}
	if (!casseg2)
	{
		m_ram->pointer()[0x10000 | (offset & 0xffff)] = data;
	}
	if (!casseg3 && (m_ram->size() > 0x20000))
	{
		m_ram->pointer()[0x20000 | (offset & 0xffff)] = data;
	}
	if (!casseg4 && (m_ram->size() > 0x30000))
	{
		m_ram->pointer()[0x30000 | (offset & 0xffff)] = data;
	}
#endif

	if (offset < 0x40000) m_ram->pointer()[offset] = data;
}


//-------------------------------------------------
//  read_pla1 - P500 PLA #1 read
//-------------------------------------------------

void p500_state::read_pla1(offs_t offset, int busy2, int clrnibcsb, int procvid, int refen, int ba, int aec, int srw,
	int *datxen, int *dramxen, int *clrniben, int *segf, int *_64kcasen, int *casenb, int *viddaten, int *viddat_tr)
{
	int sphi2 = m_vic->phi0_r();
	int bras = 1;

	uint32_t input = P0 << 15 | P2 << 14 | bras << 13 | P1 << 12 | P3 << 11 | busy2 << 10 | m_statvid << 9 | sphi2 << 8 |
			clrnibcsb << 7 | m_dramon << 6 | procvid << 5 | refen << 4 | m_vicdotsel << 3 | ba << 2 | aec << 1 | srw;

	uint32_t data = m_pla1->read(input);

	*datxen = BIT(data, 0);
	*dramxen = BIT(data, 1);
	*clrniben = BIT(data, 2);
	*segf = BIT(data, 3);
	*_64kcasen = BIT(data, 4);
	*casenb = BIT(data, 5);
	*viddaten = BIT(data, 6);
	*viddat_tr = BIT(data, 7);
}


//-------------------------------------------------
//  read_pla2 - P500 PLA #2 read
//-------------------------------------------------

void p500_state::read_pla2(offs_t offset, offs_t va, int ba, int vicen, int ae, int segf, int bank0,
	int *clrnibcsb, int *extbufcs, int *discromcs, int *buframcs, int *charomcs, int *procvid, int *viccs, int *vidmatcs)
{
	int sphi2 = m_vic->phi0_r();
	int bcas = 1;

	uint32_t input = VA12 << 15 | ba << 14 | A13 << 13 | A15 << 12 | A14 << 11 | A11 << 10 | A10 << 9 | A12 << 8 |
			sphi2 << 7 | vicen << 6 | m_statvid << 5 | m_vicdotsel << 4 | ae << 3 | segf << 2 | bcas << 1 | bank0;

	uint32_t data = m_pla2->read(input);

	*clrnibcsb = BIT(data, 0);
	*extbufcs = BIT(data, 1);
	*discromcs = BIT(data, 2);
	*buframcs = BIT(data, 3);
	*charomcs = BIT(data, 4);
	*procvid = BIT(data, 5);
	*viccs = BIT(data, 6);
	*vidmatcs = BIT(data, 7);
}


//-------------------------------------------------
//  bankswitch -
//-------------------------------------------------

void p500_state::bankswitch(offs_t offset, offs_t va, int srw, int ba, int ae, int busy2, int refen,
	int *datxen, int *dramxen, int *clrniben, int *_64kcasen, int *casenb, int *viddaten, int *viddat_tr,
	int *clrnibcs, int *extbufcs, int *discromcs, int *buframcs, int *charomcs, int *viccs, int *vidmatcs,
	int *csbank1, int *csbank2, int *csbank3, int *basiclocs, int *basichics, int *kernalcs,
	int *cs1, int *sidcs, int *extprtcs, int *ciacs, int *aciacs, int *tript1cs, int *tript2cs, int *aec, int *vsysaden)
{
	int sphi2 = m_vic->phi0_r();
	int sphi1 = !sphi2;
	//int ba = !m_vic->ba_r();
	//int ae = m_vic->aec_r();
	int bcas = 0;

	*aec = !((m_statvid || ae) && sphi2);
	*vsysaden = sphi1 || ba;

	int clrnibcsb = 1, procvid = 1, segf = 1;

	read_pla1(offset, busy2, clrnibcsb, procvid, refen, ba, *aec, srw,
		datxen, dramxen, clrniben, &segf, _64kcasen, casenb, viddaten, viddat_tr);

	int bank0 = 1, vicen = 1;

	if (!*aec && !segf)
	{
		switch ((offset >> 13) & 0x07)
		{
		case 0: bank0 = 0; break;
		case 1: *csbank1 = 0; break;
		case 2: *csbank2 = 0; break;
		case 3: *csbank3 = 0; break;
		case 4: *basiclocs = 0; break;
		case 5: *basichics = 0; break;
		case 6:
			if (A12 && A11)
			{
				switch ((offset >> 8) & 0x07)
				{
				case 0: vicen = 0; break;
				case 1: *cs1 = 0; break;
				case 2: *sidcs = 0; break;
				case 3: *extprtcs = 0; break;
				case 4: *ciacs = 0; break;
				case 5: *aciacs = 0; break;
				case 6: *tript1cs = 0; break;
				case 7: *tript2cs = 0; break;
				}
			}
			break;

		case 7: *kernalcs = 0; break;
		}
	}

	int vidmatcsb = 1;

	read_pla2(offset, va, ba, vicen, ae, segf, bank0,
		&clrnibcsb, extbufcs, discromcs, buframcs, charomcs, &procvid, viccs, &vidmatcsb);

	*clrnibcs = clrnibcsb || bcas;
	*vidmatcs = vidmatcsb || bcas;

	read_pla1(offset, busy2, clrnibcsb, procvid, refen, ba, *aec, srw,
		datxen, dramxen, clrniben, &segf, _64kcasen, casenb, viddaten, viddat_tr);
}


//-------------------------------------------------
//  read_memory -
//-------------------------------------------------

uint8_t p500_state::read_memory(offs_t offset, offs_t va, int ba, int ae)
{
	int srw = 1, busy2 = 1, refen = 0;

	int datxen = 1, dramxen = 1, clrniben = 1, _64kcasen = 1, casenb = 1, viddaten = 1, viddat_tr = 1;
	int clrnibcs = 1, extbufcs = 1, discromcs = 1, buframcs = 1, charomcs = 1, viccs = 1, vidmatcs = 1;
	int csbank1 = 1, csbank2 = 1, csbank3 = 1, basiclocs = 1, basichics = 1, kernalcs = 1;
	int cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;
	int aec = 1, vsysaden = 1;

	bankswitch(offset, va, srw, ba, ae, busy2, refen,
		&datxen, &dramxen, &clrniben, &_64kcasen, &casenb, &viddaten, &viddat_tr,
		&clrnibcs, &extbufcs, &discromcs, &buframcs, &charomcs, &viccs, &vidmatcs,
		&csbank1, &csbank2, &csbank3, &basiclocs, &basichics, &kernalcs,
		&cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs, &aec, &vsysaden);

	uint8_t data = 0xff;

	if (clrniben)
	{
		if (!clrnibcs && !vsysaden)
		{
			data = m_color_ram[offset & 0x3ff];
		}
	}

	if (!dramxen)
	{
		if (casenb)
		{
			switch (offset >> 16)
			{
			case 1: data = m_ram->pointer()[0x10000 + (offset & 0xffff)]; break;
			case 2: if (m_ram->size() > 0x20000) data = m_ram->pointer()[0x20000 + (offset & 0xffff)]; break;
			case 3: if (m_ram->size() > 0x30000) data = m_ram->pointer()[0x30000 + (offset & 0xffff)]; break;
			}
		}
	}

	if (!datxen)
	{
		if (!_64kcasen && !aec)
		{
			data = m_ram->pointer()[offset & 0xffff];
		}
		if (!buframcs)
		{
			data = m_buffer_ram[offset & 0x7ff];
		}
		if (!vidmatcs && !vsysaden && !viddaten && viddat_tr)
		{
			data = m_video_ram[offset & 0x3ff];
		}
		if (!basiclocs || !basichics)
		{
			data = m_basic->base()[offset & 0x3fff];
		}
		if (!kernalcs)
		{
			data = m_kernal->base()[offset & 0x1fff];
		}
		if (!charomcs && !vsysaden && !viddaten && viddat_tr)
		{
			data = m_charom->base()[offset & 0xfff];
		}
		if (!viccs && !viddaten && viddat_tr)
		{
			data = m_vic->read(offset & 0x3f);
		}
		if (!sidcs)
		{
			data = m_sid->read(offset & 0x1f);
		}
		if (!ciacs)
		{
			data = m_cia->read(offset & 0x0f);
		}
		if (!aciacs)
		{
			data = m_acia->read(offset & 0x03);
		}
		if (!tript1cs)
		{
			data = m_tpi1->read(offset & 0x07);
		}
		if (!tript2cs)
		{
			data = m_tpi2->read(offset & 0x07);
		}

		data = m_exp->read(offset & 0x1fff, data, csbank1, csbank2, csbank3);
	}

	return data;
}


//-------------------------------------------------
//  write_memory -
//-------------------------------------------------

void p500_state::write_memory(offs_t offset, uint8_t data, int ba, int ae)
{
	int srw = 0, busy2 = 1, refen = 0;
	offs_t va = 0xffff;

	int datxen = 1, dramxen = 1, clrniben = 1, _64kcasen = 1, casenb = 1, viddaten = 1, viddat_tr = 1;
	int clrnibcs = 1, extbufcs = 1, discromcs = 1, buframcs = 1, charomcs = 1, viccs = 1, vidmatcs = 1;
	int csbank1 = 1, csbank2 = 1, csbank3 = 1, basiclocs = 1, basichics = 1, kernalcs = 1;
	int cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;
	int aec = 1, vsysaden = 1;

	bankswitch(offset, va, srw, ba, ae, busy2, refen,
		&datxen, &dramxen, &clrniben, &_64kcasen, &casenb, &viddaten, &viddat_tr,
		&clrnibcs, &extbufcs, &discromcs, &buframcs, &charomcs, &viccs, &vidmatcs,
		&csbank1, &csbank2, &csbank3, &basiclocs, &basichics, &kernalcs,
		&cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs, &aec, &vsysaden);

	if (clrniben)
	{
		if (!clrnibcs && !vsysaden)
		{
			m_color_ram[offset & 0x3ff] = data & 0x0f;
		}
	}

	if (!dramxen)
	{
		if (casenb)
		{
			switch (offset >> 16)
			{
			case 1: m_ram->pointer()[0x10000 + (offset & 0xffff)] = data; break;
			case 2: if (m_ram->size() > 0x20000) m_ram->pointer()[0x20000 + (offset & 0xffff)] = data; break;
			case 3: if (m_ram->size() > 0x30000) m_ram->pointer()[0x30000 + (offset & 0xffff)] = data; break;
			}
		}
	}

	if (!datxen)
	{
		if (!_64kcasen && !aec)
		{
			m_ram->pointer()[offset & 0xffff] = data;
		}
		if (!buframcs)
		{
			m_buffer_ram[offset & 0x7ff] = data;
		}
		if (!vidmatcs && !vsysaden && !viddaten && !viddat_tr)
		{
			m_video_ram[offset & 0x3ff] = data;
		}
		if (!viccs && !viddaten && !viddat_tr)
		{
			m_vic->write(offset & 0x3f, data);
		}
		if (!sidcs)
		{
			m_sid->write(offset & 0x1f, data);
		}
		if (!ciacs)
		{
			m_cia->write(offset & 0x0f, data);
		}
		if (!aciacs)
		{
			m_acia->write(offset & 0x03, data);
		}
		if (!tript1cs)
		{
			m_tpi1->write(offset & 0x07, data);
		}
		if (!tript2cs)
		{
			m_tpi2->write(offset & 0x07, data);
		}

		m_exp->write(offset & 0x1fff, data, csbank1, csbank2, csbank3);
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t p500_state::read(offs_t offset)
{
	int ba = 0, ae = 1;
	offs_t va = 0xffff;

	return read_memory(offset, va, ba, ae);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void p500_state::write(offs_t offset, uint8_t data)
{
	int ba = 0, ae = 1;

	write_memory(offset, data, ba, ae);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

uint8_t p500_state::vic_videoram_r(offs_t offset)
{
	int srw = 1, busy2 = 1, refen = 0;
	int ba = !m_vic->ba_r(), ae = m_vic->aec_r();
	int datxen = 1, dramxen = 1, clrniben = 1, _64kcasen = 1, casenb = 1, viddaten = 1, viddat_tr = 1;
	int clrnibcs = 1, extbufcs = 1, discromcs = 1, buframcs = 1, charomcs = 1, viccs = 1, vidmatcs = 1;
	int csbank1 = 1, csbank2 = 1, csbank3 = 1, basiclocs = 1, basichics = 1, kernalcs = 1;
	int cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;
	int aec = 1, vsysaden = 1;

	bankswitch(0, offset, srw, ba, ae, busy2, refen,
		&datxen, &dramxen, &clrniben, &_64kcasen, &casenb, &viddaten, &viddat_tr,
		&clrnibcs, &extbufcs, &discromcs, &buframcs, &charomcs, &viccs, &vidmatcs,
		&csbank1, &csbank2, &csbank3, &basiclocs, &basichics, &kernalcs,
		&cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs, &aec, &vsysaden);

	uint8_t data = 0xff;
//  uint8_t clrnib = 0xf;

	if (vsysaden)
	{
		if (!_64kcasen && !aec && !viddaten && !viddat_tr)
		{
			data = m_ram->pointer()[(m_vicbnksel << 14) | offset];
		}
/*      if (!clrnibcs)
        {
            clrnib = m_color_ram[offset & 0x3ff];
        }*/
		if (!vidmatcs)
		{
			data = m_video_ram[offset & 0x3ff];
		}
		if (!charomcs)
		{
			data = m_charom->base()[offset & 0xfff];
		}
	}

	return data;
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

uint8_t p500_state::vic_colorram_r(offs_t offset)
{
	int srw = 1, busy2 = 1, refen = 0;
	int ba = !m_vic->ba_r(), ae = m_vic->aec_r();
	int datxen = 1, dramxen = 1, clrniben = 1, _64kcasen = 1, casenb = 1, viddaten = 1, viddat_tr = 1;
	int clrnibcs = 1, extbufcs = 1, discromcs = 1, buframcs = 1, charomcs = 1, viccs = 1, vidmatcs = 1;
	int csbank1 = 1, csbank2 = 1, csbank3 = 1, basiclocs = 1, basichics = 1, kernalcs = 1;
	int cs1 = 1, sidcs = 1, extprtcs = 1, ciacs = 1, aciacs = 1, tript1cs = 1, tript2cs = 1;
	int aec = 1, vsysaden = 1;

	bankswitch(0, offset, srw, ba, ae, busy2, refen,
		&datxen, &dramxen, &clrniben, &_64kcasen, &casenb, &viddaten, &viddat_tr,
		&clrnibcs, &extbufcs, &discromcs, &buframcs, &charomcs, &viccs, &vidmatcs,
		&csbank1, &csbank2, &csbank3, &basiclocs, &basichics, &kernalcs,
		&cs1, &sidcs, &extprtcs, &ciacs, &aciacs, &tript1cs, &tript2cs, &aec, &vsysaden);

	uint8_t data = 0x0f;

	if (!clrnibcs)
	{
		data = m_color_ram[offset & 0x3ff];
	}

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( cbm2_mem )
//-------------------------------------------------

void cbm2_state::cbm2_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(cbm2_state::read), FUNC(cbm2_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( ext_mem )
//-------------------------------------------------

void cbm2_state::ext_mem(address_map &map)
{
	map(0x00000, 0xeffff).r(FUNC(cbm2_state::ext_read)).w(FUNC(cbm2_state::ext_write));
	map(0xf0000, 0xf0fff).mirror(0xf000).rom().region(EXT_I8088_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( ext_io )
//-------------------------------------------------

void cbm2_state::ext_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x0000, 0x0001).mirror(0x1e).rw(m_ext_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0020, 0x0027).mirror(0x18).rw(EXT_MOS6525_TAG, FUNC(tpi6525_device::read), FUNC(tpi6525_device::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( p500_mem )
//-------------------------------------------------

void p500_state::p500_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(p500_state::read), FUNC(p500_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

void p500_state::vic_videoram_map(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(p500_state::vic_videoram_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

void p500_state::vic_colorram_map(address_map &map)
{
	map(0x000, 0x3ff).r(FUNC(p500_state::vic_colorram_r));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( cbm2 )
//-------------------------------------------------

static INPUT_PORTS_START( cbm2 )
	PORT_START("PB0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F9") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F10") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT" \xC2\xA3") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(UCHAR_MAMEKEY(TILDE)) PORT_CHAR(U'£')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(U'π')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS/DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C=") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR/HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad ?")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("OFF/RVS") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad CE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NORM/GRAPH") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 00") PORT_CODE(KEYCODE_00_PAD) PORT_CHAR(UCHAR_MAMEKEY(00_PAD))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN/STOP") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad /") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad ENTER") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LOCK")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE
	PORT_BIT( 0xef, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( cbm2_de )
//-------------------------------------------------

static INPUT_PORTS_START( cbm2_de )
	PORT_INCLUDE(cbm2)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( cbm2_hu )
//-------------------------------------------------

static INPUT_PORTS_START( cbm2_hu )
	PORT_INCLUDE(cbm2)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( cbm2_se )
//------------------------------------------------

static INPUT_PORTS_START( cbm2_se )
	PORT_INCLUDE(cbm2)

	PORT_MODIFY("PA0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(U'ö') PORT_CHAR(U'Ö')

	PORT_MODIFY("PA1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(U'å') PORT_CHAR(U'Å')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(U'ä') PORT_CHAR(U'Ä')

	PORT_MODIFY("PA2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT" \xCF\x80") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(UCHAR_MAMEKEY(TILDE)) PORT_CHAR(U'π')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(';') PORT_CHAR(':')
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  mc6845
//-------------------------------------------------

MC6845_UPDATE_ROW( cbm2_state::crtc_update_row )
{
	pen_t const *const pen = m_palette->pens();

	int x = 0;

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_video_ram[(ma + column) & 0x7ff];
		offs_t char_rom_addr = (ma & 0x1000) | (m_graphics << 11) | ((code & 0x7f) << 4) | (ra & 0x0f);
		uint8_t data = m_charom->base()[char_rom_addr & 0xfff];

		for (int bit = 0; bit < 9; bit++)
		{
			int color = BIT(data, 7) ^ BIT(code, 7) ^ BIT(ma, 13);
			if (cursor_x == column) color ^= 1;
			color &= de;

			bitmap.pix(vbp + y, hbp + x++) = pen[color];

			if (bit < 8 || !m_graphics) data <<= 1;
		}
	}
}


//-------------------------------------------------
//  MOS6581_INTERFACE( sid_intf )
//-------------------------------------------------

uint8_t cbm2_state::sid_potx_r()
{
	uint8_t data = 0xff;

	switch (m_cia_pa >> 6)
	{
	case 1: data = m_joy1->read_pot_x(); break;
	case 2: data = m_joy2->read_pot_x(); break;
	case 3:
		if (m_joy1->has_pot_x() && m_joy2->has_pot_x())
		{
			data = 1 / (1 / m_joy1->read_pot_x() + 1 / m_joy2->read_pot_x());
		}
		else if (m_joy1->has_pot_x())
		{
			data = m_joy1->read_pot_x();
		}
		else if (m_joy2->has_pot_x())
		{
			data = m_joy2->read_pot_x();
		}
		break;
	}

	return data;
}

uint8_t cbm2_state::sid_poty_r()
{
	uint8_t data = 0xff;

	switch (m_cia_pa >> 6)
	{
	case 1: data = m_joy1->read_pot_y(); break;
	case 2: data = m_joy2->read_pot_y(); break;
	case 3:
		if (m_joy1->has_pot_y() && m_joy2->has_pot_y())
		{
			data = 1 / (1 / m_joy1->read_pot_y() + 1 / m_joy2->read_pot_y());
		}
		else if (m_joy1->has_pot_y())
		{
			data = m_joy1->read_pot_y();
		}
		else if (m_joy2->has_pot_y())
		{
			data = m_joy2->read_pot_y();
		}
		break;
	}

	return data;
}


//-------------------------------------------------
//  tpi6525_interface tpi1_intf
//-------------------------------------------------

uint8_t cbm2_state::tpi1_pa_r()
{
	/*

	    bit     description

	    0       0
	    1       0
	    2       REN
	    3       ATN
	    4       DAV
	    5       EOI
	    6       NDAC
	    7       NRFD

	*/

	uint8_t data = 0;

	// IEEE-488
	data |= m_ieee2->ren_r() << 2;
	data |= m_ieee2->atn_r() << 3;
	data |= m_ieee2->dav_r() << 4;
	data |= m_ieee2->eoi_r() << 5;
	data |= m_ieee2->ndac_r() << 6;
	data |= m_ieee2->nrfd_r() << 7;

	return data;
}

void cbm2_state::tpi1_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0       75161A DC
	    1       75161A TE
	    2       REN
	    3       ATN
	    4       DAV
	    5       EOI
	    6       NDAC
	    7       NRFD

	*/

	// IEEE-488
	m_ieee2->dc_w(BIT(data, 0));

	m_ieee1->te_w(BIT(data, 1));
	m_ieee2->te_w(BIT(data, 1));

	m_ieee2->ren_w(BIT(data, 2));
	m_ieee2->atn_w(BIT(data, 3));
	m_ieee2->dav_w(BIT(data, 4));
	m_ieee2->eoi_w(BIT(data, 5));
	m_ieee2->ndac_w(BIT(data, 6));
	m_ieee2->nrfd_w(BIT(data, 7));
}

uint8_t cbm2_state::tpi1_pb_r()
{
	/*

	    bit     description

	    0       IFC
	    1       SRQ
	    2       user port PB2
	    3       user port PB3
	    4
	    5
	    6
	    7       CASS SW

	*/

	uint8_t data = 0;

	// IEEE-488
	data |= m_ieee2->ifc_r();
	data |= m_ieee2->srq_r() << 1;

	// user port
	data |= m_user->pb2_r() << 2;
	data |= m_user->pb3_r() << 3;

	// cassette
	data |= m_cassette->sense_r() << 7;

	return data;
}

void cbm2_state::tpi1_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0       IFC
	    1       SRQ
	    2       user port PB2
	    3       user port PB3
	    4       DRAMON
	    5       CASS WRT
	    6       CASS MTR
	    7

	*/

	// IEEE-488
	m_ieee2->ifc_w(BIT(data, 0));
	m_ieee2->srq_w(BIT(data, 1));

	// user port
	m_user->pb2_w(BIT(data, 2));
	m_user->pb3_w(BIT(data, 3));

	// memory
	m_dramon = BIT(data, 4);
	if (m_busy2) m_busen1 = m_dramon;

	// cassette
	m_cassette->write(BIT(data, 5));
	m_cassette->motor_w(BIT(data, 6));
}

void cbm2_state::tpi1_ca_w(int state)
{
	m_graphics = state;
}

void p500_state::tpi1_ca_w(int state)
{
	m_statvid = state;
}

void p500_state::tpi1_cb_w(int state)
{
	m_vicdotsel = state;
}

//-------------------------------------------------
//  tpi6525_interface tpi2_intf
//-------------------------------------------------

uint8_t cbm2_state::read_keyboard()
{
	uint8_t data = 0xff;

	if (!BIT(m_tpi2_pa, 0)) data &= m_pa[0]->read();
	if (!BIT(m_tpi2_pa, 1)) data &= m_pa[1]->read();
	if (!BIT(m_tpi2_pa, 2)) data &= m_pa[2]->read();
	if (!BIT(m_tpi2_pa, 3)) data &= m_pa[3]->read();
	if (!BIT(m_tpi2_pa, 4)) data &= m_pa[4]->read();
	if (!BIT(m_tpi2_pa, 5)) data &= m_pa[5]->read();
	if (!BIT(m_tpi2_pa, 6)) data &= m_pa[6]->read();
	if (!BIT(m_tpi2_pa, 7)) data &= m_pa[7]->read();
	if (!BIT(m_tpi2_pb, 0)) data &= m_pb[0]->read() & m_lock->read();
	if (!BIT(m_tpi2_pb, 1)) data &= m_pb[1]->read();
	if (!BIT(m_tpi2_pb, 2)) data &= m_pb[2]->read();
	if (!BIT(m_tpi2_pb, 3)) data &= m_pb[3]->read();
	if (!BIT(m_tpi2_pb, 4)) data &= m_pb[4]->read();
	if (!BIT(m_tpi2_pb, 5)) data &= m_pb[5]->read();
	if (!BIT(m_tpi2_pb, 6)) data &= m_pb[6]->read();
	if (!BIT(m_tpi2_pb, 7)) data &= m_pb[7]->read();

	return data;
}

void cbm2_state::tpi2_pa_w(uint8_t data)
{
	m_tpi2_pa = data;
}

void cbm2_state::tpi2_pb_w(uint8_t data)
{
	m_tpi2_pb = data;
}

uint8_t cbm2_state::tpi2_pc_r()
{
	/*

	    bit     description

	    0       COLUMN 0
	    1       COLUMN 1
	    2       COLUMN 2
	    3       COLUMN 3
	    4       COLUMN 4
	    5       COLUMN 5
	    6       0=PAL, 1=NTSC
	    7       0

	*/

	return (m_ntsc << 6) | (read_keyboard() & 0x3f);
}

uint8_t cbm2hp_state::tpi2_pc_r()
{
	/*

	    bit     description

	    0       COLUMN 0
	    1       COLUMN 1
	    2       COLUMN 2
	    3       COLUMN 3
	    4       COLUMN 4
	    5       COLUMN 5
	    6       1
	    7       1

	*/

	return read_keyboard();
}

uint8_t p500_state::tpi2_pc_r()
{
	/*

	    bit     description

	    0       COLUMN 0
	    1       COLUMN 1
	    2       COLUMN 2
	    3       COLUMN 3
	    4       COLUMN 4
	    5       COLUMN 5
	    6       0
	    7       0

	*/

	return read_keyboard();
}

void p500_state::tpi2_pc_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6       VICBNKSEL0
	    7       VICBNKSEL1

	*/

	m_vicbnksel = data >> 6;
}

//-------------------------------------------------
//  MOS6526_INTERFACE( cia_intf )
//-------------------------------------------------

uint8_t cbm2_state::cia_pa_r()
{
	/*

	    bit     description

	    0       IEEE-488 D0, user port 1D0
	    1       IEEE-488 D1, user port 1D1
	    2       IEEE-488 D2, user port 1D2
	    3       IEEE-488 D3, user port 1D3
	    4       IEEE-488 D4, user port 1D4
	    5       IEEE-488 D5, user port 1D5
	    6       IEEE-488 D6, user port 1D6, LTPN
	    7       IEEE-488 D7, user port 1D7, GAME TRIGGER 24

	*/

	uint8_t data = 0;

	// IEEE-488
	data |= m_ieee1->read();

	// user port
	data &= m_user->d1_r();

	// joystick
	data &= ~(!BIT(m_joy1->read_joy(), 5) << 6);
	data &= ~(!BIT(m_joy2->read_joy(), 5) << 7);

	return data;
}

void cbm2_state::cia_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0       IEEE-488 D0, user port 1D0
	    1       IEEE-488 D1, user port 1D1
	    2       IEEE-488 D2, user port 1D2
	    3       IEEE-488 D3, user port 1D3
	    4       IEEE-488 D4, user port 1D4
	    5       IEEE-488 D5, user port 1D5
	    6       IEEE-488 D6, user port 1D6
	    7       IEEE-488 D7, user port 1D7

	*/

	// IEEE-488
	m_ieee1->write(data);

	// user port
	m_user->d1_w(data);

	// joystick
	m_cia_pa = data;
}

uint8_t cbm2_state::cia_pb_r()
{
	/*

	    bit     description

	    0       user port 2D0, GAME10
	    1       user port 2D1, GAME11
	    2       user port 2D2, GAME12
	    3       user port 2D3, GAME13
	    4       user port 2D4, GAME20
	    5       user port 2D5, GAME21
	    6       user port 2D6, GAME22
	    7       user port 2D7, GAME23

	*/

	uint8_t data = 0;

	// joystick
	data |= m_joy1->read_joy() & 0x0f;
	data |= (m_joy2->read_joy() & 0x0f) << 4;

	// user port
	data &= m_user->d2_r();

	return data;
}


//-------------------------------------------------
//  tpi6525_interface ext_tpi_intf
//-------------------------------------------------

void cbm2_state::set_busy2(int state)
{
	m_busy2 = state;

	if (m_busy2)
	{
		//m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

		m_busen1 = m_dramon;
	}
	else
	{
		//m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

		m_busen1 = 0;
	}
}

uint8_t cbm2_state::ext_tpi_pb_r()
{
	/*

	    bit     description

	    0       _BUSY1
	    1       _BUSY2
	    2       _REQ
	    3       _ACK
	    4       DATA/_CMD
	    5       DIR
	    6       1
	    7       1

	*/

	uint8_t data = 0xc0;

	// _BUSY1
	data |= !m_busen1;

	// _BUSY2
	data |= m_busy2 << 1;

	// CIA
	data |= m_ext_tpi_pb & m_ext_cia_pb & 0x3c;

	return data;
}

void cbm2_state::ext_tpi_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1       _BUSY2
	    2
	    3
	    4
	    5
	    6       CIA FLAG
	    7

	*/

	m_ext_tpi_pb = data;

	// _BUSY2
	if (!BIT(data, 1))
	{
		set_busy2(0);
	}

	// FLAG
	m_ext_cia->flag_w(BIT(data, 6));
}

void cbm2_state::ext_tpi_pc_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       BSYCLK
	    6
	    7

	*/

	// _BUSY2
	if (BIT(data, 5))
	{
		set_busy2(1);
	}
}

//-------------------------------------------------
//  MOS6526_INTERFACE( ext_cia_intf )
//-------------------------------------------------

void cbm2_state::ext_cia_irq_w(int state)
{
	m_tpi1->i3_w(!state);
}

uint8_t cbm2_state::ext_cia_pb_r()
{
	/*

	    bit     description

	    0       _BUSY1
	    1       _BUSY2
	    2       _REQ
	    3       _ACK
	    4       DATA/_CMD
	    5       DIR
	    6       1
	    7       1

	*/

	uint8_t data = 0xc0;

	// _BUSY1
	data |= !m_busen1;

	// _BUSY2
	data |= m_busy2 << 1;

	// TPI
	data |= m_ext_tpi_pb & m_ext_cia_pb & 0x3c;

	return data;
}

void cbm2_state::ext_cia_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1       _BUSY2
	    2
	    3
	    4
	    5
	    6       _INT1
	    7       _INT2

	*/

	m_ext_cia_pb = data;

	// _BUSY2
	if (!BIT(data, 1))
	{
		set_busy2(0);
	}

	if (!BIT(data, 6))
	{
		set_busy2(0);
	}

	m_ext_pic->ir0_w(!BIT(data, 6));
	m_ext_pic->ir7_w(BIT(data, 7));
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  tod_tick - advance the TOD clock
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(cbm2_state::tod_tick)
{
	m_tpi1->i0_w(m_todclk);

	if (m_ext_pic) m_ext_pic->ir2_w(m_todclk);

	m_todclk = !m_todclk;
}


//-------------------------------------------------
//  MACHINE_START( cbm2 )
//-------------------------------------------------

MACHINE_START_MEMBER( cbm2_state, cbm2 )
{
	// allocate timer
	int todclk = (m_ntsc ? 60 : 50) * 2;

	m_todclk_timer = timer_alloc(FUNC(cbm2_state::tod_tick), this);
	m_todclk_timer->adjust(attotime::from_hz(todclk), 0, attotime::from_hz(todclk));

	// state saving
	save_item(NAME(m_dramon));
	save_item(NAME(m_busen1));
	save_item(NAME(m_busy2));
	save_item(NAME(m_graphics));
	save_item(NAME(m_ntsc));
	save_item(NAME(m_todclk));
	save_item(NAME(m_tpi2_pa));
	save_item(NAME(m_tpi2_pb));
	save_item(NAME(m_cia_pa));
}


//-------------------------------------------------
//  MACHINE_START( cbm2_ntsc )
//-------------------------------------------------

MACHINE_START_MEMBER( cbm2_state, cbm2_ntsc )
{
	m_ntsc = 1;

	MACHINE_START_CALL_MEMBER(cbm2);
}


//-------------------------------------------------
//  MACHINE_START( cbm2_pal )
//-------------------------------------------------

MACHINE_START_MEMBER( cbm2_state, cbm2_pal )
{
	m_ntsc = 0;

	MACHINE_START_CALL_MEMBER(cbm2);
}


//-------------------------------------------------
//  MACHINE_START( cbm2x_ntsc )
//-------------------------------------------------

MACHINE_START_MEMBER( cbm2_state, cbm2x_ntsc )
{
	MACHINE_START_CALL_MEMBER(cbm2_ntsc);
}


//-------------------------------------------------
//  MACHINE_START( cbm2x_pal )
//-------------------------------------------------

MACHINE_START_MEMBER( cbm2_state, cbm2x_pal )
{
	MACHINE_START_CALL_MEMBER(cbm2_pal);
}


//-------------------------------------------------
//  MACHINE_START( p500 )
//-------------------------------------------------

MACHINE_START_MEMBER( p500_state, p500 )
{
	m_video_ram_size = 0x400;

	MACHINE_START_CALL_MEMBER(cbm2);

	// state saving
	save_item(NAME(m_statvid));
	save_item(NAME(m_vicdotsel));
	save_item(NAME(m_vicbnksel));
}


//-------------------------------------------------
//  MACHINE_START( p500_ntsc )
//-------------------------------------------------

MACHINE_START_MEMBER( p500_state, p500_ntsc )
{
	m_ntsc = 1;

	MACHINE_START_CALL_MEMBER(p500);
}


//-------------------------------------------------
//  MACHINE_START( p500_pal )
//-------------------------------------------------

MACHINE_START_MEMBER( p500_state, p500_pal )
{
	m_ntsc = 0;

	MACHINE_START_CALL_MEMBER(p500);
}


MACHINE_RESET_MEMBER( cbm2_state, cbm2 )
{
	m_dramon = 1;
	m_busen1 = 1;
	m_busy2 = 1;
	m_graphics = 1;

m_ext_tpi_pb = 0xff;
m_ext_cia_pb = 0xff;

	m_maincpu->reset();

	if (m_crtc) m_crtc->reset();
	m_sid->reset();
	m_tpi1->reset();
	m_tpi2->reset();
	m_acia->reset();
	m_cia->reset();

	m_ieee->reset();
}


MACHINE_RESET_MEMBER( p500_state, p500 )
{
	MACHINE_RESET_CALL_MEMBER(cbm2);

	m_vic->reset();

	m_statvid = 1;
	m_vicdotsel = 1;
	m_vicbnksel = 0x03;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( 128k )
//-------------------------------------------------

void cbm2_state::_128k(machine_config &config)
{
	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("256K");
}


//-------------------------------------------------
//  machine_config( 256k )
//-------------------------------------------------

void cbm2_state::_256k(machine_config &config)
{
	RAM(config, RAM_TAG).set_default_size("256K");
}


//-------------------------------------------------
//  machine_config( p500_ntsc )
//-------------------------------------------------

void p500_state::p500_ntsc(machine_config &config)
{
	MCFG_MACHINE_START_OVERRIDE(p500_state, p500_ntsc)
	MCFG_MACHINE_RESET_OVERRIDE(p500_state, p500)

	// basic hardware
	M6509(config, m_maincpu, XTAL(14'318'181)/14);
	m_maincpu->set_addrmap(AS_PROGRAM, &p500_state::p500_mem);
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m6509_device::IRQ_LINE);

	// video hardware
	mos6567_device &mos6567(MOS6567(config, MOS6567_TAG, XTAL(14'318'181)/14));
	mos6567.set_cpu(m_maincpu);
	mos6567.irq_callback().set("mainirq", FUNC(input_merger_device::in_w<0>));
	mos6567.set_screen(SCREEN_TAG);
	mos6567.set_addrmap(0, &p500_state::vic_videoram_map);
	mos6567.set_addrmap(1, &p500_state::vic_colorram_map);

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(VIC6567_VRETRACERATE);
	screen.set_size(VIC6567_COLUMNS, VIC6567_LINES);
	screen.set_visarea(0, VIC6567_VISIBLECOLUMNS - 1, 0, VIC6567_VISIBLELINES - 1);
	screen.set_screen_update(MOS6567_TAG, FUNC(mos6567_device::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	MOS6581(config, m_sid, XTAL(14'318'181)/14);
	m_sid->potx().set(FUNC(p500_state::sid_potx_r));
	m_sid->poty().set(FUNC(p500_state::sid_poty_r));
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);

	// devices
	PLS100(config, m_pla1);
	PLS100(config, m_pla2);

	TPI6525(config, m_tpi1, 0);
	m_tpi1->out_irq_cb().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_tpi1->in_pa_cb().set(FUNC(cbm2_state::tpi1_pa_r));
	m_tpi1->out_pa_cb().set(FUNC(cbm2_state::tpi1_pa_w));
	m_tpi1->in_pb_cb().set(FUNC(cbm2_state::tpi1_pb_r));
	m_tpi1->out_pa_cb().set(FUNC(cbm2_state::tpi1_pb_w));
	m_tpi1->out_ca_cb().set(FUNC(p500_state::tpi1_ca_w));
	m_tpi1->out_cb_cb().set(FUNC(p500_state::tpi1_cb_w));

	TPI6525(config, m_tpi2, 0);
	m_tpi2->out_pa_cb().set(FUNC(cbm2_state::tpi2_pa_w));
	m_tpi2->out_pb_cb().set(FUNC(cbm2_state::tpi2_pb_w));
	m_tpi2->in_pc_cb().set(FUNC(p500_state::tpi2_pc_r));
	m_tpi2->out_pc_cb().set(FUNC(p500_state::tpi2_pc_w));

	MOS6551(config, m_acia, VIC6567_CLOCK);
	m_acia->set_xtal(XTAL(1'843'200));
	m_acia->irq_handler().set(m_tpi1, FUNC(tpi6525_device::i4_w));
	m_acia->txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	m_acia->dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	m_acia->rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));
	m_acia->rxc_handler().set(RS232_TAG, FUNC(rs232_port_device::write_etc));

	MOS6526A(config, m_cia, XTAL(14'318'181)/14);
	m_cia->set_tod_clock(60);
	m_cia->irq_wr_callback().set(m_tpi1, FUNC(tpi6525_device::i2_w));
	m_cia->cnt_wr_callback().set(m_user, FUNC(cbm2_user_port_device::cnt_w));
	m_cia->sp_wr_callback().set(m_user, FUNC(cbm2_user_port_device::sp_w));
	m_cia->pa_rd_callback().set(FUNC(cbm2_state::cia_pa_r));
	m_cia->pa_wr_callback().set(FUNC(cbm2_state::cia_pa_w));
	m_cia->pb_rd_callback().set(FUNC(cbm2_state::cia_pb_r));
	m_cia->pb_wr_callback().set(m_user, FUNC(cbm2_user_port_device::d2_w));
	m_cia->pc_wr_callback().set(m_user, FUNC(cbm2_user_port_device::pc_w));

	DS75160A(config, m_ieee1, 0);
	m_ieee1->read_callback().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_ieee1->write_callback().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));

	DS75161A(config, m_ieee2, 0);
	m_ieee2->in_ren().set(IEEE488_TAG, FUNC(ieee488_device::ren_r));
	m_ieee2->in_ifc().set(IEEE488_TAG, FUNC(ieee488_device::ifc_r));
	m_ieee2->in_ndac().set(IEEE488_TAG, FUNC(ieee488_device::ndac_r));
	m_ieee2->in_nrfd().set(IEEE488_TAG, FUNC(ieee488_device::nrfd_r));
	m_ieee2->in_dav().set(IEEE488_TAG, FUNC(ieee488_device::dav_r));
	m_ieee2->in_eoi().set(IEEE488_TAG, FUNC(ieee488_device::eoi_r));
	m_ieee2->in_atn().set(IEEE488_TAG, FUNC(ieee488_device::atn_r));
	m_ieee2->in_srq().set(IEEE488_TAG, FUNC(ieee488_device::srq_r));
	m_ieee2->out_ren().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));
	m_ieee2->out_ifc().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_ieee2->out_ndac().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_ieee2->out_nrfd().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_ieee2->out_dav().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_ieee2->out_eoi().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_ieee2->out_atn().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_ieee2->out_srq().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));

	IEEE488(config, m_ieee, 0);
	ieee488_slot_device::add_cbm_defaults(config, "c8050");
	m_ieee->srq_callback().set(m_tpi1, FUNC(tpi6525_device::i1_w));

	PET_DATASSETTE_PORT(config, m_cassette, cbm_datassette_devices, nullptr);
	m_cassette->read_handler().set(m_cia, FUNC(mos6526_device::flag_w));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, nullptr);
	m_joy1->trigger_wr_callback().set(MOS6567_TAG, FUNC(mos6567_device::lp_w));
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, nullptr);

	CBM2_EXPANSION_SLOT(config, m_exp, XTAL(14'318'181)/14, cbm2_expansion_cards, nullptr);

	CBM2_USER_PORT(config, m_user, cbm2_user_port_cards, nullptr);
	m_user->irq_callback().set("mainirq", FUNC(input_merger_device::in_w<1>));
	m_user->sp_callback().set(MOS6526_TAG, FUNC(mos6526_device::sp_w));
	m_user->cnt_callback().set(MOS6526_TAG, FUNC(mos6526_device::cnt_w));
	m_user->flag_callback().set(MOS6526_TAG, FUNC(mos6526_device::flag_w));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

	QUICKLOAD(config, "quickload", "p00,prg", CBM_QUICKLOAD_DELAY).set_load_callback(FUNC(p500_state::quickload_p500));

	// internal ram
	_128k(config);

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("cbm2_cart").set_filter("NTSC");
	SOFTWARE_LIST(config, "flop_list").set_original("p500_flop").set_filter("NTSC");
}


//-------------------------------------------------
//  machine_config( p500_pal )
//-------------------------------------------------

void p500_state::p500_pal(machine_config &config)
{
	MCFG_MACHINE_START_OVERRIDE(p500_state, p500_pal)
	MCFG_MACHINE_RESET_OVERRIDE(p500_state, p500)

	// basic hardware
	M6509(config, m_maincpu, XTAL(17'734'472)/18);
	m_maincpu->set_addrmap(AS_PROGRAM, &p500_state::p500_mem);
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m6509_device::IRQ_LINE);

	// video hardware
	mos6569_device &mos6569(MOS6569(config, MOS6569_TAG, XTAL(17'734'472)/18));
	mos6569.set_cpu(m_maincpu);
	mos6569.irq_callback().set("mainirq", FUNC(input_merger_device::in_w<0>));
	mos6569.set_screen(SCREEN_TAG);
	mos6569.set_addrmap(0, &p500_state::vic_videoram_map);
	mos6569.set_addrmap(1, &p500_state::vic_colorram_map);

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(VIC6569_VRETRACERATE);
	screen.set_size(VIC6569_COLUMNS, VIC6569_LINES);
	screen.set_visarea(0, VIC6569_VISIBLECOLUMNS - 1, 0, VIC6569_VISIBLELINES - 1);
	screen.set_screen_update(MOS6569_TAG, FUNC(mos6569_device::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	MOS6581(config, m_sid, XTAL(17'734'472)/18);
	m_sid->potx().set(FUNC(p500_state::sid_potx_r));
	m_sid->poty().set(FUNC(p500_state::sid_poty_r));
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);

	// devices
	PLS100(config, m_pla1);
	PLS100(config, m_pla2);

	TPI6525(config, m_tpi1, 0);
	m_tpi1->out_irq_cb().set("mainirq", FUNC(input_merger_device::in_w<1>));
	m_tpi1->in_pa_cb().set(FUNC(cbm2_state::tpi1_pa_r));
	m_tpi1->out_pa_cb().set(FUNC(cbm2_state::tpi1_pa_w));
	m_tpi1->in_pb_cb().set(FUNC(cbm2_state::tpi1_pb_r));
	m_tpi1->out_pa_cb().set(FUNC(cbm2_state::tpi1_pb_w));
	m_tpi1->out_ca_cb().set(FUNC(p500_state::tpi1_ca_w));
	m_tpi1->out_cb_cb().set(FUNC(p500_state::tpi1_cb_w));

	TPI6525(config, m_tpi2, 0);
	m_tpi2->out_pa_cb().set(FUNC(cbm2_state::tpi2_pa_w));
	m_tpi2->out_pb_cb().set(FUNC(cbm2_state::tpi2_pb_w));
	m_tpi2->in_pc_cb().set(FUNC(p500_state::tpi2_pc_r));
	m_tpi2->out_pc_cb().set(FUNC(p500_state::tpi2_pc_w));

	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(XTAL(1'843'200));
	m_acia->irq_handler().set(m_tpi1, FUNC(tpi6525_device::i4_w));
	m_acia->txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));

	MOS6526A(config, m_cia, XTAL(17'734'472)/18);
	m_cia->set_tod_clock(50);
	m_cia->irq_wr_callback().set(m_tpi1, FUNC(tpi6525_device::i2_w));
	m_cia->cnt_wr_callback().set(m_user, FUNC(cbm2_user_port_device::cnt_w));
	m_cia->sp_wr_callback().set(m_user, FUNC(cbm2_user_port_device::sp_w));
	m_cia->pa_rd_callback().set(FUNC(cbm2_state::cia_pa_r));
	m_cia->pa_wr_callback().set(FUNC(cbm2_state::cia_pa_w));
	m_cia->pb_rd_callback().set(FUNC(cbm2_state::cia_pb_r));
	m_cia->pb_wr_callback().set(m_user, FUNC(cbm2_user_port_device::d2_w));
	m_cia->pc_wr_callback().set(m_user, FUNC(cbm2_user_port_device::pc_w));

	DS75160A(config, m_ieee1, 0);
	m_ieee1->read_callback().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_ieee1->write_callback().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));

	DS75161A(config, m_ieee2, 0);
	m_ieee2->in_ren().set(IEEE488_TAG, FUNC(ieee488_device::ren_r));
	m_ieee2->in_ifc().set(IEEE488_TAG, FUNC(ieee488_device::ifc_r));
	m_ieee2->in_ndac().set(IEEE488_TAG, FUNC(ieee488_device::ndac_r));
	m_ieee2->in_nrfd().set(IEEE488_TAG, FUNC(ieee488_device::nrfd_r));
	m_ieee2->in_dav().set(IEEE488_TAG, FUNC(ieee488_device::dav_r));
	m_ieee2->in_eoi().set(IEEE488_TAG, FUNC(ieee488_device::eoi_r));
	m_ieee2->in_atn().set(IEEE488_TAG, FUNC(ieee488_device::atn_r));
	m_ieee2->in_srq().set(IEEE488_TAG, FUNC(ieee488_device::srq_r));
	m_ieee2->out_ren().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));
	m_ieee2->out_ifc().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_ieee2->out_ndac().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_ieee2->out_nrfd().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_ieee2->out_dav().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_ieee2->out_eoi().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_ieee2->out_atn().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_ieee2->out_srq().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));

	IEEE488(config, m_ieee, 0);
	ieee488_slot_device::add_cbm_defaults(config, "c8050");
	m_ieee->srq_callback().set(m_tpi1, FUNC(tpi6525_device::i1_w));

	PET_DATASSETTE_PORT(config, m_cassette, cbm_datassette_devices, nullptr);
	m_cassette->read_handler().set(m_cia, FUNC(mos6526_device::flag_w));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, nullptr);
	m_joy1->trigger_wr_callback().set(MOS6567_TAG, FUNC(mos6567_device::lp_w));
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, nullptr);

	CBM2_EXPANSION_SLOT(config, m_exp, XTAL(17'734'472)/18, cbm2_expansion_cards, nullptr);

	CBM2_USER_PORT(config, m_user, cbm2_user_port_cards, nullptr);
	m_user->irq_callback().set("mainirq", FUNC(input_merger_device::in_w<2>));
	m_user->sp_callback().set(MOS6526_TAG, FUNC(mos6526_device::sp_w));
	m_user->cnt_callback().set(MOS6526_TAG, FUNC(mos6526_device::cnt_w));
	m_user->flag_callback().set(MOS6526_TAG, FUNC(mos6526_device::flag_w));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

	QUICKLOAD(config, "quickload", "p00,prg", CBM_QUICKLOAD_DELAY).set_load_callback(FUNC(p500_state::quickload_p500));

	// internal ram
	_128k(config);

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("cbm2_cart").set_filter("PAL");
	SOFTWARE_LIST(config, "flop_list").set_original("p500_flop").set_filter("PAL");
}


//-------------------------------------------------
//  machine_config( cbm2lp_ntsc )
//-------------------------------------------------

void cbm2_state::cbm2lp_ntsc(machine_config &config)
{
	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2_ntsc)
	MCFG_MACHINE_RESET_OVERRIDE(cbm2_state, cbm2)

	// basic hardware
	M6509(config, m_maincpu, XTAL(18'000'000)/9);
	m_maincpu->set_addrmap(AS_PROGRAM, &cbm2_state::cbm2_mem);
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m6509_device::IRQ_LINE);

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_screen_update(MC68B45_TAG, FUNC(mc6845_device::screen_update));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(768, 312);
	screen.set_visarea(0, 768-1, 0, 312-1);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	MC6845(config, m_crtc, XTAL(18'000'000)/9);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(cbm2_state::crtc_update_row));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	MOS6581(config, m_sid, XTAL(18'000'000)/9);
	m_sid->potx().set(FUNC(p500_state::sid_potx_r));
	m_sid->poty().set(FUNC(p500_state::sid_poty_r));
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);

	// devices
	PLS100(config, m_pla1);

	TPI6525(config, m_tpi1, 0);
	m_tpi1->out_irq_cb().set("mainirq", FUNC(input_merger_device::in_w<1>));
	m_tpi1->in_pa_cb().set(FUNC(cbm2_state::tpi1_pa_r));
	m_tpi1->out_pa_cb().set(FUNC(cbm2_state::tpi1_pa_w));
	m_tpi1->in_pb_cb().set(FUNC(cbm2_state::tpi1_pb_r));
	m_tpi1->out_pb_cb().set(FUNC(cbm2_state::tpi1_pb_w));
	m_tpi1->out_ca_cb().set(FUNC(cbm2_state::tpi1_ca_w));

	TPI6525(config, m_tpi2, 0);
	m_tpi2->out_pa_cb().set(FUNC(cbm2_state::tpi2_pa_w));
	m_tpi2->out_pb_cb().set(FUNC(cbm2_state::tpi2_pb_w));
	m_tpi2->in_pc_cb().set(FUNC(cbm2_state::tpi2_pc_r));

	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(XTAL(1'843'200));
	m_acia->irq_handler().set(m_tpi1, FUNC(tpi6525_device::i4_w));
	m_acia->txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));

	MOS6526A(config, m_cia, XTAL(18'000'000)/9);
	m_cia->set_tod_clock(60);
	m_cia->irq_wr_callback().set(m_tpi1, FUNC(tpi6525_device::i2_w));
	m_cia->cnt_wr_callback().set(m_user, FUNC(cbm2_user_port_device::cnt_w));
	m_cia->sp_wr_callback().set(m_user, FUNC(cbm2_user_port_device::sp_w));
	m_cia->pa_rd_callback().set(FUNC(cbm2_state::cia_pa_r));
	m_cia->pa_wr_callback().set(FUNC(cbm2_state::cia_pa_w));
	m_cia->pb_rd_callback().set(FUNC(cbm2_state::cia_pb_r));
	m_cia->pb_wr_callback().set(m_user, FUNC(cbm2_user_port_device::d2_w));
	m_cia->pc_wr_callback().set(m_user, FUNC(cbm2_user_port_device::pc_w));

	DS75160A(config, m_ieee1, 0);
	m_ieee1->read_callback().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_ieee1->write_callback().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));

	DS75161A(config, m_ieee2, 0);
	m_ieee2->in_ren().set(IEEE488_TAG, FUNC(ieee488_device::ren_r));
	m_ieee2->in_ifc().set(IEEE488_TAG, FUNC(ieee488_device::ifc_r));
	m_ieee2->in_ndac().set(IEEE488_TAG, FUNC(ieee488_device::ndac_r));
	m_ieee2->in_nrfd().set(IEEE488_TAG, FUNC(ieee488_device::nrfd_r));
	m_ieee2->in_dav().set(IEEE488_TAG, FUNC(ieee488_device::dav_r));
	m_ieee2->in_eoi().set(IEEE488_TAG, FUNC(ieee488_device::eoi_r));
	m_ieee2->in_atn().set(IEEE488_TAG, FUNC(ieee488_device::atn_r));
	m_ieee2->in_srq().set(IEEE488_TAG, FUNC(ieee488_device::srq_r));
	m_ieee2->out_ren().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));
	m_ieee2->out_ifc().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_ieee2->out_ndac().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_ieee2->out_nrfd().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_ieee2->out_dav().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_ieee2->out_eoi().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_ieee2->out_atn().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_ieee2->out_srq().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));

	IEEE488(config, m_ieee, 0);
	ieee488_slot_device::add_cbm_defaults(config, "c8050");
	m_ieee->srq_callback().set(m_tpi1, FUNC(tpi6525_device::i1_w));

	PET_DATASSETTE_PORT(config, m_cassette, cbm_datassette_devices, nullptr);
	m_cassette->read_handler().set(m_cia, FUNC(mos6526_device::flag_w));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, nullptr);
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, nullptr);

	CBM2_EXPANSION_SLOT(config, m_exp, XTAL(18'000'000)/9, cbm2_expansion_cards, nullptr);

	CBM2_USER_PORT(config, m_user, cbm2_user_port_cards, nullptr);
	m_user->irq_callback().set("mainirq", FUNC(input_merger_device::in_w<2>));
	m_user->sp_callback().set(MOS6526_TAG, FUNC(mos6526_device::sp_w));
	m_user->cnt_callback().set(MOS6526_TAG, FUNC(mos6526_device::cnt_w));
	m_user->flag_callback().set(MOS6526_TAG, FUNC(mos6526_device::flag_w));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

	QUICKLOAD(config, "quickload", "p00,prg,t64", CBM_QUICKLOAD_DELAY).set_load_callback(FUNC(cbm2_state::quickload_cbmb));

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("cbm2_cart").set_filter("NTSC");
	SOFTWARE_LIST(config, "flop_list").set_original("cbm2_flop").set_filter("NTSC");
}


//-------------------------------------------------
//  machine_config( b128 )
//-------------------------------------------------

void cbm2_state::b128(machine_config &config)
{
	cbm2lp_ntsc(config);
	_128k(config);
}


//-------------------------------------------------
//  machine_config( b256 )
//-------------------------------------------------

void cbm2_state::b256(machine_config &config)
{
	cbm2lp_ntsc(config);
	_256k(config);
}


//-------------------------------------------------
//  machine_config( cbm2lp_pal )
//-------------------------------------------------

void cbm2_state::cbm2lp_pal(machine_config &config)
{
	cbm2lp_ntsc(config);
	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2_pal)
	m_cia->set_tod_clock(50);
}


//-------------------------------------------------
//  machine_config( cbm610 )
//-------------------------------------------------

void cbm2_state::cbm610(machine_config &config)
{
	cbm2lp_pal(config);
	_128k(config);
}


//-------------------------------------------------
//  machine_config( cbm620 )
//-------------------------------------------------

void cbm2_state::cbm620(machine_config &config)
{
	cbm2lp_pal(config);
	_256k(config);
}


//-------------------------------------------------
//  machine_config( cbm2hp_ntsc )
//-------------------------------------------------

void cbm2_state::cbm2hp_ntsc(machine_config &config)
{
	cbm2lp_ntsc(config);
	m_tpi2->in_pc_cb().set(FUNC(cbm2hp_state::tpi2_pc_r));
}


//-------------------------------------------------
//  machine_config( b128hp )
//-------------------------------------------------

void cbm2hp_state::b128hp(machine_config &config)
{
	cbm2hp_ntsc(config);
	_128k(config);
}


//-------------------------------------------------
//  machine_config( b256hp )
//-------------------------------------------------

void cbm2hp_state::b256hp(machine_config &config)
{
	cbm2hp_ntsc(config);
	_256k(config);
}


//-------------------------------------------------
//  machine_config( bx256hp )
//-------------------------------------------------

void cbm2hp_state::bx256hp(machine_config &config)
{
	b256hp(config);
	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2x_ntsc)

	I8088(config, m_ext_cpu, XTAL(12'000'000));
	m_ext_cpu->set_addrmap(AS_PROGRAM, &cbm2hp_state::ext_mem);
	m_ext_cpu->set_addrmap(AS_IO, &cbm2hp_state::ext_io);
	m_ext_cpu->set_irq_acknowledge_callback(EXT_I8259A_TAG, FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_ext_pic, 0);
	m_ext_pic->out_int_callback().set_inputline(m_ext_cpu, INPUT_LINE_IRQ0);

	TPI6525(config, m_ext_tpi, 0);
	m_ext_tpi->in_pa_cb().set(m_ext_cia, FUNC(mos6526_device::pa_r));
	m_ext_tpi->in_pb_cb().set(FUNC(cbm2_state::ext_tpi_pb_r));
	m_ext_tpi->out_pb_cb().set(FUNC(cbm2_state::ext_tpi_pb_w));
	m_ext_tpi->out_pc_cb().set(FUNC(cbm2_state::ext_tpi_pc_w));

	MOS6526(config, m_ext_cia, XTAL(18'000'000)/9);
	m_ext_cia->set_tod_clock(60);
	m_ext_cia->irq_wr_callback().set(FUNC(cbm2_state::ext_cia_irq_w));
	m_ext_cia->pa_rd_callback().set(m_ext_tpi, FUNC(tpi6525_device::pa_r));
	m_ext_cia->pb_rd_callback().set(FUNC(cbm2_state::ext_cia_pb_r));
	m_ext_cia->pb_wr_callback().set(FUNC(cbm2_state::ext_cia_pb_w));

	SOFTWARE_LIST(config, "flop_list2").set_original("bx256hp_flop");
}


//-------------------------------------------------
//  machine_config( cbm2hp_pal )
//-------------------------------------------------

void cbm2_state::cbm2hp_pal(machine_config &config)
{
	cbm2hp_ntsc(config);
	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2_pal)

	// devices
	m_tpi2->in_pc_cb().set(FUNC(cbm2hp_state::tpi2_pc_r));
	m_cia->set_tod_clock(50);
}


//-------------------------------------------------
//  machine_config( cbm710 )
//-------------------------------------------------

void cbm2hp_state::cbm710(machine_config &config)
{
	cbm2hp_pal(config);
	_128k(config);
}


//-------------------------------------------------
//  machine_config( cbm720 )
//-------------------------------------------------

void cbm2hp_state::cbm720(machine_config &config)
{
	cbm2hp_pal(config);
	_256k(config);
}


//-------------------------------------------------
//  machine_config( cbm730 )
//-------------------------------------------------

void cbm2hp_state::cbm730(machine_config &config)
{
	cbm720(config);
	MCFG_MACHINE_START_OVERRIDE(cbm2_state, cbm2x_pal)

	I8088(config, m_ext_cpu, XTAL(12'000'000));
	m_ext_cpu->set_addrmap(AS_PROGRAM, &cbm2hp_state::ext_mem);
	m_ext_cpu->set_addrmap(AS_IO, &cbm2hp_state::ext_io);
	m_ext_cpu->set_irq_acknowledge_callback(EXT_I8259A_TAG, FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_ext_pic, 0);
	m_ext_pic->out_int_callback().set_inputline(m_ext_cpu, INPUT_LINE_IRQ0);

	TPI6525(config, m_ext_tpi, 0);
	m_ext_tpi->in_pa_cb().set(EXT_MOS6526_TAG, FUNC(mos6526_device::pa_r));
	m_ext_tpi->in_pb_cb().set(FUNC(cbm2_state::ext_tpi_pb_r));
	m_ext_tpi->out_pb_cb().set(FUNC(cbm2_state::ext_tpi_pb_w));
	m_ext_tpi->out_pc_cb().set(FUNC(cbm2_state::ext_tpi_pc_w));

	MOS6526(config, m_ext_cia, XTAL(18'000'000)/9);
	m_ext_cia->set_tod_clock(50);
	m_ext_cia->irq_wr_callback().set(FUNC(cbm2_state::ext_cia_irq_w));
	m_ext_cia->pa_rd_callback().set(m_ext_tpi, FUNC(tpi6525_device::pa_r));
	m_ext_cia->pb_rd_callback().set(FUNC(cbm2_state::ext_cia_pb_r));
	m_ext_cia->pb_wr_callback().set(FUNC(cbm2_state::ext_cia_pb_w));

	SOFTWARE_LIST(config, "flop_list2").set_original("bx256hp_flop");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( p500 )
//-------------------------------------------------

ROM_START( p500 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )

	ROM_REGION( 0x4000, "basic", 0 )
	ROMX_LOAD( "901236-01.u84", 0x0000, 0x2000, CRC(33eb6aa2) SHA1(7e3497ae2edbb38c753bd31ed1bf3ae798c9a976), ROM_BIOS(0) )
	ROMX_LOAD( "901235-01.u83", 0x2000, 0x2000, CRC(18a27feb) SHA1(951b5370dd7db762b8504a141f9f26de345069bb), ROM_BIOS(0) )
	ROMX_LOAD( "901236-02.u84", 0x0000, 0x2000, CRC(c62ab16f) SHA1(f50240407bade901144f7e9f489fa9c607834eca), ROM_BIOS(1) )
	ROMX_LOAD( "901235-02.u83", 0x2000, 0x2000, CRC(20b7df33) SHA1(1b9a55f12f8cf025754d8029cc5324b474c35841), ROM_BIOS(1) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROMX_LOAD( "901234-01.u82", 0x0000, 0x2000, CRC(67962025) SHA1(24b41b65c85bf30ab4e2911f677ce9843845b3b1), ROM_BIOS(0) )
	ROMX_LOAD( "901234-02.u82", 0x0000, 0x2000, CRC(f46bbd2b) SHA1(097197d4d08e0b82e0466a5f1fbd49a24f3d2523), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u76", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-02.u78", 0x00, 0xf5, CRC(6436b20b) SHA1(57ebebe771791288051afd1abe9b7500bd2df847) )

	ROM_REGION( 0xf5, PLA2_TAG, 0 )
	ROM_LOAD( "906114-03.u88", 0x00, 0xf5, CRC(668c073e) SHA1(1115858bb2dc91ea9e2016ba2e23ec94239358b4) )
ROM_END

#define rom_p500p   rom_p500


//-------------------------------------------------
//  ROM( b500 )
//-------------------------------------------------

ROM_START( b500 )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901243-01.u59",  0x0000, 0x2000, CRC(22822706) SHA1(901bbf59d8b8682b481be8b2de99b406fffa4bab) )
	ROM_LOAD( "901242-01a.u60", 0x2000, 0x2000, CRC(ef13d595) SHA1(2fb72985d7d4ab69c5780179178828c931a9f5b0) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "901244-01.u61",  0x0000, 0x2000, CRC(93414213) SHA1(a54a593dbb420ae1ac39b0acde9348160f7840ff) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901237-01.u25", 0x0000, 0x1000, CRC(1acf5098) SHA1(e63bf18da48e5a53c99ef127c1ae721333d1d102) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-04.u18", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END


//-------------------------------------------------
//  ROM( b128 )
//-------------------------------------------------

ROM_START( b128 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )

	ROM_REGION( 0x4000, "basic", 0 )
	ROMX_LOAD( "901243-02b.u59", 0x0000, 0x2000, CRC(9d0366f9) SHA1(625f7337ea972a8bce2bdf2daababc0ed0b3b69b), ROM_BIOS(0) )
	ROMX_LOAD( "901242-02b.u60", 0x2000, 0x2000, CRC(837978b5) SHA1(56e8d2f86bf73ba36b3d3cb84dd75806b66c530a), ROM_BIOS(0) )
	ROMX_LOAD( "901243-04a.u59", 0x0000, 0x2000, CRC(b0dcb56d) SHA1(08d333208060ee2ce84d4532028d94f71c016b96), ROM_BIOS(1) )
	ROMX_LOAD( "901242-04a.u60", 0x2000, 0x2000, CRC(de04ea4f) SHA1(7c6de17d46a3343dc597d9b9519cf63037b31908), ROM_BIOS(1) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROMX_LOAD( "901244-03b.u61", 0x0000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(0) )
	ROMX_LOAD( "901244-04a.u61", 0x0000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901237-01.u25", 0x0000, 0x1000, CRC(1acf5098) SHA1(e63bf18da48e5a53c99ef127c1ae721333d1d102) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-04.u18", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END

#define rom_cbm610  rom_b128
#define rom_cbm620  rom_b256


//-------------------------------------------------
//  ROM( b256 )
//-------------------------------------------------

ROM_START( b256 )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "901244-03b.u61", 0x0000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "901244-04a.u61", 0x0000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901237-01.u25", 0x0000, 0x1000, CRC(1acf5098) SHA1(e63bf18da48e5a53c99ef127c1ae721333d1d102) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-04.u18", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END


//-------------------------------------------------
//  ROM( cbm620_hu )
//-------------------------------------------------

ROM_START( cbm620_hu )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "610.u60", 0x0000, 0x4000, CRC(8eed0d7e) SHA1(9d06c5c3c012204eaaef8b24b1801759b62bf57e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "kernhun.u61", 0x0000, 0x2000, CRC(0ea8ca4d) SHA1(9977c9f1136ee9c04963e0b50ae0c056efa5663f) )

	ROM_REGION( 0x2000, "charom", 0 )
	ROM_LOAD( "charhun.u25", 0x0000, 0x2000, CRC(1fb5e596) SHA1(3254e069f8691b30679b19a9505b6afdfedce6ac) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-04.u18", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END


//-------------------------------------------------
//  ROM( b128hp )
//-------------------------------------------------

ROM_START( b128hp )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )

	ROM_REGION( 0x4000, "basic", 0 )
	ROMX_LOAD( "901243-02b.u59", 0x0000, 0x2000, CRC(9d0366f9) SHA1(625f7337ea972a8bce2bdf2daababc0ed0b3b69b), ROM_BIOS(0) )
	ROMX_LOAD( "901242-02b.u60", 0x2000, 0x2000, CRC(837978b5) SHA1(56e8d2f86bf73ba36b3d3cb84dd75806b66c530a), ROM_BIOS(0) )
	ROMX_LOAD( "901243-04a.u59", 0x0000, 0x2000, CRC(b0dcb56d) SHA1(08d333208060ee2ce84d4532028d94f71c016b96), ROM_BIOS(1) )
	ROMX_LOAD( "901242-04a.u60", 0x2000, 0x2000, CRC(de04ea4f) SHA1(7c6de17d46a3343dc597d9b9519cf63037b31908), ROM_BIOS(1) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROMX_LOAD( "901244-03b.u61", 0x0000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(0) )
	ROMX_LOAD( "901244-04a.u61", 0x0000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901232-01.u25", 0x0000, 0x1000, CRC(3a350bc3) SHA1(e7f3cbc8e282f79a00c3e95d75c8d725ee3c6287) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-05.u75", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END

#define rom_cbm710  rom_b128hp


//-------------------------------------------------
//  ROM( b256hp )
//-------------------------------------------------

ROM_START( b256hp )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "901244-03b.u61", 0x0000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "901244-04a.u61", 0x0000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901232-01.u25", 0x0000, 0x1000, CRC(3a350bc3) SHA1(e7f3cbc8e282f79a00c3e95d75c8d725ee3c6287) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-05.u75", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END

#define rom_cbm720  rom_b256hp


//-------------------------------------------------
//  ROM( bx256hp )
//-------------------------------------------------

ROM_START( bx256hp )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x1000, EXT_I8088_TAG, 0)
	ROM_LOAD( "8088.u14", 0x0000, 0x1000, CRC(195e0281) SHA1(ce8acd2a5fb6cbd70d837811d856d656544a1f97) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "901244-03b.u61", 0x0000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "901244-04a.u61", 0x0000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901232-01.u25", 0x0000, 0x1000, CRC(3a350bc3) SHA1(e7f3cbc8e282f79a00c3e95d75c8d725ee3c6287) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-05.u75", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END

#define rom_cbm730  rom_bx256hp


//-------------------------------------------------
//  ROM( cbm720_de )
//-------------------------------------------------

ROM_START( cbm720_de )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "324866-03a.u61", 0x0000, 0x2000, CRC(554b008d) SHA1(1483a46924308d86f4c7f9cb71c34851c510fcf4) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "324867-02.u25", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-05.u75", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END


//-------------------------------------------------
//  ROM( cbm720_se )
//-------------------------------------------------

ROM_START( cbm720_se )
	ROM_REGION( 0x4000, "basic", 0 )
	ROM_LOAD( "901241-03.u59", 0x0000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0x2000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "swe-901244-03.u61", 0x0000, 0x2000, CRC(87bc142b) SHA1(fa711f6082741b05a9c80744f5aee68dc8c1dcf4) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901233-03.u25", 0x0000, 0x1000, CRC(09518b19) SHA1(2e28491e31e2c0a3b6db388055216140a637cd09) )

	ROM_REGION( 0xf5, PLA1_TAG, 0 )
	ROM_LOAD( "906114-05.u75", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT    CLASS         INIT    COMPANY                         FULLNAME                    FLAGS
COMP( 1983, p500,      0,      0,      p500_ntsc, cbm2,    p500_state,   empty_init, "Commodore Business Machines",  "P500 (NTSC)",              MACHINE_SUPPORTS_SAVE )
COMP( 1983, p500p,     p500,   0,      p500_pal,  cbm2,    p500_state,   empty_init, "Commodore Business Machines",  "P500 (PAL)",               MACHINE_SUPPORTS_SAVE )
COMP( 1983, b500,      0,      0,      b128,      cbm2,    cbm2_state,   empty_init, "Commodore Business Machines",  "B500",                     MACHINE_SUPPORTS_SAVE )
COMP( 1983, b128,      b500,   0,      b128,      cbm2,    cbm2_state,   empty_init, "Commodore Business Machines",  "B128",                     MACHINE_SUPPORTS_SAVE )
COMP( 1983, b256,      b500,   0,      b256,      cbm2,    cbm2_state,   empty_init, "Commodore Business Machines",  "B256",                     MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm610,    b500,   0,      cbm610,    cbm2,    cbm2_state,   empty_init, "Commodore Business Machines",  "CBM 610",                  MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm620,    b500,   0,      cbm620,    cbm2,    cbm2_state,   empty_init, "Commodore Business Machines",  "CBM 620",                  MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm620_hu, b500,   0,      cbm620,    cbm2_hu, cbm2_state,   empty_init, "Commodore Business Machines",  "CBM 620 (Hungary)",        MACHINE_SUPPORTS_SAVE )
COMP( 1983, b128hp,    0,      0,      b128hp,    cbm2,    cbm2hp_state, empty_init, "Commodore Business Machines",  "B128-80HP",                MACHINE_SUPPORTS_SAVE )
COMP( 1983, b256hp,    b128hp, 0,      b256hp,    cbm2,    cbm2hp_state, empty_init, "Commodore Business Machines",  "B256-80HP",                MACHINE_SUPPORTS_SAVE )
COMP( 1983, bx256hp,   b128hp, 0,      bx256hp,   cbm2,    cbm2hp_state, empty_init, "Commodore Business Machines",  "BX256-80HP",               MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // 8088 co-processor is missing
COMP( 1983, cbm710,    b128hp, 0,      cbm710,    cbm2,    cbm2hp_state, empty_init, "Commodore Business Machines",  "CBM 710",                  MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm720,    b128hp, 0,      cbm720,    cbm2,    cbm2hp_state, empty_init, "Commodore Business Machines",  "CBM 720",                  MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm720_de, b128hp, 0,      cbm720,    cbm2_de, cbm2hp_state, empty_init, "Commodore Business Machines",  "CBM 720 (Germany)",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm720_se, b128hp, 0,      cbm720,    cbm2_se, cbm2hp_state, empty_init, "Commodore Business Machines",  "CBM 720 (Sweden/Finland)", MACHINE_SUPPORTS_SAVE )
COMP( 1983, cbm730,    b128hp, 0,      cbm730,    cbm2,    cbm2hp_state, empty_init, "Commodore Business Machines",  "CBM 730",                  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // 8088 co-processor is missing
