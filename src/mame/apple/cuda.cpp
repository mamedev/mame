// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "Cuda" ADB/I2C/power management microcontroller
    Emulation by R. Belmont

    Cuda (so-named because they "c(o)u(l)da got it right the first time" with Egret) is a 68HC05E1 (2.x)
    or 68HC05E5 (Cuda Lite, Cuda 3.0) with 2 8-bit GPIO ports and 1 4-bit GPIO port.  Cuda handles
    ADB, I2C, and basic power management.

    TODO: make proper 68HC05E1 and E5 base classes and rebase this and Egret on them.

    Port definitions, primarily from the schematics.

    Port A:

    x-------  O  ADB data line out
    -x------  I  ADB data line in
    --x-----  I  "1" for passive or soft power, "off sense" for secure
    ---x----  O  DFAC latch
    ----x---  O  "Fast Reset"
    -----x--  I  Keyboard power switch
    ------x-  I  1 for secure or passive power, chassis switch for soft
    -------x  ?  1 for passive, power supply control on secure/soft power, input on secure/soft also?

    Port B:

    x-------  O  DFAC bit clock (IIC SCL, pulled up to +5V externally)
    -x------  B  DFAC data I/O (IIC SDA, used in both directions, pulled up to +5V externally)
    --x-----  B  VIA shift register data (used in both directions)
    ---x----  O  VIA clock
    ----x---  I  VIA TIP
    -----x--  I  VIA BYTEACK
    ------x-  O  VIA TREQ
    -------x  I  +5v sense

    Port C:
    x---      O  680x0 reset
    -x--      ?  680x0 IPL 2 (used in both directions)
    --x-      ?  IPL 1/pull up for passive power, trickle sense for soft and secure
    ---x      ?  IPL 0 for passive power, pull-up for soft power, file server switch for secure power

    Cuda version spotting:
    341S0262 - 0x0003f200 (3.02) - some PMac 6500, Bondi blue iMac
    341S0285 - No version (x.xx) - PMac 4400 + Mac clones ("Cuda Lite" with 768 bytes more ROM + PS/2 keyboard/mouse support)
    341S0060 - 0x00020028 (2.40) - Performa/Quadra 6xx, PMac 6200, x400, some x500, Pippin, "Gossamer" G3, others?
                                    (verified found in PMac 5500-225, G3-333)
    341S???? - 0x00020026 (2.38) - Macintosh TV
    341S0788 - 0x00020025 (2.37) - LC 475/575/Quadra 605, Quadra 660AV/840AV, PMac 7200
    341S0417 - 0x00020023 (2.35) - Color Classic
*/

#include "emu.h"
#include "cuda.h"
#include "cpu/m6805/m6805.h"

#define LOG_ADB         (1U << 1)   // low-level ADB details
#define LOG_I2C         (1U << 2)   // low-level I2C details
#define LOG_PRAM        (1U << 3)   // PRAM handling info
#define LOG_HOSTCOMM    (1U << 4)   // communications with the host
#define LOG_PACKETS     (1U << 5)   // Show command packets being sent from the host (may only work on v2.40)

#define VERBOSE (0)
#include "logmacro.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CUDA_V2XX, cuda_2xx_device, "cuda", "Apple Cuda v2.xx ADB/I2C")
DEFINE_DEVICE_TYPE(CUDA_V302, cuda_302_device, "cuda302", "Apple Cuda v3.02 ADB/I2C")
DEFINE_DEVICE_TYPE(CUDA_LITE, cuda_lite_device,"cudalite","Apple Cuda Lite ADB+I2C+PS/2")

ROM_START( cuda )
	ROM_REGION(0x7e00, "roms", ROMREGION_ERASE00)
	ROM_REGION(0x100, "defaultnv", ROMREGION_ERASE00)
ROM_END

void cuda_device::cuda_map(address_map &map)
{
	map(0x0000, 0x0002).rw(FUNC(cuda_device::ports_r), FUNC(cuda_device::ports_w));
	map(0x0004, 0x0006).rw(FUNC(cuda_device::ddr_r), FUNC(cuda_device::ddr_w));
	map(0x0007, 0x0007).rw(FUNC(cuda_device::pll_r), FUNC(cuda_device::pll_w));
	map(0x0008, 0x0008).rw(FUNC(cuda_device::timer_ctrl_r), FUNC(cuda_device::timer_ctrl_w));
	map(0x0009, 0x0009).r(FUNC(cuda_device::timer_counter_r));
	map(0x0012, 0x0012).rw(FUNC(cuda_device::onesec_r), FUNC(cuda_device::onesec_w));
	map(0x0090, 0x00ff).ram().share(m_internal_ram);    // work RAM and stack.  RTC at 0xab-0xae on 2.37 and 2.40
	map(0x0100, 0x01ff).rw(FUNC(cuda_device::pram_r), FUNC(cuda_device::pram_w));
	map(0x0f00, 0x1fff).rom().region("roms", 0);
}

void cuda_device::device_add_mconfig(machine_config &config)
{
	M68HC05EG(config, m_maincpu, XTAL(32'768)*128);   // Intended to run 4.1 MHz, the ADB timings in uS are twice as long as spec at 2.1
	m_maincpu->set_addrmap(AS_PROGRAM, &cuda_device::cuda_map);
}

const tiny_rom_entry *cuda_device::device_rom_region() const
{
	return ROM_NAME( cuda );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cuda_device::cuda_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	write_reset(*this),
	write_linechange(*this),
	write_via_clock(*this),
	write_via_data(*this),
	write_iic_scl(*this),
	write_iic_sda(*this),
	m_maincpu(*this, "cudamcu"),
	m_internal_ram(*this, "internal_ram"),
	m_rom(*this, "roms"),
	m_default_nvram(*this, "defaultnv"),
	m_pll_ctrl(0), m_timer_ctrl(0), m_onesec(0),
	m_treq(0), m_byteack(0), m_tip(0), m_via_data(0), m_via_clock(0),  m_last_adb(0),
	m_iic_sda(0), m_last_adb_time(0), m_cuda_controls_power(false), m_adb_in(false),
	m_reset_line(0), m_adb_dtime(0), m_pram_loaded(false)
{
	std::fill(std::begin(m_pram), std::end(m_pram), 0);
	std::fill(std::begin(m_disk_pram), std::end(m_disk_pram), 0);
	std::fill(std::begin(m_ports), std::end(m_ports), 0);
	std::fill(std::begin(m_ddrs), std::end(m_ddrs), 0);
}

void cuda_device::device_start()
{
	m_timer = timer_alloc(FUNC(cuda_device::seconds_tick), this);
	m_prog_timer = timer_alloc(FUNC(cuda_device::timer_tick), this);

	save_item(NAME(m_ddrs[0]));
	save_item(NAME(m_ddrs[1]));
	save_item(NAME(m_ddrs[2]));
	save_item(NAME(m_ports[0]));
	save_item(NAME(m_ports[1]));
	save_item(NAME(m_ports[2]));
	save_item(NAME(m_pll_ctrl));
	save_item(NAME(m_timer_ctrl));
	save_item(NAME(m_onesec));
	save_item(NAME(m_treq));
	save_item(NAME(m_byteack));
	save_item(NAME(m_tip));
	save_item(NAME(m_via_data));
	save_item(NAME(m_via_clock));
	save_item(NAME(m_adb_in));
	save_item(NAME(m_reset_line));
	save_item(NAME(m_adb_dtime));
	save_item(NAME(m_pram_loaded));
	save_item(NAME(m_pram));
	save_item(NAME(m_disk_pram));

#if ((VERBOSE & LOG_PACKETS) == LOG_PACKETS)
	m_maincpu->space().install_read_tap(0xba, 0xba, 0, "cuda", [this](offs_t offset, u8 &data, u8 mem_mask) {
										if (m_maincpu->pc() == 0x12b3) {
											LOG("Got command %02x\n", data);
										}
									});
#endif
}

void cuda_device::device_reset()
{
	m_timer->adjust(attotime::never);
	m_prog_timer->adjust(attotime::never);

	m_last_adb_time = m_maincpu->total_cycles();
}

void cuda_device::send_port(u8 offset, u8 data)
{
	switch (offset)
	{
		case 0: // port A
			if ((data & 0x80) != m_last_adb)
			{
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

				m_adb_dtime = (int)(machine().time().as_ticks(1000000) - m_last_adb_time);
				write_linechange(((data & 0x80) >> 7) ^ 1);

				m_last_adb = data & 0x80;
				m_last_adb_time = machine().time().as_ticks(1000000);
			}
			break;

		case 1: // port B
			{
				if (m_treq != ((data>>1)&1))
				{
					LOGMASKED(LOG_HOSTCOMM, "CU-> TREQ: %d (PC=%x)\n", (data>>1)&1, m_maincpu->pc());
					m_treq = (data>>1) & 1;
				}
				if (m_via_data != ((data>>5)&1))
				{
					LOGMASKED(LOG_HOSTCOMM, "CU-> VIA_DATA: %d (PC=%x)\n", (data>>5)&1, m_maincpu->pc());
					m_via_data = (data>>5) & 1;
					write_via_data(m_via_data);
				}
				if (m_via_clock != ((data>>4)&1))
				{
					LOGMASKED(LOG_HOSTCOMM, "CU-> VIA_CLOCK: %d (PC=%x)\n", ((data>>4)&1)^1, m_maincpu->pc());
					m_via_clock = (data>>4) & 1;
					write_via_clock(m_via_clock);
				}
			}
			break;

		case 2: // port C
			if ((data & 8) != m_reset_line)
			{
				LOGMASKED(LOG_HOSTCOMM, "680x0 reset: %d -> %d (PC=%x)\n", (m_ports[2] & 8)>>3, (data & 8)>>3, m_maincpu->pc());
				m_reset_line = (data & 8);
				// falling edge, should reset the machine too
				if ((m_ports[2] & 8) && !(data&8))
				{
					write_reset(ASSERT_LINE);
					write_reset(CLEAR_LINE);

					// if PRAM's waiting to be loaded, transfer it now
					if (!m_pram_loaded)
					{
						memcpy(m_pram, m_disk_pram, 0x100);

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
						m_pram_loaded = true;
					}
				}
			}
			break;
	}
}

u8 cuda_device::ddr_r(offs_t offset)
{
	return m_ddrs[offset];
}

void cuda_device::ddr_w(offs_t offset, u8 data)
{
	send_port(offset, m_ports[offset] & data);

	m_ddrs[offset] = data;

	if (offset == 1)    // port B
	{
		// For IIC, Cuda sets the SCL and SDA data bits to 0 and toggles the lines
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

u8 cuda_device::ports_r(offs_t offset)
{
	u8 incoming = 0;

	switch (offset)
	{
		case 0:     // port A
			if (m_cuda_controls_power)
			{
				incoming = 0x20; // pull up + chassis switch (which is 0 = on)
			}
			else
			{
				incoming = 0x02 | 0x01;   // pull-up + PFW
			}

			if (m_adb_in)
			{
				incoming |= 0x40;
			}
			break;

		case 1:     // port B
			incoming |= 0x01;   // +5v sense
			incoming |= m_byteack<<2;
			incoming |= m_tip<<3;
			incoming |= m_via_data<<5;
			incoming |= m_iic_sda ? 0x40 : 0;
			incoming |= 0x80;
			break;

		case 2:     // port C
			if (m_cuda_controls_power)
			{
				incoming = 0x02 | 0x01; // soft power: trickle sense + pull-up
			}
			else
			{
				incoming = 0x02 | 0x01; // secure power: pull-up + file server
			}
			break;
	}

	// apply data direction registers
	incoming &= (m_ddrs[offset] ^ 0xff);
	// add in ddr-masked version of port writes
	incoming |= (m_ports[offset] & m_ddrs[offset]);

	// HACK: don't know how this works on h/w...
	if (!offset)
	{
		incoming |= 0x01;
	}

	return incoming;
}

void cuda_device::ports_w(offs_t offset, u8 data)
{
	send_port(offset, data);

	m_ports[offset] = data;
}

u8 cuda_device::pll_r()
{
	return m_pll_ctrl;
}

void cuda_device::pll_w(u8 data)
{
	// Motorola documentation for both the 68HC05E1 and E5 says that rate 3 (4 MHz) is illegal.
	// The Cuda code sets it to 2 MHz, but comments in the code as well as the cycle counts in
	// the ADB routines indicate the CPU is intended to run at 4.2 MHz, not 2.1.
	// So we do this little cheat.
	if ((data & 3) == 2)
	{
		data |= 3;
	}

	if (m_pll_ctrl != data)
	{
		static const int clocks[4] = { 524288, 1048576, 2097152, 4194304 };
		LOG("PLL ctrl: clock %d TCS:%d BCS:%d AUTO:%d BWC:%d PLLON:%d (PC=%x)\n", clocks[data&3],
			(data & 0x80) ? 1 : 0,
			(data & 0x40) ? 1 : 0,
			(data & 0x20) ? 1 : 0,
			(data & 0x10) ? 1 : 0,
			(data & 0x08) ? 1 : 0, m_maincpu->pc());

		m_prog_timer->adjust(attotime::from_hz(clocks[data & 3]/1024), 0, attotime::from_hz(clocks[data & 3]/1024));
	}
	m_pll_ctrl = data;
}

u8 cuda_device::timer_ctrl_r()
{
	return m_timer_ctrl;
}

void cuda_device::timer_ctrl_w(u8 data)
{
	if ((m_timer_ctrl & 0x80) && !(data & 0x80))
	{
		m_maincpu->set_input_line(M68HC05EG_INT_TIMER, CLEAR_LINE);
		m_timer_ctrl &= ~0x80;
	}
	else if ((m_timer_ctrl & 0x40) && !(data & 0x40))
	{
		m_maincpu->set_input_line(M68HC05EG_INT_TIMER, CLEAR_LINE);
		m_timer_ctrl &= ~0x40;
	}

	m_timer_ctrl &= 0xc0;
	m_timer_ctrl |= (data & ~0xc0);
}

u8 cuda_device::timer_counter_r()
{
	// this returns an always-incrementing 8-bit value incremented at 1/4th of the CPU's clock rate.
	return (m_maincpu->total_cycles() / 4) % 256;
}

u8 cuda_device::onesec_r()
{
	return m_onesec;
}

void cuda_device::onesec_w(u8 data)
{
	m_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	if ((m_onesec & 0x40) && !(data & 0x40))
	{
		m_maincpu->set_input_line(M68HC05EG_INT_CPI, CLEAR_LINE);
	}

	m_onesec = data;
}

u8 cuda_device::pram_r(offs_t offset)
{
	return m_pram[offset];
}

void cuda_device::pram_w(offs_t offset, u8 data)
{
	m_pram[offset] = data;
}

TIMER_CALLBACK_MEMBER(cuda_device::seconds_tick)
{
	m_onesec |= 0x40;

	if (m_onesec & 0x10)
	{
		m_maincpu->set_input_line(M68HC05EG_INT_CPI, ASSERT_LINE);
	}
}

TIMER_CALLBACK_MEMBER(cuda_device::timer_tick)
{
	m_timer_ctrl |= 0x80;

	if (m_timer_ctrl & 0x20)
	{
		m_maincpu->set_input_line(M68HC05EG_INT_TIMER, ASSERT_LINE);
	}
}

// The 6805 program clears PRAM on startup (on h/w it's always running once a battery is inserted).
// We deal with that by loading PRAM from disk to a secondary buffer and then slapping it into "live"
// once Cuda starts the host processor.
void cuda_device::nvram_default()
{
	LOGMASKED(LOG_PRAM, "Using default PRAM\n");
	memset(m_pram, 0, 0x100);
	memcpy(m_disk_pram, m_default_nvram, 256);
	m_pram_loaded = false;
}

bool cuda_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_disk_pram, 0x100);
	if (!err && (actual == 0x100))
	{
		LOGMASKED(LOG_PRAM, "Loaded PRAM from disk\n");
		return true;
	}
	return false;
}


bool cuda_device::nvram_write(util::write_stream &file)
{
	LOGMASKED(LOG_PRAM, "Writing PRAM to disk\n");
	auto const [err, actual] = write(file, m_pram, 0x100);
	return !err;
}

// Cuda v2.XX ------------------------------------------------------------------------

ROM_START( cuda2xx )
	ROM_REGION(0x1100, "roms", 0)
	ROM_DEFAULT_BIOS("341s0788")

	ROM_SYSTEM_BIOS(0, "341s0417", "Cuda 2.35 (341S0417)")
	ROMX_LOAD( "341s0417.bin",  0x0000, 0x1100, CRC(571f24c9) SHA1(a2ae12492389a00e5f4b1ef19b267d6f3a8eadc3), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "341s0788", "Cuda 2.37 (341S0788)")
	ROMX_LOAD("341s0788.bin", 0x0000, 0x1100, CRC(df6e1b43) SHA1(ec23cc6214c472d61b98964928c40589517a3172), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "341s0789", "Cuda 2.38 (341S0789)")
	ROMX_LOAD("341s0789.bin", 0x0000, 0x1100, CRC(682d2ace) SHA1(81a9e25204f58363ed2f5945763ac19a1a66234e), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "341s0060", "Cuda 2.40 (341S0060)")
	ROMX_LOAD("341s0060.bin", 0x0000, 0x1100, CRC(0f5e7b4a) SHA1(972b3778146d9787b18c3a9874d505cf606b3e15), ROM_BIOS(3))

	ROM_REGION(0x100, "defaultnv", 0)
	ROM_LOAD( "cuda_nvram.bin", 0x000000, 0x000100, CRC(6e3da389) SHA1(e5b13a2a904cc9fc612ed25b76718c501c11b00a) )
ROM_END

const tiny_rom_entry *cuda_2xx_device::device_rom_region() const
{
	return ROM_NAME( cuda2xx );
}

cuda_2xx_device::cuda_2xx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cuda_device(mconfig, CUDA_V2XX, tag, owner, clock)
{
}

// Cuda v3.02 ------------------------------------------------------------------------

void cuda_302_device::cuda_map(address_map &map)
{
	cuda_device::cuda_map(map);
	map(0x000b, 0x000b).r(FUNC(cuda_302_device::portb_r));
	map(0x0080, 0x008f).ram();
	map(0x0b00, 0x1fff).rom().region("roms", 0);
}

ROM_START( cuda302 )
	ROM_REGION(0x1500, "roms", 0)
	ROM_LOAD( "341s0262.bin",  0x0000, 0x1500, CRC(f43a803d) SHA1(c9b2f2c4ea174a01073a9d20b16b362ddc3715a6) )

	ROM_REGION(0x100, "defaultnv", 0)
	ROM_LOAD( "cuda_nvram.bin", 0x000000, 0x000100, CRC(6e3da389) SHA1(e5b13a2a904cc9fc612ed25b76718c501c11b00a) )
ROM_END

const tiny_rom_entry *cuda_302_device::device_rom_region() const
{
	return ROM_NAME( cuda302 );
}

cuda_302_device::cuda_302_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cuda_device(mconfig, CUDA_V302, tag, owner, clock)
{
}

u8 cuda_302_device::portb_r()
{
	return 0x80;
}

// Cuda Lite ------------------------------------------------------------------------
void cuda_lite_device::cuda_map(address_map &map)
{
	cuda_device::cuda_map(map);
	map(0x000b, 0x000b).r(FUNC(cuda_lite_device::portb_r));
	map(0x0080, 0x008f).ram();
	map(0x0c00, 0x1fff).rom().region("roms", 0);
}

ROM_START( cudalite )
	ROM_REGION(0x1400, "roms", 0)
	ROM_LOAD( "341s0285.bin",  0x0000, 0x1400, CRC(ba2707da) SHA1(3fb8d610cd738699b2981d37e9fa37c1e515a423) )

	ROM_REGION(0x100, "defaultnv", 0)
	ROM_LOAD( "cuda_nvram.bin", 0x000000, 0x000100, CRC(6e3da389) SHA1(e5b13a2a904cc9fc612ed25b76718c501c11b00a) )
ROM_END

const tiny_rom_entry *cuda_lite_device::device_rom_region() const
{
	return ROM_NAME( cudalite );
}

cuda_lite_device::cuda_lite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cuda_device(mconfig, CUDA_LITE, tag, owner, clock)
{
}

u8 cuda_lite_device::portb_r()
{
	return 0x80;
}
