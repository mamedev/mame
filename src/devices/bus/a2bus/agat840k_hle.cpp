// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat840k_hle.cpp

    High-level simulation of the Agat 840K floppy controller card

    http://agatcomp.ru/Reading/docs/es5323.txt
    https://github.com/sintech/AGAT/blob/master/docs/agat-840k-format.txt
    http://www.torlus.com/floppy/forum/viewtopic.php?f=19&t=1385

*********************************************************************/

#include "emu.h"
#include "agat840k_hle.h"

#include "formats/agat840k_hle_dsk.h"

//#define VERBOSE 1
#include "logmacro.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_AGAT840K_HLE, a2bus_agat840k_hle_device, "agat840k_hle", "Agat 840K floppy card")

#define AGAT840K_ROM_REGION  "agat840k_hle_rom"


ROM_START( agat840k_hle )
	ROM_REGION(0x100, AGAT840K_ROM_REGION, 0)
	// "Zagorsk" variant
	ROM_LOAD( "teac.rom", 0x0000, 0x0100, CRC(94266928) SHA1(5d369bad6cdd6a70b0bb16480eba69640de87a2e) )
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

static const floppy_interface agat840k_hle_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(agat840k_hle),
	"floppy_5_25"
};

void a2bus_agat840k_hle_device::device_add_mconfig(machine_config &config)
{
	legacy_floppy_image_device &floppy0(LEGACY_FLOPPY(config, FLOPPY_0, 0, &agat840k_hle_floppy_interface));
	floppy0.out_idx_cb().set(FUNC(a2bus_agat840k_hle_device::index_0_w));
	legacy_floppy_image_device &floppy1(LEGACY_FLOPPY(config, FLOPPY_1, 0, &agat840k_hle_floppy_interface));
	floppy1.out_idx_cb().set(FUNC(a2bus_agat840k_hle_device::index_1_w));

	I8255(config, m_d14);
	// PA not connected
	m_d14->in_pb_callback().set(FUNC(a2bus_agat840k_hle_device::d14_i_b)); // status signals from drive
	m_d14->out_pc_callback().set(FUNC(a2bus_agat840k_hle_device::d14_o_c)); // control

	I8255(config, m_d15);
	m_d15->in_pa_callback().set(FUNC(a2bus_agat840k_hle_device::d15_i_a)); // read data
//  m_d15->out_pb_callback().set(FUNC(a2bus_agat840k_hle_device::d15_o_b)); // write data
	m_d15->in_pc_callback().set(FUNC(a2bus_agat840k_hle_device::d15_i_c));
	m_d15->out_pc_callback().set(FUNC(a2bus_agat840k_hle_device::d15_o_c));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_agat840k_hle_device::device_rom_region() const
{
	return ROM_NAME(agat840k_hle);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_agat840k_hle_device::a2bus_agat840k_hle_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, m_d14(*this, "d14")
	, m_d15(*this, "d15")
	, m_rom(nullptr)
{
}

a2bus_agat840k_hle_device::a2bus_agat840k_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_agat840k_hle_device(mconfig, A2BUS_AGAT840K_HLE, tag, owner, clock)
{
}

WRITE_LINE_MEMBER(a2bus_agat840k_hle_device::index_0_w)
{
	index_callback(0, state);
}

WRITE_LINE_MEMBER(a2bus_agat840k_hle_device::index_1_w)
{
	index_callback(1, state);
}

void a2bus_agat840k_hle_device::index_callback(int unit, int state)
{
	if (unit != m_unit) return;

	LOG("index: unit %d state %d (%s)\n", unit, state, m_seen_magic ? "MAGIC" : "magic");

#if 0
	if (!state && !m_seen_magic)
	{
		m_seen_magic = true;
		m_count_read = 0;
		m_count_write = 0;
		m_d15->pc4_w(0); // latch data into port A
		m_d15->pc4_w(1);
		m_timer_wait->adjust(attotime::from_usec(m_waittime), 0, attotime::from_usec(m_waittime));
	}
#endif
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_agat840k_hle_device::device_start()
{
	m_rom = device().machine().root_device().memregion(this->subtag(AGAT840K_ROM_REGION).c_str())->base();

	m_mxcs = MXCSR_SYNC;

	m_timer_wait = timer_alloc(TIMER_ID_WAIT);
	m_timer_seek = timer_alloc(TIMER_ID_SEEK);

	m_seektime = 6000; // 6 ms, per es5323.txt
	m_waittime = 32;   // 16 bits x 2 us
}

void a2bus_agat840k_hle_device::device_reset()
{
	u8 buf[256];

	for (int i = 0; i < 2; i++)
	{
		legacy_floppy_image_device *img = floppy_image(i);
		if (img)
		{
			img->floppy_drive_set_ready_state(FLOPPY_DRIVE_READY, 0);
			img->floppy_drive_set_rpm(300.);
			img->floppy_drive_seek(-img->floppy_drive_get_current_track());
		}
	}
	m_floppy = floppy_image(0);

	// generate track images in memory, using default volume ID and gap padding bytes
	int t = 0;
	for (auto &elem : m_tracks)
	{
		elem = std::make_unique<uint16_t[]>(6250);

		for (int i = 0; i < 13; i++)
		{
			elem[i] = 0xaa;
		}
		for (int j = 0; j < 21; j++)
		{
			const int s = (j * 1) % 21;
			int cksum = 0;

			m_floppy->floppy_drive_read_sector_data(t & 1, s, buf, 256);

			enum
			{
				BAUX = 22,
				BLOB = 256 + 19 + BAUX
			};

			for (int k = 0; k < 256; k++)
			{
				if (cksum > 255) { cksum++; cksum &= 255; }
				cksum += buf[k];
				elem[13 + (BLOB * j) + 17 + k] = buf[k];
			}
			cksum &= 255;

			elem[13 + (BLOB * j) +  0] = 0xa4;
			elem[13 + (BLOB * j) +  1] = 0x80ff; // desync
			elem[13 + (BLOB * j) +  2] = 0x95;
			elem[13 + (BLOB * j) +  3] = 0x6a;
			elem[13 + (BLOB * j) +  4] = 0x40fe; // volume id
			elem[13 + (BLOB * j) +  5] = t;
			elem[13 + (BLOB * j) +  6] = s;
			elem[13 + (BLOB * j) +  7] = 0x5a;
			elem[13 + (BLOB * j) +  8] = 0xaa;
			elem[13 + (BLOB * j) +  9] = 0xaa;
			elem[13 + (BLOB * j) + 10] = 0xaa;
			elem[13 + (BLOB * j) + 11] = 0xaa;
			elem[13 + (BLOB * j) + 12] = 0xaa;
			elem[13 + (BLOB * j) + 13] = 0xa4;
			elem[13 + (BLOB * j) + 14] = 0x80ff; // desync
			elem[13 + (BLOB * j) + 15] = 0x6a;
			elem[13 + (BLOB * j) + 16] = 0x95;
			elem[13 + (BLOB * j) + 17 + 256] = cksum + 0x2000;
			elem[13 + (BLOB * j) + 17 + 257] = 0x5a;

			// gap3
			for (int k = 0; k < BAUX; k++)
			{
				elem[13 + (BLOB * j) + 17 + 258 + k] = 0xaa;
			}
		}

		t++;
		if ((t & 1) == 0)
		{
			m_floppy->floppy_drive_seek(1);
		}
	}
	m_floppy->floppy_drive_seek(-m_floppy->floppy_drive_get_current_track());

	m_mxcs |= MXCSR_SYNC;
	m_mxcs &= ~MXCSR_TR;
}

void a2bus_agat840k_hle_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ID_WAIT:
	{
		m_count_read++;
		m_count_read %= 6250;
		m_d15->pc4_w(0);
		m_d15->pc4_w(1);
		if (BIT(m_tracks[(2 * m_floppy->floppy_drive_get_current_track()) + m_side][m_count_read], 15))
			m_mxcs &= ~MXCSR_SYNC;
	}
	break;

	case TIMER_ID_SEEK:
		m_floppy->floppy_stp_w(1);
		m_floppy->floppy_stp_w(0);
		break;
	}
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_agat840k_hle_device::read_c0nx(uint8_t offset)
{
	u8 data;

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

void a2bus_agat840k_hle_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0: case 1: case 2: case 3:
		m_d14->write(offset, data);
		break;

	case 4: case 5: case 6: case 7:
		m_d15->write(offset - 4, data);
		break;

	case 8: // write desync
		break;

	case 9: // step
		LOG("step at %11.6f\n", machine().time().as_double());
		m_seen_magic = false;
		m_timer_wait->adjust(attotime::from_usec(m_seektime), 0, attotime::from_usec(m_waittime));
		m_floppy->floppy_stp_w(1);
		m_floppy->floppy_stp_w(0);
		break;

	case 10: // reset desync flipflop
		m_mxcs |= MXCSR_SYNC;
		break;

	default:
		break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_agat840k_hle_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset];
}

legacy_floppy_image_device *a2bus_agat840k_hle_device::floppy_image(int drive)
{
	const char *floppy_name = nullptr;

	switch (drive)
	{
	case 0:
		floppy_name = FLOPPY_0;
		break;
	case 1:
		floppy_name = FLOPPY_1;
		break;
	}
	return subdevice<legacy_floppy_image_device>(floppy_name);
}

// all signals active low.  write support not implemented; WPT is always active.
READ8_MEMBER(a2bus_agat840k_hle_device::d14_i_b)
{
	u8 data = 0x03; // one drive present, because drive select is broken

	m_floppy->floppy_drive_set_ready_state(FLOPPY_DRIVE_READY, 1);

	data |= (m_floppy->floppy_index_r() << 4) ^ 0x10;
//  data |= m_floppy->floppy_wpt_r() << 5;
	data |= m_floppy->floppy_tk00_r() << 6;
	data |= m_floppy->floppy_ready_r() << 7;

	LOG("status A: %s %s (t %d) %s %s\n", BIT(data, 7) ? "ready" : "READY", BIT(data, 6) ? "tk00" : "TK00",
		m_floppy->floppy_drive_get_current_track(),
		BIT(data, 5) ? "wpt" : "WPT", BIT(data, 4) ? "index" : "INDEX");

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
WRITE8_MEMBER(a2bus_agat840k_hle_device::d14_o_c)
{
	// drive select is broken in legacy flopdrv.cpp -- floppy_get_drive
	m_unit = BIT(data, 3);
	m_floppy = floppy_image(m_unit);
	if (m_unit)
		m_floppy->floppy_ds1_w(m_unit != 1);
	else
		m_floppy->floppy_ds0_w(m_unit != 0);

	m_floppy->floppy_drtn_w(!BIT(data, 2));
	m_side = BIT(data, 4);
	m_floppy->floppy_wtg_w(!BIT(data, 6));
	m_floppy->floppy_mon_w(!BIT(data, 7)); // tied to 'drive select', 'motor on' and 'head load'

	if (!BIT(data, 7))
	{
		m_seen_magic = false;
		m_timer_wait->adjust(attotime::never);
	}
	else
	{
		m_d15->pc4_w(0);
		m_d15->pc4_w(1);
		m_timer_wait->adjust(attotime::from_usec(m_waittime), 0, attotime::from_usec(m_waittime));
	}

	LOG("D14 C <- %02X (unit %d side %d drtn %d wtg %d mon %d)\n",
		data, m_unit, m_side, !BIT(data, 2), !BIT(data, 6), !BIT(data, 7));
}

// data are latched in by write to PC4
READ8_MEMBER(a2bus_agat840k_hle_device::d15_i_a)
{
	const u16 data = m_tracks[(2 * m_floppy->floppy_drive_get_current_track()) + m_side][m_count_read];
	LOG("sector data: %02x @ %4d (head %d track %2d)%s\n", data & 0xff, m_count_read,
		m_side, m_floppy->floppy_drive_get_current_track(),
		BIT(data, 14) ? " volume" : (BIT(data, 13) ? " cksum" : ""));

	return data & 0xff;
}

// C0x6
//
// b6   AL  desync detected
// b7   AH  read or write data ready
READ8_MEMBER(a2bus_agat840k_hle_device::d15_i_c)
{
	LOG("status B:       @ %4d %s %s (%s)\n", m_count_read,
		BIT(m_mxcs, 7) ? "ready" : "READY", BIT(m_mxcs, 6) ? "SYNC" : "sync",
		m_seen_magic ? "MAGIC" : "magic");

	return m_mxcs;
}

// C0x7
//
// b0   --  connected to b7, set if m_intr[PORT_B]
// b2   AH  b7 = ready for write data
// b3   --  connected to b7, set if m_intr[PORT_A]
// b4   AH  b7 = read data ready
WRITE8_MEMBER(a2bus_agat840k_hle_device::d15_o_c)
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
