// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat_fdc.c

    Implementation of the Agat 840K floppy controller card

    Technical manual:
    http://agatcomp.ru/Reading/serkov/hainfo/023-01to.shtml
    http://agatcomp.ru/Reading/serkov/hainfo/023-01to1.shtml

    Schematic:
    http://agatcomp.ru/Reading/fl800k/FD840/TEAC_023-adj-HI.jpg

    On-disk format:
    https://github.com/sintech/AGAT/blob/master/docs/agat-840k-format.txt
    http://www.torlus.com/floppy/forum/viewtopic.php?f=19&t=1385

*********************************************************************/

#include "emu.h"
#include "agat_fdc.h"

#include "formats/aim_dsk.h"
#include "formats/ds9_dsk.h"


#define LOG_LSS     (1U << 1)   // Show warnings
#define LOG_SHIFT   (1U << 2)   // Shows shift register contents

//#define VERBOSE (LOG_GENERAL | LOG_SHIFT | LOG_LSS)
//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

#define LOGLSS(...)      LOGMASKED(LOG_LSS, __VA_ARGS__)
#define LOGSHIFT(...)    LOGMASKED(LOG_SHIFT, __VA_ARGS__)


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_AGAT_FDC, a2bus_agat_fdc_device, "agat_fdc", "Agat 840K floppy controller card")

#define AGAT_FDC_ROM_REGION     "agat_fdc_rom"
#define AGAT_FDC_ROM_D6_REGION  "agat_fdc_d6_rom"

ROM_START( agat9 )
	ROM_REGION(0x100, AGAT_FDC_ROM_REGION, 0)
	// zagorsk
	ROM_LOAD( "teac.rom", 0x0000, 0x0100, CRC(94266928) SHA1(5d369bad6cdd6a70b0bb16480eba69640de87a2e) )

	ROM_REGION(0x200, AGAT_FDC_ROM_D6_REGION, 0)
	ROM_LOAD( "d6encdec.bin", 0x0000, 0x0200, CRC(66e7e896) SHA1(b5305e82c81240a6fdc932a559b5493c59f302c6) )
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( a2bus_agat_fdc_device::floppy_formats )
	FLOPPY_DS9_FORMAT,
	FLOPPY_AIM_FORMAT
FLOPPY_FORMATS_END

static void agat_floppies(device_slot_interface &device)
{
	device.option_add("525dsqd", FLOPPY_525_QD);
}

void a2bus_agat_fdc_device::device_add_mconfig (machine_config &config)
{
	FLOPPY_CONNECTOR(config, floppy0, agat_floppies, "525dsqd", a2bus_agat_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, floppy1, agat_floppies, "525dsqd", a2bus_agat_fdc_device::floppy_formats);

	I8255(config, m_d14);
	// PA not connected
	m_d14->in_pb_callback().set(FUNC(a2bus_agat_fdc_device::d14_i_b)); // status signals from drive
	m_d14->out_pc_callback().set(FUNC(a2bus_agat_fdc_device::d14_o_c)); // control

	I8255(config, m_d15);
	m_d15->in_pa_callback().set(FUNC(a2bus_agat_fdc_device::d15_i_a)); // read data
//  m_d15->out_pb_callback().set(FUNC(a2bus_agat_fdc_device::d15_o_b)); // write data
	m_d15->in_pc_callback().set(FUNC(a2bus_agat_fdc_device::d15_i_c));
	m_d15->out_pc_callback().set(FUNC(a2bus_agat_fdc_device::d15_o_c));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_agat_fdc_device::device_rom_region() const
{
	return ROM_NAME( agat9 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_agat_fdc_device::a2bus_agat_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, m_d14(*this, "d14")
	, m_d15(*this, "d15")
	, floppy0(*this, "0")
	, floppy1(*this, "1")
	, m_rom(nullptr)
	, m_rom_d6(nullptr)
{
}

a2bus_agat_fdc_device::a2bus_agat_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_agat_fdc_device(mconfig, A2BUS_AGAT_FDC, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_agat_fdc_device::device_start()
{
	set_unscaled_clock((XTAL(14'300'000) / 14.0) * 4.0);

	m_rom = device().machine().root_device().memregion(this->subtag(AGAT_FDC_ROM_REGION).c_str())->base();
	m_rom_d6 = device().machine().root_device().memregion(this->subtag(AGAT_FDC_ROM_D6_REGION).c_str())->base();

	floppy = nullptr;
	if (floppy0)
	{
		floppy = floppy0->get_device();
	}

	m_mxcs = MXCSR_SYNC;

	m_timer_lss = timer_alloc(TIMER_ID_LSS);
	m_timer_motor = timer_alloc(TIMER_ID_MOTOR);

	m_seektime = 6; // ms, per es5323.txt
	m_waittime = 32; // us - 16 bits x 2 us
}

void a2bus_agat_fdc_device::device_reset()
{
	active = 0;
	cycles = time_to_cycles(machine().time());
	data_reg = 0x00;
	address = 0xff;

	m_mxcs |= MXCSR_SYNC;
	m_mxcs &= ~MXCSR_TR;

	// Just a timer to be sure that the lss is updated from time to
	// time, so that there's no hiccup when it's talked to again.
	m_timer_lss->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

void a2bus_agat_fdc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ID_LSS:
		lss_sync();
		break;

	case TIMER_ID_MOTOR:
		active = 0;
		floppy->mon_w(1);
		break;
	}
}

uint64_t a2bus_agat_fdc_device::time_to_cycles(const attotime &tm)
{
	// Clock is falling edges of the ~4Mhz clock

	uint64_t cycles = tm.as_ticks(clock()*2);
	cycles = (cycles+1) >> 1;
	return cycles;
}

attotime a2bus_agat_fdc_device::cycles_to_time(uint64_t cycles)
{
	return attotime::from_ticks(cycles*2+1, clock()*2);
}

void a2bus_agat_fdc_device::lss_start()
{
	cycles = time_to_cycles(machine().time()) + 1;
	data_reg = 0x00;
	address = 0xff;
	bits = 8;
}

void a2bus_agat_fdc_device::lss_sync()
{
	if(!active)
		return;

	attotime next_flux = floppy ? floppy->get_next_transition(cycles_to_time(cycles-1)) : attotime::never;
	uint64_t cycles_limit = time_to_cycles(machine().time());
	uint64_t cycles_next_flux = next_flux != attotime::never ? time_to_cycles(next_flux) : uint64_t(-1);
	uint64_t cycles_next_flux_down = cycles_next_flux != uint64_t(-1) ? cycles_next_flux+1 : uint64_t(-1);

	LOGLSS("LSS at %11.6f: %d (limit %d next %d) in %02x\n",
		machine().time().as_double(), cycles, cycles_limit, cycles_next_flux, address);

	if(cycles >= cycles_next_flux && cycles < cycles_next_flux_down)
		address &= ~0x40;
	else
		address |= 0x40;

	while (cycles < cycles_limit) {
		uint64_t cycles_next_trans = cycles_limit;
		if(cycles_next_trans > cycles_next_flux && cycles < cycles_next_flux)
		{
			cycles_next_trans = cycles_next_flux;
			LOGLSS("lss: next_trans up (%d < %d; %d)\n", cycles, cycles_next_flux, cycles_limit);
		}
		if(cycles_next_trans > cycles_next_flux_down && cycles < cycles_next_flux_down)
		{
			cycles_next_trans = cycles_next_flux_down;
			LOGLSS("lss: next_trans down (%d < %d; %d)\n", cycles, cycles_next_flux_down, cycles_limit);
		}

		while (cycles < cycles_next_trans) {
			uint8_t opcode = m_rom_d6[address];

			if (cycles_next_flux != uint64_t(-1))
			{
				LOGLSS("lss: %d (limit %d next %d) in %03x out %02x (addr %02x end %d bit %d sync %d)\n",
				cycles, cycles_limit, cycles_next_flux, address, opcode,
				opcode & 0x3f, BIT(opcode, 6), BIT(opcode, 5), BIT(opcode, 7));
			}
			if (!BIT(opcode, 6)) // end bit
			{
				data_reg <<= 1;
				data_reg |= !BIT(opcode, 5);
				LOGSHIFT("lss shift: %d (to %02x, %2d bits) at %d%s\n", BIT(opcode, 5), data_reg, bits, cycles,
					BIT(opcode, 7) ? " (sync)":"");

				if (BIT(opcode, 7) == BIT(opcode, 5)) // hack
				{
					bits++;
				}
				else
				{
					m_mxcs &= ~MXCSR_SYNC;
					bits = 0;
				}

				if (bits == 16)
				{
					address |= 0x80;
					m_d15->pc4_w(0);
					m_d15->pc4_w(1);
					bits = 8;
					LOGSHIFT("lss data: %02x\n", data_reg);
				}
				else
				{
					address &= ~0x80;
				}
			}

			address &= ~0x3f;
			address |= (opcode & 0x3f);

			cycles++;
		}

		if(cycles == cycles_next_flux)
			address &= ~0x40;
		else if(cycles == cycles_next_flux_down) {
			address |= 0x40;
			next_flux = floppy ? floppy->get_next_transition(cycles_to_time(cycles)) : attotime::never;
			if (next_flux != attotime::never) {
				cycles_next_flux = time_to_cycles(next_flux);
				LOGLSS("lss next: %d cycles\n", cycles_next_flux+1-cycles_next_flux_down);
				cycles_next_flux_down = cycles_next_flux+1;
			} else {
				cycles_next_flux = uint64_t(-1);
				cycles_next_flux_down = uint64_t(-1);
			}
		}
	}
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_agat_fdc_device::read_c0nx(uint8_t offset)
{
	u8 data;

	lss_sync();

	switch (offset)
	{
	case 0: case 1: case 2: case 3:
		data = m_d14->read(offset);
		break;

	case 4: case 5: case 6: case 7:
		data = m_d15->read(offset - 4);
		break;

	default:
		data = 0xff;
		break;
	}

	return data;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_agat_fdc_device::write_c0nx(uint8_t offset, uint8_t data)
{
	lss_sync();

	switch (offset)
	{
	case 0: case 1: case 2: case 3:
		m_d14->write(offset, data);
		break;

	case 4: case 5: case 6: case 7:
		m_d15->write(offset - 4, data);
		break;

	case 8: // D9.15 - write desync
		break;

	case 9: // D9.14 - step
		LOG("step at %11.6f\n", machine().time().as_double());
		if (floppy && active)
		{
			floppy->stp_w(1);
			floppy->stp_w(0);
		}
		break;

	case 10: // D9.13 - reset desync flipflop
		m_mxcs |= MXCSR_SYNC;
		break;

	default:
		break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_agat_fdc_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset];
}


/*
 * all signals active low.  write support not implemented; WPT is always active.
 *
 * b0-b1    type of drive 2: 00 - ES 5323.01 "1000 KB", 01 - "500 KB", 10 - "250 KB", 11 - not present
 * b2-b3    type of drive 1: -""-
 * b4       INDEX/SECTOR
 * b5       WRITE PROTECT
 * b6       TRACK 0
 * b7       READY
 *
 * C0x1
 */
READ8_MEMBER(a2bus_agat_fdc_device::d14_i_b)
{
	u8 data = 0x3;

	// all signals active low
	if (floppy)
	{
		data |= (floppy->idx_r() << 4) ^ 0x10;
//      data |= floppy->wpt_r() << 5;
		data |= floppy->trk00_r() << 6;
		data |= floppy->ready_r() << 7;
	}
	else
	{
		data |= 0xf0;
	}

	LOG("status A: %s %s (t %d) %s %s\n", BIT(data, 7) ? "ready" : "READY", BIT(data, 6) ? "tk00" : "TK00",
		floppy ? floppy->get_cyl() : -1, BIT(data, 5) ? "wpt" : "WPT", BIT(data, 4) ? "index" : "INDEX");

	return data;
}

/*
 * b0   AH  strong write precomp
 * b1   --  NC
 * b2   --  step direction (1 - inward, 0 - outward)
 * b3   --  drive select (0 - drive 1, 1 - drive 2)
 * b4   --  head select (0 - bottom, 1 - top)
 * b5   AH  write precomp off
 * b6   AH  write enable
 * b7   AH  motor on
 *
 * C0x2
 */
WRITE8_MEMBER(a2bus_agat_fdc_device::d14_o_c)
{
	m_unit = BIT(data, 3);

	if (floppy)
	{
		floppy->dir_w(!BIT(data, 2));
		floppy->ss_w(BIT(data, 4));
//      floppy->wtg_w(!BIT(data, 6));
//      floppy->mon_w(!BIT(data, 7)); // tied to 'drive select', 'motor on' and 'head load'
	}
	if (BIT(data, 7))
	{
		m_d15->pc4_w(0);
		m_d15->pc4_w(1);
		floppy->mon_w(0);
		if (!active)
		{
			active = 1;
			lss_start();
		}
	}
#if 0
	else
	{
		m_timer_motor->adjust(attotime::from_msec(1000));
	}
#endif

	LOG("D14 C <- %02X (unit %d side %d drtn %d wtg %d mon %d)\n",
		data, m_unit, BIT(data, 4), !BIT(data, 2), !BIT(data, 6), !BIT(data, 7));
}

// data are latched in by write to PC4
READ8_MEMBER(a2bus_agat_fdc_device::d15_i_a)
{
	return data_reg;
}

// C0x6
//
// b6   AL  desync detected
// b7   AH  read or write data ready
READ8_MEMBER(a2bus_agat_fdc_device::d15_i_c)
{
	LOG("status B:       @ %4d %s %s\n", 0,
		BIT(m_mxcs, 7) ? "ready" : "READY", BIT(m_mxcs, 6) ? "SYNC" : "sync");

	return m_mxcs;
}

// C0x7
//
// b0   --  connected to b7, set if m_intr[PORT_B]
// b2   AH  b7 = ready for write data
// b3   --  connected to b7, set if m_intr[PORT_A]
// b4   AH  b7 = read data ready
WRITE8_MEMBER(a2bus_agat_fdc_device::d15_o_c)
{
	if (BIT(data, 0) || BIT(data, 3))
	{
		m_mxcs |= MXCSR_TR;
	}
	else
	{
		m_mxcs &= ~MXCSR_TR;
	}
}

