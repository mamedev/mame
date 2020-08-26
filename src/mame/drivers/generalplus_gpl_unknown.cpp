// license:BSD-3-Clause
// copyright-holders:David Haywood

// these contain the similar game selections to the games in unk6502_st2xxx.cpp but on updated hardware

// These use SPI ROMs and unSP2.0 instructions, so will be GeneralPlus branded parts, not SunPlus
// possibly the framebuffer based video ones rather than the ones with tile layers

/*

for pcp8728 long jumps are done indirect via a call to RAM

990c 20ec       r4 = 20ec
d9dd            [1d] = r4
990c 0007       r4 = 0007
d9de            [1e] = r4
fe80 28f7       goto 0028f7

the code to handle this is copied in at startup.

Almost all function calls in the game are handled via a call to RAM which copies data inline from SPI for execution
these calls manage their own stack, and copying back the caller function on return etc.

The largest function in RAM at any one time is ~0x600 bytes.

This appears to be incredibly inefficient but the system can't execute directly from SPI ROM, and doesn't have any
RAM outside of the small area internal to the Sunplus SoC

Graphics likewise appear to be loaded pixel by pixel from the SPI to framebuffer every single time there is a draw
call.  Sound is almost certainly handled in the same way.

There is a missing internal ROM that acts as bootstrap and provides some basic functions.  It is at least 0x1000
words in size, with the lowest call being to 0xf000.  It is potentially larger than this.

The internal ROM will also need to provide trampolining for the interrupts, there is a single pointer near the
start of the SPI ROM '02000A: 0041 0002' which points to 20041 (assuming you map the SPI ROM base as word address
0x20000, so that the calls to get code align with ROM addresses)

The function pointed to for the interrupt has the same form of the other functions that get loaded into RAM via
calls to functions in the RAM area.

--------------------------------------------------------

BIOS (internal ROM) calls:

0xf000 - copy dword from SPI using provided pointer

0xf56f - unknown, after some time, done with PC = f56f, only in one place

0xf58f - unknown, soon after startup (only 1 call)

00f7a0 - unknown - 3 calls

0xf931 - unknown, just one call

00fa1d - unknown, just one call

0xfb26 - unknown, after some time (done with pc = fb26 and calls)

0xfb4f - unknown, just one call

0xfbbf - unknown, 3 calls

code currently goes off the rails after some of these unhandled calls (one to f56f?)

--

use 'go 2938' to get to the inline code these load on the fly

the first piece of code copied appears to attempt to checksum the internal BIOS!

*/

#include "emu.h"

#include "cpu/unsp/unsp.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class pcp8718_state : public driver_device
{
public:
	pcp8718_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_spirom(*this, "spi")
	{ }

	void pcp8718(machine_config &config);

	void spi_init();

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<unsp_20_device> m_maincpu;
	required_shared_ptr<uint16_t> m_mainram;

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	void map(address_map &map);

	uint16_t simulate_f000_r(offs_t offset);

	uint16_t ramcall_2060_logger_r();
	uint16_t ramcall_2189_logger_r();

	uint16_t ramcall_2434_logger_r();

	uint16_t ramcall_2829_logger_r();
	uint16_t ramcall_287a_logger_r();
	uint16_t ramcall_28f7_logger_r();
	uint16_t ramcall_2079_logger_r();


	required_region_ptr<uint8_t> m_spirom;

	uint16_t unk_7abf_r();
	uint16_t unk_7860_r();

	void unk_7868_w(uint16_t data);

	uint16_t spi_misc_control_r();
	uint16_t spi_rx_fifo_r();
	void spi_tx_fifo_w(uint16_t data);

	void spi_control_w(uint16_t data);


	void spi_process_tx_data(uint8_t data);
	uint8_t spi_process_rx();
	uint8_t spi_rx();

	uint8_t m_rx_fifo[4]; // actually 8 bytes? or 8 half-bytes?

	uint32_t m_spiaddress;

	enum spistate : const int
	{
	   SPI_STATE_READY = 0,
	   SPI_STATE_WAITING_HIGH_ADDR = 1,
	   SPI_STATE_WAITING_MID_ADDR = 2,
	   SPI_STATE_WAITING_LOW_ADDR = 3,
	   // probably not
	   SPI_STATE_WAITING_DUMMY1_ADDR = 4,
	   SPI_STATE_WAITING_DUMMY2_ADDR = 5,
	   SPI_STATE_READING = 6

	};

	spistate m_spistate;

};

uint32_t pcp8718_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_mainram[0x12] |= 0x8000; // some code waits on this, what is it?
	m_mainram[0x09] &= 0xfff0; // should be cleared by IRQ?
	return 0;
}




void pcp8718_state::machine_start()
{
}

static INPUT_PORTS_START( pcp8718 )
INPUT_PORTS_END

uint16_t pcp8718_state::unk_7abf_r()
{
	return 0x0001;
}

uint16_t pcp8718_state::unk_7860_r()
{
	return machine().rand() & 0x8;
}


uint16_t pcp8718_state::spi_misc_control_r()
{
	logerror("%06x: spi_misc_control_r\n", machine().describe_context());
	return 0x0000;
}


uint16_t pcp8718_state::spi_rx_fifo_r()
{
	logerror("%06x: spi_rx_fifo_r\n", machine().describe_context());
	return spi_rx();
}

void pcp8718_state::spi_process_tx_data(uint8_t data)
{
	logerror("transmitting %02x\n", data);

	switch (m_spistate)
	{
	case SPI_STATE_READY:
	{
		if (data == 0x03)
		{
			logerror("set to read mode (need address) %02x\n", data);
			m_spistate = SPI_STATE_WAITING_HIGH_ADDR;
		}
		else
		{
			logerror("invalid state request %02x\n", data);
		}
		break;
	}

	case SPI_STATE_WAITING_HIGH_ADDR:
	{
		m_spiaddress = (m_spiaddress & 0xff00ffff) | data << 16;
		logerror("set to high address %02x address is now %08x\n", data, m_spiaddress);
		m_spistate = SPI_STATE_WAITING_MID_ADDR;
		break;
	}

	case SPI_STATE_WAITING_MID_ADDR:
	{
		m_spiaddress = (m_spiaddress & 0xffff00ff) | data << 8;
		logerror("set to mid address %02x address is now %08x\n", data, m_spiaddress);
		m_spistate = SPI_STATE_WAITING_LOW_ADDR;
		break;
	}

	case SPI_STATE_WAITING_LOW_ADDR:
	{
		m_spiaddress = (m_spiaddress & 0xffffff00) | data;
		logerror("set to low address %02x address is now %08x\n", data, m_spiaddress);
		m_spistate = SPI_STATE_READING;
		break;
	}

	case SPI_STATE_READING:
	{
		// writes when in read mode clock in data?
		logerror("write while in read mode (clock data?)\n", data, m_spiaddress);
		break;
	}

	case SPI_STATE_WAITING_DUMMY1_ADDR:
	{
		m_spistate = SPI_STATE_WAITING_DUMMY2_ADDR;
		break;
	}

	case SPI_STATE_WAITING_DUMMY2_ADDR:
	{
	//  m_spistate = SPI_STATE_READY;
		break;
	}
	}
}

uint8_t pcp8718_state::spi_process_rx()
{

	switch (m_spistate)
	{
	case SPI_STATE_READING:
	{
		uint8_t dat = m_spirom[m_spiaddress & 0x3fffff];

		// hack internal BIOS checksum check
		if (m_spiaddress == ((0x49d13 - 0x20000) * 2)+1)
			if (dat == 0x4e)
				dat = 0x5e;

		logerror("reading SPI %02x from SPI Address %08x (adjusted word offset %08x)\n", dat, m_spiaddress, (m_spiaddress/2)+0x20000);
		m_spiaddress++;
		return dat;
	}

	default:
	{
		logerror("reading FIFO in unknown state\n");
		return 0x00;
	}
	}

	return 0x00;
}


uint8_t pcp8718_state::spi_rx()
{
	uint8_t ret = m_rx_fifo[0];

	m_rx_fifo[0] = m_rx_fifo[1];
	m_rx_fifo[1] = m_rx_fifo[2];
	m_rx_fifo[2] = m_rx_fifo[3];
	m_rx_fifo[3] = spi_process_rx();

	return ret;
}


void pcp8718_state::spi_tx_fifo_w(uint16_t data)
{
	data &= 0x00ff;
	logerror("%06x: spi_tx_fifo_w %02x\n", machine().describe_context(), data);

	spi_process_tx_data(data);
}

// this is probably 'port b' but when SPI is enabled some points of this can become SPI control pins
// it's accessed after each large data transfer, probably to reset the SPI into 'ready for command' state?
void pcp8718_state::unk_7868_w(uint16_t data)
{
	logerror("%06x: unk_7868_w %02x (Port B + SPI reset?)\n", machine().describe_context(), data);

	for (int i = 0; i < 4; i++)
		m_rx_fifo[i] = 0xff;

	m_spistate = SPI_STATE_READY;

}


void pcp8718_state::spi_control_w(uint16_t data)
{
	logerror("%06x: spi_control_w %04x\n", machine().describe_context(), data);
}

void pcp8718_state::map(address_map &map)
{
	// there are calls to 01xxx and 02xxx regions
	// (RAM populated by internal ROM?, TODO: check to make sure code copied there isn't from SPI ROM like the GPL16250 bootstrap
	//  does from NAND, it doesn't seem to have a header in the same format at least)
	map(0x000000, 0x006fff).ram().share("mainram");
	map(0x007000, 0x0077ff).ram(); // might be registers, but the call stubs for RAM calls explicitly use addresses in here for private stack so that previous snippets can be restored?

	map(0x007860, 0x007860).r(FUNC(pcp8718_state::unk_7860_r));

	map(0x007868, 0x007868).w(FUNC(pcp8718_state::unk_7868_w));

	map(0x007940, 0x007940).w(FUNC(pcp8718_state::spi_control_w));
	// 7941 SPI Transmit Status
	map(0x007942, 0x007942).w(FUNC(pcp8718_state::spi_tx_fifo_w));
	// 7943 SPI Receive Status
	map(0x007944, 0x007944).r(FUNC(pcp8718_state::spi_rx_fifo_r));
	map(0x007945, 0x007945).r(FUNC(pcp8718_state::spi_misc_control_r));

	map(0x007abf, 0x007abf).r(FUNC(pcp8718_state::unk_7abf_r));


	// registers at 7xxx are similar to GPL16250, but not identical? (different video system?)

	// there are calls to 0x0f000 (internal ROM?)
	map(0x00f000, 0x00ffff).rom().region("maincpu", 0x00000);
}



uint16_t pcp8718_state::simulate_f000_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		uint16_t pc = m_maincpu->state_int(UNSP_PC);
		uint16_t sr = m_maincpu->state_int(UNSP_SR);
		int realpc = (pc | (sr << 16)) & 0x003fffff;

		if ((offset + 0xf000) == (realpc))
		{
			if (realpc == 0xf000)
			{
				address_space& mem = m_maincpu->space(AS_PROGRAM);

				uint32_t source = (mem.read_word(0x001e) << 16) | mem.read_word(0x001d);

				if (source >= 0x20000)
				{
					uint16_t data = m_spirom[((source - 0x20000) * 2) + 0] | (m_spirom[((source - 0x20000) * 2) + 1] << 8);
					uint16_t data2 = m_spirom[((source - 0x20000) * 2) + 2] | (m_spirom[((source - 0x20000) * 2) + 3] << 8);

					logerror("call to 0xf000 - copying from %08x to 04/05\n", source); // some code only uses 04, but other code copies pointers and expects results in 04 and 05

					mem.write_word(0x0004, data);
					mem.write_word(0x0005, data2);
				}

				return 0x9a90; // retf
			}
			else if (realpc == 0xf58f)
			{
				logerror("call to 0xf58f - unknown function\n");
				return 0x9a90; // retf
			}
			else if (realpc == 0xfb26) // done with a call, and also a pc =
			{
				logerror("call to 0xfb26 - unknown function\n");
				return 0x9a90; // retf
			}
			else if (realpc == 0xf56f) // done with a pc =
			{
				logerror("call to 0xf56f - unknown function\n");
				return 0x9a90; // retf
			}
			else
			{
				fatalerror("simulate_f000_r unhandled BIOS simulation offset %04x\n", offset);
			}

		}
		else
		{
			logerror("simulate_f000_r reading BIOS area (for checksum?) %04x\n", offset);
		}
	}
	return 0x0000;
}

uint16_t pcp8718_state::ramcall_2060_logger_r()
{
	if (!machine().side_effects_disabled())
	{
		logerror("call to 0x2060 in RAM (set SPI to read mode, set address, do dummy FIFO reads)\n");
	}
	return m_mainram[0x2060];
}

uint16_t pcp8718_state::ramcall_2189_logger_r()
{
	if (!machine().side_effects_disabled())
	{
		logerror("call to 0x2189 in RAM (unknown)\n");
	}
	return m_mainram[0x2189];
}


uint16_t pcp8718_state::ramcall_2829_logger_r()
{
	// this in turn calls 28f7 but has restore logic too
	if (!machine().side_effects_disabled())
	{
		logerror("call to 0x2829 in RAM (load+call function from SPI address %08x)\n", (m_mainram[0x1e] << 16) | m_mainram[0x1d]);
	}
	return m_mainram[0x2829];
}




uint16_t pcp8718_state::ramcall_287a_logger_r()
{
	if (!machine().side_effects_disabled())
	{
		logerror("call to 0x287a in RAM (unknown)\n");
	}
	return m_mainram[0x287a];
}

uint16_t pcp8718_state::ramcall_28f7_logger_r()
{
	if (!machine().side_effects_disabled())
	{
		// no  restore logic?
		logerror("call to 0x28f7 in RAM (load+GO TO function from SPI address %08x)\n", (m_mainram[0x1e] << 16) | m_mainram[0x1d]);
	}
	return m_mainram[0x28f7];
}

uint16_t pcp8718_state::ramcall_2079_logger_r()
{
	if (!machine().side_effects_disabled())
	{
		logerror("call to 0x2079 in RAM (maybe drawing related?)\n"); // called in the 'dummy' loop that doesn't actually draw? and other places? as well as after the actual draw command below in the real loop
	}
	return m_mainram[0x2079];
}

uint16_t pcp8718_state::ramcall_2434_logger_r()
{
	if (!machine().side_effects_disabled())
	{
		logerror("call to 0x2434 in RAM (drawing related?)\n"); // [1d] as the tile / sprite number, [1e] as xpos, [1f] as ypos, [20] as 0. [21] as ff in some title drawing calls
	}
	return m_mainram[0x2434];
}



void pcp8718_state::machine_reset()
{
	// this looks like it might actually be part of the IRQ handler (increase counter at 00 at the very start) rather than where we should end up after startup
	// it also looks like (from the pc = 2xxx opcodes) that maybe this code should be being executed in RAM as those don't give correct offsets in the data segment.
	m_maincpu->set_state_int(UNSP_PC, 0x4000);
	m_maincpu->set_state_int(UNSP_SR, 0x0000);

	//uint16_t* ROM = (uint16_t*)memregion("maincpu")->base();
	//ROM[0x0000] = 0x9a90; // retf from internal ROM call to 0xf000 (unknown purpose)

	// there doesn't appear to be any code to set the SP, so it must be done by the internal ROM
	m_maincpu->set_state_int(UNSP_SP, 0x5fff);

	m_maincpu->space(AS_PROGRAM).install_read_handler(0xf000, 0xffff, read16sm_delegate(*this, FUNC(pcp8718_state::simulate_f000_r)));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2060, 0x2060, read16smo_delegate(*this, FUNC(pcp8718_state::ramcall_2060_logger_r)));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2079, 0x2079, read16smo_delegate(*this, FUNC(pcp8718_state::ramcall_2079_logger_r)));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2189, 0x2189, read16smo_delegate(*this, FUNC(pcp8718_state::ramcall_2189_logger_r)));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2434, 0x2434, read16smo_delegate(*this, FUNC(pcp8718_state::ramcall_2434_logger_r)));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2829, 0x2829, read16smo_delegate(*this, FUNC(pcp8718_state::ramcall_2829_logger_r)));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x287a, 0x287a, read16smo_delegate(*this, FUNC(pcp8718_state::ramcall_287a_logger_r)));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x28f7, 0x28f7, read16smo_delegate(*this, FUNC(pcp8718_state::ramcall_28f7_logger_r)));

	m_spistate = SPI_STATE_READY;
	m_spiaddress = 0;

}


void pcp8718_state::pcp8718(machine_config &config)
{

	UNSP_20(config, m_maincpu, 20000000); // unknown CPU, unsp20 based
	m_maincpu->set_addrmap(AS_PROGRAM, &pcp8718_state::map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(pcp8718_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);
}

// pcp8718 and pcp8728 both contain user data (player name?) and will need to be factory defaulted once they work
// the ROM code is slightly different between them

ROM_START( pcp8718 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x2000, NO_DUMP ) // exact size unknown

	ROM_REGION( 0x800000, "spi", ROMREGION_ERASEFF )
	//ROM_LOAD16_WORD_SWAP( "8718_en25f32.bin", 0x000000, 0x400000, CRC(cc138db4) SHA1(379af3d94ae840f52c06416d6cf32e25923af5ae) ) // bad dump, some blocks are corrupt
	ROM_LOAD( "eyecare_25q32av1g_ef4016.bin", 0x000000, 0x400000, CRC(58415e10) SHA1(b1adcc03f2ad8d741544204671677740e904ce1a) )
ROM_END

ROM_START( pcp8728 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x2000, NO_DUMP ) // exact size unknown

	ROM_REGION( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD( "pcp 8728 788 in 1.bin", 0x000000, 0x400000, CRC(60115f21) SHA1(e15c39f11e442a76fae3823b6d510178f6166926) )
ROM_END

ROM_START( unkunsp )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x2000, NO_DUMP ) // exact size unknown

	ROM_REGION( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD( "fm25q16a.bin", 0x000000, 0x200000, CRC(aeb472ac) SHA1(500c24b725f6d3308ef8cbdf4259f5be556c7c92) )
ROM_END


void pcp8718_state::spi_init()
{
	uint8_t* rom = memregion("spi")->base();

	uint32_t start = rom[0] | (rom[1] << 8) | (rom[2] << 16) | (rom[3] << 24);
	uint32_t end =   rom[4] | (rom[5] << 8) | (rom[2] << 16) | (rom[3] << 24);


	logerror("start: %08x\n", start);
	logerror("end: %08x\n", end);

	int writebase = 0x4000;

	if (start > end)
	{
		logerror("invalid initial copy?\n");
		return;
	}
	else
	{
		start -= 0x20000;
		end -= 0x20000;

		for (int i = start*2; i <= end*2; i+=2)
		{
			uint16_t dat = rom[i] | (rom[i+1] << 8);
			m_mainram[writebase++] = dat;
		}
	}

}


CONS( 200?, pcp8718,      0,       0,      pcp8718,   pcp8718, pcp8718_state, spi_init, "PCP", "PCP 8718 - HD 360 Degrees Rocker Palm Eyecare Console - 788 in 1", MACHINE_IS_SKELETON )
CONS( 200?, pcp8728,      0,       0,      pcp8718,   pcp8718, pcp8718_state, spi_init, "PCP", "PCP 8728 - 788 in 1", MACHINE_IS_SKELETON ) // what name was this sold under?

// maybe different hardware, first 0x2000 bytes in ROM is blank, so bootstrap pointers aren't there at least
CONS( 200?, unkunsp,      0,       0,      pcp8718,   pcp8718, pcp8718_state, empty_init, "<unknown>", "unknown unSP-based handheld", MACHINE_IS_SKELETON )
