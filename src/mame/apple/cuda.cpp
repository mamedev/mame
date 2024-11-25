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
    ---x----  O  DFAC latch / wake-up
    ----x---  O  "Fast Reset"
    -----x--  I  Keyboard power switch
    ------x-  I  1 for secure or passive power, chassis switch for soft
    -------x  ?  Power Fail warning, 1 = power ok, 0 = failing

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
    341S0789 - 0x00020026 (2.38) - Macintosh TV
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

void cuda_device::device_add_mconfig(machine_config &config)
{
	M68HC05E1(config, m_maincpu, XTAL(32'768) * 128); // Intended to run 4.1 MHz, the ADB timings in uS are twice as long as spec at 2.1
	m_maincpu->read_p<0>().set(FUNC(cuda_device::pa_r));
	m_maincpu->read_p<1>().set(FUNC(cuda_device::pb_r));
	m_maincpu->read_p<2>().set(FUNC(cuda_device::pc_r));
	m_maincpu->write_p<0>().set(FUNC(cuda_device::pa_w));
	m_maincpu->write_p<1>().set(FUNC(cuda_device::pb_w));
	m_maincpu->write_p<2>().set(FUNC(cuda_device::pc_w));
	m_maincpu->set_pullups<1>(0xc0); // pull-ups on port B bits 6 & 7 (DFAC/I2C SDA & SCL)
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
	write_dfac_latch(*this),
	m_maincpu(*this, "cudamcu"),
	m_default_nvram(*this, "defaultnv"),
	m_treq(0), m_byteack(0), m_tip(0), m_via_data(0), m_last_adb(0),
	m_iic_sda(0), m_last_adb_time(0), m_cuda_controls_power(false), m_adb_in(false),
	m_reset_line(0), m_adb_dtime(0), m_pram_loaded(false)
{
	std::fill(std::begin(m_disk_pram), std::end(m_disk_pram), 0);
}

void cuda_device::device_start()
{
	save_item(NAME(m_treq));
	save_item(NAME(m_byteack));
	save_item(NAME(m_tip));
	save_item(NAME(m_via_data));
	save_item(NAME(m_adb_in));
	save_item(NAME(m_reset_line));
	save_item(NAME(m_adb_dtime));
	save_item(NAME(m_pram_loaded));
	save_item(NAME(m_disk_pram));

#if ((VERBOSE & LOG_PACKETS) == LOG_PACKETS)
	m_maincpu->space().install_read_tap(0xba, 0xba, 0, "cudamcu", [this](offs_t offset, u8 &data, u8 mem_mask)
	{
		if (m_maincpu->pc() == 0x12b3)
		{
			LOG("Got command %02x\n", data);
		}
	});
#endif

	m_maincpu->space().install_write_tap(4, 4, 0, "cudapfw", [](offs_t offset, u8 &data, u8 mem_mask)
	{
		// Intercept DDR write for port A to make PFW (bit 0) always an input.  The way the Cuda firmware is written
		// would not work on a stock 68HC05E1, so it's likely the actual part was lightly customized in this manner.
		data &= ~0x01;
		return data;
	});
}

void cuda_device::device_reset()
{
	m_last_adb_time = m_maincpu->total_cycles();
}

void cuda_device::pa_w(u8 data)
{
	write_dfac_latch(BIT(data, 4));

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
}

void cuda_device::pb_w(u8 data)
{
	if (m_treq != BIT(data, 1))
	{
		LOGMASKED(LOG_HOSTCOMM, "CU-> TREQ: %d (PC=%x)\n", (data>>1)&1, m_maincpu->pc());
		m_treq = (data>>1) & 1;
	}

	LOGMASKED(LOG_HOSTCOMM, "CU-> VIA_DATA: %d VIA_CLOCK %d (PC=%x)\n", BIT(data, 5), BIT(data, 4), m_maincpu->pc());
	write_via_data(BIT(data, 5));
	write_via_clock(BIT(data, 4));
//  printf("SDA: %d SCL: %d\n", BIT(data, 6), BIT(data, 7));
	write_iic_sda(BIT(data, 6));
	write_iic_scl(BIT(data, 7));
}

void cuda_device::pc_w(u8 data)
{
	if ((data & 8) != m_reset_line)
	{
		LOGMASKED(LOG_HOSTCOMM, "680x0 reset: %d -> %d (PC=%x)\n", m_reset_line, BIT(data, 3), m_maincpu->pc());
		// falling edge, should reset the machine too
		if (!m_reset_line && BIT(data, 3))
		{
			write_reset(ASSERT_LINE);
			write_reset(CLEAR_LINE);
			m_reset_line = BIT(data, 3);

			// if PRAM's waiting to be loaded, transfer it now
			if (!m_pram_loaded)
			{
				for (int byte = 0; byte < 256; byte++)
				{
					m_maincpu->write_internal_ram(0x70 + byte, m_disk_pram[byte]);
				}

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
				m_pram_loaded = true;
			}
		}
	}
}

u8 cuda_device::pa_r()
{
	u8 rv = 0;

	if (m_cuda_controls_power)
	{
		rv = 0x20 | 0x01; // pull up + chassis switch (which is 0 = on)
	}
	else
	{
		rv = 0x02 | 0x01;   // pull-up + PFW
	}

	if (m_adb_in)
	{
		rv |= 0x40;
	}

	return rv;
}

u8 cuda_device::pb_r()
{
	u8 rv = 0;

	rv |= 0x01; // +5v sense
	rv |= m_byteack << 2;
	rv |= m_tip << 3;
	rv |= m_via_data << 5;
	rv |= m_iic_sda ? 0x40 : 0;
	rv |= 0x80;

	return rv;
}

u8 cuda_device::pc_r()
{
	u8 rv = 0;

	if (m_cuda_controls_power)
	{
		rv = 0x02 | 0x01; // soft power: trickle sense + pull-up
	}
	else
	{
		rv = 0x02 | 0x01; // secure power: pull-up + file server
	}
	return rv;
}

// The 6805 program clears PRAM on startup (on h/w it's always running once a battery is inserted).
// We deal with that by loading PRAM from disk to a secondary buffer and then slapping it into "live"
// once Cuda starts the host processor.
void cuda_device::nvram_default()
{
	LOGMASKED(LOG_PRAM, "Using default PRAM\n");
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
	for (int byte = 0; byte < 256; byte++)
	{
		m_disk_pram[byte] = m_maincpu->read_internal_ram(0x70 + byte);
	}

	auto const [err, actual] = write(file, m_disk_pram, 0x100);
	return !err;
}

// Cuda v2.XX ------------------------------------------------------------------------

ROM_START( cuda2xx )
	ROM_REGION(0x1100, "cudamcu", 0)
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

ROM_START( cuda302 )
	ROM_REGION(0x1500, "cudamcu", 0)
	ROM_LOAD( "341s0262.bin",  0x0000, 0x1500, CRC(f43a803d) SHA1(c9b2f2c4ea174a01073a9d20b16b362ddc3715a6) )

	ROM_REGION(0x100, "defaultnv", 0)
	ROM_LOAD( "cuda_nvram.bin", 0x000000, 0x000100, CRC(6e3da389) SHA1(e5b13a2a904cc9fc612ed25b76718c501c11b00a) )
ROM_END

const tiny_rom_entry *cuda_302_device::device_rom_region() const
{
	return ROM_NAME( cuda302 );
}

void cuda_302_device::device_add_mconfig(machine_config &config)
{
	M68HC05E5(config, m_maincpu, XTAL(32'768) * 128); // Intended to run 4.1 MHz, the ADB timings in uS are twice as long as spec at 2.1
	m_maincpu->read_p<0>().set(FUNC(cuda_302_device::pa_r));
	m_maincpu->read_p<1>().set(FUNC(cuda_302_device::pb_r));
	m_maincpu->read_p<2>().set(FUNC(cuda_302_device::pc_r));
	m_maincpu->write_p<0>().set(FUNC(cuda_302_device::pa_w));
	m_maincpu->write_p<1>().set(FUNC(cuda_302_device::pb_w));
	m_maincpu->write_p<2>().set(FUNC(cuda_302_device::pc_w));
	m_maincpu->set_pullups<1>(0xc0); // pull-ups on port B bits 6 & 7 (DFAC/I2C SDA & SCL)
}

cuda_302_device::cuda_302_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cuda_device(mconfig, CUDA_V302, tag, owner, clock)
{
}

// Cuda Lite ------------------------------------------------------------------------

ROM_START( cudalite )
	ROM_REGION(0x1500, "cudamcu", 0)
	ROM_LOAD( "341s0285.bin",  0x0100, 0x1400, CRC(ba2707da) SHA1(3fb8d610cd738699b2981d37e9fa37c1e515a423) )

	ROM_REGION(0x100, "defaultnv", 0)
	ROM_LOAD( "cuda_nvram.bin", 0x000000, 0x000100, CRC(6e3da389) SHA1(e5b13a2a904cc9fc612ed25b76718c501c11b00a) )
ROM_END

void cuda_lite_device::device_add_mconfig(machine_config &config)
{
	M68HC05E5(config, m_maincpu, XTAL(32'768) * 128); // Intended to run 4.1 MHz, the ADB timings in uS are twice as long as spec at 2.1
	m_maincpu->read_p<0>().set(FUNC(cuda_lite_device::pa_r));
	m_maincpu->read_p<1>().set(FUNC(cuda_lite_device::pb_r));
	m_maincpu->read_p<2>().set(FUNC(cuda_lite_device::pc_r));
	m_maincpu->write_p<0>().set(FUNC(cuda_lite_device::pa_w));
	m_maincpu->write_p<1>().set(FUNC(cuda_lite_device::pb_w));
	m_maincpu->write_p<2>().set(FUNC(cuda_lite_device::pc_w));
	m_maincpu->set_pullups<1>(0xc0); // pull-ups on port B bits 6 & 7 (DFAC/I2C SDA & SCL)
}

const tiny_rom_entry *cuda_lite_device::device_rom_region() const
{
	return ROM_NAME( cudalite );
}

cuda_lite_device::cuda_lite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cuda_device(mconfig, CUDA_LITE, tag, owner, clock)
{
}
