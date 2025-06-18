// license:BSD-3-Clause
// copyright-holders:Philip Bennett

/***************************************************************************

    Atari Metal Maniax

    Preliminary driver

***************************************************************************/

/***************************************************************************

Metal Maniax
Atari Prototype

This is a truly insane board stack. Side view:

 +----(1l)----+              +----(1r)----+
+-----(2l)------------+    +------(2r)------------+
+-----(3l)-------------+   +------(3r)------------+
+-----------------------(4)-----------------------+
+-----------------------(5)-----------------------+

The two bottom boards (4) and (5) are common.
The top stack of 3 boards are duplicated twice.

Here, in detail, is a list of components on each board:

----------------------
Layer 1: (1l) and (1r)
----------------------
Dimensions: 5" x 8"
ID: ROMCH31 A053443 ATARI GAMES (c) 94 MADE IN U.S.A.
Title: "Teenage Kicks Right Through the Night"

Programmable:
16 x 27C040-10 EPROMs, labelled datametl 0-15, 10/19/94 11:08:05
 1 x 27C040-10 EPROM, labelled bootmetl rel 34, 10/19/94 16:22:49
 1 x GAL16V8B, labelled 136103-1500

Logic:
 3 x SN74F245

 |------------------------------------------------------------------|
 |                      datametl4                       datametl12  |
 |      datametl0                       datametl8                   |
 |                      datametl5                       datametl13  |
 |      datametl1                       datametl9                   |
 |                      datametl6                       datametl14  |
 |      datametl2                       datametl10                  |
 |                      datametl7                       datametl15  |
 |      datametl3                       datametl11                  |
 |                                                      bootmetl34  |
 |              SN74F245    SN74F245                                |
 |              136103-1500 SN74F245                                |
 |      |------------------------------|socket                      |
 |      |------------------------------|JROMBUS                     |
 |------------------------------------------------------------------|


----------------------
Layer 2: (2l) and (2r)
----------------------
Dimensions: 5.5" x 8"
ID: CH31.2 A053304 ATARI GAMES (c) 93 MADE IN U.S.A.
Title: "Silly Putty: Thixotropic or Dilatent"

Programmable:
 4 x unpopulated 42-pin sockets (11B-11E)
 1 x unpopulated 32-pin socket (11A)
 1 x GAL16V8A-25, labelled DSP
 1 x GAL16V8A-25, labelled IRQ2
 1 x GAL16V8A-25, labelled XDEC

CPU/DSP:
 1 x TMS320C31PQL DSP
 1 x 33.8688MHz crystal

RAM:
 4 x CY7C199-20 32k*8 SRAM

DAC:
 2 x Asahi Kasei AK4316-VS

Logic:
 1 x SN74F138
 4 x SN74LS374

 |--------------------------------------------------|
 |                                                  |
 |                                                  |
 |  JXBUS                                           |
 |  ||          AK4136-VS   AK4136-VS               |
 |  ||                                              |
 |  ||                                              |
 |  ||  SN74LS374   CY7C199     --------            |
 |  ||  SN74LS374   CY7C199     |320C31|    SN74F138|
 |  ||  SN74LS374   CY7C199     |      |            |
 |  ||  SN74LS374   CY7C199     --------        LED |
 |  ||                                  XDEC        |
 |  ||  11A   11B   11C   11D   11E     IRQ2Q       |
 |                                              LED |
 |                                      DSP         |
 |                                          33.8MHz |
 |      |------------------------------|socket      |
 |      |------------------------------|JROMBUS     |
 |--------------------------------------------------|


----------------------
Layer 3: (3l) and (3r)
----------------------
Dimensions: 9" x 12"
ID: MCUBE A052137 ATARI GAMES (c) 93

Programmable:
 4 x 27C040-10 EPROMs, labelled tgs665e 0-3, 9/28/94 15:53:52
 4 x 27C040-10 EPROMs, labelled st665e 0-3, 9/28/94 15:56:02
 1 x 27C512-IJL EPROM, labelled MTL MANIAX 136103-2308PP
 1 x GAL20V8B-15, labelled 136103-1300

CPU/DSP:
 1 x ADSP-2105 KP-40 DSP
 1 x 10.000MHz crystal
 1 x MC68EC020FG16 CPU
 1 x 14.318180MHz crystal

RAM:
 1 x MK48Z02B-15 ZEROPOWER RAM
 1 x MK48T02B-15 TIMEKEEPER RAM
 4 x MOSEL 62256H-20NC 32k*8 SRAM

Unknown:
 1 x XILINX FPGA XC3142-3 TQ144C X25839M AIG9326
 1 x XILINX 1736DPC, labelled 3302 (checksum?)
 1 x DALLAS DS1232 MicroMonitor

ADCs:
 1 x AD7582KN 12-bit ADC
 1 x ADC0809CCN 8-bit ADC with 8-way mux

Logic:
 1 x SN74F138
 7 x SN74F244
15 x SN74F245
 1 x SN74LS240
 3 x SN74LS259
 5 x SN74LS374
 1 x SN75ALS194
 1 x SN75ALS195
 1 x 74F04PC

Misc:
 1 x 8-position DIP switch
 3 x green LEDs
 2 x red LEDs


------------
Layer 4: (4)
------------
Dimensions: 8" x 18"
ID: TGSMATH A052043 ATARI GAMES (c) 93 MADE IN U.S.A.

Programmable:
 2 x GAL22V10B, labelled 136103-1200
 1 x GAL22V10B, labelled 136103-1202
 1 x GAL22V10B, labelled 136103-1203
 1 x GAL22V10B, labelled 136103-1204
 1 x GAL22V10B, labelled 136103-1205
 1 x GAL22V10B, labelled 136103-1218
 1 x GAL20V8B, labelled 136103-1217
 4 x GAL16V8B, labelled 136103-1219

CPU/DSP:
 4 x DSP32C F33 DSPs, labelled Freda, Gus, Herb, Irene

RAM:
32 x MOSEL 62256H-20NC 32k*8 SRAM
16 x MT5C1008-20 128k*8 SRAM

Logic:
 4 x 74F04PC
 1 x SN74LS74
 2 x SN74LS174
16 x SN74ALS373
 2 x SN74ALS374


------------
Layer 5: (5)
------------
Dimensions: 12" x 18"
ID: ATARI GAMES (c) 93 MADE IN U.S.A. TGS A051985

Programmable:
 8 x 27C040-10, labelled TEX0-7 0
 8 x 27C040-10, labelled TEX0-7 1
 2 x GAL16V8B, labelled 136103-1114
 1 x GAL16V8B, labelled 136103-1116
 1 x GAL22V10B, labelled 136103-1106
 1 x GAL22V10B, labelled 136103-1107
 2 x GAL22V10B, labelled 136103-1108
 2 x GAL22V10B, labelled 136103-1111
 2 x GAL22V10B, labelled 136103-1112
 2 x N82S147 512*8 BPROM, labelled RED
 2 x N82S147 512*8 BPROM, labelled GREEN
 2 x N82S147 512*8 BPROM, labelled BLUE

CPU/DSP:
 2 x TMS34020-40
 1 x unknown crystal
 1 x 16.00000MHz crystal

RAM:
 4 x TMS45160DZ-80 256k*16 DRAM
 4 x TMS55160DGH-70 256k*16 VRAM

Unknown:
 1 x HP FPGA 1FY5-0003 9351-HONG KONG NPFY5B5880
 1 x XILINX FPGA XC3190-3 PQ160C X35728M AIG9427
 1 x XILINX FPGA CX4003-6 PQ100C X25788M ASG9325
 2 x XILINX 1765DPC

Logic:
 8 x SN74BCT652
20 x DM74ALS541
 5 x ACT7814-25 64x18 strobed FIFO
 1 x 74F04PC
 4 x 74F273
 1 x SN74ALS2232
 6 x 74HCT374
 1 x 74ALS164


    The board stack drives a two-seat cabinet.
    Each side uses the following CPUs:

    Main:         68EC020
    Network:      ADSP-2105
    2D Engine:    TMS34020
    3D Math:      2 x DSP32C
    Sound (CAGE): TMS320C31

    Two cabinets can be linked together to support 4 players.

    Interrupts:
    IRQ2 = ?
    IRQ3 = ?
    IRQ4 = TMS34020
    IRQ5 = Network
    IRQ6 = Timer

   2/3 are either CAGE or the Mathbox CPUs

***************************************************************************/

#include "emu.h"

#include "cage.h"

#include "cpu/adsp2100/adsp2100.h"
#include "cpu/m68000/m68020.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/dsp32/dsp32.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class metalmx_state : public driver_device
{
public:
	metalmx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gsp(*this, "gsp"),
		m_adsp(*this, "adsp"),
		m_dsp32c(*this, "dsp32c_%u", 1U),
		m_cage(*this, "cage"),
		m_adsp_internal_program_ram(*this, "adsp_intprog"),
		m_gsp_dram(*this, "gsp_dram"),
		m_gsp_vram(*this, "gsp_vram")
	{ }

	void init_metalmx();
	void metalmx(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<m68ec020_device> m_maincpu;
	required_device<tms34020_device> m_gsp;
	required_device<adsp2105_device> m_adsp;
	required_device_array<dsp32c_device, 2> m_dsp32c;
	required_device<atari_cage_device> m_cage;

	required_shared_ptr<uint32_t> m_adsp_internal_program_ram;
	required_shared_ptr<uint32_t> m_gsp_dram;
	required_shared_ptr<uint32_t> m_gsp_vram;

	uint32_t unk_r();
	uint32_t watchdog_r();
	void shifter_w(uint32_t data);
	void motor_w(uint32_t data);
	void reset_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sound_data_r(offs_t offset, uint32_t mem_mask = ~0);
	void sound_data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Chip> void dsp32c_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Chip> uint32_t dsp32c_r(offs_t offset, uint32_t mem_mask = ~0);
	void host_gsp_w(offs_t offset, uint32_t data);
	uint32_t host_gsp_r(offs_t offset);
	uint32_t host_dram_r(offs_t offset);
	void host_dram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t host_vram_r(offs_t offset);
	void host_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void timer_w(offs_t offset, uint32_t data);
	void cage_irq_callback(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void adsp_data_map(address_map &map) ATTR_COLD;
	void adsp_program_map(address_map &map) ATTR_COLD;
	void dsp32c_1_map(address_map &map) ATTR_COLD;
	void dsp32c_2_map(address_map &map) ATTR_COLD;
	void gsp_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Forward definitions
 *
 *************************************/




/*************************************
 *
 *  Video hardware (move to /video)
 *
 *************************************/

void metalmx_state::video_start()
{
}

uint32_t metalmx_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// TODO: TMS34020 should take care of this

//  uint32_t *src_base = &gsp_vram[(vreg_base[0x40/4] & 0x40) ? 0x20000 : 0];
	uint32_t const *const src_base = &m_gsp_vram[0];

	for (int y = (std::max)(0, cliprect.min_y); y <= (std::min)(383, cliprect.max_y); ++y)
	{
		uint16_t *pix = &bitmap.pix(y);
		for (int x = 0; x < 256; x++)
		{
			*pix++ = src_base[256 * y + x] & 0xffff;
			*pix++ = src_base[256 * y + x] >> 16;
		}
	}

	return 0;
}


/*************************************
 *
 *  Miscellany
 *
 *************************************/

uint32_t metalmx_state::unk_r()
{
	return 0;//machine().rand();
}

uint32_t metalmx_state::watchdog_r()
{
	return 0xffffffff;
}

void metalmx_state::shifter_w(uint32_t data)
{
}

void metalmx_state::motor_w(uint32_t data)
{
}

void metalmx_state::reset_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		data >>= 16;
		m_dsp32c[0]->set_input_line(INPUT_LINE_RESET, data & 2 ? CLEAR_LINE : ASSERT_LINE);
		m_dsp32c[1]->set_input_line(INPUT_LINE_RESET, data & 1 ? CLEAR_LINE : ASSERT_LINE);
	}
}


/*************************************
 *
 *  Sound I/O
 *
 *************************************/

uint32_t metalmx_state::sound_data_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;

	if (ACCESSING_BITS_0_15)
		result |= m_cage->control_r();
	if (ACCESSING_BITS_16_31)
		result |= m_cage->main_r() << 16;
	return result;
}

void metalmx_state::sound_data_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_cage->control_w(data);
	if (ACCESSING_BITS_16_31)
		m_cage->main_w(data >> 16);
}

void metalmx_state::cage_irq_callback(uint8_t data)
{
	/* TODO */
}

/*************************************
 *
 *  Host/DSP32C parallel interface
 *
 *************************************/

template<int Chip>
void metalmx_state::dsp32c_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	offset <<= 1;

	if (ACCESSING_BITS_0_15)
		offset += 1;
	else if (ACCESSING_BITS_16_31)
		data >>= 16;

	m_dsp32c[Chip]->pio_w(offset, data);
}

template<int Chip>
uint32_t metalmx_state::dsp32c_r(offs_t offset, uint32_t mem_mask)
{
	offset <<= 1;

	if (ACCESSING_BITS_0_15)
		offset += 1;

	uint32_t data = m_dsp32c[Chip]->pio_r(offset);

	if (ACCESSING_BITS_16_31)
		data <<= 16;

	return data;
}


/*************************************
 *
 *  Host/TMS34020 accessors
 *
 *************************************/

void metalmx_state::host_gsp_w(offs_t offset, uint32_t data)
{
	address_space &gsp_space = m_gsp->space(AS_PROGRAM);

	gsp_space.write_dword(0xc0000000 + (offset << 5), (data << 16) | (data >> 16));
}

uint32_t metalmx_state::host_gsp_r(offs_t offset)
{
	address_space &gsp_space = m_gsp->space(AS_PROGRAM);

	uint32_t val = gsp_space.read_dword(0xc0000000 + (offset << 5));
	return (val << 16) | (val >> 16);
}


uint32_t metalmx_state::host_dram_r(offs_t offset)
{
	return (m_gsp_dram[offset] << 16) | (m_gsp_dram[offset] >> 16);
}

void metalmx_state::host_dram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = (data << 16) | (data >> 16);
	mem_mask = (mem_mask << 16) | (mem_mask >> 16);
	COMBINE_DATA(&m_gsp_dram[offset]);
}

uint32_t metalmx_state::host_vram_r(offs_t offset)
{
	return (m_gsp_vram[offset] << 16) | (m_gsp_vram[offset] >> 16);
}

void metalmx_state::host_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = (data << 16) | (data >> 16);
	mem_mask = (mem_mask << 16) | (mem_mask >> 16);
	COMBINE_DATA(&m_gsp_vram[offset]);
}


void metalmx_state::timer_w(offs_t offset, uint32_t data)
{
	// Offsets
	// 9000 with 1 changes to external clock source
	// 1000 with 1 changes to link clock
	// 8000 with 1 changes to 4ms clock rate
	// 0000 with 1 changes to 1ms clock rate
	// a000 with 1 changes to 4ms counter rate
	// 2000 with 1 changes to 1ms counter rate
	// 3000  ?
	// b000  ?
	// f08000 with 1 resets
}


/*************************************
 *
 *  Main 68EC020 Memory Map
 *
 *************************************/

void metalmx_state::main_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x3fffff).rom();
	map(0x400000, 0x4000ff).rw(FUNC(metalmx_state::host_gsp_r), FUNC(metalmx_state::host_gsp_w));
	map(0x600000, 0x6fffff).rw(FUNC(metalmx_state::host_dram_r), FUNC(metalmx_state::host_dram_w));
	map(0x700000, 0x7fffff).rw(FUNC(metalmx_state::host_vram_r), FUNC(metalmx_state::host_vram_w));
	map(0x800000, 0x80001f).rw(FUNC(metalmx_state::dsp32c_r<1>), FUNC(metalmx_state::dsp32c_w<1>));
	map(0x800020, 0x85ffff).noprw();         // Unknown
	map(0x880000, 0x88001f).rw(FUNC(metalmx_state::dsp32c_r<0>), FUNC(metalmx_state::dsp32c_w<0>));
	map(0x980000, 0x9800ff).w(FUNC(metalmx_state::reset_w));
	map(0xb40000, 0xb40003).rw(FUNC(metalmx_state::sound_data_r), FUNC(metalmx_state::sound_data_w));
	map(0xf00000, 0xf00003).ram();         // Network message port
	map(0xf02000, 0xf02003).rw(FUNC(metalmx_state::watchdog_r), FUNC(metalmx_state::shifter_w));
	map(0xf03000, 0xf03003).portr("P1").w(FUNC(metalmx_state::motor_w));
	map(0xf04000, 0xf04003).portr("P2");
	map(0xf05000, 0xf05fff).nopw();    // Lamps // f06000 = ADC  // f01xxx = ADC
	map(0xf19000, 0xf19003).nopw();    // Network
	map(0xf1a000, 0xf1a003).nopw();
	map(0xf1b000, 0xf1b003).nopw();
	map(0xf1e000, 0xf1e003).ram();         // Network status flags : 1000 = LIRQ  4000 = SFLAG  8000 = 68FLAG
	map(0xf20000, 0xf2ffff).w(FUNC(metalmx_state::timer_w));
	map(0xfc0000, 0xfc1fff).ram();         // Zero power RAM
	map(0xfd0000, 0xffffff).ram();         // Scratch RAM
}


/*************************************
 *
 *  Network ADSP-2105 Memory Map
 *
 *************************************/

void metalmx_state::adsp_program_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share(m_adsp_internal_program_ram);
}

void metalmx_state::adsp_data_map(address_map &map)
{
	map(0x3800, 0x39ff).ram();
	map(0x2000, 0x2007).ram();
	map(0x3fe0, 0x3fff).ram(); // TODO: CPU control registers
}


/*************************************
 *
 *  2D Engine TMS34020 Memory Map
 *
 *************************************/

void metalmx_state::gsp_map(address_map &map)
{
	map(0x88800000, 0x8880001f).ram(); // ?
	map(0x88c00000, 0x88c0001f).ram(); // ?
	map(0xff000000, 0xff7fffff).ram().share(m_gsp_dram);
	map(0xff800000, 0xffffffff).ram().share(m_gsp_vram);
}


/*************************************
 *
 *  Math Box DSP32C 1 Memory Map
 *
 *************************************/

void metalmx_state::dsp32c_1_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x03ffff).ram();
	map(0x600000, 0x67ffff).ram();
	map(0x700000, 0x700003).nopw();    // LEDs?
	map(0xa00000, 0xa00003).r(FUNC(metalmx_state::unk_r));
	map(0xb00000, 0xb00003).r(FUNC(metalmx_state::unk_r));
	map(0xc00000, 0xc00003).ram();         // FIFO?
	map(0xf00000, 0xffffff).ram();         // 3D registers
}

/*************************************
 *
 *  Math Box DSP32C 2 Memory Map
 *
 *************************************/

void metalmx_state::dsp32c_2_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x03ffff).ram();
	map(0x600000, 0x67ffff).ram();
	map(0x700000, 0x700003).nopw();    // LEDs?
	map(0xa00000, 0xa00003).r(FUNC(metalmx_state::unk_r));
	map(0xb00000, 0xb00003).r(FUNC(metalmx_state::unk_r));
	map(0xc00000, 0xc00003).ram();         // FIFO?
	map(0xf00000, 0xffffff).ram();         // 3D registers
}


/*************************************
 *
 *  Input definitions
 *
 *************************************/

static INPUT_PORTS_START( metalmx )
	PORT_START("P1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// COINS
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00200000, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )

	// AUX
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gear Shift") PORT_TOGGLE
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Fire")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("View Change")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void metalmx_state::metalmx(machine_config &config)
{
	M68EC020(config, m_maincpu, XTAL(14'318'181));
	m_maincpu->set_addrmap(AS_PROGRAM, &metalmx_state::main_map);

	ADSP2105(config, m_adsp, XTAL(10'000'000));
	m_adsp->set_addrmap(AS_PROGRAM, &metalmx_state::adsp_program_map);
	m_adsp->set_addrmap(AS_DATA, &metalmx_state::adsp_data_map);

	TMS34020(config, m_gsp, 40'000'000);         // Unverified
	m_gsp->set_addrmap(AS_PROGRAM, &metalmx_state::gsp_map);
	m_gsp->set_halt_on_reset(true);
	m_gsp->set_pixel_clock(4'000'000);
	m_gsp->set_pixels_per_clock(2);
	m_gsp->output_int().set_inputline("maincpu", 4);

	DSP32C(config, m_dsp32c[0], 40'000'000);      // Unverified
	m_dsp32c[0]->set_addrmap(AS_PROGRAM, &metalmx_state::dsp32c_1_map);

	DSP32C(config, m_dsp32c[1], 40'000'000);      // Unverified
	m_dsp32c[1]->set_addrmap(AS_PROGRAM, &metalmx_state::dsp32c_2_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 384);
	screen.set_visarea(0, 511, 0, 383);
	screen.set_screen_update(FUNC(metalmx_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_565);

	// TODO: copied from atarigt.cpp; Same configurations as T-Mek?
	// 5 Channel output (4 Channel input connected to Quad Amp PCB)
	SPEAKER(config, "speaker", 4).front().headrest_left(2).headrest_right(3);
	//SPEAKER(config, "subwoofer").lfe(); Not implemented, Quad Amp PCB output;

	ATARI_CAGE(config, m_cage, 0);
	m_cage->set_speedup(0); // TODO: speedup address
	m_cage->irq_handler().set(FUNC(metalmx_state::cage_irq_callback));
	m_cage->add_route(0, "speaker", 1.0, 1); // Foward Right
	m_cage->add_route(1, "speaker", 1.0, 2); // Back Left
	m_cage->add_route(2, "speaker", 1.0, 0); // Foward Left
	m_cage->add_route(3, "speaker", 1.0, 3); // Back Right
}


void metalmx_state::init_metalmx()
{
	uint8_t *adsp_boot = (uint8_t*)memregion("adsp")->base();

	m_adsp->load_boot_data(adsp_boot, m_adsp_internal_program_ram);
}

void metalmx_state::machine_reset()
{
	m_dsp32c[0]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dsp32c[1]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( metalmx )
/* ------------------------------------------------
    MCUBE A052137 (there are 2 of these boards)
-------------------------------------------------*/
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "st665e.0",  0x000000, 0x80000, CRC(b2a90fd0) SHA1(ae483ab0aa68493904ea8d1906e22fdaa16a8a27) )
	ROM_LOAD32_BYTE( "st665e.1",  0x000001, 0x80000, CRC(559fecb7) SHA1(092e7e358d02f179a59849db0cafad0b4a95c0ed) )
	ROM_LOAD32_BYTE( "st665e.2",  0x000002, 0x80000, CRC(ee64b773) SHA1(8b1f51450804f16e73045ac9198daa7bd92d993b) )
	ROM_LOAD32_BYTE( "st665e.3",  0x000003, 0x80000, CRC(42b78cde) SHA1(a441fcd1cd34e1a8234be44ab33e56fbb73bac79) )

	ROM_LOAD32_BYTE( "tgs665e.0", 0x200000, 0x80000, CRC(2a4102f1) SHA1(2d21956b27cc9ac3d418f4595c1b8aa08a0298f6) )
	ROM_LOAD32_BYTE( "tgs665e.1", 0x200001, 0x80000, CRC(7d0eff8f) SHA1(c815cfa55619c3363c86c843047fe8487daaa6c1) )
	ROM_LOAD32_BYTE( "tgs665e.2", 0x200002, 0x80000, CRC(3f965f1a) SHA1(7333e1cd5a9428d78236a5532b6a60203fd1485b) )
	ROM_LOAD32_BYTE( "tgs665e.3", 0x200003, 0x80000, CRC(bb0ea984) SHA1(92a273675b32a2e1782012d49a404c9e8658eb2d) )

	ROM_REGION( 0x10000, "adsp", 0 )
	ROM_LOAD( "103-2308.bin", 0x00000, 0x10000, CRC(af1781d0) SHA1(f3f69e2fd83a949b447b71410ba9e165229e5aad) )

	ROM_REGION( 0x80000, "mcube_pals", 0 )
	ROM_LOAD( "103-1300.bin",  0x000, 0x157, CRC(eec18a29) SHA1(cc41cc921f59d385df805217f3b09bc289379f79) )

/* ------------------------------------------------
    ROMCH31 A053443 (there are 2 of these boards)
-------------------------------------------------*/
	ROM_REGION32_LE( 0x200000, "cage:boot", 0 )
	ROM_LOAD32_BYTE( "bootmetl.r34", 0x00000, 0x80000, CRC(ec799644) SHA1(32c77abb70fee1da8e3d7141bce2032e73e0eb35) )

	ROM_REGION32_LE( 0x80000, "cage", 0 )
	ROM_LOAD( "datametl.0", 0x00000, 0x80000, CRC(004dc445) SHA1(e52e539cc38afa917d1c769f9ad1794f4bd833b2) )
	ROM_LOAD( "datametl.1", 0x00000, 0x80000, CRC(e0465fc5) SHA1(cd8584e48b6cf33bc103cdfdb68d32eef3f2bec5) )
	ROM_LOAD( "datametl.2", 0x00000, 0x80000, CRC(2fb4b470) SHA1(b5ec1f9579200684cfd7e46c7774687df9330e19) )
	ROM_LOAD( "datametl.3", 0x00000, 0x80000, CRC(00f2bab2) SHA1(20ee4b185c9ac637be28cdc96c003c8f3c7ba3ee) )
	ROM_LOAD( "datametl.4", 0x00000, 0x80000, CRC(45f3d3ed) SHA1(cc83e61f4572ebe0a32c12b7c463daa1db0b7d09) )
	ROM_LOAD( "datametl.5", 0x00000, 0x80000, CRC(866bad4d) SHA1(67ed2f183a8e0424ca8963032d34c24c1f0a1e9a) )
	ROM_LOAD( "datametl.6", 0x00000, 0x80000, CRC(82c52a2f) SHA1(4250749c5e7281248315986150040f00529439c4) )
	ROM_LOAD( "datametl.7", 0x00000, 0x80000, CRC(6d4f81ab) SHA1(94fcad3f9348e95022327562803583b2e0b94caa) )
	ROM_LOAD( "datametl.8", 0x00000, 0x80000, CRC(697dda37) SHA1(2d5b8c1c5ce52f10472c3bb0c4aa3d14cb67be27) )
	ROM_LOAD( "datametl.9", 0x00000, 0x80000, CRC(a54ab7a9) SHA1(c2cba71188aec74766413961aa78444e64005634) )
	ROM_LOAD( "datametl.10",0x00000, 0x80000, CRC(532ace72) SHA1(84f3b112a7edba794c9ce7fdd0d81c57157203c8) )
	ROM_LOAD( "datametl.11",0x00000, 0x80000, CRC(3a7ce25d) SHA1(2dc4cad858207b645b4cb71ce5be74e3ce5383d3) )
	ROM_LOAD( "datametl.12",0x00000, 0x80000, CRC(13b2b1c7) SHA1(ab37b04e04b8a6a6929e5f039c2bcf6fbd330545) )
	ROM_LOAD( "datametl.13",0x00000, 0x80000, CRC(e8894400) SHA1(7c094502404b897347950f943c79f6fa80c984a1) )
	ROM_LOAD( "datametl.14",0x00000, 0x80000, CRC(709df01f) SHA1(639d8d4cf79ae7e0406d11ed1c475986f3e595f6) )
	ROM_LOAD( "datametl.15",0x00000, 0x80000, CRC(c55dbfbf) SHA1(63de1b24f5024e94601bcdffa7a3e418a195342f) )

	ROM_REGION( 0x80000, "cage_rom_pals", 0 )
	ROM_LOAD( "103-1500.bin",  0x000, 0x117, CRC(9883af90) SHA1(62b4a0cce5832628149c48e6810055ad3919cc5b) )

/* ------------------------------------------------
    CH31.2 A053304 (there are 2 of these boards)
-------------------------------------------------*/
	ROM_REGION( 0x80000, "cage_pals", 0 )
	ROM_LOAD( "dsp.bin",   0x000, 0x117, CRC(321b5250) SHA1(8d9c2ed9b0375c4624ea3efa80ba0a78135b756c) )
	ROM_LOAD( "irq2q.bin", 0x000, 0x117, CRC(983cb6e1) SHA1(0222fd2b79691b3b249f6b353f5687be83e291a7) )
	ROM_LOAD( "xdec.bin",  0x000, 0x117, CRC(fcb37143) SHA1(c95a6714f151868d42722684c8b67e9356f70544) )

/* ------------------------------------------------
    TGSMATH A052043
-------------------------------------------------*/
	ROM_REGION( 0x80000, "tgsmath_pals", 0 )
	ROM_LOAD( "103-1200.bin",  0x000, 0x2e5, CRC(cf1d4df4) SHA1(2ae592df2f16af070620766b7ebf60918c7f725d) )
	ROM_LOAD( "103-1202.bin",  0x000, 0x2e5, CRC(dab3b17f) SHA1(ac825063a293a32ae6ba5cbf5cfb52aa4aea62c9) )
	ROM_LOAD( "103-1204.bin",  0x000, 0x2e5, CRC(bb675ce0) SHA1(e3b188ebfe13e826e3d14c1cc456810d85750314) )
	ROM_LOAD( "103-1217.bin",  0x000, 0x157, CRC(a6418db0) SHA1(5b23fcb6123f0402f7483ffd38625846b0432efa) )
	ROM_LOAD( "103-1219.bin",  0x000, 0x117, CRC(bf08d1e8) SHA1(5710cbc941830594087320ed2b22bcbb2b5c48de) )

/* ------------------------------------------------
    TGS A051985
-------------------------------------------------*/
	ROM_REGION( 0x100000, "tex", 0 )
	ROM_LOAD16_BYTE( "tex0h.bin", 0x00001, 0x80000, CRC(b4dc459e) SHA1(cd2c6616951bdedeb5bb0c81179b4eee42ed5148) )
	ROM_LOAD16_BYTE( "tex0l.bin", 0x00000, 0x80000, CRC(370f9dca) SHA1(e26633f0f78f821fc59d8070ad927507f33d04d2) )
	ROM_LOAD16_BYTE( "tex1h.bin", 0x00001, 0x80000, CRC(e9224c54) SHA1(741f34cfad23d54c5bb70536489d661e7a505812) )
	ROM_LOAD16_BYTE( "tex1l.bin", 0x00000, 0x80000, CRC(0cb0c44b) SHA1(463736719dc0d00df8b22455effb17d00355ffd7) )
	ROM_LOAD16_BYTE( "tex2h.bin", 0x00001, 0x80000, CRC(f7096b75) SHA1(cf976039e8c0e4a5b5155fa4f108ae6a19712dc8) )
	ROM_LOAD16_BYTE( "tex2l.bin", 0x00000, 0x80000, CRC(33c47b1e) SHA1(20f6aff0ef31d71244ce3d0277d264fac7fa5229) )
	ROM_LOAD16_BYTE( "tex3h.bin", 0x00001, 0x80000, CRC(0cf25571) SHA1(08d78dfcb80742fb1f15b92e2f686b554109b01b) )
	ROM_LOAD16_BYTE( "tex3l.bin", 0x00000, 0x80000, CRC(56ab7d89) SHA1(0f88bba8b685f4cc7264ce042b3950dbaadbb495) )
	ROM_LOAD16_BYTE( "tex4h.bin", 0x00001, 0x80000, CRC(900d9ecf) SHA1(d0f0cb6dcfc25cd0ff10716d8242a82e4b000773) )
	ROM_LOAD16_BYTE( "tex4l.bin", 0x00000, 0x80000, CRC(d87d909c) SHA1(0eb26e37449ffd3d5a132189cdcbfc9129808c50) )
	ROM_LOAD16_BYTE( "tex5h.bin", 0x00001, 0x80000, CRC(7fcf4516) SHA1(4bd307afda9f499d26c9a802775bf57989fd7376) )
	ROM_LOAD16_BYTE( "tex5l.bin", 0x00000, 0x80000, CRC(da3fa060) SHA1(338dc2510a0fbb242c054b03264bd88ba9aa152f) )
	ROM_LOAD16_BYTE( "tex6h.bin", 0x00001, 0x80000, CRC(b6e6a0b3) SHA1(e302c997be5a612b37bb947245331d2f941df8fa) )
	ROM_LOAD16_BYTE( "tex6l.bin", 0x00000, 0x80000, CRC(c0b0638f) SHA1(11805da9efff04ec835042c91cec861a80f27b6d) )
	ROM_LOAD16_BYTE( "tex7h.bin", 0x00001, 0x80000, CRC(ef32fb90) SHA1(393c0f219ff550c9e9d60dbd872349d770c33580) )
	ROM_LOAD16_BYTE( "tex7l.bin", 0x00000, 0x80000, CRC(d9278afd) SHA1(6d86e0608cbb7697d63d3dbb74365d46be74074b) )

	ROM_REGION( 0x80000, "proms", 0 )
	ROM_LOAD( "blue.bpr",  0x000, 0x200, CRC(58e48ec5) SHA1(b2dfc6ad2b60fa3d9898c5d7a5fcc65af39117a1) )
	ROM_LOAD( "green.bpr", 0x000, 0x200, CRC(a80fd47d) SHA1(5bdffe022cbe2527c89c17f37e90e7e49c48be99) )
	ROM_LOAD( "red.bpr",   0x000, 0x200, CRC(bc8a3266) SHA1(87f98ea3657ae08ae80282c7940f00f31a407035) )

	ROM_REGION( 0x80000, "tgs_pals", 0 )
	ROM_LOAD( "103-1106.bin",  0x000, 0x2e5, CRC(fd8a872e) SHA1(03f4033d77617d2c372f4656cb1cdb6ea6bd20d6) )
	ROM_LOAD( "103-1107.bin",  0x000, 0x2e5, CRC(c16e71fe) SHA1(6d1e9fa1894778381bb6b8954023e4d1816241bb) )
	ROM_LOAD( "103-1108.bin",  0x000, 0x2e5, CRC(b3680191) SHA1(36d006ca6bff5592a84fba812df43f2f5ddfe927) )
	ROM_LOAD( "103-1109.bin",  0x000, 0x2e5, CRC(ce803542) SHA1(9da822cedf0015399939524639fec9a0720626d4) )
	ROM_LOAD( "103-1111.bin",  0x000, 0x2e5, CRC(a48b2dc1) SHA1(18713aea428c8109fda4b6abec2ef0bb75f38078) )
	ROM_LOAD( "103-1112.bin",  0x000, 0x2e5, CRC(087bfb83) SHA1(330daf872e2e7dbdc30d65d74e487904e2091374) )
	ROM_LOAD( "103-1114.bin",  0x000, 0x117, CRC(47443136) SHA1(83ea193d9d10d74fed941d5a14dd84c8a03d229f) )
	ROM_LOAD( "103-1116.bin",  0x000, 0x117, CRC(37edc36c) SHA1(be53131c52e84cb3fe055af5ca4e2f6aa5442ff0) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1994, metalmx, 0, metalmx, metalmx, metalmx_state, init_metalmx, ROT0, "Atari Games", "Metal Maniax (prototype)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
