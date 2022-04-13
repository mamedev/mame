// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "Egret" ADB/system controller MCU
    Emulation by R. Belmont

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

    x-------  O  DFAC bit clock
    -x------  ?  DFAC data I/O (used in both directions)
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
*/


//#define EGRET_SUPER_VERBOSE

#include "emu.h"
#include "egret.h"
#include "cpu/m6805/m6805.h"
#include "sound/asc.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define EGRET_CPU_TAG   "egret"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EGRET, egret_device, "egret", "Apple Egret")

ROM_START( egret )
	ROM_REGION(0x4400, EGRET_CPU_TAG, 0)
	ROM_LOAD( "341s0851.bin", 0x1100, 0x1100, CRC(ea9ea6e4) SHA1(8b0dae3ec66cdddbf71567365d2c462688aeb571) )
	ROM_LOAD( "341s0850.bin", 0x2200, 0x1100, CRC(4906ecd0) SHA1(95e08ba0c5d4b242f115f104aba9905dbd3fd87c) )
	ROM_LOAD( "344s0100.bin", 0x3300, 0x1100, CRC(59e2b6b6) SHA1(540e752b7da521f1bdb16e0ad7c5f46ddc92d4e9) )
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
	map(0x0090, 0x00ff).ram();                         // work RAM and stack
	map(0x0100, 0x01ff).rw(FUNC(egret_device::pram_r), FUNC(egret_device::pram_w));
	map(0x0f00, 0x1fff).rom().region(EGRET_CPU_TAG, 0);
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

void egret_device::send_port(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0: // port A
		/*          printf("ADB:%d DFAC:%d PowerEnable:%d\n",
		        (data & 0x80) ? 1 : 0,
		        (data & 0x10) ? 1 : 0,
		        (data & 0x02) ? 1 : 0);*/

#if USE_BUS_ADB
		// the line goes to a mosfet pulling the adb data line to graound, hence the inversion
		m_adb_out = !(data & 0x80);
		adb_change();
#else
		if ((data & 0x80) != last_adb)
		{
			m_adb_dtime = (int)(machine().time().as_ticks(1000000) - last_adb_time);
			/*
			    if (data & 0x80)
			    {
			        printf("EG ADB: 1->0 time %d\n", m_adb_dtime);
			    }
			    else
			    {
			        printf("EG ADB: 0->1 time %d\n", m_adb_dtime);
			    }
			    */
			// allow the linechange handler to override us
			adb_in = (data & 0x80) ? true : false;

			write_linechange(((data & 0x80) >> 7) ^ 1);

			last_adb = data & 0x80;
			last_adb_time = machine().time().as_ticks(1000000);
			}
#endif
			break;

		case 1: // port B
			{
				if (xcvr_session != ((data>>1)&1))
				{
					#ifdef EGRET_SUPER_VERBOSE
					printf("EG-> XCVR_SESSION: %d (PC=%x)\n", (data>>1)&1, m_maincpu->pc());
					#endif
					xcvr_session = (data>>1) & 1;
				}
				if (via_data != ((data>>5)&1))
				{
					#ifdef EGRET_SUPER_VERBOSE
					printf("EG-> VIA_DATA: %d (PC=%x)\n", (data>>5)&1, m_maincpu->pc());
					#endif
					via_data = (data>>5) & 1;
					write_via_data(via_data);
				}
				if (via_clock != ((data>>4)&1))
				{
					#ifdef EGRET_SUPER_VERBOSE
					printf("EG-> VIA_CLOCK: %d (PC=%x)\n", ((data>>4)&1)^1, m_maincpu->pc());
					#endif
					via_clock = (data>>4) & 1;
					write_via_clock(via_clock);
				}
			}
			break;

		case 2: // port C
			if ((data & 8) != reset_line)
			{
				#ifdef EGRET_SUPER_VERBOSE
				printf("680x0 reset: %d -> %d\n", (ports[2] & 8)>>3, (data & 8)>>3);
				#endif
				reset_line = (data & 8);

				// falling edge, should reset the machine too
				if ((ports[2] & 8) && !(data&8))
				{
					// if PRAM's waiting to be loaded, transfer it now
					if (!pram_loaded)
					{
						memcpy(pram, disk_pram, 0x100);
						pram_loaded = true;
					}
				}

				write_reset((reset_line & 8) ? ASSERT_LINE : CLEAR_LINE);
			}
			break;
	}
}

uint8_t egret_device::ddr_r(offs_t offset)
{
	return ddrs[offset];
}

void egret_device::ddr_w(offs_t offset, uint8_t data)
{
/*  printf("%02x to DDR %c\n", data, 'A' + offset);*/

	send_port(offset, ports[offset] & data);

	ddrs[offset] = data;
}

uint8_t egret_device::ports_r(offs_t offset)
{
	uint8_t incoming = 0;

	switch (offset)
	{
		case 0:     // port A
#if USE_BUS_ADB
			incoming |= (m_adb_out & m_adb_device_out[0] & m_adb_device_out[1]) ? 0x40 : 0;
			incoming |= (m_adb_device_poweron[0] & m_adb_device_poweron[1]) ? 0x04 : 0;
#else
			incoming |= adb_in ? 0x40 : 0;
#endif
			if (egret_controls_power)
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
			incoming |= via_full<<2;
			incoming |= sys_session<<3;
			incoming |= via_data<<5;
			incoming |= 0x40;   // show DFAC line high
			break;

		case 2:     // port C
			if (egret_controls_power)
			{
				incoming |= 0x3;    // trickle sense active, bit 0 pulled up as per schematics
			}
			break;
	}

	// apply data direction registers
	incoming &= (ddrs[offset] ^ 0xff);
	// add in ddr-masked version of port writes
	incoming |= (ports[offset] & ddrs[offset]);

	return incoming;
}

void egret_device::ports_w(offs_t offset, uint8_t data)
{
	send_port(offset, data);

	ports[offset] = data;
}

uint8_t egret_device::pll_r()
{
	return pll_ctrl;
}

void egret_device::pll_w(uint8_t data)
{
	#ifdef EGRET_SUPER_VERBOSE
	if (pll_ctrl != data)
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
	pll_ctrl = data;
}

uint8_t egret_device::timer_ctrl_r()
{
	return timer_ctrl;
}

void egret_device::timer_ctrl_w(uint8_t data)
{
//  printf("%02x to timer control\n", data);
	timer_ctrl = data;
}

uint8_t egret_device::timer_counter_r()
{
	return timer_counter;
}

void egret_device::timer_counter_w(uint8_t data)
{
//  printf("%02x to timer/counter\n", data);
	timer_counter = data;
}

uint8_t egret_device::onesec_r()
{
	return onesec;
}

void egret_device::onesec_w(uint8_t data)
{
//  printf("%02x to one-second control\n", data);

	m_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	if ((onesec & 0x40) && !(data & 0x40))
	{
		m_maincpu->set_input_line(M68HC05EG_INT_CPI, CLEAR_LINE);
	}

	onesec = data;
}

uint8_t egret_device::pram_r(offs_t offset)
{
	return pram[offset];
}

void egret_device::pram_w(offs_t offset, uint8_t data)
{
	pram[offset] = data;
}

//-------------------------------------------------
//  egret_device - constructor
//-------------------------------------------------

egret_device::egret_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, EGRET, tag, owner, clock),
	  device_nvram_interface(mconfig, *this),
	  write_reset(*this),
	  write_linechange(*this),
	  write_via_clock(*this),
	  write_via_data(*this),
	  m_maincpu(*this, EGRET_CPU_TAG)
#if USE_BUS_ADB
	, m_adb_connector{{*this, "adb1"}, {*this, finder_base::DUMMY_TAG}}
#endif
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void egret_device::device_start()
{
	write_reset.resolve_safe();
	write_linechange.resolve_safe();
	write_via_clock.resolve_safe();
	write_via_data.resolve_safe();

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

	m_timer = timer_alloc(0);
	save_item(NAME(ddrs[0]));
	save_item(NAME(ddrs[1]));
	save_item(NAME(ddrs[2]));
	save_item(NAME(ports[0]));
	save_item(NAME(ports[1]));
	save_item(NAME(ports[2]));
	save_item(NAME(pll_ctrl));
	save_item(NAME(timer_ctrl));
	save_item(NAME(timer_counter));
	save_item(NAME(onesec));
	save_item(NAME(xcvr_session));
	save_item(NAME(via_full));
	save_item(NAME(sys_session));
	save_item(NAME(via_data));
	save_item(NAME(via_clock));
	save_item(NAME(adb_in));
	save_item(NAME(reset_line));
	save_item(NAME(m_adb_dtime));
	save_item(NAME(pram_loaded));
	save_item(NAME(pram));
	save_item(NAME(disk_pram));
#if USE_BUS_ADB
	save_item(NAME(m_adb_out));
	save_item(NAME(m_adb_device_out));
	save_item(NAME(m_adb_device_poweron));
#endif

	uint8_t *rom = device().machine().root_device().memregion(device().subtag(EGRET_CPU_TAG))->base();

	if (rom)
	{
		memcpy(rom, rom+rom_offset, 0x1100);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void egret_device::device_reset()
{
	ddrs[0] = ddrs[1] = ddrs[2] = 0;
	ports[0] = ports[1] = ports[2] = 0;

	#if USE_BUS_ADB
	m_adb_device_out[0] = m_adb_device_out[1] = true;
	m_adb_device_poweron[0] = m_adb_device_poweron[1] = true;
	#endif

	m_timer->adjust(attotime::never);

	egret_controls_power = false;   // set to hard power control
	adb_in = true;  // line is pulled up to +5v, so nothing plugged in would read as "1"
	reset_line = 0;
	xcvr_session = 0;
	via_full = 0;
	sys_session = 0;
	via_data = 0;
	via_clock = 0;
	pll_ctrl = 0;
	timer_ctrl = 0;
	timer_counter = 0;
	last_adb_time = m_maincpu->total_cycles();
	onesec = 0;
	last_adb = 0;
}

void egret_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	onesec |= 0x40;

	if (onesec & 0x10)
	{
		m_maincpu->set_input_line(M68HC05EG_INT_CPI, ASSERT_LINE);
	}
}

// the 6805 program clears PRAM on startup (on h/w it's always running once a battery is inserted)
// we deal with that by loading pram from disk to a secondary buffer and then slapping it into "live"
// once the Egret reboots the 68k
void egret_device::nvram_default()
{
	memset(pram, 0, 0x100);
	memset(disk_pram, 0, 0x100);

	// IIsi and IIvx both default PRAM to this, it seems a reasonable default for Egret systems
	pram[0x1] = 0x80;
	pram[0x2] = 0x4f;
	pram[0x3] = 0x48;
	pram[0x8] = 0x13;
	pram[0x9] = 0x88;
	pram[0xb] = 0x4c;
	pram[0xc] = 0x4e;
	pram[0xd] = 0x75;
	pram[0xe] = 0x4d;
	pram[0xf] = 0x63;
	pram[0x10] = 0xa8;
	pram[0x14] = 0xcc;
	pram[0x15] = 0x0a;
	pram[0x16] = 0xcc;
	pram[0x17] = 0x0a;
	pram[0x1d] = 0x02;
	pram[0x1e] = 0x63;
	pram[0x6f] = 0x28;
	pram[0x70] = 0x83;
	pram[0x71] = 0x26;
	pram[0x77] = 0x01;
	pram[0x78] = 0xff;
	pram[0x79] = 0xff;
	pram[0x7a] = 0xff;
	pram[0x7b] = 0xdf;
	pram[0x7d] = 0x09;
	pram[0xf3] = 0x12;
	pram[0xf9] = 0x01;
	pram[0xf3] = 0x12;
	pram[0xfb] = 0x8d;
	pram_loaded = false;
}

bool egret_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	if (!file.read(disk_pram, 0x100, actual) && actual == 0x100)
	{
		pram_loaded = false;
		return true;
	}
	return false;
}

bool egret_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(pram, 0x100, actual) && actual == 0x100;
}
