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

#include "emu.h"
#include "mpu401.h"

#include "bus/midi/midi.h"

#define M6801_TAG   "mpu6801"
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

void mpu401_device::mpu401_map(address_map &map)
{
	map(0x0020, 0x0021).rw(FUNC(mpu401_device::asic_r), FUNC(mpu401_device::asic_w));
	map(0x0800, 0x0fff).ram(); // external RAM
}

ROM_START( mpu401 )
	ROM_REGION(0x1000, M6801_TAG, 0)
	ROM_LOAD( "roland__6801v0b55p__15179222.bin", 0x000000, 0x001000, CRC(65d3a151) SHA1(00efbfb96aeb997b69bb16981c6751d3c784bb87) ) /* Mask MCU; Label: "Roland // 6801V0B55P // 5A1 JAPAN // 15179222"; This is the final version (1.5A) of the mpu401 firmware; version is located at offsets 0x649 (0x15) and 0x64f (0x01) */
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MPU401, mpu401_device, "mpu401", "Roland MPU-401 I/O box")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mpu401_device::device_add_mconfig(machine_config &config)
{
	HD6801V0(config, m_ourcpu, 4000000); /* 4 MHz as per schematics */
	m_ourcpu->set_addrmap(AS_PROGRAM, &mpu401_device::mpu401_map);
	m_ourcpu->in_p1_cb().set(FUNC(mpu401_device::port1_r));
	m_ourcpu->out_p1_cb().set(FUNC(mpu401_device::port1_w));
	m_ourcpu->in_p2_cb().set(FUNC(mpu401_device::port2_r));
	m_ourcpu->out_p2_cb().set(FUNC(mpu401_device::port2_w));
	m_ourcpu->out_ser_tx_cb().set(MIDIOUT_TAG, FUNC(midi_port_device::write_txd));

	MIDI_PORT(config, MIDIIN_TAG, midiin_slot, "midiin").rxd_handler().set(DEVICE_SELF, FUNC(mpu401_device::midi_rx_w));

	MIDI_PORT(config, MIDIOUT_TAG, midiout_slot, "midiout");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mpu401_device::device_rom_region() const
{
	return ROM_NAME( mpu401 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mpu401_device - constructor
//-------------------------------------------------

mpu401_device::mpu401_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MPU401, tag, owner, clock),
	m_ourcpu(*this, M6801_TAG),
	write_irq(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mpu401_device::device_start()
{
	m_timer = timer_alloc(FUNC(mpu401_device::serial_tick), this);
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
//  serial_tick - update the 6801's serial clock
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mpu401_device::serial_tick)
{
	m_ourcpu->clock_serial();
}

uint8_t mpu401_device::port1_r()
{
	return 0xff;
}

void mpu401_device::port1_w(uint8_t data)
{
//  printf("port1_w: %02x met %x syncout %x DSRD %d DRRD %d\n", data, data & 3, (data>>3) & 3, (data>>6) & 1, (data>>7) & 1);
}

uint8_t mpu401_device::port2_r()
{
//  printf("%s Read P2\n", machine().describe_context().c_str());
	return m_port2;
}

void mpu401_device::port2_w(uint8_t data)
{
//  printf("port2_w: %02x SYCOUT %d SYCIN %d SRCK %d MIDI OUT %d\n", data, (data & 1), (data>>1) & 1, (data>>2) & 1, (data>>4) & 1);
}

uint8_t mpu401_device::mpu_r(offs_t offset)
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

void mpu401_device::mpu_w(offs_t offset, uint8_t data)
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

uint8_t mpu401_device::asic_r(offs_t offset)
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

void mpu401_device::asic_w(offs_t offset, uint8_t data)
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
void mpu401_device::midi_rx_w(int state)
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
