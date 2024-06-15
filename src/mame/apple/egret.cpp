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

#define LOG_ADB (1U << 1)      // low-level ADB details
#define LOG_PRAM (1U << 2)     // PRAM handling info
#define LOG_HOSTCOMM (1U << 3) // communications with the host

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(EGRET, egret_device, "egret", "Apple Egret ADB/I2C")

ROM_START( egret )
	ROM_REGION(0x1100, "egret", 0)
	ROM_DEFAULT_BIOS("341s0851")

	ROM_SYSTEM_BIOS(0, "344s0100", "Egret 1.00 (344S0100)")
	ROMX_LOAD("344s0100.bin", 0x0000, 0x1100, CRC(59e2b6b6) SHA1(540e752b7da521f1bdb16e0ad7c5f46ddc92d4e9), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "341s0850", "Egret 1.01 (earlier) (341S0850)")
	ROMX_LOAD("341s0850.bin", 0x0000, 0x1100, CRC(4906ecd0) SHA1(95e08ba0c5d4b242f115f104aba9905dbd3fd87c), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "341s0851", "Egret 1.01 (341S0851)")
	ROMX_LOAD("341s0851.bin", 0x0000, 0x1100, CRC(ea9ea6e4) SHA1(8b0dae3ec66cdddbf71567365d2c462688aeb571), ROM_BIOS(2))
ROM_END

void egret_device::device_add_mconfig(machine_config &config)
{
	M68HC05E1(config, m_maincpu, XTAL(32'768)*128);  // Intended to run 4.1 MHz, the ADB timings are twice as long as spec at 2.048 MHz
	m_maincpu->read_p<0>().set(FUNC(egret_device::pa_r));
	m_maincpu->read_p<1>().set(FUNC(egret_device::pb_r));
	m_maincpu->read_p<2>().set(FUNC(egret_device::pc_r));
	m_maincpu->write_p<0>().set(FUNC(egret_device::pa_w));
	m_maincpu->write_p<1>().set(FUNC(egret_device::pb_w));
	m_maincpu->write_p<2>().set(FUNC(egret_device::pc_w));
	m_maincpu->set_pullups<1>(0x40);    // pull-up on port B bit 6

#if USE_BUS_ADB
	ADB_CONNECTOR(config, "adb1", adb_device::default_devices, "a9m0330", false);
	#endif
}

const tiny_rom_entry *egret_device::device_rom_region() const
{
	return ROM_NAME( egret );
}

egret_device::egret_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, EGRET, tag, owner, clock),
	  device_nvram_interface(mconfig, *this),
	  write_reset(*this),
	  write_linechange(*this),
	  write_via_clock(*this),
	  write_via_data(*this),
	  write_dfac_scl(*this),
	  write_dfac_sda(*this),
	  write_dfac_latch(*this),
	  m_maincpu(*this, "egret"),
	  m_xcvr_session(0), m_via_full(0), m_sys_session(0), m_via_data(0), m_last_adb(0),
	  m_last_adb_time(0), m_egret_controls_power(false), m_adb_in(false),
	  m_reset_line(0), m_adb_dtime(0), m_pram_loaded(false)
#if USE_BUS_ADB
	  ,
	  m_adb_connector{{*this, "adb1"}, {*this, finder_base::DUMMY_TAG}}
#endif
{
	std::fill(std::begin(m_disk_pram), std::end(m_disk_pram), 0);
}

void egret_device::device_start()
{
#if USE_BUS_ADB
	for (int i = 0; i < 2; i++)
	{
		m_adb_device[i] = m_adb_connector[i] ? m_adb_connector[i]->get_device() : nullptr;
		if (m_adb_device[i])
		{
			m_adb_device[i]->adb_r().set([this, i](int state) { adb_w(i, state); });
			m_adb_device[i]->poweron_r().set([this, i](int state) { adb_poweron_w(i, state); });
		}
	}
#endif

	save_item(NAME(m_xcvr_session));
	save_item(NAME(m_via_full));
	save_item(NAME(m_sys_session));
	save_item(NAME(m_via_data));
	save_item(NAME(m_adb_in));
	save_item(NAME(m_reset_line));
	save_item(NAME(m_adb_dtime));
	save_item(NAME(m_pram_loaded));
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
	LOG("adb c:%d 1:%d 2:%d -> %d (%02x %02x)\n", m_adb_out, m_adb_device_out[0], m_adb_device_out[1], adb, ddrs[0], ports[0]);
	for (int i = 0; i != 2; i++)
		if (m_adb_device[i])
		{
			m_adb_device[i]->adb_w(adb);
		}
}
#endif

void egret_device::pa_w(u8 data)
{
	write_dfac_latch(BIT(data, 4));

#if USE_BUS_ADB
	// the line goes to a mosfet pulling the adb data line to ground, hence the inversion
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
}

void egret_device::pb_w(u8 data)
{
	if (m_xcvr_session != BIT(data, 1))
	{
		LOGMASKED(LOG_HOSTCOMM, "EG-> XCVR_SESSION: %d (PC=%x)\n", (data >> 1) & 1, m_maincpu->pc());
		m_xcvr_session = BIT(data, 1);
	}

	LOGMASKED(LOG_HOSTCOMM, "EG->VIA VIA_DATA: %d VIA_CLOCK: %d (PC=%x)\n", BIT(data, 5), BIT(data, 4), m_maincpu->pc());
	write_via_data(BIT(data, 5));
	write_via_clock(BIT(data, 4));
	write_dfac_sda(BIT(data, 6));
	write_dfac_scl(BIT(data, 7));
}

void egret_device::pc_w(u8 data)
{
	if ((data & 8) != m_reset_line)
	{
		LOGMASKED(LOG_HOSTCOMM, "680x0 reset\n");

		// falling edge, should reset the machine too
		// TODO: find falling edge
		if ((m_reset_line & 8) && !(data & 8))
		{
			// if PRAM's waiting to be loaded, transfer it now
			if (!m_pram_loaded)
			{
				for (int byte = 0; byte < 256; byte++)
				{
					m_maincpu->write_internal_ram(0x70 + byte, m_disk_pram[byte]);
				}
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
				m_maincpu->write_internal_ram(0xae - 0x90, seconds & 0xff);
				m_maincpu->write_internal_ram(0xad - 0x90, (seconds >> 8) & 0xff);
				m_maincpu->write_internal_ram(0xac - 0x90, (seconds >> 16) & 0xff);
				m_maincpu->write_internal_ram(0xab - 0x90, (seconds >> 24) & 0xff);

				LOGMASKED(LOG_PRAM, "Syncing PRAM to saved/default and RTC to current date/time %08x\n", seconds);
			}
		}

		m_reset_line = (data & 8);
		write_reset((m_reset_line & 8) ? ASSERT_LINE : CLEAR_LINE);
	}
}

u8 egret_device::pa_r()
{
	u8 rv = 0;

#if USE_BUS_ADB
	rv |= (m_adb_out & m_adb_device_out[0] & m_adb_device_out[1]) ? 0x40 : 0;
	rv |= (m_adb_device_poweron[0] & m_adb_device_poweron[1]) ? 0x04 : 0;
#else
	rv |= m_adb_in ? 0x40 : 0;
#endif
	if (m_egret_controls_power)
	{
		rv |= 0x02;   // indicate soft power, indicate chassis switch on
	}
	else
	{
		//rv |= 0x01; // enable control panel enabled
	}

	return rv;
}

u8 egret_device::pb_r()
{
	u8 rv = 0;
	rv |= 0x01; // always show +5v active
	rv |= m_via_full << 2;
	rv |= m_sys_session << 3;
	rv |= m_via_data << 5;
	rv |= 0x40;
	return rv;
}

u8 egret_device::pc_r()
{
	u8 rv = 0;
	if (m_egret_controls_power)
	{
		rv |= 0x3; // trickle sense active, bit 0 pulled up as per schematics
	}

	return rv;
}

// the 6805 program clears PRAM on startup (on h/w it's always running once a battery is inserted)
// we deal with that by loading pram from disk to a secondary buffer and then slapping it into "live"
// once the Egret reboots the 68k
void egret_device::nvram_default()
{
	memset(m_disk_pram, 0, 0x100);

	LOGMASKED(LOG_PRAM, "PRAM reset to default");

	// IIsi and IIvx both default PRAM to this, it seems a reasonable default for Egret systems
	m_disk_pram[0x1] = 0x80;
	m_disk_pram[0x2] = 0x4f;
	m_disk_pram[0x3] = 0x48;
	m_disk_pram[0x8] = 0x13;
	m_disk_pram[0x9] = 0x88;
	m_disk_pram[0xb] = 0x4c;
	m_disk_pram[0xc] = 0x4e;
	m_disk_pram[0xd] = 0x75;
	m_disk_pram[0xe] = 0x4d;
	m_disk_pram[0xf] = 0x63;
	m_disk_pram[0x10] = 0xa8;
	m_disk_pram[0x14] = 0xcc;
	m_disk_pram[0x15] = 0x0a;
	m_disk_pram[0x16] = 0xcc;
	m_disk_pram[0x17] = 0x0a;
	m_disk_pram[0x1d] = 0x02;
	m_disk_pram[0x1e] = 0x63;
	m_disk_pram[0x6f] = 0x28;
	m_disk_pram[0x70] = 0x83;
	m_disk_pram[0x71] = 0x26;
	m_disk_pram[0x77] = 0x01;
	m_disk_pram[0x78] = 0xff;
	m_disk_pram[0x79] = 0xff;
	m_disk_pram[0x7a] = 0xff;
	m_disk_pram[0x7b] = 0xdf;
	m_disk_pram[0x7d] = 0x09;
	m_disk_pram[0xf3] = 0x12;
	m_disk_pram[0xf9] = 0x01;
	m_disk_pram[0xf3] = 0x12;
	m_disk_pram[0xfb] = 0x8d;
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
	for (int byte = 0; byte < 256; byte++)
	{
		m_disk_pram[byte] = m_maincpu->read_internal_ram(0x70 + byte);
	}

	auto const [err, actual] = write(file, m_disk_pram, 0x100);
	return !err;
}
