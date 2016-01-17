// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Epson PX-4

    Note: We are missing a dump of the slave 7508 CPU that controls
    the keyboard and some other things.

***************************************************************************/

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "bus/epson_sio/epson_sio.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "sound/speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "coreutil.h"
#include "softlist.h"
#include "px4.lh"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VERBOSE 1


//**************************************************************************
//  MACROS
//**************************************************************************

#define ART_TX_ENABLED  (BIT(m_artcr, 0))
#define ART_RX_ENABLED  (BIT(m_artcr, 2))
#define ART_BREAK       (BIT(m_artcr, 3))


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class px4_state : public driver_device, public device_serial_interface
{
public:
	px4_state(const machine_config &mconfig, device_type type, std::string tag) :
	driver_device(mconfig, type, tag),
	device_serial_interface(mconfig, *this),
	m_z80(*this, "maincpu"),
	m_ram(*this, RAM_TAG),
	m_nvram(*this, "nvram"),
	m_centronics(*this, "centronics"),
	m_ext_cas(*this, "extcas"),
	m_ext_cas_timer(*this, "extcas_timer"),
	m_speaker(*this, "speaker"),
	m_sio(*this, "sio"),
	m_rs232(*this, "rs232"),
	m_caps1(*this, "capsule1"), m_caps2(*this, "capsule2"),
	m_caps1_rom(nullptr), m_caps2_rom(nullptr),
	m_ctrl1(0), m_icrb(0), m_bankr(0),
	m_isr(0), m_ier(0), m_sior(0xbf),
	m_frc_value(0), m_frc_latch(0),
	m_vadr(0), m_yoff(0),
	m_artdir(0xff), m_artdor(0xff), m_artsr(0), m_artcr(0),
	m_one_sec_int_enabled(true),
	m_key_status(0), m_interrupt_status(0),
	m_time(), m_clock_state(0),
	m_ear_last_state(0),
	m_sio_pin(0), m_serial_rx(0), m_rs232_dcd(0), m_rs232_cts(0),
	m_centronics_busy(0), m_centronics_perror(0)
	{ }

	DECLARE_DRIVER_INIT( px4 );

	DECLARE_PALETTE_INIT( px4 );
	UINT32 screen_update_px4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( icrlc_r );
	DECLARE_WRITE8_MEMBER( ctrl1_w );
	DECLARE_READ8_MEMBER( icrhc_r );
	DECLARE_WRITE8_MEMBER( cmdr_w );
	DECLARE_READ8_MEMBER( icrlb_r );
	DECLARE_WRITE8_MEMBER( ctrl2_w );
	DECLARE_READ8_MEMBER( icrhb_r );
	DECLARE_READ8_MEMBER( isr_r );
	DECLARE_WRITE8_MEMBER( ier_w );
	DECLARE_READ8_MEMBER( str_r );
	DECLARE_WRITE8_MEMBER( bankr_w );
	DECLARE_READ8_MEMBER( sior_r );
	DECLARE_WRITE8_MEMBER( sior_w );
	DECLARE_WRITE8_MEMBER( vadr_w );
	DECLARE_WRITE8_MEMBER( yoff_w );
	DECLARE_WRITE8_MEMBER( fr_w );
	DECLARE_WRITE8_MEMBER( spur_w );
	DECLARE_READ8_MEMBER( ctgif_r );
	DECLARE_WRITE8_MEMBER( ctgif_w );
	DECLARE_READ8_MEMBER( artdir_r );
	DECLARE_WRITE8_MEMBER( artdor_w );
	DECLARE_READ8_MEMBER( artsr_r );
	DECLARE_WRITE8_MEMBER( artmr_w );
	DECLARE_READ8_MEMBER( iostr_r );
	DECLARE_WRITE8_MEMBER( artcr_w );
	DECLARE_WRITE8_MEMBER( swr_w );
	DECLARE_WRITE8_MEMBER( ioctlr_w );

	DECLARE_INPUT_CHANGED_MEMBER( key_callback );

	TIMER_DEVICE_CALLBACK_MEMBER( ext_cassette_read );
	TIMER_DEVICE_CALLBACK_MEMBER( frc_tick );
	TIMER_DEVICE_CALLBACK_MEMBER( upd7508_1sec_callback );

	// serial
	DECLARE_WRITE_LINE_MEMBER( sio_rx_w );
	DECLARE_WRITE_LINE_MEMBER( sio_pin_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_rx_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_dcd_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_dsr_w );
	DECLARE_WRITE_LINE_MEMBER( rs232_cts_w );

	// centronics
	DECLARE_WRITE_LINE_MEMBER( centronics_busy_w ) { m_centronics_busy = state; }
	DECLARE_WRITE_LINE_MEMBER( centronics_perror_w ) { m_centronics_perror = state; }

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// z80 interrupt sources
	enum
	{
		INT0_7508 = 0x01,
		INT1_ART  = 0x02,
		INT2_ICF  = 0x04,
		INT3_OVF  = 0x08,
		INT4_EXT  = 0x10
	};

	// 7508 interrupt sources
	enum
	{
		UPD7508_INT_ALARM      = 0x02,
		UPD7508_INT_POWER_FAIL = 0x04,
		UPD7508_INT_7508_RESET = 0x08,
		UPD7508_INT_Z80_RESET  = 0x10,
		UPD7508_INT_ONE_SECOND = 0x20
	};

	// art (asynchronous receiver transmitter)
	enum
	{
		ART_TXRDY   = 0x01, // output buffer empty
		ART_RXRDY   = 0x02, // data byte received
		ART_TXEMPTY = 0x04, // transmit buffer empty
		ART_PE      = 0x08, // parity error
		ART_OE      = 0x10, // overrun error
		ART_FE      = 0x20  // framing error
	};

	void gapnit_interrupt();

	DECLARE_WRITE_LINE_MEMBER( serial_rx_w );
	void txd_w(int data);

	void install_rom_capsule(address_space &space, int size, memory_region *mem);

	// internal devices
	required_device<cpu_device> m_z80;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_ext_cas;
	required_device<timer_device> m_ext_cas_timer;
	required_device<speaker_sound_device> m_speaker;
	required_device<epson_sio_device> m_sio;
	required_device<rs232_port_device> m_rs232;
	required_device<generic_slot_device> m_caps1;
	required_device<generic_slot_device> m_caps2;

	memory_region *m_caps1_rom;
	memory_region *m_caps2_rom;

	// gapnit register
	UINT8 m_ctrl1;
	UINT16 m_icrb;
	UINT8 m_bankr;
	UINT8 m_isr;
	UINT8 m_ier;
	UINT8 m_sior;

	// gapnit internal
	UINT16 m_frc_value;
	UINT16 m_frc_latch;

	// gapndi register
	UINT8 m_vadr;
	UINT8 m_yoff;

	// gapnio
	UINT8 m_artdir;
	UINT8 m_artdor;
	UINT8 m_artsr;
	UINT8 m_artcr;
	UINT8 m_swr;

	// 7508 internal
	bool m_one_sec_int_enabled;
	bool m_key_int_enabled;

	UINT8 m_key_status;
	UINT8 m_interrupt_status;

	system_time m_time;
	int m_clock_state;

	// external cassette/barcode reader
	int m_ear_last_state;

	// serial
	int m_sio_pin;
	int m_serial_rx;
	int m_rs232_dcd;
	int m_rs232_cts;

	// centronics
	int m_centronics_busy;
	int m_centronics_perror;
};

class px4p_state : public px4_state
{
public:
	px4p_state(const machine_config &mconfig, device_type type, std::string tag) :
	px4_state(mconfig, type, tag),
	m_rdnvram(*this, "rdnvram"),
	m_rdsocket(*this, "ramdisk_socket"),
	m_ramdisk_address(0),
	m_ramdisk(nullptr)
	{ }

	DECLARE_DRIVER_INIT( px4p );

	DECLARE_PALETTE_INIT( px4p );

	DECLARE_WRITE8_MEMBER( ramdisk_address_w );
	DECLARE_READ8_MEMBER( ramdisk_data_r );
	DECLARE_WRITE8_MEMBER( ramdisk_data_w );
	DECLARE_READ8_MEMBER( ramdisk_control_r );

protected:
	// driver_device overrides
	virtual void machine_start() override;

private:
	required_device<nvram_device> m_rdnvram;
	required_device<generic_slot_device> m_rdsocket;

	offs_t m_ramdisk_address;
	std::unique_ptr<UINT8[]> m_ramdisk;
};


//**************************************************************************
//  GAPNIT
//**************************************************************************

// process interrupts
void px4_state::gapnit_interrupt()
{
	// any interrupts enabled and pending?
	if (m_ier & m_isr & INT0_7508)
	{
		m_isr &= ~INT0_7508;
		m_z80->set_input_line_and_vector(0, ASSERT_LINE, 0xf0);
	}
	else if (m_ier & m_isr & INT1_ART)
		m_z80->set_input_line_and_vector(0, ASSERT_LINE, 0xf2);
	else if (m_ier & m_isr & INT2_ICF)
		m_z80->set_input_line_and_vector(0, ASSERT_LINE, 0xf4);
	else if (m_ier & m_isr & INT3_OVF)
		m_z80->set_input_line_and_vector(0, ASSERT_LINE, 0xf6);
	else if (m_ier & m_isr & INT4_EXT)
		m_z80->set_input_line_and_vector(0, ASSERT_LINE, 0xf8);
	else
		m_z80->set_input_line(0, CLEAR_LINE);
}

// external cassette or barcode reader input
TIMER_DEVICE_CALLBACK_MEMBER( px4_state::ext_cassette_read )
{
	UINT8 result;
	int trigger = 0;

	// sample input state
	result = (m_ext_cas->input() > 0) ? 1 : 0;

	// detect transition
	switch ((m_ctrl1 >> 1) & 0x03)
	{
	case 0: // trigger inhibit
		trigger = 0;
		break;
	case 1: // falling edge trigger
		trigger = m_ear_last_state == 1 && result == 0;
		break;
	case 2: // rising edge trigger
		trigger = m_ear_last_state == 0 && result == 1;
		break;
	case 3: // rising/falling edge trigger
		trigger = m_ear_last_state != result;
		break;
	}

	// generate an interrupt if we need to trigger
	if (trigger)
	{
		m_icrb = m_frc_value;
		m_isr |= INT2_ICF;
		gapnit_interrupt();
	}

	// save last state
	m_ear_last_state = result;
}

// free running counter
TIMER_DEVICE_CALLBACK_MEMBER( px4_state::frc_tick )
{
	m_frc_value++;

	if (m_frc_value == 0)
	{
		m_isr |= INT3_OVF;
		gapnit_interrupt();
	}
}

// input capture register low command trigger
READ8_MEMBER( px4_state::icrlc_r )
{
	if (VERBOSE)
		logerror("%s: icrlc_r\n", machine().describe_context());

	// latch value
	m_frc_latch = m_frc_value;

	return m_frc_latch & 0xff;
}

// control register 1
WRITE8_MEMBER( px4_state::ctrl1_w )
{
	const int rcv_rates[] = { 110, 150, 300, 600, 1200, 2400, 4800, 9600, 75, 1200, 19200, 38400, 200 };
	const int tra_rates[] = { 110, 150, 300, 600, 1200, 2400, 4800, 9600, 1200, 75, 19200, 38400, 200 };

	if (VERBOSE)
		logerror("%s: ctrl1_w (0x%02x)\n", machine().describe_context(), data);

	// baudrate generator
	int baud = data >> 4;

	if (baud <= 12)
	{
		if (VERBOSE)
			logerror("rcv baud = %d, tra baud = %d\n", rcv_rates[baud], tra_rates[baud]);

		set_rcv_rate(rcv_rates[baud]);
		set_tra_rate(tra_rates[baud]);
	}

	m_ctrl1 = data;
}

// input capture register high command trigger
READ8_MEMBER( px4_state::icrhc_r )
{
	if (VERBOSE)
		logerror("%s: icrhc_r\n", machine().describe_context());

	return (m_frc_latch >> 8) & 0xff;
}

// command register
WRITE8_MEMBER( px4_state::cmdr_w )
{
	if (0)
		logerror("%s: cmdr_w (0x%02x)\n", machine().describe_context(), data);

	// clear overflow interrupt?
	if (BIT(data, 2))
	{
		m_isr &= ~INT3_OVF;
		gapnit_interrupt();
	}
}

// input capture register low barcode trigger
READ8_MEMBER( px4_state::icrlb_r )
{
	if (VERBOSE)
		logerror("%s: icrlb_r\n", machine().describe_context());

	return m_icrb & 0xff;
}

// control register 2
WRITE8_MEMBER( px4_state::ctrl2_w )
{
	if (VERBOSE)
		logerror("%s: ctrl2_w (0x%02x)\n", machine().describe_context(), data);

	// bit 0, MIC, cassette output
	m_ext_cas->output( BIT(data, 0) ? -1.0 : +1.0);

	// bit 1, RMT, cassette motor
	if (BIT(data, 1))
	{
		m_ext_cas->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
		m_ext_cas_timer->adjust(attotime::zero, 0, attotime::from_hz(44100));
	}
	else
	{
		m_ext_cas->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
		m_ext_cas_timer->adjust(attotime::zero);
	}
}

// input capture register high barcode trigger
READ8_MEMBER( px4_state::icrhb_r )
{
	if (VERBOSE)
		logerror("%s: icrhb_r\n", machine().describe_context());

	// clear icf interrupt
	m_isr &= ~INT2_ICF;
	gapnit_interrupt();

	return (m_icrb >> 8) & 0xff;
}

// interrupt status register
READ8_MEMBER( px4_state::isr_r )
{
	if (VERBOSE)
		logerror("%s: isr_r\n", machine().describe_context());

	return m_isr;
}

// interrupt enable register
WRITE8_MEMBER( px4_state::ier_w )
{
	if (0)
		logerror("%s: ier_w (0x%02x)\n", machine().describe_context(), data);

	m_ier = data;
	gapnit_interrupt();
}

// status register
READ8_MEMBER( px4_state::str_r )
{
	UINT8 data = 0;

	if (0)
		logerror("%s: str_r\n", machine().describe_context());

	data |= (m_ext_cas)->input() > 0 ? 1 : 0;
	data |= 1 << 1;   // BCRD, barcode reader input
	data |= 1 << 2;   // RDY signal from 7805
	data |= 1 << 3;   // RDYSIO, enable access to the 7805
	data |= m_bankr & 0xf0;   // bit 4-7, BANK - memory bank

	return data;
}

// helper function to map rom capsules
void px4_state::install_rom_capsule(address_space &space, int size, memory_region *mem)
{
	// ram, part 1
	space.install_ram(0x0000, 0xdfff - size, 0, 0, m_ram->pointer());

	// actual rom data, part 1
	if (mem)
		space.install_rom(0xe000 - size, 0xffff, 0, 0, mem->base() + (size - 0x2000));

	// rom data, part 2
	if (mem && size != 0x2000)
		space.install_rom(0x10000 - size, 0xdfff, 0, 0, mem->base());

	// ram, continued
	space.install_ram(0xe000, 0xffff, 0, 0, m_ram->pointer() + 0xe000);
}

// bank register
WRITE8_MEMBER( px4_state::bankr_w )
{
	address_space &space_program = m_z80->space(AS_PROGRAM);

	if (0)
		logerror("%s: bankr_w (0x%02x)\n", machine().describe_context(), data);

	m_bankr = data;

	// bank switch
	switch (data >> 4)
	{
	case 0x00:
		// system bank
		space_program.install_rom(0x0000, 0x7fff, 0, 0, memregion("os")->base());
		space_program.install_ram(0x8000, 0xffff, 0, 0, m_ram->pointer() + 0x8000);
		break;

	case 0x04:
		// memory
		space_program.install_ram(0x0000, 0xffff, 0, 0, m_ram->pointer());
		break;

	case 0x08: install_rom_capsule(space_program, 0x2000, m_caps1_rom); break;
	case 0x09: install_rom_capsule(space_program, 0x4000, m_caps1_rom); break;
	case 0x0a: install_rom_capsule(space_program, 0x8000, m_caps1_rom); break;
	case 0x0c: install_rom_capsule(space_program, 0x2000, m_caps2_rom); break;
	case 0x0d: install_rom_capsule(space_program, 0x4000, m_caps2_rom); break;
	case 0x0e: install_rom_capsule(space_program, 0x8000, m_caps2_rom); break;

	default:
		if (VERBOSE)
			logerror("invalid bank switch value: 0x%02x\n", data >> 4);
		break;
	}
}

// serial io register
READ8_MEMBER( px4_state::sior_r )
{
	if (0)
		logerror("%s: sior_r 0x%02x\n", machine().describe_context(), m_sior);

	// reading clock?
	if (m_clock_state > 0)
	{
		switch (m_clock_state++)
		{
		case 1: m_sior = (dec_2_bcd(m_time.local_time.year) >> 4) & 0xf; break;
		case 2: m_sior = dec_2_bcd(m_time.local_time.year) & 0xf; break;
		case 3: m_sior = dec_2_bcd(m_time.local_time.month + 1); break;
		case 4: m_sior = dec_2_bcd(m_time.local_time.mday); break;
		case 5: m_sior = dec_2_bcd(m_time.local_time.hour); break;
		case 6: m_sior = dec_2_bcd(m_time.local_time.minute); break;
		case 7: m_sior = dec_2_bcd(m_time.local_time.second); break;
		case 8: m_sior = dec_2_bcd(m_time.local_time.weekday); break;
		}

		// done?
		if (m_clock_state == 9)
			m_clock_state = 0;
	}

	return m_sior;
}

// serial io register
WRITE8_MEMBER( px4_state::sior_w )
{
	if (0)
		logerror("%s: sior_w (0x%02x)\n", machine().describe_context(), data);

	// writing clock?
	if (m_clock_state > 0)
	{
		time_t time = m_time.time;
		struct tm *t = localtime(&time);

		switch (m_clock_state++)
		{
		case 1:
			{
				int year = dec_2_bcd(m_time.local_time.year);
				year = (year & 0xff0f) | ((data & 0xf) << 4);
				t->tm_year = bcd_2_dec(year) - 1900;
			}
			break;
		case 2:
			{
				int year = dec_2_bcd(m_time.local_time.year);
				year = (year & 0xfff0) | (data & 0xf);
				t->tm_year = bcd_2_dec(year) - 1900;
			}
			break;
		case 3: t->tm_mon = bcd_2_dec(data & 0x7f) - 1; break;
		case 4: t->tm_mday = bcd_2_dec(data & 0x7f); break;
		case 5: t->tm_hour = bcd_2_dec(data & 0x7f); break;
		case 6: t->tm_min = bcd_2_dec(data & 0x7f); break;
		case 7: t->tm_sec = bcd_2_dec(data & 0x7f); break;
		case 8: t->tm_wday = bcd_2_dec(data & 0x7f); break;
		}

		// update
		m_time.set(mktime(t));

		// done?
		if (m_clock_state == 9)
			m_clock_state = 0;
	}
	else
	{
		m_sior = data;

		switch (data)
		{
		case 0x01:
			if (VERBOSE)
				logerror("7508 cmd: Power OFF\n");

			break;

		case 0x02:
			if (VERBOSE)
				logerror("7508 cmd: Read Status\n");

			if (m_interrupt_status != 0)
			{
				if (VERBOSE)
					logerror("> 7508 has interrupts pending: 0x%02x\n", m_interrupt_status);

				// signal the interrupt(s)
				m_sior = 0xc1 | m_interrupt_status;
				m_interrupt_status = 0x00;
			}
			else if (m_key_status != 0xff)
			{
				m_sior = m_key_status;
				m_key_status = 0xff;
			}
			else
			{
				// nothing happened
				m_sior = 0xbf;
			}

			break;

		case 0x03: if (VERBOSE) logerror("7508 cmd: KB Reset\n"); break;
		case 0x04: if (VERBOSE) logerror("7508 cmd: KB Repeat Timer 1 Set\n"); break;
		case 0x14: if (VERBOSE) logerror("7508 cmd: KB Repeat Timer 2 Set\n"); break;
		case 0x24: if (VERBOSE) logerror("7508 cmd: KB Repeat Timer 1 Read\n"); break;
		case 0x34: if (VERBOSE) logerror("7508 cmd: KB Repeat Timer 2 Read\n"); break;
		case 0x05: if (VERBOSE) logerror("7508 cmd: KB Repeat OFF\n"); break;
		case 0x15: if (VERBOSE) logerror("7508 cmd: KB Repeat ON\n"); break;

		case 0x06:
			if (VERBOSE)
				logerror("7508 cmd: KB Interrupt OFF\n");

			m_key_int_enabled = false;
			break;

		case 0x16:
			if (VERBOSE)
				logerror("7508 cmd: KB Interrupt ON\n");

			m_key_int_enabled = true;
			break;

		case 0x07:
			if (VERBOSE)
				logerror("7508 cmd: Clock Read\n");

			m_clock_state = 1;
			break;

		case 0x17:
			if (VERBOSE)
				logerror("7508 cmd: Clock Write\n");

			m_clock_state = 1;
			break;

		case 0x08:
			if (VERBOSE)
				logerror("7508 cmd: Power Switch Read\n");

			// indicate that the power switch is in the "ON" position
			m_sior = 0x01;
			break;

		case 0x09: if (VERBOSE) logerror("7508 cmd: Alarm Read\n"); break;
		case 0x19: if (VERBOSE) logerror("7508 cmd: Alarm Set\n"); break;
		case 0x29: if (VERBOSE) logerror("7508 cmd: Alarm OFF\n"); break;
		case 0x39: if (VERBOSE) logerror("7508 cmd: Alarm ON\n"); break;

		case 0x0a:
			if (VERBOSE)
				logerror("7508 cmd: DIP Switch Read\n");
			m_sior = ioport("dips")->read();
			break;

		case 0x0b: if (VERBOSE) logerror("7508 cmd: Stop Key Interrupt disable\n"); break;
		case 0x1b: if (VERBOSE) logerror("7508 cmd: Stop Key Interrupt enable\n"); break;
		case 0x0c: if (VERBOSE) logerror("7508 cmd: 7 chr. Buffer\n"); break;
		case 0x1c: if (VERBOSE) logerror("7508 cmd: 1 chr. Buffer\n"); break;

		case 0x0d:
			if (VERBOSE)
				logerror("7508 cmd: 1 sec. Interrupt OFF\n");

			m_one_sec_int_enabled = false;
			break;

		case 0x1d:
			if (VERBOSE)
				logerror("7508 cmd: 1 sec. Interrupt ON\n");

			m_one_sec_int_enabled = true;
			break;

		case 0x0e:
			if (VERBOSE)
				logerror("7508 cmd: KB Clear\n");

			m_sior = 0xbf;
			break;

		case 0x0f: if (VERBOSE) logerror("7508 cmd: System Reset\n"); break;
		}
	}
}


//**************************************************************************
//  GAPNDL
//**************************************************************************

// vram start address register
WRITE8_MEMBER( px4_state::vadr_w )
{
	if (VERBOSE)
		logerror("%s: vadr_w (0x%02x)\n", machine().describe_context(), data);

	m_vadr = data;
}

// y offset register
WRITE8_MEMBER( px4_state::yoff_w )
{
	if (VERBOSE)
		logerror("%s: yoff_w (0x%02x)\n", machine().describe_context(), data);

	m_yoff = data;
}

// frame register
WRITE8_MEMBER( px4_state::fr_w )
{
	if (VERBOSE)
		logerror("%s: fr_w (0x%02x)\n", machine().describe_context(), data);
}

// speed-up register
WRITE8_MEMBER( px4_state::spur_w )
{
	if (VERBOSE)
		logerror("%s: spur_w (0x%02x)\n", machine().describe_context(), data);
}


//**************************************************************************
//  GAPNIO
//**************************************************************************

WRITE_LINE_MEMBER( px4_state::serial_rx_w )
{
	m_serial_rx = state;
	device_serial_interface::rx_w(state);
}

WRITE_LINE_MEMBER( px4_state::sio_rx_w )
{
	if (!BIT(m_swr, 3) && BIT(m_swr, 2))
		serial_rx_w(state);
}

WRITE_LINE_MEMBER( px4_state::sio_pin_w )
{
	m_sio_pin = state;
}

WRITE_LINE_MEMBER( px4_state::rs232_rx_w )
{
	if (BIT(m_swr, 3))
		serial_rx_w(state);
}

WRITE_LINE_MEMBER( px4_state::rs232_dcd_w )
{
	m_rs232_dcd = state;
}

WRITE_LINE_MEMBER( px4_state::rs232_dsr_w )
{
	m_artsr |= !state << 7;
}

WRITE_LINE_MEMBER( px4_state::rs232_cts_w )
{
	m_rs232_cts = state;
}

void px4_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}

void px4_state::tra_callback()
{
	if (ART_TX_ENABLED)
	{
		if (ART_BREAK)
			txd_w(0); // transmit break
		else
			txd_w(transmit_register_get_data_bit()); // transmit data
	}
	else
		txd_w(1); // transmit mark
}

void px4_state::tra_complete()
{
	if (m_artsr & ART_TXRDY)
	{
		// no more bytes, buffer now empty
		m_artsr |= ART_TXEMPTY;
	}
	else
	{
		// transfer next byte
		transmit_register_setup(m_artdor);
		m_artsr |= ART_TXRDY;
	}
}

void px4_state::rcv_callback()
{
	if (ART_RX_ENABLED)
		receive_register_update_bit(m_serial_rx); // receive data
}

void px4_state::rcv_complete()
{
	receive_register_extract();
	m_artdir = get_received_char();

	// TODO: verify parity and framing

	// overrun?
	if (m_artsr & ART_RXRDY)
		m_artsr |= ART_OE;

	// set ready and signal interrupt
	m_artsr |= ART_RXRDY;
	m_isr |= INT1_ART;
	gapnit_interrupt();
}

// helper function to write to selected serial port
void px4_state::txd_w(int data)
{
	if (BIT(m_swr, 2))
		// to sio
		m_sio->tx_w(data);
	else
		if (BIT(m_swr, 3))
			// to rs232
			m_rs232->write_txd(data);
		// else to cartridge
}

// cartridge interface
READ8_MEMBER( px4_state::ctgif_r )
{
	if (VERBOSE)
		logerror("%s: ctgif_r @ 0x%02x\n", machine().describe_context(), offset);

	return 0x00;
}

// cartridge interface
WRITE8_MEMBER( px4_state::ctgif_w )
{
	if (VERBOSE)
		logerror("%s: ctgif_w (0x%02x @ 0x%02x)\n", machine().describe_context(), data, offset);
}

// art data input register
READ8_MEMBER( px4_state::artdir_r )
{
	if (VERBOSE)
		logerror("%s: artdir_r (%02x)\n", machine().describe_context(), m_artdir);

	// clear ready
	m_artsr &= ~ART_RXRDY;

	// clear interrupt
	m_isr &= ~INT1_ART;
	gapnit_interrupt();

	return m_artdir;
}

// art data output register
WRITE8_MEMBER( px4_state::artdor_w )
{
	if (VERBOSE)
		logerror("%s: artdor_w (0x%02x)\n", machine().describe_context(), data);

	m_artdor = data;

	// new data to transmit?
	if (ART_TX_ENABLED && is_transmit_register_empty())
	{
		transmit_register_setup(m_artdor);
		m_artsr &= ~ART_TXEMPTY;
	}
	else
	{
		// clear ready
		m_artsr &= ~ART_TXRDY;
	}
}

// art status register
READ8_MEMBER( px4_state::artsr_r )
{
	if (0)
		logerror("%s: artsr_r (%02x)\n", machine().describe_context(), m_artsr);

	return m_artsr;
}

// art mode register
WRITE8_MEMBER( px4_state::artmr_w )
{
	int data_bit_count = BIT(data, 2) ? 8 : 7;
	parity_t parity = BIT(data, 4) ? (BIT(data, 5) ? PARITY_EVEN : PARITY_ODD) : PARITY_NONE;
	stop_bits_t stop_bits = BIT(data, 7) ? STOP_BITS_2 : STOP_BITS_1;

	if (VERBOSE)
		logerror("%s: serial frame setup: %d-%s-%d\n", tag().c_str(), data_bit_count, device_serial_interface::parity_tostring(parity), stop_bits);

	set_data_frame(1, data_bit_count, parity, stop_bits);
}

// io status register
READ8_MEMBER( px4_state::iostr_r )
{
	UINT8 data = 0;

	// centronics status
	data |= m_centronics_busy << 0;
	data |= m_centronics_perror << 1;

	// sio status
	data |= !m_sio_pin << 2;

	// serial data
	data |= m_serial_rx << 3;

	// rs232 status
	data |= !m_rs232_dcd << 4;
	data |= !m_rs232_cts << 5;

	data |= 1 << 6;   // bit 6, csel, cartridge option select signal
	data |= 0 << 7;   // bit 7, caud - audio input from cartridge

	if (0)
		logerror("%s: iostr_r: rx = %d, dcd = %d, cts = %d\n", machine().describe_context(), BIT(data, 3), BIT(data, 4), BIT(data, 5));

	return data;
}

// art command register
WRITE8_MEMBER( px4_state::artcr_w )
{
	if (VERBOSE)
		logerror("%s: artcr_w (0x%02x)\n", machine().describe_context(), data);

	m_artcr = data;

	// error reset
	if (BIT(data, 4))
		m_artsr &= ~(ART_PE | ART_OE | ART_FE);

	// rs232
	m_rs232->write_dtr(!BIT(data, 1));
	m_rs232->write_rts(!BIT(data, 5));
}

// switch register
WRITE8_MEMBER( px4_state::swr_w )
{
	if (VERBOSE)
	{
		const char *cart_mode[] = { "hs", "io", "db", "ot" };
		const char *ser_mode[] = { "cart sio / cart sio", "sio / sio", "rs232 / rs232", "rs232 / sio" };
		logerror("%s: px4_swr_w: cartridge mode: %s, serial mode: %s, audio: %s\n", machine().describe_context(),
			cart_mode[data & 0x03], ser_mode[(data >> 2) & 0x03], BIT(data, 4) ? "on" : "off");
	}

	m_swr = data;
}

// io control register
WRITE8_MEMBER( px4_state::ioctlr_w )
{
	if (VERBOSE)
		logerror("%s: ioctlr_w (0x%02x)\n", machine().describe_context(), data);

	m_centronics->write_strobe(!BIT(data, 0));
	m_centronics->write_init(BIT(data, 1));

	m_sio->pout_w(BIT(data, 2));

	// bit 3, cartridge reset

	output().set_value("led_0", BIT(data, 4)); // caps lock
	output().set_value("led_1", BIT(data, 5)); // num lock
	output().set_value("led_2", BIT(data, 6)); // "led 2"

	m_speaker->level_w(BIT(data, 7));
}


//**************************************************************************
//  7508 RELATED
//**************************************************************************

TIMER_DEVICE_CALLBACK_MEMBER( px4_state::upd7508_1sec_callback )
{
	// adjust interrupt status
	m_interrupt_status |= UPD7508_INT_ONE_SECOND;

	// are interrupts enabled?
	if (m_one_sec_int_enabled)
	{
		m_isr |= INT0_7508;
		gapnit_interrupt();
	}

	// update clock
	m_time.set(m_time.time + 1);
}

INPUT_CHANGED_MEMBER( px4_state::key_callback )
{
	UINT32 oldvalue = oldval * field.mask(), newvalue = newval * field.mask();
	UINT32 delta = oldvalue ^ newvalue;
	int i, scancode = 0xff, down = 0;

	for (i = 0; i < 32; i++)
	{
		if (delta & (1 << i))
		{
			down = (newvalue & (1 << i)) ? 0x10 : 0x00;
			scancode = (FPTR)param * 32 + i;

			// control keys
			if ((scancode & 0xa0) == 0xa0)
				scancode |= down;

			if (VERBOSE)
				logerror("upd7508: key callback, key=0x%02x\n", scancode);

			break;
		}
	}

	if (down || (scancode & 0xa0) == 0xa0)
	{
		m_key_status = scancode;

		if (m_key_int_enabled)
		{
			if (VERBOSE)
				logerror("upd7508: key interrupt\n");

			m_isr |= INT0_7508;
			gapnit_interrupt();
		}
	}
}


//**************************************************************************
//  EXTERNAL RAM-DISK
//**************************************************************************

WRITE8_MEMBER( px4p_state::ramdisk_address_w )
{
	switch (offset)
	{
	case 0x00: m_ramdisk_address = (m_ramdisk_address & 0xffff00) | ((data & 0xff) <<  0); break;
	case 0x01: m_ramdisk_address = (m_ramdisk_address & 0xff00ff) | ((data & 0xff) <<  8); break;
	case 0x02: m_ramdisk_address = (m_ramdisk_address & 0x00ffff) | ((data & 0x07) << 16); break;
	}
}

READ8_MEMBER( px4p_state::ramdisk_data_r )
{
	UINT8 ret = 0xff;

	if (m_ramdisk_address < 0x20000)
	{
		// read from ram
		ret = m_ramdisk[m_ramdisk_address];
	}
	else if (m_ramdisk_address < 0x40000)
	{
		// read from rom
		ret = m_rdsocket->read_rom(space, m_ramdisk_address);
	}

	m_ramdisk_address = (m_ramdisk_address & 0xffff00) | ((m_ramdisk_address & 0xff) + 1);

	return ret;
}

WRITE8_MEMBER( px4p_state::ramdisk_data_w )
{
	if (m_ramdisk_address < 0x20000)
		m_ramdisk[m_ramdisk_address] = data;

	m_ramdisk_address = (m_ramdisk_address & 0xffff00) | ((m_ramdisk_address & 0xff) + 1);
}

READ8_MEMBER( px4p_state::ramdisk_control_r )
{
	// bit 7 determines the presence of a ram-disk
	return 0x7f;
}

//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

UINT32 px4_state::screen_update_px4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// display enabled?
	if (BIT(m_yoff, 7))
	{
		int y, x;

		// get vram start address
		UINT8 *vram = &m_ram->pointer()[(m_vadr & 0xf8) << 8];

		for (y = 0; y < 64; y++)
		{
			// adjust against y-offset
			UINT8 row = (y - (m_yoff & 0x3f)) & 0x3f;

			for (x = 0; x < 240/8; x++)
			{
				bitmap.pix16(row, x * 8 + 0) = BIT(*vram, 7);
				bitmap.pix16(row, x * 8 + 1) = BIT(*vram, 6);
				bitmap.pix16(row, x * 8 + 2) = BIT(*vram, 5);
				bitmap.pix16(row, x * 8 + 3) = BIT(*vram, 4);
				bitmap.pix16(row, x * 8 + 4) = BIT(*vram, 3);
				bitmap.pix16(row, x * 8 + 5) = BIT(*vram, 2);
				bitmap.pix16(row, x * 8 + 6) = BIT(*vram, 1);
				bitmap.pix16(row, x * 8 + 7) = BIT(*vram, 0);

				vram++;
			}

			// skip the last 2 unused bytes
			vram += 2;
		}
	}
	else
	{
		// display is disabled, draw an empty screen
		bitmap.fill(0, cliprect);
	}

	return 0;
}


//**************************************************************************
//  DRIVER INIT
//**************************************************************************

DRIVER_INIT_MEMBER( px4_state, px4 )
{
	// map os rom and last half of memory
	membank("bank1")->set_base(memregion("os")->base());
	membank("bank2")->set_base(m_ram->pointer() + 0x8000);
}

DRIVER_INIT_MEMBER( px4p_state, px4p )
{
	DRIVER_INIT_CALL(px4);

	// reserve memory for external ram-disk
	m_ramdisk = std::make_unique<UINT8[]>(0x20000);
}

void px4_state::machine_start()
{
	std::string region_tag;
	m_caps1_rom = memregion(region_tag.assign(m_caps1->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	m_caps2_rom = memregion(region_tag.assign(m_caps2->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	m_nvram->set_base(m_ram->pointer(), 0x10000);

	// initialize clock
	machine().base_datetime(m_time);
}

void px4_state::machine_reset()
{
	m_artsr = ART_TXRDY | ART_TXEMPTY | (!m_rs232->dsr_r() << 7);
	receive_register_reset();
	transmit_register_reset();
}

void px4p_state::machine_start()
{
	px4_state::machine_start();
	m_rdnvram->set_base(m_ramdisk.get(), 0x20000);
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( px4_mem, AS_PROGRAM, 8, px4_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_RAMBANK("bank2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( px4_io, AS_IO, 8, px4_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// gapnit, 0x00-0x07
	AM_RANGE(0x00, 0x00) AM_READWRITE( icrlc_r, ctrl1_w )
	AM_RANGE(0x01, 0x01) AM_READWRITE( icrhc_r, cmdr_w )
	AM_RANGE(0x02, 0x02) AM_READWRITE( icrlb_r, ctrl2_w )
	AM_RANGE(0x03, 0x03) AM_READ( icrhb_r )
	AM_RANGE(0x04, 0x04) AM_READWRITE( isr_r, ier_w )
	AM_RANGE(0x05, 0x05) AM_READWRITE( str_r, bankr_w )
	AM_RANGE(0x06, 0x06) AM_READWRITE( sior_r, sior_w )
	AM_RANGE(0x07, 0x07) AM_NOP
	// gapndl, 0x08-0x0f
	AM_RANGE(0x08, 0x08) AM_WRITE( vadr_w )
	AM_RANGE(0x09, 0x09) AM_WRITE( yoff_w )
	AM_RANGE(0x0a, 0x0a) AM_WRITE( fr_w )
	AM_RANGE(0x0b, 0x0b) AM_WRITE( spur_w )
	AM_RANGE(0x0c, 0x0f) AM_NOP
	// gapnio, 0x10-0x1f
	AM_RANGE(0x10, 0x13) AM_READWRITE( ctgif_r, ctgif_w )
	AM_RANGE(0x14, 0x14) AM_READWRITE( artdir_r, artdor_w )
	AM_RANGE(0x15, 0x15) AM_READWRITE( artsr_r, artmr_w )
	AM_RANGE(0x16, 0x16) AM_READWRITE( iostr_r, artcr_w )
	AM_RANGE(0x17, 0x17) AM_DEVWRITE("cent_data_out", output_latch_device, write)
	AM_RANGE(0x18, 0x18) AM_WRITE( swr_w )
	AM_RANGE(0x19, 0x19) AM_WRITE( ioctlr_w )
	AM_RANGE(0x1a, 0x1f) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( px4p_io, AS_IO, 8, px4p_state )
	AM_IMPORT_FROM(px4_io)
	AM_RANGE(0x90, 0x92) AM_WRITE(ramdisk_address_w )
	AM_RANGE(0x93, 0x93) AM_READWRITE(ramdisk_data_r, ramdisk_data_w )
	AM_RANGE(0x94, 0x94) AM_READ(ramdisk_control_r)
ADDRESS_MAP_END


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

/* The PX-4 has an exchangeable keyboard. Available is a standard ASCII
 * keyboard and an "item" keyboard, as well as regional variants for
 * UK, France, Germany, Denmark, Sweden, Norway, Italy and Spain.
 */

// configuration dip switch found on the rom capsule board
static INPUT_PORTS_START( px4_dips )
	PORT_START("dips")

	PORT_DIPNAME(0x0f, 0x0f, "Character set")
	PORT_DIPLOCATION("PX-4:8,7,6,5")
	PORT_DIPSETTING(0x0f, "ASCII")
	PORT_DIPSETTING(0x0e, "France")
	PORT_DIPSETTING(0x0d, "Germany")
	PORT_DIPSETTING(0x0c, "England")
	PORT_DIPSETTING(0x0b, "Denmark")
	PORT_DIPSETTING(0x0a, "Sweden")
	PORT_DIPSETTING(0x09, "Italy")
	PORT_DIPSETTING(0x08, "Spain")
	PORT_DIPSETTING(0x07, DEF_STR(Japan))
	PORT_DIPSETTING(0x06, "Norway")

	PORT_DIPNAME(0x30, 0x30, "LST device")
	PORT_DIPLOCATION("PX-4:4,3")
	PORT_DIPSETTING(0x00, "SIO")
	PORT_DIPSETTING(0x10, "Cartridge printer")
	PORT_DIPSETTING(0x20, "RS-232C")
	PORT_DIPSETTING(0x30, "Centronics printer")

	// available for user applications
	PORT_DIPNAME(0x40, 0x40, "Not used")
	PORT_DIPLOCATION("PX-4:2")
	PORT_DIPSETTING(0x40, "Enable")
	PORT_DIPSETTING(0x00, "Disable")

	// this is automatically selected by the os, the switch has no effect
	PORT_DIPNAME(0x80, 0x00, "Keyboard type")
	PORT_DIPLOCATION("PX-4:1")
	PORT_DIPSETTING(0x80, "Item keyboard")
	PORT_DIPSETTING(0x00, "Standard keyboard")
INPUT_PORTS_END

// US ASCII keyboard
static INPUT_PORTS_START( px4_h450a )
	PORT_INCLUDE(px4_dips)

	PORT_START("keyboard_0")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(ESC)) // 00
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(PAUSE))   // 01
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F6))    PORT_NAME("Help") // 02
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F1))    PORT_NAME("PF1")  // 03
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F2))    PORT_NAME("PF2")  // 04
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F3))    PORT_NAME("PF3")  // 05
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F4))    PORT_NAME("PF4")  // 06
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F5))    PORT_NAME("PF5")  // 07
	PORT_BIT(0x0000ff00, IP_ACTIVE_HIGH, IPT_UNUSED)    // 08-0f
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_ESC)  PORT_CHAR(UCHAR_MAMEKEY(CANCEL)) PORT_NAME("Stop")  // 10
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_1)   PORT_CHAR('1') PORT_CHAR('!')    // 11
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_2)   PORT_CHAR('2') PORT_CHAR('"')    // 12
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_3)   PORT_CHAR('3') PORT_CHAR('#')    // 13
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_4)   PORT_CHAR('4') PORT_CHAR('$')    // 14
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_5)   PORT_CHAR('5') PORT_CHAR('%')    // 15
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_6)   PORT_CHAR('6') PORT_CHAR('&')    // 16
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)nullptr) PORT_CODE(KEYCODE_7)   PORT_CHAR('7') PORT_CHAR('\'')   // 17
	PORT_BIT(0xff000000, IP_ACTIVE_HIGH, IPT_UNUSED)    // 18-1f

	PORT_START("keyboard_1")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')  // 20
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')  // 21
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')  // 22
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')  // 23
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')  // 24
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')  // 25
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')  // 26
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')  // 27
	PORT_BIT(0x0000ff00, IP_ACTIVE_HIGH, IPT_UNUSED)    // 28-2f
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D')  // 30
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F')  // 31
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')  // 32
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H')  // 33
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')  // 34
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')  // 35
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')  // 36
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)1) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')  // 37
	PORT_BIT(0xff000000, IP_ACTIVE_HIGH, IPT_UNUSED)    // 38-3f

	PORT_START("keyboard_2")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')  // 40
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_N)     PORT_CHAR('n') PORT_CHAR('N')  // 41
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_M)     PORT_CHAR('m') PORT_CHAR('M')  // 42
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')  // 43
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')  // 44
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')  // 45
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_F9)    PORT_CHAR('[') PORT_CHAR('{')  // 46
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_F10)   PORT_CHAR(']') PORT_CHAR('}')  // 47
	PORT_BIT(0x0000ff00, IP_ACTIVE_HIGH, IPT_UNUSED)    // 48-4f
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')  // 50
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')  // 51
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_0)         PORT_CHAR('0') PORT_CHAR('_')  // 52
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('=')  // 53
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('^') PORT_CHAR('~')  // 54
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))   // 55
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)   PORT_CHAR(UCHAR_MAMEKEY(HOME))  // 56
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)2) PORT_CODE(KEYCODE_TAB)       PORT_CHAR('\t')    // 57
	PORT_BIT(0xff000000, IP_ACTIVE_HIGH, IPT_UNUSED)    // 58-5f

	PORT_START("keyboard_3")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O')  // 60
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_P)         PORT_CHAR('p') PORT_CHAR('P')  // 61
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR(96)   // 62
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT)) // 63
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN)) // 64
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))    // 65
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')  // 66
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_S)         PORT_CHAR('s') PORT_CHAR('S')  // 67
	PORT_BIT(0x0000ff00, IP_ACTIVE_HIGH, IPT_UNUSED)    // 48-4f
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(':')  PORT_CHAR('*') // 70
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)  // 71
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') // 72
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ') // 73
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_Z)         PORT_CHAR('z') PORT_CHAR('Z')  // 74
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_X)         PORT_CHAR('x') PORT_CHAR('X')  // 75
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_C)         PORT_CHAR('c') PORT_CHAR('C')  // 76
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)3) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')  // 77
	PORT_BIT(0xff000000, IP_ACTIVE_HIGH, IPT_UNUSED)    // 58-5f

	PORT_START("keyboard_4")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)4) PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CHAR(UCHAR_MAMEKEY(PRTSCR)) // 80
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)4) PORT_CODE(KEYCODE_DEL)    PORT_CHAR(UCHAR_MAMEKEY(DEL))    PORT_CHAR(12)   // 81
	PORT_BIT(0xfffffffc, IP_ACTIVE_HIGH, IPT_UNUSED)    // 82-9f

	PORT_START("keyboard_5")
	PORT_BIT(0x00000003, IP_ACTIVE_HIGH, IPT_UNUSED)    // a0-a1
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)5) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)    // a2
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)5) PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)    // a3
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)5) PORT_CODE(KEYCODE_LALT)     PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))  // a4
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)5) PORT_CODE(KEYCODE_RALT)     PORT_NAME("Graph")  // a5
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)5) PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)    // a6
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, px4_state, key_callback, (void *)5) PORT_CODE(KEYCODE_NUMLOCK)  PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))   // a7
	PORT_BIT(0xffffff00, IP_ACTIVE_HIGH, IPT_UNUSED)    // a8-bf /* b2-b7 are the 'make' codes for the above keys */
INPUT_PORTS_END

#if 0

/* item keyboard */
static INPUT_PORTS_START( px4_h421a )
	PORT_INCLUDE(px4_dips)
INPUT_PORTS_END

#endif

//**************************************************************************
//  PALETTE
//**************************************************************************

PALETTE_INIT_MEMBER( px4_state, px4 )
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

PALETTE_INIT_MEMBER( px4p_state, px4p )
{
	palette.set_pen_color(0, rgb_t(149, 157, 130));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static MACHINE_CONFIG_START( px4, px4_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, XTAL_7_3728MHz / 2)    // uPD70008
	MCFG_CPU_PROGRAM_MAP(px4_mem)
	MCFG_CPU_IO_MAP(px4_io)

	// video hardware
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(72)
	MCFG_SCREEN_SIZE(240, 64)
	MCFG_SCREEN_VISIBLE_AREA(0, 239, 0, 63)
	MCFG_SCREEN_UPDATE_DRIVER(px4_state, screen_update_px4)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_px4)

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(px4_state, px4)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("one_sec", px4_state, upd7508_1sec_callback, attotime::from_seconds(1))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("frc", px4_state, frc_tick, attotime::from_hz(XTAL_7_3728MHz / 2 / 6))

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64k")
	MCFG_NVRAM_ADD_NO_FILL("nvram")

	// centronics printer
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(px4_state, centronics_busy_w))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(px4_state, centronics_perror_w))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	// external cassette
	MCFG_CASSETTE_ADD("extcas")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED)

	MCFG_TIMER_DRIVER_ADD("extcas_timer", px4_state, ext_cassette_read)

	// sio port
	MCFG_EPSON_SIO_ADD("sio", nullptr)
	MCFG_EPSON_SIO_RX(WRITELINE(px4_state, sio_rx_w))
	MCFG_EPSON_SIO_PIN(WRITELINE(px4_state, sio_pin_w))

	// rs232 port
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(px4_state, rs232_rx_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE(px4_state, rs232_dcd_w))
	MCFG_RS232_DSR_HANDLER(WRITELINE(px4_state, rs232_dsr_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE(px4_state, rs232_cts_w))

	// rom capsules
	MCFG_GENERIC_CARTSLOT_ADD("capsule1", generic_plain_slot, "px4_cart")
	MCFG_GENERIC_CARTSLOT_ADD("capsule2", generic_plain_slot, "px4_cart")

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list", "px4_cart")
	MCFG_SOFTWARE_LIST_ADD("epson_cpm_list", "epson_cpm")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( px4p, px4, px4p_state )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(px4p_io)

	MCFG_NVRAM_ADD_0FILL("rdnvram")

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(px4p_state, px4p)

	MCFG_GENERIC_CARTSLOT_ADD("ramdisk_socket", generic_plain_slot, "px4_cart")
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// Note: We are missing "Kana OS V1.0" and "Kana OS V2.0" (Japanese version)

ROM_START( px4 )
	ROM_REGION(0x8000, "os", 0)
	ROM_SYSTEM_BIOS(0, "default",  "PX-4 OS ROM")
	ROMX_LOAD("m25122aa_po_px4.10c", 0x0000, 0x8000, CRC(62d60dc6) SHA1(3d32ec79a317de7c84c378302e95f48d56505502), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "ramtest",  "PX-4/PX-8 DRAM Test Ver. 1.0")
	ROMX_LOAD("ramtest.10c", 0x0000, 0x8000, CRC(f8aced5f) SHA1(a5a2f398e602aa349c3636d6659dd0c7eaba07fb), ROM_BIOS(2))

	ROM_REGION(0x1000, "slave", 0)
	ROM_LOAD("upd7508.bin", 0x0000, 0x1000, NO_DUMP)
ROM_END

ROM_START( px4p )
	ROM_REGION(0x8000, "os", 0)
	ROM_SYSTEM_BIOS(0, "default",  "PX-4+ OS ROM")
	ROMX_LOAD("b0_pxa.10c", 0x0000, 0x8000, CRC(d74b9ef5) SHA1(baceee076c12f5a16f7a26000e9bc395d021c455), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "ramtest",  "PX-4/PX-8 DRAM Test Ver. 1.0")
	ROMX_LOAD("ramtest.10c", 0x0000, 0x8000, CRC(f8aced5f) SHA1(a5a2f398e602aa349c3636d6659dd0c7eaba07fb), ROM_BIOS(2))

	ROM_REGION(0x1000, "slave", 0)
	ROM_LOAD("upd7508.bin", 0x0000, 0x1000, NO_DUMP)
ROM_END


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT      CLASS       INIT  COMPANY  FULLNAME  FLAGS
COMP( 1985, px4,  0,      0,      px4,     px4_h450a, px4_state,  px4,  "Epson", "PX-4",   0 )
COMP( 1985, px4p, px4,    0,      px4p,    px4_h450a, px4p_state, px4p, "Epson", "PX-4+",  0 )
