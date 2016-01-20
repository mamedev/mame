// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Roland MPU-401 core

  This emulates the MPU-401 external box with the 6801, ASIC, and RAM in it.

  We do it this way to facilitate the various PC, Apple II, C64, and other
  possible hookups.

  6801 GPIO port hookups (from the schematics)

  P10 / P11 / P12: drive the metronome and speaker
  P13 / P14 / P15: drive 3 pins on the SYNC OUT connector
  P16: to DSRD on gate array
  P17: to DRRD on gate array

  P20: to SYC OUT on gate array
  P21: to SYC IN on gate array, pulled up to Vcc via 4.7K resistor
       programmed as output of timer (OLVL)
  P22: to SRCK on gate array, inverted
  P23: MIDI IN serial data (SCI in)
  P24: MIDI OUT serial data (SCI out)

  ASIC addresses from the 6801:
  0x20: (r) read pending byte from the PC (w) apparently nothing
  0x21: (r) ASIC status, see STAT_xxx bits below (w) send new byte to PC data port

  Theory of operation: 6801's timer/counter is set up to drive a pulse stream
  out P21 to the ASIC's SYC IN pin.  The ASIC in turn generates the MIDI baud
  rate (times 8) and returns that on pin P22.

  The 6801 is believed to run in mode 2, based on a combination of the
  schematics and the behavior (ie, internal RAM from 80-FF is clearly
  present from the program's behavior, and ports 3/4 are obviously external
  address/data buses)

***************************************************************************/

#include "machine/mpu401.h"
#include "bus/midi/midi.h"

#define M6801_TAG   "mpu6801"
#define ROM_TAG     "mpurom"
#define MIDIIN_TAG  "mdin"
#define MIDIOUT_TAG "mdout"

#define P2_SYNC_OUT (0x01)
#define P2_SYNC_IN  (0x02)
#define P2_SRCK_OUT (0x04)
#define P2_MIDI_IN  (0x08)
#define P2_MIDI_OUT (0x10)

#define STAT_CMD_PORT (0x01)    // set if the new byte indicated by TX FULL was written to the command port, clear for data port
#define STAT_TX_FULL  (0x40)    // indicates the PC has written a new byte we haven't read yet
#define STAT_RX_EMPTY (0x80)    // indicates we've written a new byte the PC hasn't read yet

static ADDRESS_MAP_START( mpu401_map, AS_PROGRAM, 8, mpu401_device )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(regs_mode2_r, regs_mode2_w)
	AM_RANGE(0x0020, 0x0021) AM_READWRITE(asic_r, asic_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM // on-chip RAM
	AM_RANGE(0x0800, 0x0fff) AM_RAM // external RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION(ROM_TAG, 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpu401_io_map, AS_IO, 8, mpu401_device )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_READWRITE(port1_r, port1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(port2_r, port2_w)
ADDRESS_MAP_END

MACHINE_CONFIG_FRAGMENT( mpu401 )
	MCFG_CPU_ADD(M6801_TAG, M6801, 4000000) /* 4 MHz as per schematics */
	MCFG_CPU_PROGRAM_MAP(mpu401_map)
	MCFG_CPU_IO_MAP(mpu401_io_map)
	MCFG_M6801_SER_TX(DEVWRITELINE(MIDIOUT_TAG, midi_port_device, write_txd))

	MCFG_MIDI_PORT_ADD(MIDIIN_TAG, midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(DEVWRITELINE(DEVICE_SELF, mpu401_device, midi_rx_w))

	MCFG_MIDI_PORT_ADD(MIDIOUT_TAG, midiout_slot, "midiout")
MACHINE_CONFIG_END

ROM_START( mpu401 )
	ROM_REGION(0x1000, ROM_TAG, 0)
	ROM_LOAD( "roland_6801v0b55p.bin", 0x000000, 0x001000, CRC(65d3a151) SHA1(00efbfb96aeb997b69bb16981c6751d3c784bb87) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type MPU401 = &device_creator<mpu401_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor mpu401_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mpu401 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *mpu401_device::device_rom_region() const
{
	return ROM_NAME( mpu401 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mpu401_device - constructor
//-------------------------------------------------

mpu401_device::mpu401_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MPU401, "Roland MPU-401 I/O box", tag, owner, clock, "mpu401", __FILE__),
	m_ourcpu(*this, M6801_TAG),
	write_irq(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mpu401_device::device_start()
{
	write_irq.resolve_safe();
	m_timer = timer_alloc(0, nullptr);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mpu401_device::device_reset()
{
	m_port2 = 0xff & ~(P2_SRCK_OUT | P2_MIDI_IN);   // prevent spurious reception
	m_command = 0;
	m_mpudata = 0;
	m_gatearrstat = 0;

	m_timer->adjust(attotime::zero, 0, attotime::from_hz(31250*8));
}

//-------------------------------------------------
//  device_timer - called when our device timer expires
//-------------------------------------------------

void mpu401_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	m_ourcpu->m6801_clock_serial();
}

READ8_MEMBER(mpu401_device::regs_mode2_r)
{
	switch (offset)
	{
		case 4:
		case 5:
		case 6:
		case 7:
		case 0xf:
//          printf("MPU401: read @ unk %x (PC=%x)\n", offset, space.device().safe_pc());
			break;

		default:
			return m_ourcpu->m6801_io_r(space, offset);
	}

	return 0xff;
}

WRITE8_MEMBER(mpu401_device::regs_mode2_w)
{
	switch (offset)
	{
		case 4:
		case 5:
		case 6:
		case 7:
		case 0xf:
//          printf("MPU401: %02x @ unk %x (PC=%x)\n", data, offset, space.device().safe_pc());
			break;

		default:
			return m_ourcpu->m6801_io_w(space, offset, data);
	}
}

READ8_MEMBER(mpu401_device::port1_r)
{
	return 0xff;
}

WRITE8_MEMBER(mpu401_device::port1_w)
{
//  printf("port1_w: %02x met %x syncout %x DSRD %d DRRD %d\n", data, data & 3, (data>>3) & 3, (data>>6) & 1, (data>>7) & 1);
}

READ8_MEMBER(mpu401_device::port2_r)
{
//  printf("Read P2 (PC=%x)\n", space.device().safe_pc());
	return m_port2;
}

WRITE8_MEMBER(mpu401_device::port2_w)
{
//  printf("port2_w: %02x SYCOUT %d SYCIN %d SRCK %d MIDI OUT %d\n", data, (data & 1), (data>>1) & 1, (data>>2) & 1, (data>>4) & 1);
}

READ8_MEMBER(mpu401_device::mpu_r)
{
//  printf("mpu_r @ %d\n", offset);

	if (offset == 1)    // status
	{
		return m_gatearrstat;
	}
	else                // data
	{
		write_irq(CLEAR_LINE);
		m_gatearrstat |= STAT_RX_EMPTY;
		return m_mpudata;
	}
}

WRITE8_MEMBER(mpu401_device::mpu_w)
{
//  printf("%02x to MPU-401 @ %d\n", data, offset);
	m_command = data;
	m_gatearrstat |= STAT_TX_FULL;

	if (offset == 1)
	{
		m_gatearrstat |= STAT_CMD_PORT;
	}
	else
	{
		m_gatearrstat &= ~STAT_CMD_PORT;
	}
}

READ8_MEMBER(mpu401_device::asic_r)
{
	if (offset == 0)
	{
		m_gatearrstat &= ~STAT_TX_FULL;
		return m_command;
	}
	else if (offset == 1)
	{
		return m_gatearrstat;
	}

	return 0xff;
}

WRITE8_MEMBER(mpu401_device::asic_w)
{
//  printf("MPU401: %02x to gate array @ %d\n", data, offset);

	if (offset == 1)
	{
		m_mpudata = data;
		m_gatearrstat &= ~STAT_RX_EMPTY;
		write_irq(ASSERT_LINE);
	}
}

// MIDI receive
WRITE_LINE_MEMBER( mpu401_device::midi_rx_w )
{
	if (state == ASSERT_LINE)
	{
		m_port2 |= P2_MIDI_IN;
	}
	else
	{
		m_port2 &= ~P2_MIDI_IN;
	}
}
