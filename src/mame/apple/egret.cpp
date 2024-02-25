// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "Egret" ADB/I2C/power management microcontroller
    Emulation by R. Belmont

    Egret is a 68HC05EG with 2 8-bit GPIO ports and 1 4-bit GPIO port.  Egret handles
    ADB, I2C, and basic power management.  The 68HC05EG is basically the same as a 68HC05E1.

    TODO: make proper 68HC05E1 base class and rebase this and Cuda on it.  Merge this with Cuda?

    Port definitions, primarily from the schematics.

    Port A:

    x-------  O  ADB data line out
    -x------  I  ADB data line in
    --x-----  I  System type.  0 = hardware power switch, 1 = Egret controls power on/off
    ---x----  O  DFAC latch
    ----x---  O  ? (asserted briefly when resetting 680x0)
    -----x--  I  Keyboard power switch
    ------x-  ?  PSU enable OUT for system type 0, otherwise chassis power switch IN
    -------x  ?  Control panel enable IN for system type 0 (e.g. LC), power supply enable OUT for type 1

    Port B:

    x-------  O  DFAC bit clock (I2C SCL)
    -x------  ?  DFAC data I/O (I2C SDA)
    --x-----  ?  VIA shift register data (used in both directions)
    ---x----  O  VIA clock
    ----x---  I  VIA SYS_SESSION
    -----x--  I  VIA VIA_FULL
    ------x-  O  VIA XCEIVER SESSION
    -------x  I  +5v sense

    Port C:
    x---      O  680x0 reset
    -x--      ?  680x0 IPL 2 (used in both directions)
    --xx      ?  IPL 1/0 for hardware power switch, trickle sense (bit 1) and pulled up to +5v (bit 0) if Egret controls the PSU

    Egret version spotting:
    341S0850 - 0x???? (1.01, earlier) - LC, LC II
    341S0851 - 0x0101 (1.01, later) - Classic II, IIsi, IIvx/IIvi, LC III
    344S0100 - 0x0100 (1.00) - Some (early production?) IIsi

    Caboose version spotting:
    341S0853 - 0x0100 (1.00) - Quadra 950

    Caboose appears to be a slightly modified version of Egret with keysense support added to its power management functions.
    It may also retain Egret's ADB interface, though the Quadra 900 & 950 disconnect its port pins from the ADB data line,
    which is instead connected to a PIC.
*/

#include "emu.h"
#include "egret.h"
#include "cpu/m6805/m6805.h"

#define LOG_ADB (1U << 1)      // low-level ADB details
#define LOG_I2C (1U << 2)      // low-level I2C details
#define LOG_PRAM (1U << 3)     // PRAM handling info
#define LOG_HOSTCOMM (1U << 4) // communications with the host

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(EGRET, egret_device, "egret", "Apple Egret ADB/I2C")

ROM_START( egret )
	ROM_REGION(0x1100, "roms", 0)
	ROM_DEFAULT_BIOS("341s0851")

	ROM_SYSTEM_BIOS(0, "344s0100", "Egret 1.00 (344S0100)")
	ROMX_LOAD("344s0100.bin", 0x0000, 0x1100, CRC(59e2b6b6) SHA1(540e752b7da521f1bdb16e0ad7c5f46ddc92d4e9), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "341s0850", "Egret 1.01 (earlier) (341S0850)")
	ROMX_LOAD("341s0850.bin", 0x0000, 0x1100, CRC(4906ecd0) SHA1(95e08ba0c5d4b242f115f104aba9905dbd3fd87c), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "341s0851", "Egret 1.01 (341S0851)")
	ROMX_LOAD("341s0851.bin", 0x0000, 0x1100, CRC(ea9ea6e4) SHA1(8b0dae3ec66cdddbf71567365d2c462688aeb571), ROM_BIOS(2))
ROM_END

	//-------------------------------------------------
	//  ADDRESS_MAP
	//-------------------------------------------------

	void egret_device::egret_map(address_map &map)
	{
		map(0x0000, 0x0002).rw(FUNC(egret_device::ports_r), FUNC(egret_device::ports_w));
		map(0x0004, 0x0006).rw(FUNC(egret_device::ddr_r), FUNC(egret_device::ddr_w));
		map(0x0007, 0x0007).rw(FUNC(egret_device::pll_r), FUNC(egret_device::pll_w));
		map(0x0008, 0x0008).rw(FUNC(egret_device::timer_ctrl_r), FUNC(egret_device::timer_ctrl_w));
		map(0x0009, 0x0009).rw(FUNC(egret_device::timer_counter_r), FUNC(egret_device::timer_counter_w));
		map(0x0012, 0x0012).rw(FUNC(egret_device::onesec_r), FUNC(egret_device::onesec_w));
		map(0x0090, 0x00ff).ram().share(m_internal_ram); // work RAM and stack
		map(0x0100, 0x01ff).rw(FUNC(egret_device::pram_r), FUNC(egret_device::pram_w));
		map(0x0f00, 0x1fff).rom().region("roms", 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void egret_device::device_add_mconfig(machine_config &config)
{
	M68HC05EG(config, m_maincpu, XTAL(32'768)*128);  // Intended to run 4.1 MHz, the ADB timings in uS are twice as long as spec at 2.1
	m_maincpu->set_addrmap(AS_PROGRAM, &egret_device::egret_map);

	#if USE_BUS_ADB
	ADB_CONNECTOR(config, "adb1", adb_device::default_devices, "a9m0330", false);
	#endif
}

const tiny_rom_entry *egret_device::device_rom_region() const
{
	return ROM_NAME( egret );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************
egret_device::egret_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, EGRET, tag, owner, clock),
	  device_nvram_interface(mconfig, *this),
	  write_reset(*this),
	  write_linechange(*this),
	  write_via_clock(*this),
	  write_via_data(*this),
	  write_iic_scl(*this),
	  write_iic_sda(*this),
	  m_maincpu(*this, "egret"),
	  m_internal_ram(*this, "internal_ram"),
	  m_rom(*this, "roms"),
	  m_pll_ctrl(0), m_timer_ctrl(0), m_onesec(0),
	  m_xcvr_session(0), m_via_full(0), m_sys_session(0), m_via_data(0), m_via_clock(0), m_last_adb(0),
	  m_last_adb_time(0), m_egret_controls_power(false), m_adb_in(false),
	  m_reset_line(0), m_adb_dtime(0), m_pram_loaded(false), m_iic_sda(1)
#if USE_BUS_ADB
	  ,
	  m_adb_connector{{*this, "adb1"}, {*this, finder_base::DUMMY_TAG}}
#endif
{
	std::fill(std::begin(m_pram), std::end(m_pram), 0);
	std::fill(std::begin(m_disk_pram), std::end(m_disk_pram), 0);
	std::fill(std::begin(m_ports), std::end(m_ports), 0);
	std::fill(std::begin(m_ddrs), std::end(m_ddrs), 0);
}

void egret_device::device_start()
{
#if USE_BUS_ADB
	for (int i = 0; i < 2; i++)
	{
		m_adb_device[i] = m_adb_connector[i] ? m_adb_connector[i]->get_device() : nullptr;
		if (m_adb_device[i])
		{
			m_adb_device[i]->adb_r().set([this, i](int state)
										 { adb_w(i, state); });
			m_adb_device[i]->poweron_r().set([this, i](int state)
											 { adb_poweron_w(i, state); });
		}
	}
#endif

	m_timer = timer_alloc(FUNC(egret_device::seconds_tick), this);

	save_item(NAME(m_ddrs[0]));
	save_item(NAME(m_ddrs[1]));
	save_item(NAME(m_ddrs[2]));
	save_item(NAME(m_ports[0]));
	save_item(NAME(m_ports[1]));
	save_item(NAME(m_ports[2]));
	save_item(NAME(m_pll_ctrl));
	save_item(NAME(m_timer_ctrl));
	save_item(NAME(m_timer_counter));
	save_item(NAME(m_onesec));
	save_item(NAME(m_xcvr_session));
	save_item(NAME(m_via_full));
	save_item(NAME(m_sys_session));
	save_item(NAME(m_via_data));
	save_item(NAME(m_via_clock));
	save_item(NAME(m_adb_in));
	save_item(NAME(m_reset_line));
	save_item(NAME(m_adb_dtime));
	save_item(NAME(m_pram_loaded));
	save_item(NAME(m_pram));
	save_item(NAME(m_disk_pram));
#if USE_BUS_ADB
	save_item(NAME(m_adb_out));
	save_item(NAME(m_adb_device_out));
	save_item(NAME(m_adb_device_poweron));
#endif
}

void egret_device::device_reset()
{
#if USE_BUS_ADB
	m_adb_device_out[0] = m_adb_device_out[1] = true;
	m_adb_device_poweron[0] = m_adb_device_poweron[1] = true;
#endif

	m_timer->adjust(attotime::never);
	m_last_adb_time = m_maincpu->total_cycles();
}

#if USE_BUS_ADB
void egret_device::adb_w(int id, int state)
{
	m_adb_device_out[id] = state;
	adb_change();
}

void egret_device::adb_poweron_w(int id, int state)
{
	m_adb_device_poweron[id] = state;
}

void egret_device::adb_change()
{
	bool adb = m_adb_out & m_adb_device_out[0] & m_adb_device_out[1];
	logerror("adb c:%d 1:%d 2:%d -> %d (%02x %02x)\n", m_adb_out, m_adb_device_out[0], m_adb_device_out[1], adb, ddrs[0], ports[0]);
	for (int i = 0; i != 2; i++)
		if (m_adb_device[i])
		{
			m_adb_device[i]->adb_w(adb);
		}
}
#endif

void egret_device::send_port(u8 offset, u8 data)
{
	switch (offset)
	{
	case 0: // port A
#if USE_BUS_ADB
		// the line goes to a mosfet pulling the adb data line to graound, hence the inversion
		m_adb_out = !(data & 0x80);
		adb_change();
#else
		if ((data & 0x80) != m_last_adb)
		{
			m_adb_dtime = (int)(machine().time().as_ticks(1000000) - m_last_adb_time);

			if (data & 0x80)
			{
				LOGMASKED(LOG_ADB, "ADB: 1->0 time %lld\n", machine().time().as_ticks(1000000) - m_last_adb_time);
			}
			else
			{
				LOGMASKED(LOG_ADB, "ADB: 0->1 time %lld\n", machine().time().as_ticks(1000000) - m_last_adb_time);
			}

			// allow the linechange handler to override us
			m_adb_in = (data & 0x80) ? true : false;

			write_linechange(((data & 0x80) >> 7) ^ 1);

			m_last_adb = data & 0x80;
			m_last_adb_time = machine().time().as_ticks(1000000);
			}
#endif
			break;

		case 1: // port B
			{
				if (m_xcvr_session != ((data>>1)&1))
				{
					LOGMASKED(LOG_HOSTCOMM, "EG-> XCVR_SESSION: %d (PC=%x)\n", (data>>1)&1, m_maincpu->pc());
					m_xcvr_session = (data>>1) & 1;
				}
				if (m_via_data != ((data>>5)&1))
				{
					LOGMASKED(LOG_HOSTCOMM, "EG-> VIA_DATA: %d (PC=%x)\n", (data >> 5) & 1, m_maincpu->pc());
					m_via_data = (data>>5) & 1;
					write_via_data(m_via_data);
				}
				if (m_via_clock != ((data>>4)&1))
				{
					LOGMASKED(LOG_HOSTCOMM, "EG-> VIA_CLOCK: %d (PC=%x)\n", ((data >> 4) & 1) ^ 1, m_maincpu->pc());
					m_via_clock = (data>>4) & 1;
					write_via_clock(m_via_clock);
				}
			}
			break;

		case 2: // port C
			if ((data & 8) != m_reset_line)
			{
				LOGMASKED(LOG_HOSTCOMM, "680x0 reset: %d -> %d\n", (m_ports[2] & 8)>>3, (data & 8)>>3);
				m_reset_line = (data & 8);

				// falling edge, should reset the machine too
				if ((m_ports[2] & 8) && !(data&8))
				{
					// if PRAM's waiting to be loaded, transfer it now
					if (!m_pram_loaded)
					{
						memcpy(m_pram, m_disk_pram, 0x100);
						m_pram_loaded = true;

						system_time systime;
						struct tm cur_time, macref;
						machine().current_datetime(systime);

						cur_time.tm_sec = systime.local_time.second;
						cur_time.tm_min = systime.local_time.minute;
						cur_time.tm_hour = systime.local_time.hour;
						cur_time.tm_mday = systime.local_time.mday;
						cur_time.tm_mon = systime.local_time.month;
						cur_time.tm_year = systime.local_time.year - 1900;
						cur_time.tm_isdst = 0;

						macref.tm_sec = 0;
						macref.tm_min = 0;
						macref.tm_hour = 0;
						macref.tm_mday = 1;
						macref.tm_mon = 0;
						macref.tm_year = 4;
						macref.tm_isdst = 0;
						u32 ref = (u32)mktime(&macref);

						u32 seconds = (u32)((u32)mktime(&cur_time) - ref);
						m_internal_ram[0xae - 0x90] = seconds & 0xff;
						m_internal_ram[0xad - 0x90] = (seconds >> 8) & 0xff;
						m_internal_ram[0xac - 0x90] = (seconds >> 16) & 0xff;
						m_internal_ram[0xab - 0x90] = (seconds >> 24) & 0xff;

						LOGMASKED(LOG_PRAM, "Syncing PRAM to saved/default and RTC to current date/time %08x\n", seconds);
					}
				}

				write_reset((m_reset_line & 8) ? ASSERT_LINE : CLEAR_LINE);
			}
			break;
	}
}

u8 egret_device::ddr_r(offs_t offset)
{
	return m_ddrs[offset];
}

void egret_device::ddr_w(offs_t offset, u8 data)
{
/*  printf("%02x to DDR %c\n", data, 'A' + offset);*/

	send_port(offset, m_ports[offset] & data);

	m_ddrs[offset] = data;

	if (offset == 1) // port B
	{
		// For IIC, the 68HC05 sets the SCL and SDA data bits to 0 and toggles the lines
		// purely with the DDRs.  When DDR is set, the 0 is driven onto the line and
		// the line is 0.  When DDR is clear, the line is not driven by the 68HC05 and
		// external pullup resistors drive the line to a 1.

		// If both SCL and SDA data are 0, we're doing IIC
		if ((m_ports[offset] & 0x80) == 0)
		{
			u8 iic_data = (data & 0xc0) ^ 0xc0;
			LOGMASKED(LOG_I2C, "I2C: SCL %d SDA %d\n", BIT(iic_data, 7), BIT(iic_data, 6));
			write_iic_sda(BIT(iic_data, 6));
			write_iic_scl(BIT(iic_data, 7));
		}
	}
}

u8 egret_device::ports_r(offs_t offset)
{
	u8 incoming = 0;

	switch (offset)
	{
		case 0:     // port A
#if USE_BUS_ADB
			incoming |= (m_adb_out & m_adb_device_out[0] & m_adb_device_out[1]) ? 0x40 : 0;
			incoming |= (m_adb_device_poweron[0] & m_adb_device_poweron[1]) ? 0x04 : 0;
#else
			incoming |= m_adb_in ? 0x40 : 0;
#endif
			if (m_egret_controls_power)
			{
				incoming |= 0x02;   // indicate soft power, indicate chassis switch on
			}
			else
			{
				//incoming |= 0x01; // enable control panel enabled
			}
			break;

		case 1:     // port B
			incoming |= 0x01;   // always show +5v active
			incoming |= m_via_full<<2;
			incoming |= m_sys_session<<3;
			incoming |= m_via_data<<5;
			incoming |= m_iic_sda ? 0x40 : 0;
			break;

		case 2:     // port C
			if (m_egret_controls_power)
			{
				incoming |= 0x3;    // trickle sense active, bit 0 pulled up as per schematics
			}
			break;
	}

	// apply data direction registers
	incoming &= (m_ddrs[offset] ^ 0xff);
	// add in ddr-masked version of port writes
	incoming |= (m_ports[offset] & m_ddrs[offset]);

	return incoming;
}

void egret_device::ports_w(offs_t offset, u8 data)
{
	send_port(offset, data);

	m_ports[offset] = data;
}

u8 egret_device::pll_r()
{
	return m_pll_ctrl;
}

void egret_device::pll_w(u8 data)
{
	#ifdef EGRET_SUPER_VERBOSE
	if (m_pll_ctrl != data)
	{
		static const int clocks[4] = { 524288, 1048576, 2097152, 4194304 };
		printf("PLL ctrl: clock %d TCS:%d BCS:%d AUTO:%d BWC:%d PLLON:%d\n", clocks[data&3],
			(data & 0x80) ? 1 : 0,
			(data & 0x40) ? 1 : 0,
			(data & 0x20) ? 1 : 0,
			(data & 0x10) ? 1 : 0,
			(data & 0x08) ? 1 : 0);
	}
	#endif
	m_pll_ctrl = data;
}

u8 egret_device::timer_ctrl_r()
{
	return m_timer_ctrl;
}

void egret_device::timer_ctrl_w(u8 data)
{
//  printf("%02x to timer control\n", data);
	m_timer_ctrl = data;
}

u8 egret_device::timer_counter_r()
{
	return m_timer_counter;
}

void egret_device::timer_counter_w(u8 data)
{
//  printf("%02x to timer/counter\n", data);
	m_timer_counter = data;
}

u8 egret_device::onesec_r()
{
	return m_onesec;
}

void egret_device::onesec_w(u8 data)
{
//  printf("%02x to one-second control\n", data);

	m_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	if ((m_onesec & 0x40) && !(data & 0x40))
	{
		m_maincpu->set_input_line(M68HC05EG_INT_CPI, CLEAR_LINE);
	}

	m_onesec = data;
}

u8 egret_device::pram_r(offs_t offset)
{
	return m_pram[offset];
}

void egret_device::pram_w(offs_t offset, u8 data)
{
	m_pram[offset] = data;
}

TIMER_CALLBACK_MEMBER(egret_device::seconds_tick)
{
	m_onesec |= 0x40;

	if (m_onesec & 0x10)
	{
		m_maincpu->set_input_line(M68HC05EG_INT_CPI, ASSERT_LINE);
	}
}

// the 6805 program clears PRAM on startup (on h/w it's always running once a battery is inserted)
// we deal with that by loading pram from disk to a secondary buffer and then slapping it into "live"
// once the Egret reboots the 68k
void egret_device::nvram_default()
{
	memset(m_pram, 0, 0x100);
	memset(m_disk_pram, 0, 0x100);

	LOGMASKED(LOG_PRAM, "PRAM reset to default");

	// IIsi and IIvx both default PRAM to this, it seems a reasonable default for Egret systems
	m_pram[0x1] = 0x80;
	m_pram[0x2] = 0x4f;
	m_pram[0x3] = 0x48;
	m_pram[0x8] = 0x13;
	m_pram[0x9] = 0x88;
	m_pram[0xb] = 0x4c;
	m_pram[0xc] = 0x4e;
	m_pram[0xd] = 0x75;
	m_pram[0xe] = 0x4d;
	m_pram[0xf] = 0x63;
	m_pram[0x10] = 0xa8;
	m_pram[0x14] = 0xcc;
	m_pram[0x15] = 0x0a;
	m_pram[0x16] = 0xcc;
	m_pram[0x17] = 0x0a;
	m_pram[0x1d] = 0x02;
	m_pram[0x1e] = 0x63;
	m_pram[0x6f] = 0x28;
	m_pram[0x70] = 0x83;
	m_pram[0x71] = 0x26;
	m_pram[0x77] = 0x01;
	m_pram[0x78] = 0xff;
	m_pram[0x79] = 0xff;
	m_pram[0x7a] = 0xff;
	m_pram[0x7b] = 0xdf;
	m_pram[0x7d] = 0x09;
	m_pram[0xf3] = 0x12;
	m_pram[0xf9] = 0x01;
	m_pram[0xf3] = 0x12;
	m_pram[0xfb] = 0x8d;
	m_pram_loaded = false;
}

bool egret_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_disk_pram, 0x100);
	if (!err && (actual == 0x100))
	{
		LOGMASKED(LOG_PRAM, "PRAM read from disk OK");
		m_pram_loaded = false;
		return true;
	}
	return false;
}

bool egret_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_pram, 0x100);
	return !err;
}
