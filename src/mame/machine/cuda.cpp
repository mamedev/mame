// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "Cuda" ADB/system controller MCU
    Emulation by R. Belmont

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

    x-------  O  DFAC bit clock
    -x------  B  DFAC data I/O (used in both directions)
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
*/


//#define CUDA_SUPER_VERBOSE

#include "emu.h"
#include "cuda.h"
#include "cpu/m6805/m6805.h"
#include "sound/asc.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CUDA_CPU_TAG    "cuda"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CUDA, cuda_device, "cuda", "Apple Cuda")

ROM_START( cuda )
	ROM_REGION(0x4400, CUDA_CPU_TAG, 0)
	ROM_LOAD( "341s0060.bin", 0x1100, 0x1100, CRC(0f5e7b4a) SHA1(972b3778146d9787b18c3a9874d505cf606b3e15) )
	ROM_LOAD( "341s0788.bin", 0x2200, 0x1100, CRC(df6e1b43) SHA1(ec23cc6214c472d61b98964928c40589517a3172) )
	ROM_LOAD( "341s0417.bin", 0x3300, 0x1100, CRC(571f24c9) SHA1(a2ae12492389a00e5f4b1ef19b267d6f3a8eadc3) )
ROM_END

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void cuda_device::cuda_map(address_map &map)
{
	map(0x0000, 0x0002).rw(FUNC(cuda_device::ports_r), FUNC(cuda_device::ports_w));
	map(0x0004, 0x0006).rw(FUNC(cuda_device::ddr_r), FUNC(cuda_device::ddr_w));
	map(0x0007, 0x0007).rw(FUNC(cuda_device::pll_r), FUNC(cuda_device::pll_w));
	map(0x0008, 0x0008).rw(FUNC(cuda_device::timer_ctrl_r), FUNC(cuda_device::timer_ctrl_w));
	map(0x0009, 0x0009).rw(FUNC(cuda_device::timer_counter_r), FUNC(cuda_device::timer_counter_w));
	map(0x0012, 0x0012).rw(FUNC(cuda_device::onesec_r), FUNC(cuda_device::onesec_w));
	map(0x0090, 0x00ff).ram();                         // work RAM and stack
	map(0x0100, 0x01ff).rw(FUNC(cuda_device::pram_r), FUNC(cuda_device::pram_w));
	map(0x0f00, 0x1fff).rom().region(CUDA_CPU_TAG, 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

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

void cuda_device::send_port(uint8_t offset, uint8_t data)
{
//    printf("PORT %c write %02x (DDR = %02x) (PC=%x)\n", 'A' + offset, data, ddrs[offset], m_maincpu->pc());

	switch (offset)
	{
	case 0: // port A
/*          printf("PORT A ADB:%d DFAC:%d PFW:%d\n",
                (data & 0x80) ? 1 : 0,
                (data & 0x10) ? 1 : 0,
                (data & 0x01) ? 1 : 0);*/

			if ((data & 0x80) != last_adb)
			{
/*                if (data & 0x80)
                {
                    printf("CU ADB: 1->0 time %lld\n", machine().time().as_ticks(1000000) - last_adb_time);
                }
                else
                {
                    printf("CU ADB: 0->1 time %lld\n", machine().time().as_ticks(1000000) - last_adb_time);
                }*/

				// allow the linechange handler to override us
				adb_in = (data & 0x80) ? true : false;

				m_adb_dtime = (int)(machine().time().as_ticks(1000000) - last_adb_time);
				write_linechange(((data & 0x80) >> 7) ^ 1);

				last_adb = data & 0x80;
				last_adb_time = machine().time().as_ticks(1000000);
			}
			break;

		case 1: // port B
			{
				if (treq != ((data>>1)&1))
				{
					#ifdef CUDA_SUPER_VERBOSE
					printf("CU-> TREQ: %d (PC=%x)\n", (data>>1)&1, m_maincpu->pc());
					#endif
					treq = (data>>1) & 1;
				}
				if (via_data != ((data>>5)&1))
				{
					#ifdef CUDA_SUPER_VERBOSE
					printf("CU-> VIA_DATA: %d (PC=%x)\n", (data>>5)&1, m_maincpu->pc());
					#endif
					via_data = (data>>5) & 1;
					write_via_data(via_data);
				}
				if (via_clock != ((data>>4)&1))
				{
					#ifdef CUDA_SUPER_VERBOSE
					printf("CU-> VIA_CLOCK: %d (PC=%x)\n", ((data>>4)&1)^1, m_maincpu->pc());
					#endif
					via_clock = (data>>4) & 1;
					write_via_clock(via_clock);
				}
			}
			break;

		case 2: // port C
			if ((data & 8) != reset_line)
			{
				#ifdef CUDA_SUPER_VERBOSE
				printf("680x0 reset: %d -> %d (PC=%x)\n", (ports[2] & 8)>>3, (data & 8)>>3, m_maincpu->pc());
				#endif
				reset_line = (data & 8);
				// falling edge, should reset the machine too
				if ((ports[2] & 8) && !(data&8))
				{
					write_reset(ASSERT_LINE);
					write_reset(CLEAR_LINE);

					// if PRAM's waiting to be loaded, transfer it now
					if (!pram_loaded)
					{
//                      memcpy(pram, disk_pram, 0x100);
						pram_loaded = true;
					}
				}
			}
			break;
	}
}

uint8_t cuda_device::ddr_r(offs_t offset)
{
	return ddrs[offset];
}

void cuda_device::ddr_w(offs_t offset, uint8_t data)
{
//    printf("%02x to PORT %c DDR (PC=%x)\n", data, 'A' + offset, m_maincpu->pc());

	send_port(offset, ports[offset] & data);

	ddrs[offset] = data;
}

uint8_t cuda_device::ports_r(offs_t offset)
{
	uint8_t incoming = 0;

	switch (offset)
	{
		case 0:     // port A
			if (cuda_controls_power)
			{
				incoming = 0x20; // pull up + chassis switch (which is 0 = on)
			}
			else
			{
				incoming = 0x02 | 0x01;   // pull-up + PFW
			}

			if (adb_in)
			{
				incoming |= 0x40;
			}
			break;

		case 1:     // port B
//            printf("Read: byteack %d tip %d via_data %d\n", byteack, tip, via_data);
			incoming |= 0x01;   // +5v sense
			incoming |= byteack<<2;
			incoming |= tip<<3;
			incoming |= via_data<<5;
			incoming |= 0xc0;   // show DFAC lines high
			break;

		case 2:     // port C
			if (cuda_controls_power)
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
	incoming &= (ddrs[offset] ^ 0xff);
	// add in ddr-masked version of port writes
	incoming |= (ports[offset] & ddrs[offset]);

	// HACK: don't know how this works on h/w...
	if (!offset)
	{
		incoming |= 0x01;
	}

//    printf("PORT %c read = %02x (DDR = %02x latch = %02x) (PC=%x)\n", 'A' + offset, ports[offset], ddrs[offset], ports[offset], m_maincpu->pc());

	return incoming;
}

void cuda_device::ports_w(offs_t offset, uint8_t data)
{
	send_port(offset, data);

	ports[offset] = data;
}

uint8_t cuda_device::pll_r()
{
	return pll_ctrl;
}

void cuda_device::pll_w(uint8_t data)
{
	#ifdef CUDA_SUPER_VERBOSE
	if (pll_ctrl != data)
	{
		static const int clocks[4] = { 524288, 1048576, 2097152, 4194304 };
		printf("PLL ctrl: clock %d TCS:%d BCS:%d AUTO:%d BWC:%d PLLON:%d (PC=%x)\n", clocks[data&3],
			(data & 0x80) ? 1 : 0,
			(data & 0x40) ? 1 : 0,
			(data & 0x20) ? 1 : 0,
			(data & 0x10) ? 1 : 0,
			(data & 0x08) ? 1 : 0, m_maincpu->pc());
	}
	#endif
	pll_ctrl = data;
}

uint8_t cuda_device::timer_ctrl_r()
{
	return timer_ctrl;
}

void cuda_device::timer_ctrl_w(uint8_t data)
{
	static const attotime rates[4][5] =
	{
		{ attotime::from_seconds(1), attotime::from_usec(31300), attotime::from_usec(15600), attotime::from_usec(7800), attotime::from_usec(3900) },
		{ attotime::from_seconds(2), attotime::from_usec(62500), attotime::from_usec(31300), attotime::from_usec(15600), attotime::from_usec(7800) },
		{ attotime::from_seconds(4), attotime::from_usec(125000), attotime::from_usec(62500), attotime::from_usec(31300), attotime::from_usec(15600) },
		{ attotime::from_seconds(8), attotime::from_usec(250000), attotime::from_usec(125100), attotime::from_usec(62500), attotime::from_usec(31300) },
	};

//    printf("%02x to timer control (PC=%x)\n", data, m_maincpu->pc());

	if (data & 0x50)
	{
		m_prog_timer->adjust(rates[data & 3][(pll_ctrl&3)+1], 1, rates[data&3][(pll_ctrl&3)+1]);
		ripple_counter = timer_counter;
	}
	else
	{
		m_prog_timer->adjust(attotime::never);
	}

	if ((timer_ctrl & 0x80) && !(data & 0x80))
	{
		m_maincpu->set_input_line(M68HC05EG_INT_TIMER, CLEAR_LINE);
		timer_ctrl &= ~0x80;
	}
	else if ((timer_ctrl & 0x40) && !(data & 0x40))
	{
		m_maincpu->set_input_line(M68HC05EG_INT_TIMER, CLEAR_LINE);
		timer_ctrl &= ~0x40;
	}

	timer_ctrl &= 0xc0;
	timer_ctrl |= (data & ~0xc0);
}

uint8_t cuda_device::timer_counter_r()
{
	return timer_counter;
}

void cuda_device::timer_counter_w(uint8_t data)
{
//    printf("%02x to timer counter (PC=%x)\n", data, m_maincpu->pc());
	timer_counter = data;
	ripple_counter = timer_counter;
}

uint8_t cuda_device::onesec_r()
{
	return onesec;
}

void cuda_device::onesec_w(uint8_t data)
{
	m_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	if ((onesec & 0x40) && !(data & 0x40))
	{
		m_maincpu->set_input_line(M68HC05EG_INT_CPI, CLEAR_LINE);
	}

	onesec = data;
}

uint8_t cuda_device::pram_r(offs_t offset)
{
	return pram[offset];
}

void cuda_device::pram_w(offs_t offset, uint8_t data)
{
	pram[offset] = data;
}

//-------------------------------------------------
//  cuda_device - constructor
//-------------------------------------------------

cuda_device::cuda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CUDA, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	write_reset(*this),
	write_linechange(*this),
	write_via_clock(*this),
	write_via_data(*this),
	m_maincpu(*this, CUDA_CPU_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cuda_device::device_start()
{
	write_reset.resolve_safe();
	write_linechange.resolve_safe();
	write_via_clock.resolve_safe();
	write_via_data.resolve_safe();

	m_timer = timer_alloc(0);
	m_prog_timer = timer_alloc(1);
	save_item(NAME(ddrs[0]));
	save_item(NAME(ddrs[1]));
	save_item(NAME(ddrs[2]));
	save_item(NAME(ports[0]));
	save_item(NAME(ports[1]));
	save_item(NAME(ports[2]));
	save_item(NAME(pll_ctrl));
	save_item(NAME(timer_ctrl));
	save_item(NAME(timer_counter));
	save_item(NAME(ripple_counter));
	save_item(NAME(onesec));
	save_item(NAME(treq));
	save_item(NAME(byteack));
	save_item(NAME(tip));
	save_item(NAME(via_data));
	save_item(NAME(via_clock));
	save_item(NAME(adb_in));
	save_item(NAME(reset_line));
	save_item(NAME(m_adb_dtime));
	save_item(NAME(pram_loaded));
	save_item(NAME(pram));
	save_item(NAME(disk_pram));

	uint8_t *rom = device().machine().root_device().memregion(device().subtag(CUDA_CPU_TAG))->base();

	if (rom)
	{
		memcpy(rom, rom+rom_offset, 0x1100);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cuda_device::device_reset()
{
	ddrs[0] = ddrs[1] = ddrs[2] = 0;
	ports[0] = ports[1] = ports[2] = 0;

	m_timer->adjust(attotime::never);
	m_prog_timer->adjust(attotime::never);

	cuda_controls_power = false;    // set to hard power control
	adb_in = true;  // line is pulled up to +5v, so nothing plugged in would read as "1"
	reset_line = 0;
	tip = 0;
	treq = 0;
	byteack = 0;
	via_data = 0;
	via_clock = 0;
	pll_ctrl = 0;
	timer_ctrl = 0;
	timer_counter = 32;
	last_adb_time = m_maincpu->total_cycles();
	onesec = 0;
	last_adb = 0;
}

void cuda_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == 0)
	{
		onesec |= 0x40;

		if (onesec & 0x10)
		{
			m_maincpu->set_input_line(M68HC05EG_INT_CPI, ASSERT_LINE);
		}
	}
	else
	{
		timer_ctrl |= 0x80;

		if (timer_ctrl & 0x20)
		{
			m_maincpu->set_input_line(M68HC05EG_INT_TIMER, ASSERT_LINE);
		}

		ripple_counter--;
		if (ripple_counter <= 0)
		{
			timer_ctrl |= 0x40;

			ripple_counter = timer_counter;

			if (timer_ctrl & 0x10)
			{
				m_maincpu->set_input_line(M68HC05EG_INT_TIMER, ASSERT_LINE);
			}
		}
	}
}

// the 6805 program clears PRAM on startup (on h/w it's always running once a battery is inserted)
// we deal with that by loading pram from disk to a secondary buffer and then slapping it into "live"
// once the cuda reboots the 68k
void cuda_device::nvram_default()
{
	memset(pram, 0, 0x100);
	memset(disk_pram, 0, 0x100);

	pram[0x1] = 0x10;
	pram[0x2] = 0x4f;
	pram[0x3] = 0x48;
	pram[0x8] = 0x13;
	pram[0x9] = 0x88;
	pram[0xb] = 0xcc;
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
	pram[0x4a] = 0x90;
	pram[0x4b] = 0xc7;
	pram[0x57] = 0x29;
	pram[0x58] = 0x80;
	pram[0x59] = 0x68;
	pram[0x5a] = 0x80;
	pram[0x5b] = 0x80;
	pram[0x6f] = 0x28;
	pram[0x77] = 0x01;
	pram[0x78] = 0xff;
	pram[0x79] = 0xff;
	pram[0x7a] = 0xff;
	pram[0x7b] = 0xdf;
	pram[0xb8] = 0x35;
	pram[0xb9] = 0x80;
	pram_loaded = false;
}

bool cuda_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	if (!file.read(disk_pram, 0x100, actual) && actual == 0x100)
	{
		pram_loaded = false;
		return true;
	}
	return false;
}


bool cuda_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(pram, 0x100, actual) && actual == 0x100;
}
