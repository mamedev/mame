// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

IREM M72 board

driver by Nicola Salmoria
protection information by Nao

                                   Year Board        Protected?
R-Type                             1987  M72             N
Battle Chopper / Mr. Heli          1987  M72             Y
Ninja Spirit                       1988  M72             Y
Image Fight                        1988  M72             Y
Legend of Hero Tonma               1989  M72             Y
X Multiply                         1989  M81             N
X Multiply                         1989  M72(1)          Y
Dragon Breed                       1989  M81             N
Dragon Breed                       1989  M72             Y
R-Type II                          1989  M82/M84(2)      N
Major Title                        1990  M84             N
Hammerin' Harry / Daiku no Gensan  1990  M82(3)          N
                  Daiku no Gensan  1990  M72(4)          Y
Pound for Pound                    1990  M85             N
Air Duel                           1990  M72?            Y
Cosmic Cop /                       1991  M84             N
  Gallop - Armed Police Unit       1991  M72             N
Ken-Go                             1991  ?           Encrypted

(1) different addressing PALs, so different memory map
(2) rtype2j has M84 written on the board, but it's the same hardware as rtype2
(3) multiple versions supported, running on different hardware
(4) normal M72 memory map, but IRQ vectors and sprite control as in X-Multiply


TODO:
- majtitle_gfx_ctrl_w is unknown, it seems to be used to disable rowscroll,
  and maybe other things

- Maybe there is a layer enable register, e.g. nspirit shows (for an instant)
  incomplete screens with bad colors when you start a game.

- A lot of unknown I/O writes from the sound CPU in Pound for Pound.

- the sprite chip triggers IRQ1 when it has finished copying the sprite RAM to its
  private buffer. This isn't implemented (all games have an empty IRQ1 handler).
  The cpu board also has support for IRQ3 and IRQ4, coming from the external
  connectors, but I don't think they are used by any game.

IRQ controller
--------------
The initialization consists of one write to port 0x40 and multiple writes
(2 or 3) to port 0x42. The first value written to 0x42 is the IRQ vector base.
Cosmic Cop and Ken-Go have a V35 CPU with its own IRQ controller built in.

Game      irqbase 0x40  0x42
----      ------- ----  ----------
rtype       0x20   17    20 0F
bchopper     "     "     "
nspirit      "     "     "
loht         "     "     "
rtype2       "     "     "
airduel      "     "     "
gallop       "     "     "
imgfight    0x20   17    20 0F 06
majtitle     "     "     "
poundfor    0x20   17    20 0F 0A
xmultipl    0x08   13    08 0F FA
dbreed       "     "     "
hharry       "     "     "
cosmccop    ---------------------
kengo       ---------------------


2008-08
Dip locations verified for the following games:
    - dbreed, hharry, loht (jpn), poundfor, rtype, rtype2 [manual]
    - airduel, gallop, imgfight [dip listing]
The other locations have been added assuming that the layout is the same
on all m72 boards. However, it would be nice to have them confirmed for
other supported games as well.


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/nec/nec.h"
#include "cpu/nec/v25.h"
#include "machine/irem_cpu.h"
#include "sound/2151intf.h"
#include "includes/iremipt.h"
#include "includes/m72.h"
#include "cpu/mcs51/mcs51.h"

#define MASTER_CLOCK        XTAL_32MHz
#define SOUND_CLOCK         XTAL_3_579545MHz



/***************************************************************************/

void m72_state::machine_start()
{
	m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m72_state::scanline_interrupt),this));
}

MACHINE_START_MEMBER(m72_state,kengo)
{
	m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m72_state::kengo_scanline_interrupt),this));
}

TIMER_CALLBACK_MEMBER(m72_state::synch_callback)
{
	//machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(8000000));
	machine().scheduler().boost_interleave(attotime::from_hz(MASTER_CLOCK/4/12), attotime::from_seconds(25));
}

void m72_state::machine_reset()
{
	m_irq_base = 0x20;
	m_mcu_sample_addr = 0;
	m_mcu_snd_cmd_latch = 0;

	m_scanline_timer->adjust(m_screen->time_until_pos(0));
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(m72_state::synch_callback),this));
}

MACHINE_RESET_MEMBER(m72_state,xmultipl)
{
	m72_state::machine_reset();
	m_irq_base = 0x08;
}

MACHINE_RESET_MEMBER(m72_state,kengo)
{
	m_scanline_timer->adjust(m_screen->time_until_pos(0));
}

TIMER_CALLBACK_MEMBER(m72_state::scanline_interrupt)
{
	int scanline = param;

	/* raster interrupt - visible area only? */
	if (scanline < 256 && scanline == m_raster_irq_position - 128)
	{
		m_screen->update_partial(scanline);
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_irq_base + 2);
	}

	/* VBLANK interrupt */
	else if (scanline == 256)
	{
		m_screen->update_partial(scanline);
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_irq_base + 0);
	}

	/* adjust for next scanline */
	if (++scanline >= m_screen->height())
		scanline = 0;
	m_scanline_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}

TIMER_CALLBACK_MEMBER(m72_state::kengo_scanline_interrupt)
{
	int scanline = param;

	/* raster interrupt - visible area only? */
	if (scanline < 256 && scanline == m_raster_irq_position - 128)
	{
		m_screen->update_partial(scanline);
		m_maincpu->set_input_line(NEC_INPUT_LINE_INTP2, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(NEC_INPUT_LINE_INTP2, CLEAR_LINE);

	/* VBLANK interrupt */
	if (scanline == 256)
	{
		m_screen->update_partial(scanline);
		m_maincpu->set_input_line(NEC_INPUT_LINE_INTP0, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(NEC_INPUT_LINE_INTP0, CLEAR_LINE);

	/* adjust for next scanline */
	if (++scanline >= m_screen->height())
		scanline = 0;
	m_scanline_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}

/***************************************************************************

Protection emulation

The protection device does

* provide startup code
* provide checksums
* feed samples to the sound cpu

***************************************************************************/

TIMER_CALLBACK_MEMBER(m72_state::delayed_ram16_w)
{
	UINT16 val = ((UINT32) param) & 0xffff;
	UINT16 offset = (((UINT32) param) >> 16) & 0xffff;
	UINT16 *ram = (UINT16 *)ptr;

	ram[offset] = val;
}


WRITE16_MEMBER(m72_state::main_mcu_sound_w)
{
	if (data & 0xfff0)
		logerror("sound_w: %04x %04x\n", mem_mask, data);

	if (ACCESSING_BITS_0_7)
	{
		m_mcu_snd_cmd_latch = data;
		m_mcu->set_input_line(1, ASSERT_LINE);
	}
}

WRITE16_MEMBER(m72_state::main_mcu_w)
{
	UINT16 val = m_protection_ram[offset];

	COMBINE_DATA(&val);

	/* 0x07fe is used for synchronization as well.
	 * This address however will not trigger an interrupt
	 */

	if (offset == 0x0fff/2 && ACCESSING_BITS_8_15)
	{
		m_protection_ram[offset] = val;
		m_mcu->set_input_line(0, ASSERT_LINE);
		/* Line driven, most likely by write line */
		//machine().scheduler().timer_set(m_mcu->cycles_to_attotime(2), FUNC(mcu_irq0_clear));
		//machine().scheduler().timer_set(m_mcu->cycles_to_attotime(0), FUNC(mcu_irq0_raise));
	}
	else
		machine().scheduler().synchronize( timer_expired_delegate(FUNC(m72_state::delayed_ram16_w),this), (offset<<16) | val, m_protection_ram);
}

WRITE8_MEMBER(m72_state::mcu_data_w)
{
	UINT16 val;
	if (offset&1) val = (m_protection_ram[offset/2] & 0x00ff) | (data << 8);
	else val = (m_protection_ram[offset/2] & 0xff00) | (data&0xff);

	machine().scheduler().synchronize( timer_expired_delegate(FUNC(m72_state::delayed_ram16_w),this), ((offset >>1 ) << 16) | val, m_protection_ram);
}

READ8_MEMBER(m72_state::mcu_data_r)
{
	UINT8 ret;

	if (offset == 0x0fff || offset == 0x0ffe)
	{
		m_mcu->set_input_line(0, CLEAR_LINE);
	}

	if (offset&1) ret = (m_protection_ram[offset/2] & 0xff00)>>8;
	else ret = (m_protection_ram[offset/2] & 0x00ff);

	return ret;
}

INTERRUPT_GEN_MEMBER(m72_state::mcu_int)
{
	//m_mcu_snd_cmd_latch |= 0x11; /* 0x10 is special as well - FIXME */
	m_mcu_snd_cmd_latch = 0x11;// | (machine.rand() & 1); /* 0x10 is special as well - FIXME */
	device.execute().set_input_line(1, ASSERT_LINE);
}

READ8_MEMBER(m72_state::mcu_sample_r)
{
	UINT8 sample;
	sample = memregion("samples")->base()[m_mcu_sample_addr++];
	return sample;
}

WRITE8_MEMBER(m72_state::mcu_ack_w)
{
	m_mcu->set_input_line(1, CLEAR_LINE);
	m_mcu_snd_cmd_latch = 0;
}

READ8_MEMBER(m72_state::mcu_snd_r)
{
	return m_mcu_snd_cmd_latch;
}

READ8_MEMBER(m72_state::mcu_port_r)
{
	logerror("port read: %02x\n", offset);
	return 0;
}

WRITE8_MEMBER(m72_state::mcu_port_w)
{
	if (offset == 1)
	{
		m_mcu_sample_latch = data;
		m_soundcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
	else
		logerror("port: %02x %02x\n", offset, data);

}

WRITE8_MEMBER(m72_state::mcu_low_w)
{
	m_mcu_sample_addr = (m_mcu_sample_addr & 0xffe000) | (data<<5);
	logerror("low: %02x %02x %08x\n", offset, data, m_mcu_sample_addr);
}

WRITE8_MEMBER(m72_state::mcu_high_w)
{
	m_mcu_sample_addr = (m_mcu_sample_addr & 0x1fff) | (data<<(8+5));
	logerror("high: %02x %02x %08x\n", offset, data, m_mcu_sample_addr);
}

READ8_MEMBER(m72_state::snd_cpu_sample_r)
{
	return m_mcu_sample_latch;
}

DRIVER_INIT_MEMBER(m72_state,m72_8751)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	address_space &io = m_maincpu->space(AS_IO);
	address_space &sndio = m_soundcpu->space(AS_IO);

	m_protection_ram = auto_alloc_array(machine(), UINT16, 0x10000/2);
	program.install_read_bank(0xb0000, 0xbffff, "bank1");
	program.install_write_handler(0xb0000, 0xb0fff, write16_delegate(FUNC(m72_state::main_mcu_w),this));
	membank("bank1")->configure_entry(0, m_protection_ram);

	save_pointer(NAME(m_protection_ram), 0x10000/2);
	save_item(NAME(m_mcu_sample_latch));
	save_item(NAME(m_mcu_sample_addr));
	save_item(NAME(m_mcu_snd_cmd_latch));

	//io.install_write_handler(0xc0, 0xc1, write16_delegate(FUNC(m72_state::loht_sample_trigger_w),this));
	io.install_write_handler(0xc0, 0xc1, write16_delegate(FUNC(m72_state::main_mcu_sound_w),this));

	/* sound cpu */
	sndio.install_write_handler(0x82, 0x82, 0xff, 0, write8_delegate(FUNC(dac_device::write_unsigned8),(dac_device*)m_dac));
	sndio.install_read_handler (0x84, 0x84, 0xff, 0, read8_delegate(FUNC(m72_state::snd_cpu_sample_r),this));

	/* lohtb2 */
#if 0
	/* running the mcu at twice the speed, the following
	 * timeouts have to be modified.
	 * At normal speed, the timing heavily depends on opcode
	 * prefetching on the V30.
	 */
	{
		UINT8 *rom=memregion("mcu")->base();

		rom[0x12d+5] += 1; printf(" 5: %d\n", rom[0x12d+5]);
		rom[0x12d+8] += 5;  printf(" 8: %d\n", rom[0x12d+8]);
		rom[0x12d+11] += 7; printf("11: %d\n", rom[0x12d+11]);
		rom[0x12d+14] += 9; printf("14: %d\n", rom[0x12d+14]);
		rom[0x12d+17] += 1; printf("17: %d\n", rom[0x12d+17]);
		rom[0x12d+20] += 10; printf("20: %d\n", rom[0x12d+20]);
		rom[0x12d+23] += 3; printf("23: %d\n", rom[0x12d+23]);
		rom[0x12d+26] += 2; printf("26: %d\n", rom[0x12d+26]);
		rom[0x12d+29] += 2; printf("29: %d\n", rom[0x12d+29]);
		rom[0x12d+32] += 16; printf("32: %d\n", rom[0x12d+32]);
	}
#endif
}


/***************************************************************************

Sample playback

In the later games, the sound CPU can program the start offset of the PCM
samples, but it seems the earlier games have them hardcoded somewhere (maybe
the protection MCU?).
So, here I provided some tables with the start offset precomputed.
They could be built automatically in most cases (00 marks the end of a
sample), but a couple of games (nspirit, loht) have holes in the numbering
so we would have to do them differently anyway.

Also, some games (dkgenm72, poundfor, airduel, gallop) have an empty NMI
handler, so the sample playback has to be handled entirely by external
hardware; we work around that by using (for all games, not just the ones
without a NMI handler) a NMI interrupt gen that mimics the behaviour of
the NMI handler in the other games.

***************************************************************************/

#if 0
int m72_state::find_sample(int num)
{
	UINT8 *rom = memregion("samples")->base();
	int len = memregion("samples")->bytes();
	int addr = 0;

	while (num--)
	{
		/* find end of sample */
		while (addr < len &&  rom[addr]) addr++;

		/* skip 0 filler between samples */
		while (addr < len && !rom[addr]) addr++;
	}

	return addr;
}
#endif

INTERRUPT_GEN_MEMBER(m72_state::fake_nmi)
{
	address_space &space = generic_space();
	int sample = m_audio->sample_r(space,0);
	if (sample)
		m_audio->sample_w(space,0,sample);
}


WRITE16_MEMBER(m72_state::bchopper_sample_trigger_w)
{
	static const int a[6] = { 0x0000, 0x0010, 0x2510, 0x6510, 0x8510, 0x9310 };
	if (ACCESSING_BITS_0_7 && (data & 0xff) < 6) m_audio->set_sample_start(a[data & 0xff]);
}

WRITE16_MEMBER(m72_state::nspirit_sample_trigger_w)
{
	static const int a[9] = { 0x0000, 0x0020, 0x2020, 0, 0x5720, 0, 0x7b60, 0x9b60, 0xc360 };
	if (ACCESSING_BITS_0_7 && (data & 0xff) < 9) m_audio->set_sample_start(a[data & 0xff]);
}

WRITE16_MEMBER(m72_state::imgfight_sample_trigger_w)
{
	static const int a[7] = { 0x0000, 0x0020, 0x44e0, 0x98a0, 0xc820, 0xf7a0, 0x108c0 };
	if (ACCESSING_BITS_0_7 && (data & 0xff) < 7) m_audio->set_sample_start(a[data & 0xff]);
}

WRITE16_MEMBER(m72_state::loht_sample_trigger_w)
{
	static const int a[7] = { 0x0000, 0x0020, 0, 0x2c40, 0x4320, 0x7120, 0xb200 };
	if (ACCESSING_BITS_0_7 && (data & 0xff) < 7) m_audio->set_sample_start(a[data & 0xff]);
}



WRITE16_MEMBER(m72_state::dbreedm72_sample_trigger_w)
{
	static const int a[9] = { 0x00000, 0x00020, 0x02c40, 0x08160, 0x0c8c0, 0x0ffe0, 0x13000, 0x15820, 0x15f40 };
	if (ACCESSING_BITS_0_7 && (data & 0xff) < 9) m_audio->set_sample_start(a[data & 0xff]);
}

WRITE16_MEMBER(m72_state::airduel_sample_trigger_w)
{
	static const int a[16] = {
		0x00000, 0x00020, 0x03ec0, 0x05640, 0x06dc0, 0x083a0, 0x0c000, 0x0eb60,
		0x112e0, 0x13dc0, 0x16520, 0x16d60, 0x18ae0, 0x1a5a0, 0x1bf00, 0x1c340 };
	if (ACCESSING_BITS_0_7 && (data & 0xff) < 16) m_audio->set_sample_start(a[data & 0xff]);
}

WRITE16_MEMBER(m72_state::dkgenm72_sample_trigger_w)
{
	static const int a[28] = {
		0x00000, 0x00020, 0x01800, 0x02da0, 0x03be0, 0x05ae0, 0x06100, 0x06de0,
		0x07260, 0x07a60, 0x08720, 0x0a5c0, 0x0c3c0, 0x0c7a0, 0x0e140, 0x0fb00,
		0x10fa0, 0x10fc0, 0x10fe0, 0x11f40, 0x12b20, 0x130a0, 0x13c60, 0x14740,
		0x153c0, 0x197e0, 0x1af40, 0x1c080 };

	if (ACCESSING_BITS_0_7 && (data & 0xff) < 28) m_audio->set_sample_start(a[data & 0xff]);
}

WRITE16_MEMBER(m72_state::gallop_sample_trigger_w)
{
	static const int a[31] = {
		0x00000, 0x00020, 0x00040, 0x01360, 0x02580, 0x04f20, 0x06240, 0x076e0,
		0x08660, 0x092a0, 0x09ba0, 0x0a560, 0x0cee0, 0x0de20, 0x0e620, 0x0f1c0,
		0x10200, 0x10220, 0x10240, 0x11380, 0x12760, 0x12780, 0x127a0, 0x13c40,
		0x140a0, 0x16760, 0x17e40, 0x18ee0, 0x19f60, 0x1bbc0, 0x1cee0 };

	if (ACCESSING_BITS_0_7 && (data & 0xff) < 31) m_audio->set_sample_start(a[data & 0xff]);
}



/***************************************************************************

Protection simulation

Most of the games running on this board have an 8751 protection mcu.
It is not known how it works in detail, however it's pretty clear that it
shares RAM at b0000-b0fff.
On startup, the game writes a pattern to the whole RAM, then reads it back
expecting it to be INVERTED. If it isn't, it reports a RAM error.
If the RAM passes the test, the program increments every byte up to b0ffb,
then calls a subroutine at b0000, which has to be provided by the mcu.
It seems that this routine is not supposed to RET, but instead it should
jump directly to the game entry point. The routine should also write some
bytes here and there in RAM (different in every game); those bytes are
checked at various points during the game, causing a crash if they aren't
right.
Note that the program keeps incrementing b0ffe while the game is running,
maybe this is done to keep the 8751 alive. We don't bother with that.

Finally, to do the ROM test the program asks the mcu to provide the correct
values. This is done only in service, so doesn't seem to be much of a
protection. Here we have provided the correct crcs for the available dumps,
of course there is no guarantee that they are actually good.

All the protection routines below are entirely made up. They get the games
running, but they have not been derived from the real 8751 code.

***************************************************************************/

#define CODE_LEN 96
#define CRC_LEN 18

/* Battle Chopper / Mr. Heli */
static const UINT8 bchopper_code[CODE_LEN] =
{
	0x68,0x00,0xa0,             // push 0a000h
	0x1f,                       // pop ds
	0xc6,0x06,0x38,0x38,0x53,   // mov [3838h], byte 053h
	0xc6,0x06,0x3a,0x38,0x41,   // mov [383ah], byte 041h
	0xc6,0x06,0x3c,0x38,0x4d,   // mov [383ch], byte 04dh
	0xc6,0x06,0x3e,0x38,0x4f,   // mov [383eh], byte 04fh
	0xc6,0x06,0x40,0x38,0x54,   // mov [3840h], byte 054h
	0xc6,0x06,0x42,0x38,0x4f,   // mov [3842h], byte 04fh
	0x68,0x00,0xb0,             // push 0b000h
	0x1f,                       // pop ds
	0xc6,0x06,0x00,0x09,0x49^0xff,  // mov [0900h], byte 049h
	0xc6,0x06,0x00,0x0a,0x49^0xff,  // mov [0a00h], byte 049h
	0xc6,0x06,0x00,0x0b,0x49^0xff,  // mov [0b00h], byte 049h
	0xc6,0x06,0x00,0x00,0xcb^0xff,  // mov [0000h], byte 0cbh ; retf : bypass protection check during the game
	0x68,0x00,0xd0,             // push 0d000h
	0x1f,                       // pop ds
	0xea,0x68,0x01,0x40,0x00    // jmp  0040:$0168
};
static const UINT8 bchopper_crc[CRC_LEN] =  {   0x1a,0x12,0x5c,0x08, 0x84,0xb6,0x73,0xd1,
												0x54,0x91,0x94,0xeb, 0x00,0x00 };

/* Ninja Spirit */
static const UINT8 nspirit_code[CODE_LEN] =
{
	0x68,0x00,0xa0,             // push 0a000h
	0x1f,                       // pop ds
	0xc6,0x06,0x38,0x38,0x4e,   // mov [3838h], byte 04eh
	0xc6,0x06,0x3a,0x38,0x49,   // mov [383ah], byte 049h
	0xc6,0x06,0x3c,0x38,0x4e,   // mov [383ch], byte 04eh
	0xc6,0x06,0x3e,0x38,0x44,   // mov [383eh], byte 044h
	0xc6,0x06,0x40,0x38,0x4f,   // mov [3840h], byte 04fh
	0xc6,0x06,0x42,0x38,0x55,   // mov [3842h], byte 055h
	0x68,0x00,0xb0,             // push 0b000h
	0x1f,                       // pop ds
	0xc6,0x06,0x00,0x09,0x49^0xff,  // mov [0900h], byte 049h
	0xc6,0x06,0x00,0x0a,0x49^0xff,  // mov [0a00h], byte 049h
	0xc6,0x06,0x00,0x0b,0x49^0xff,  // mov [0b00h], byte 049h
	0x68,0x00,0xd0,             // push 0d000h
	0x1f,                       // pop ds
	0xea,0x00,0x00,0x40,0x00    // jmp  0040:$0000
};
static const UINT8 nspirit_crc[CRC_LEN] =   {   0xfe,0x94,0x6e,0x4e, 0xc8,0x33,0xa7,0x2d,
												0xf2,0xa3,0xf9,0xe1, 0xa9,0x6c,0x02,0x95, 0x00,0x00 };
/* Image Fight */
static const UINT8 imgfight_code[CODE_LEN] =
{
	0x68,0x00,0xa0,             // push 0a000h
	0x1f,                       // pop ds
	0xc6,0x06,0x38,0x38,0x50,   // mov [3838h], byte 050h
	0xc6,0x06,0x3a,0x38,0x49,   // mov [383ah], byte 049h
	0xc6,0x06,0x3c,0x38,0x43,   // mov [383ch], byte 043h
	0xc6,0x06,0x3e,0x38,0x4b,   // mov [383eh], byte 04bh
	0xc6,0x06,0x40,0x38,0x45,   // mov [3840h], byte 045h
	0xc6,0x06,0x42,0x38,0x54,   // mov [3842h], byte 054h
	0x68,0x00,0xb0,             // push 0b000h
	0x1f,                       // pop ds
	0xc6,0x06,0x00,0x09,0x49^0xff,  // mov [0900h], byte 049h
	0xc6,0x06,0x00,0x0a,0x49^0xff,  // mov [0a00h], byte 049h
	0xc6,0x06,0x00,0x0b,0x49^0xff,  // mov [0b00h], byte 049h
	0xc6,0x06,0x20,0x09,0x49^0xff,  // mov [0920h], byte 049h
	0xc6,0x06,0x21,0x09,0x4d^0xff,  // mov [0921h], byte 04dh
	0xc6,0x06,0x22,0x09,0x41^0xff,  // mov [0922h], byte 041h
	0xc6,0x06,0x23,0x09,0x47^0xff,  // mov [0923h], byte 047h
	0x68,0x00,0xd0,             // push 0d000h
	0x1f,                       // pop ds
	0xea,0x00,0x00,0x40,0x00    // jmp  0040:$0000
};

// these are for the japan set where we have the mcu anyway...
static const UINT8 imgfightj_crc[CRC_LEN] =  {  0x7e,0xcc,0xec,0x03, 0x04,0x33,0xb6,0xc5,
												0xbf,0x37,0x92,0x94, 0x00,0x00 };

/* Legend of Hero Tonma */
static const UINT8 loht_code[CODE_LEN] =
{
	0x68,0x00,0xa0,             // push 0a000h
	0x1f,                       // pop ds
	0xc6,0x06,0x3c,0x38,0x47,   // mov [383ch], byte 047h
	0xc6,0x06,0x3d,0x38,0x47,   // mov [383dh], byte 047h
	0xc6,0x06,0x42,0x38,0x44,   // mov [3842h], byte 044h
	0xc6,0x06,0x43,0x38,0x44,   // mov [3843h], byte 044h
	0x68,0x00,0xb0,             // push 0b000h
	0x1f,                       // pop ds
	0xc6,0x06,0x00,0x09,0x49^0xff,  // mov [0900h], byte 049h
	0xc6,0x06,0x00,0x0a,0x49^0xff,  // mov [0a00h], byte 049h
	0xc6,0x06,0x00,0x0b,0x49^0xff,  // mov [0b00h], byte 049h


	0xea,0x5d,0x01,0x40,0x00    // jmp  0040:$015d

};
static const UINT8 loht_crc[CRC_LEN] =    { 0x39,0x00,0x82,0xae, 0x2c,0x9d,0x4b,0x73,
												0xfb,0xac,0xd4,0x6d, 0x6d,0x5b,0x77,0xc0, 0x00,0x00 };



/* Dragon Breed */
static const UINT8 dbreedm72_code[CODE_LEN] =
{
	0xea,0x6c,0x00,0x00,0x00    // jmp  0000:$006c
};
static const UINT8 dbreedm72_crc[CRC_LEN] =   { 0xa4,0x96,0x5f,0xc0, 0xab,0x49,0x9f,0x19,
												0x84,0xe6,0xd6,0xca, 0x00,0x00 };

/* Air Duel */
static const UINT8 airduel_code[CODE_LEN] =
{
	0x68,0x00,0xd0,             // push 0d000h
	0x1f,                       // pop ds
	// the game checks for
	// "This game can only be played in Japan..." message in the video text buffer
	// the message is nowhere to be found in the ROMs, so has to be displayed by the mcu
	0xc6,0x06,0xc0,0x1c,0x57,   // mov [1cc0h], byte 057h
	0xea,0x69,0x0b,0x00,0x00    // jmp  0000:$0b69
};
static const UINT8 airduel_crc[CRC_LEN] =     { 0x72,0x9c,0xca,0x85, 0xc9,0x12,0xcc,0xea,
												0x00,0x00 };

/* Daiku no Gensan */
static const UINT8 dkgenm72_code[CODE_LEN] =
{
	0xea,0x3d,0x00,0x00,0x10    // jmp  1000:$003d
};
static const UINT8 dkgenm72_crc[CRC_LEN] =  {   0xc8,0xb4,0xdc,0xf8, 0xd3,0xba,0x48,0xed,
												0x79,0x08,0x1c,0xb3, 0x00,0x00 };



void m72_state::copy_le(UINT16 *dest, const UINT8 *src, UINT8 bytes)
{
	int i;

	for (i = 0; i < bytes; i += 2)
		dest[i/2] = src[i+0] | (src[i+1] << 8);
}

READ16_MEMBER(m72_state::protection_r)
{
	if (ACCESSING_BITS_8_15)
		copy_le(m_protection_ram,m_protection_code,CODE_LEN);
	return m_protection_ram[0xffa/2+offset];
}

WRITE16_MEMBER(m72_state::protection_w)
{
	data ^= 0xffff;
	COMBINE_DATA(&m_protection_ram[offset]);
	data ^= 0xffff;

	if (offset == 0x0fff/2 && ACCESSING_BITS_8_15 && (data >> 8) == 0)
		copy_le(&m_protection_ram[0x0fe0],m_protection_crc,CRC_LEN);
}

void m72_state::install_protection_handler(const UINT8 *code,const UINT8 *crc)
{
	m_protection_ram = auto_alloc_array(machine(), UINT16, 0x1000/2);
	m_protection_code = code;
	m_protection_crc =  crc;
	m_maincpu->space(AS_PROGRAM).install_read_bank(0xb0000, 0xb0fff, "bank1");
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xb0ffa, 0xb0ffb, read16_delegate(FUNC(m72_state::protection_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xb0000, 0xb0fff, write16_delegate(FUNC(m72_state::protection_w),this));
	membank("bank1")->configure_entry(0, m_protection_ram);

	save_pointer(NAME(m_protection_ram), 0x1000/2);
}

DRIVER_INIT_MEMBER(m72_state,bchopper)
{
	install_protection_handler(bchopper_code,bchopper_crc);
	m_maincpu->space(AS_IO).install_write_handler(0xc0, 0xc1, write16_delegate(FUNC(m72_state::bchopper_sample_trigger_w),this));
}


DRIVER_INIT_MEMBER(m72_state,nspirit)
{
	install_protection_handler(nspirit_code,nspirit_crc);
	m_maincpu->space(AS_IO).install_write_handler(0xc0, 0xc1, write16_delegate(FUNC(m72_state::nspirit_sample_trigger_w),this));
}

DRIVER_INIT_MEMBER(m72_state,imgfight)
{
	install_protection_handler(imgfight_code,imgfightj_crc);
	m_maincpu->space(AS_IO).install_write_handler(0xc0, 0xc1, write16_delegate(FUNC(m72_state::imgfight_sample_trigger_w),this));
}

DRIVER_INIT_MEMBER(m72_state,loht)
{
	install_protection_handler(loht_code,loht_crc);

	m_maincpu->space(AS_IO).install_write_handler(0xc0, 0xc1, write16_delegate(FUNC(m72_state::loht_sample_trigger_w),this));

	/* since we skip the startup tests, clear video RAM to prevent garbage on title screen */
	memset(m_videoram2,0,0x4000);
}


DRIVER_INIT_MEMBER(m72_state,dbreedm72)
{
	install_protection_handler(dbreedm72_code,dbreedm72_crc);
	m_maincpu->space(AS_IO).install_write_handler(0xc0, 0xc1, write16_delegate(FUNC(m72_state::dbreedm72_sample_trigger_w),this));
}

DRIVER_INIT_MEMBER(m72_state,airduel)
{
	install_protection_handler(airduel_code,airduel_crc);
	m_maincpu->space(AS_IO).install_write_handler(0xc0, 0xc1, write16_delegate(FUNC(m72_state::airduel_sample_trigger_w),this));
}

DRIVER_INIT_MEMBER(m72_state,dkgenm72)
{
	install_protection_handler(dkgenm72_code,dkgenm72_crc);
	m_maincpu->space(AS_IO).install_write_handler(0xc0, 0xc1, write16_delegate(FUNC(m72_state::dkgenm72_sample_trigger_w),this));
}

DRIVER_INIT_MEMBER(m72_state,gallop)
{
	m_maincpu->space(AS_IO).install_write_handler(0xc0, 0xc1, write16_delegate(FUNC(m72_state::gallop_sample_trigger_w),this));
}






READ16_MEMBER(m72_state::soundram_r)
{
	return m_soundram[offset * 2 + 0] | (m_soundram[offset * 2 + 1] << 8);
}

WRITE16_MEMBER(m72_state::soundram_w)
{
	if (ACCESSING_BITS_0_7)
		m_soundram[offset * 2 + 0] = data;
	if (ACCESSING_BITS_8_15)
		m_soundram[offset * 2 + 1] = data >> 8;
}


READ16_MEMBER(m72_state::poundfor_trackball_r)
{
	static const char *const axisnames[] = { "TRACK0_X", "TRACK0_Y", "TRACK1_X", "TRACK1_Y" };

	if (offset == 0)
	{
		int i,curr;

		for (i = 0;i < 4;i++)
		{
			curr = ioport(axisnames[i])->read();
			m_diff[i] = (curr - m_prev[i]);
			m_prev[i] = curr;
		}
	}

	switch (offset)
	{
		default:
		case 0:
			return (m_diff[0] & 0xff) | ((m_diff[2] & 0xff) << 8);
		case 1:
			return ((m_diff[0] >> 8) & 0x1f) | (m_diff[2] & 0x1f00) | (ioport("IN0")->read() & 0xe0e0);
		case 2:
			return (m_diff[1] & 0xff) | ((m_diff[3] & 0xff) << 8);
		case 3:
			return ((m_diff[1] >> 8) & 0x1f) | (m_diff[3] & 0x1f00);
	}
}


#define CPU1_MEMORY(NAME,ROMSIZE,WORKRAM)                               \
static ADDRESS_MAP_START( NAME##_map, AS_PROGRAM, 16 , m72_state )      \
	AM_RANGE(0x00000, ROMSIZE-1) AM_ROM                                 \
	AM_RANGE(WORKRAM, WORKRAM+0x3fff) AM_RAM    /* work RAM */          \
	AM_RANGE(0xc0000, 0xc03ff) AM_RAM AM_SHARE("spriteram") \
	AM_RANGE(0xc8000, 0xc8bff) AM_READWRITE(palette1_r, palette1_w) AM_SHARE("paletteram")          \
	AM_RANGE(0xcc000, 0xccbff) AM_READWRITE(palette2_r, palette2_w) AM_SHARE("paletteram2")     \
	AM_RANGE(0xd0000, 0xd3fff) AM_RAM_WRITE(videoram1_w) AM_SHARE("videoram1")      \
	AM_RANGE(0xd8000, 0xdbfff) AM_RAM_WRITE(videoram2_w) AM_SHARE("videoram2")      \
	AM_RANGE(0xe0000, 0xeffff) AM_READWRITE(soundram_r, soundram_w)                         \
	AM_RANGE(0xffff0, 0xfffff) AM_ROM                                   \
ADDRESS_MAP_END


/*                        ROMSIZE  WORKRAM */
CPU1_MEMORY( m72,         0x80000, 0xa0000 )
CPU1_MEMORY( rtype,       0x40000, 0x40000 )
CPU1_MEMORY( xmultiplm72, 0x80000, 0x80000 )
CPU1_MEMORY( dbreedm72,   0x80000, 0x90000 )

static ADDRESS_MAP_START( xmultipl_map, AS_PROGRAM, 16, m72_state )
	AM_RANGE(0x00000, 0x7ffff) AM_ROM
	AM_RANGE(0x9c000, 0x9ffff) AM_RAM   /* work RAM */
	AM_RANGE(0xb0ffe, 0xb0fff) AM_WRITEONLY /* leftover from protection?? */
	AM_RANGE(0xc0000, 0xc03ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc8000, 0xc8bff) AM_READWRITE(palette1_r, palette1_w) AM_SHARE("paletteram")
	AM_RANGE(0xcc000, 0xccbff) AM_READWRITE(palette2_r, palette2_w) AM_SHARE("paletteram2")
	AM_RANGE(0xd0000, 0xd3fff) AM_RAM_WRITE(videoram1_w) AM_SHARE("videoram1")
	AM_RANGE(0xd8000, 0xdbfff) AM_RAM_WRITE(videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xffff0, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dbreed_map, AS_PROGRAM, 16, m72_state )
	AM_RANGE(0x00000, 0x7ffff) AM_ROM
	AM_RANGE(0x88000, 0x8bfff) AM_RAM   /* work RAM */
	AM_RANGE(0xb0ffe, 0xb0fff) AM_WRITEONLY /* leftover from protection?? */
	AM_RANGE(0xc0000, 0xc03ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc8000, 0xc8bff) AM_READWRITE(palette1_r, palette1_w) AM_SHARE("paletteram")
	AM_RANGE(0xcc000, 0xccbff) AM_READWRITE(palette2_r, palette2_w) AM_SHARE("paletteram2")
	AM_RANGE(0xd0000, 0xd3fff) AM_RAM_WRITE(videoram1_w) AM_SHARE("videoram1")
	AM_RANGE(0xd8000, 0xdbfff) AM_RAM_WRITE(videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xffff0, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rtype2_map, AS_PROGRAM, 16, m72_state )
	AM_RANGE(0x00000, 0x7ffff) AM_ROM
	AM_RANGE(0xb0000, 0xb0001) AM_WRITE(irq_line_w)
	AM_RANGE(0xbc000, 0xbc001) AM_WRITE(dmaon_w)
	AM_RANGE(0xc0000, 0xc03ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc8000, 0xc8bff) AM_READWRITE(palette1_r, palette1_w) AM_SHARE("paletteram")
	AM_RANGE(0xd0000, 0xd3fff) AM_RAM_WRITE(videoram1_w) AM_SHARE("videoram1")
	AM_RANGE(0xd4000, 0xd7fff) AM_RAM_WRITE(videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xd8000, 0xd8bff) AM_READWRITE(palette2_r, palette2_w) AM_SHARE("paletteram2")
	AM_RANGE(0xe0000, 0xe3fff) AM_RAM   /* work RAM */
	AM_RANGE(0xffff0, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( majtitle_map, AS_PROGRAM, 16, m72_state )
	AM_RANGE(0x00000, 0x7ffff) AM_ROM
	AM_RANGE(0xa0000, 0xa03ff) AM_RAM AM_SHARE("majtitle_rowscr")
	AM_RANGE(0xa4000, 0xa4bff) AM_READWRITE(palette2_r, palette2_w) AM_SHARE("paletteram2")
	AM_RANGE(0xac000, 0xaffff) AM_RAM_WRITE(videoram1_w) AM_SHARE("videoram1")
	AM_RANGE(0xb0000, 0xbffff) AM_RAM_WRITE(videoram2_w) AM_SHARE("videoram2")  /* larger than the other games */
	AM_RANGE(0xc0000, 0xc03ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc8000, 0xc83ff) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0xcc000, 0xccbff) AM_READWRITE(palette1_r, palette1_w) AM_SHARE("paletteram")
	AM_RANGE(0xd0000, 0xd3fff) AM_RAM   /* work RAM */
	AM_RANGE(0xe0000, 0xe0001) AM_WRITE(irq_line_w)
	AM_RANGE(0xe4000, 0xe4001) AM_WRITEONLY /* playfield enable? 1 during screen transitions, 0 otherwise */
	AM_RANGE(0xec000, 0xec001) AM_WRITE(dmaon_w)
	AM_RANGE(0xffff0, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hharry_map, AS_PROGRAM, 16, m72_state )
	AM_RANGE(0x00000, 0x7ffff) AM_ROM
	AM_RANGE(0xa0000, 0xa3fff) AM_RAM   /* work RAM */
	AM_RANGE(0xb0ffe, 0xb0fff) AM_WRITEONLY /* leftover from protection?? */
	AM_RANGE(0xc0000, 0xc03ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc8000, 0xc8bff) AM_READWRITE(palette1_r, palette1_w) AM_SHARE("paletteram")
	AM_RANGE(0xcc000, 0xccbff) AM_READWRITE(palette2_r, palette2_w) AM_SHARE("paletteram2")
	AM_RANGE(0xd0000, 0xd3fff) AM_RAM_WRITE(videoram1_w) AM_SHARE("videoram1")
	AM_RANGE(0xd8000, 0xdbfff) AM_RAM_WRITE(videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xffff0, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hharryu_map, AS_PROGRAM, 16, m72_state )
	AM_RANGE(0x00000, 0x7ffff) AM_ROM
	AM_RANGE(0xa0000, 0xa0bff) AM_READWRITE(palette1_r, palette1_w) AM_SHARE("paletteram")
	AM_RANGE(0xa8000, 0xa8bff) AM_READWRITE(palette2_r, palette2_w) AM_SHARE("paletteram2")
	AM_RANGE(0xb0000, 0xb0001) AM_WRITE(irq_line_w)
	AM_RANGE(0xbc000, 0xbc001) AM_WRITE(dmaon_w)
	AM_RANGE(0xb0ffe, 0xb0fff) AM_WRITEONLY /* leftover from protection?? */
	AM_RANGE(0xc0000, 0xc03ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd0000, 0xd3fff) AM_RAM_WRITE(videoram1_w) AM_SHARE("videoram1")
	AM_RANGE(0xd4000, 0xd7fff) AM_RAM_WRITE(videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xe0000, 0xe3fff) AM_RAM   /* work RAM */
	AM_RANGE(0xffff0, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kengo_map, AS_PROGRAM, 16, m72_state )
	AM_RANGE(0x00000, 0x7ffff) AM_ROM
	AM_RANGE(0xa0000, 0xa0bff) AM_READWRITE(palette1_r, palette1_w) AM_SHARE("paletteram")
	AM_RANGE(0xa8000, 0xa8bff) AM_READWRITE(palette2_r, palette2_w) AM_SHARE("paletteram2")
	AM_RANGE(0xb0000, 0xb0001) AM_WRITE(irq_line_w)
	AM_RANGE(0xb4000, 0xb4001) AM_WRITENOP  /* ??? */
	AM_RANGE(0xbc000, 0xbc001) AM_WRITE(dmaon_w)
	AM_RANGE(0xc0000, 0xc03ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x80000, 0x83fff) AM_RAM_WRITE(videoram1_w) AM_SHARE("videoram1")
	AM_RANGE(0x84000, 0x87fff) AM_RAM_WRITE(videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xe0000, 0xe3fff) AM_RAM   /* work RAM */
	AM_RANGE(0xffff0, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( m72_portmap, AS_IO, 16, m72_state )
	AM_RANGE(0x00, 0x01) AM_READ_PORT("IN0")
	AM_RANGE(0x02, 0x03) AM_READ_PORT("IN1")
	AM_RANGE(0x04, 0x05) AM_READ_PORT("DSW")
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("m72", m72_audio_device, sound_command_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(port02_w) /* coin counters, reset sound cpu, other stuff? */
	AM_RANGE(0x04, 0x05) AM_WRITE(dmaon_w)
	AM_RANGE(0x06, 0x07) AM_WRITE(irq_line_w)
	//AM_RANGE(0x40, 0x43) AM_WRITENOP /* Interrupt controller, only written to at bootup */
	AM_RANGE(0x80, 0x81) AM_WRITE(scrolly1_w)
	AM_RANGE(0x82, 0x83) AM_WRITE(scrollx1_w)
	AM_RANGE(0x84, 0x85) AM_WRITE(scrolly2_w)
	AM_RANGE(0x86, 0x87) AM_WRITE(scrollx2_w)
/*  { 0xc0, 0xc0      trigger sample, filled by init_ function */
ADDRESS_MAP_END

static ADDRESS_MAP_START( rtype2_portmap, AS_IO, 16, m72_state )
	AM_RANGE(0x00, 0x01) AM_READ_PORT("IN0")
	AM_RANGE(0x02, 0x03) AM_READ_PORT("IN1")
	AM_RANGE(0x04, 0x05) AM_READ_PORT("DSW")
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("m72", m72_audio_device, sound_command_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(rtype2_port02_w)
	AM_RANGE(0x40, 0x43) AM_WRITENOP /* Interrupt controller, only written to at bootup */
	AM_RANGE(0x80, 0x81) AM_WRITE(scrolly1_w)
	AM_RANGE(0x82, 0x83) AM_WRITE(scrollx1_w)
	AM_RANGE(0x84, 0x85) AM_WRITE(scrolly2_w)
	AM_RANGE(0x86, 0x87) AM_WRITE(scrollx2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( poundfor_portmap, AS_IO, 16, m72_state )
	AM_RANGE(0x02, 0x03) AM_READ_PORT("IN1")
	AM_RANGE(0x04, 0x05) AM_READ_PORT("DSW")
	AM_RANGE(0x08, 0x0f) AM_READ(poundfor_trackball_r)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("m72", m72_audio_device, sound_command_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(rtype2_port02_w)
	AM_RANGE(0x40, 0x43) AM_WRITENOP /* Interrupt controller, only written to at bootup */
	AM_RANGE(0x80, 0x81) AM_WRITE(scrolly1_w)
	AM_RANGE(0x82, 0x83) AM_WRITE(scrollx1_w)
	AM_RANGE(0x84, 0x85) AM_WRITE(scrolly2_w)
	AM_RANGE(0x86, 0x87) AM_WRITE(scrollx2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( majtitle_portmap, AS_IO, 16, m72_state )
	AM_RANGE(0x00, 0x01) AM_READ_PORT("IN0")
	AM_RANGE(0x02, 0x03) AM_READ_PORT("IN1")
	AM_RANGE(0x04, 0x05) AM_READ_PORT("DSW")
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("m72", m72_audio_device, sound_command_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(rtype2_port02_w)
	AM_RANGE(0x40, 0x43) AM_WRITENOP /* Interrupt controller, only written to at bootup */
	AM_RANGE(0x80, 0x81) AM_WRITE(scrolly1_w)
	AM_RANGE(0x82, 0x83) AM_WRITE(scrollx1_w)
	AM_RANGE(0x84, 0x85) AM_WRITE(scrolly2_w)
	AM_RANGE(0x86, 0x87) AM_WRITE(scrollx2_w)
	AM_RANGE(0x8e, 0x8f) AM_WRITE(majtitle_gfx_ctrl_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hharry_portmap, AS_IO, 16, m72_state )
	AM_RANGE(0x00, 0x01) AM_READ_PORT("IN0")
	AM_RANGE(0x02, 0x03) AM_READ_PORT("IN1")
	AM_RANGE(0x04, 0x05) AM_READ_PORT("DSW")
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("m72", m72_audio_device, sound_command_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(rtype2_port02_w)  /* coin counters, reset sound cpu, other stuff? */
	AM_RANGE(0x04, 0x05) AM_WRITE(dmaon_w)
	AM_RANGE(0x06, 0x07) AM_WRITE(irq_line_w)
	AM_RANGE(0x40, 0x43) AM_WRITENOP /* Interrupt controller, only written to at bootup */
	AM_RANGE(0x80, 0x81) AM_WRITE(scrolly1_w)
	AM_RANGE(0x82, 0x83) AM_WRITE(scrollx1_w)
	AM_RANGE(0x84, 0x85) AM_WRITE(scrolly2_w)
	AM_RANGE(0x86, 0x87) AM_WRITE(scrollx2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kengo_portmap, AS_IO, 16, m72_state )
	AM_RANGE(0x00, 0x01) AM_READ_PORT("IN0")
	AM_RANGE(0x02, 0x03) AM_READ_PORT("IN1")
	AM_RANGE(0x04, 0x05) AM_READ_PORT("DSW")
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("m72", m72_audio_device, sound_command_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(rtype2_port02_w)
	AM_RANGE(0x80, 0x81) AM_WRITE(scrolly1_w)
	AM_RANGE(0x82, 0x83) AM_WRITE(scrollx1_w)
	AM_RANGE(0x84, 0x85) AM_WRITE(scrolly2_w)
	AM_RANGE(0x86, 0x87) AM_WRITE(scrollx2_w)
//  AM_RANGE(0x8c, 0x8f) AM_WRITENOP    /* ??? */
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_ram_map, AS_PROGRAM, 8, m72_state )
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_SHARE("soundram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_rom_map, AS_PROGRAM, 8, m72_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rtype_sound_portmap, AS_IO, 8, m72_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x02, 0x02) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x06, 0x06) AM_DEVWRITE("m72", m72_audio_device, sound_irq_ack_w)
	AM_RANGE(0x84, 0x84) AM_DEVREAD("m72", m72_audio_device, sample_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, m72_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x02, 0x02) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x06, 0x06) AM_DEVWRITE("m72", m72_audio_device, sound_irq_ack_w)
	AM_RANGE(0x82, 0x82) AM_DEVWRITE("m72", m72_audio_device, sample_w)
	AM_RANGE(0x84, 0x84) AM_DEVREAD("m72", m72_audio_device, sample_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rtype2_sound_portmap, AS_IO, 8, m72_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x80, 0x80) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("m72", m72_audio_device, rtype2_sample_addr_w)
	AM_RANGE(0x82, 0x82) AM_DEVWRITE("m72", m72_audio_device, sample_w)
	AM_RANGE(0x83, 0x83) AM_DEVWRITE("m72", m72_audio_device, sound_irq_ack_w)
	AM_RANGE(0x84, 0x84) AM_DEVREAD("m72", m72_audio_device, sample_r)
//  AM_RANGE(0x87, 0x87) AM_WRITENOP    /* ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( poundfor_sound_portmap, AS_IO, 8, m72_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x13) AM_DEVWRITE("m72", m72_audio_device, poundfor_sample_addr_w)
	AM_RANGE(0x40, 0x41) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x42, 0x42) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x42, 0x42) AM_DEVWRITE("m72", m72_audio_device, sound_irq_ack_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_io_map, AS_IO, 8, m72_state )
	/* External access */
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(mcu_sample_r, mcu_low_w)
	AM_RANGE(0x0001, 0x0001) AM_WRITE(mcu_high_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(mcu_snd_r, mcu_ack_w)
	/* shared at b0000 - b0fff on the main cpu */
	AM_RANGE(0xc000, 0xcfff) AM_READWRITE(mcu_data_r,mcu_data_w )

	/* Ports */
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READWRITE(mcu_port_r, mcu_port_w)
ADDRESS_MAP_END

#define COIN_MODE_1 \
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coinage ) ) PORT_CONDITION("DSW", 0x0400, NOTEQUALS, 0x0000) PORT_DIPLOCATION("SW1:5,6,7,8") \
	PORT_DIPSETTING(      0x00a0, DEF_STR( 6C_1C ) ) \
	PORT_DIPSETTING(      0x00b0, DEF_STR( 5C_1C ) ) \
	PORT_DIPSETTING(      0x00c0, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(      0x00d0, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( 8C_3C ) ) \
	PORT_DIPSETTING(      0x00e0, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( 5C_3C ) ) \
	PORT_DIPSETTING(      0x0030, DEF_STR( 3C_2C ) ) \
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(      0x0090, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_6C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )

#define COIN_MODE_2_A \
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_CONDITION("DSW", 0x0400, EQUALS, 0x0000) PORT_DIPLOCATION("SW1:5,6") \
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) ) \
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_CONDITION("DSW", 0x0400, EQUALS, 0x0000) PORT_DIPLOCATION("SW1:7,8") \
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )

static INPUT_PORTS_START( common )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE ) /* 0x20 is another test mode */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL )  /* sprite DMA complete */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( rtype )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, "50K 150K 250K 400K 600K" )
	PORT_DIPSETTING(      0x0008, "100K 200K 350K 500K 700K"  )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Invulnerability" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8")
INPUT_PORTS_END

/* identical but Demo Sounds is inverted */
static INPUT_PORTS_START( rtypep )
	PORT_INCLUDE( rtype )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bchopper )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "80K 200K 350K" )
	PORT_DIPSETTING(      0x0000, "100K 250K 400K" )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Invulnerability" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( nspirit )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Invulnerability" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( imgfight )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x0000, "Debug Mode 2 lap" ) /* Not used according to manual */
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( loht )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW1:4" )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hardest ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Invulnerability" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( xmultipl )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, "1" )
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, "Upright (single)" )      PORT_CONDITION("DSW", 0x1000, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )     PORT_CONDITION("DSW", 0x1000, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, "Upright (double) On" )   PORT_CONDITION("DSW", 0x1000, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0200, "Upright (double) Off" )  PORT_CONDITION("DSW", 0x1000, EQUALS, 0x0000)
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Upright (double) Mode" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( dbreed )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( rtype2 )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x1800, 0x1000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, "Upright (2P)" )
	PORT_DIPSETTING(      0x1800, DEF_STR( Cocktail ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" ) /* Always OFF according to the manual */
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( hharry )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Continue Limit" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0400, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, "Upright (2P)" )
	PORT_DIPSETTING(      0x0600, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin mode 2 */
	IREM_COIN_MODE_2_HIGH
INPUT_PORTS_END

static INPUT_PORTS_START( poundfor )
	PORT_START("IN0")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* high bits of trackball X */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1f00, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* high bits of trackball X */
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE ) /* 0x0020 is another test mode */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL )  /* sprite DMA complete */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, "Round Time" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "60" )
	PORT_DIPSETTING(      0x0003, "90" )
	PORT_DIPSETTING(      0x0001, "120" )
	PORT_DIPSETTING(      0x0000, "150" )
	PORT_DIPNAME( 0x0004, 0x0004, "Matches/Credit (2P)" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0008, 0x0008, "Rounds/Match" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Trackball Size" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, "Small" )
	PORT_DIPSETTING(      0x0000, "Large" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8")
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0400, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, "Upright (2P)" )
	PORT_DIPSETTING(      0x0600, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	/* Coin Mode 1 */
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_CONDITION("DSW", 0x0800, NOTEQUALS, 0x0000) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, "1 Coin/1 Credit, 1 Coin/Continue" )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("TRACK0_X")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("TRACK0_Y")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACK1_X")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("TRACK1_Y")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( airduel )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" )     /* Manual says not used, must be OFF */
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW1:6" )     /* Manual says not used, must be OFF */
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0200, IP_ACTIVE_LOW, "SW1:2" )     /* Manual says not used, must be OFF */
	PORT_DIPUNUSED_DIPLOC( 0x0400, IP_ACTIVE_LOW, "SW1:3" )     /* Manual says not used, must be OFF */
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	/* Coin Mode 1 */
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_CONDITION("DSW", 0x0800, NOTEQUALS, 0x0000) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
//  PORT_DIPSETTING(      0x1000, DEF_STR( Free-Play ) )  /* another free play */
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH
INPUT_PORTS_END

static INPUT_PORTS_START( gallop )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, "Upright (2P)" )
	PORT_DIPSETTING(      0x0600, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin mode 2 */
	IREM_COIN_MODE_2_HIGH
INPUT_PORTS_END

static INPUT_PORTS_START( kengo )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin mode 2 */
	IREM_COIN_MODE_2_HIGH
INPUT_PORTS_END



static const gfx_layout tilelayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,4),  /* NUM characters */
	4,  /* 4 bits per pixel */
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,4),  /* NUM characters */
	4,  /* 4 bits per pixel */
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( m72 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,    0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,    256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,    256, 16 )
GFXDECODE_END

static GFXDECODE_START( rtype2 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,     256, 16 )
GFXDECODE_END

static GFXDECODE_START( majtitle )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,     256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,     0, 16 )
GFXDECODE_END


static MACHINE_CONFIG_FRAGMENT( m72_audio_chips )
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("m72", M72, 0);

	MCFG_YM2151_ADD("ymsnd", SOUND_CLOCK)
	MCFG_YM2151_IRQ_HANDLER(DEVWRITELINE("m72", m72_audio_device, ym2151_irq_handler))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( m72_base, m72_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",V30,MASTER_CLOCK/2/2)    /* 16 MHz external freq (8MHz internal) */
	MCFG_CPU_PROGRAM_MAP(m72_map)
	MCFG_CPU_IO_MAP(m72_portmap)

	MCFG_CPU_ADD("soundcpu",Z80, SOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_ram_map)
	MCFG_CPU_IO_MAP(sound_portmap)


	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", m72)
	MCFG_PALETTE_ADD("palette", 512)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(m72_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(m72_state,m72)

	MCFG_FRAGMENT_ADD(m72_audio_chips)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( m72, m72_base )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PERIODIC_INT_DRIVER(m72_state, fake_nmi, 128*55)   /* clocked by V1? (Vigilante) */
							/* IRQs are generated by main Z80 and YM2151 */
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( m72_8751, m72_base )

	MCFG_CPU_ADD("mcu",I8751, XTAL_8MHz) /* Uses its own XTAL */
	MCFG_CPU_IO_MAP(mcu_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", m72_state,  mcu_int)

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( rtype, m72_base )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(rtype_map)

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_IO_MAP(rtype_sound_portmap)

	MCFG_DEVICE_REMOVE("dac")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( xmultiplm72, m72_8751 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(xmultiplm72_map)

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PERIODIC_INT_DRIVER(m72_state, nmi_line_pulse, 128*55) /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */


	MCFG_MACHINE_RESET_OVERRIDE(m72_state,xmultipl)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( dbreedm72, m72_base )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(dbreedm72_map)

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PERIODIC_INT_DRIVER(m72_state, nmi_line_pulse, 128*55) /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */

	MCFG_MACHINE_RESET_OVERRIDE(m72_state,xmultipl)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( dkgenm72, m72 ) // dervices from 'm72' because we use 'fake nmi' on the soundcpu

	MCFG_MACHINE_RESET_OVERRIDE(m72_state,xmultipl)
MACHINE_CONFIG_END


// not m72
static MACHINE_CONFIG_DERIVED( xmultipl, m72 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(xmultipl_map)
	MCFG_CPU_IO_MAP(hharry_portmap)

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PROGRAM_MAP(sound_rom_map)
	MCFG_CPU_IO_MAP(rtype2_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(m72_state, nmi_line_pulse, 128*55) /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */


	MCFG_MACHINE_RESET_OVERRIDE(m72_state,xmultipl)

	MCFG_VIDEO_START_OVERRIDE(m72_state,xmultipl)
MACHINE_CONFIG_END


// not m72, different video system (more sprites)
static MACHINE_CONFIG_START( majtitle, m72_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30,MASTER_CLOCK/2/2)   /* 16 MHz external freq (8MHz internal) */
	MCFG_CPU_PROGRAM_MAP(majtitle_map)
	MCFG_CPU_IO_MAP(majtitle_portmap)

	MCFG_CPU_ADD("soundcpu", Z80, SOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_rom_map)
	MCFG_CPU_IO_MAP(rtype2_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(m72_state, nmi_line_pulse, 128*55) /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", majtitle)
	MCFG_PALETTE_ADD("palette", 512)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(m72_state, screen_update_majtitle)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(m72_state,majtitle)

	MCFG_FRAGMENT_ADD(m72_audio_chips)
MACHINE_CONFIG_END

// not m72, different video system (less tiles regions?)
static MACHINE_CONFIG_START( rtype2, m72_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30,MASTER_CLOCK/2/2)   /* 16 MHz external freq (8MHz internal) */
	MCFG_CPU_PROGRAM_MAP(rtype2_map)
	MCFG_CPU_IO_MAP(rtype2_portmap)

	MCFG_CPU_ADD("soundcpu", Z80, SOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_rom_map)
	MCFG_CPU_IO_MAP(rtype2_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(m72_state, nmi_line_pulse, 128*55) /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rtype2)
	MCFG_PALETTE_ADD("palette", 512)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(m72_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(m72_state,rtype2)

	MCFG_FRAGMENT_ADD(m72_audio_chips)
MACHINE_CONFIG_END

// not m72, different video system (less tiles regions?)
static MACHINE_CONFIG_START( dbreed, m72_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",V30,MASTER_CLOCK/2/2)    /* 16 MHz external freq (8MHz internal) */
	MCFG_CPU_PROGRAM_MAP(dbreed_map)
	MCFG_CPU_IO_MAP(hharry_portmap)

	MCFG_CPU_ADD("soundcpu",Z80, SOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_rom_map)
	MCFG_CPU_IO_MAP(rtype2_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(m72_state, nmi_line_pulse, 128*55) /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */


	MCFG_MACHINE_RESET_OVERRIDE(m72_state,xmultipl)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rtype2)
	MCFG_PALETTE_ADD("palette", 512)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(m72_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(m72_state,hharry)

	MCFG_FRAGMENT_ADD(m72_audio_chips)
MACHINE_CONFIG_END

// not m72, different video system (less tiles regions?)
static MACHINE_CONFIG_START( hharry, m72_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30,MASTER_CLOCK/2/2)   /* 16 MHz external freq (8MHz internal) */
	MCFG_CPU_PROGRAM_MAP(hharry_map)
	MCFG_CPU_IO_MAP(hharry_portmap)

	MCFG_CPU_ADD("soundcpu", Z80, SOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_rom_map)
	MCFG_CPU_IO_MAP(rtype2_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(m72_state, nmi_line_pulse, 128*55) /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */


	MCFG_MACHINE_RESET_OVERRIDE(m72_state,xmultipl)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rtype2)
	MCFG_PALETTE_ADD("palette", 512)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(m72_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(m72_state,hharry)

	MCFG_FRAGMENT_ADD(m72_audio_chips)

MACHINE_CONFIG_END

// not m72, different video system (less tiles regions?)
static MACHINE_CONFIG_START( hharryu, m72_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30,MASTER_CLOCK/2/2)   /* 16 MHz external freq (8MHz internal) */
	MCFG_CPU_PROGRAM_MAP(hharryu_map)
	MCFG_CPU_IO_MAP(rtype2_portmap)

	MCFG_CPU_ADD("soundcpu", Z80, SOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_rom_map)
	MCFG_CPU_IO_MAP(rtype2_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(m72_state, nmi_line_pulse, 128*55) /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */


	MCFG_MACHINE_RESET_OVERRIDE(m72_state,xmultipl)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rtype2)
	MCFG_PALETTE_ADD("palette", 512)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(m72_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(m72_state,hharryu)

	MCFG_FRAGMENT_ADD(m72_audio_chips)

MACHINE_CONFIG_END

// not m72, different video system (less tiles regions?)
static MACHINE_CONFIG_START( poundfor, m72_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30,MASTER_CLOCK/2/2)   /* 16 MHz external freq (8MHz internal) */
	MCFG_CPU_PROGRAM_MAP(rtype2_map)
	MCFG_CPU_IO_MAP(poundfor_portmap)

	MCFG_CPU_ADD("soundcpu", Z80, SOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_rom_map)
	MCFG_CPU_IO_MAP(poundfor_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(m72_state, fake_nmi, 128*55)   /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rtype2)
	MCFG_PALETTE_ADD("palette", 512)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(m72_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(m72_state,poundfor)

	MCFG_FRAGMENT_ADD(m72_audio_chips)
MACHINE_CONFIG_END

// not m72, different video system (less tiles regions?)
static MACHINE_CONFIG_START( cosmccop, m72_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V35,MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(kengo_map)
	MCFG_CPU_IO_MAP(kengo_portmap)

	MCFG_CPU_ADD("soundcpu", Z80, SOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_rom_map)
	MCFG_CPU_IO_MAP(rtype2_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(m72_state, nmi_line_pulse, 128*55) /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */

	MCFG_MACHINE_START_OVERRIDE(m72_state,kengo)
	MCFG_MACHINE_RESET_OVERRIDE(m72_state,kengo)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rtype2)
	MCFG_PALETTE_ADD("palette", 512)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(m72_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(m72_state,poundfor)

	MCFG_FRAGMENT_ADD(m72_audio_chips)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( kengo, cosmccop )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_V25_CONFIG(gunforce_decryption_table)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( rtype )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Roms located on the M72-ROM-C rom board */
	ROM_LOAD16_BYTE( "rt_r-h0-b.1b", 0x00001, 0x10000, CRC(591c7754) SHA1(0b9d5474bc5963224923126cf84d74a39b8270cc) )
	ROM_LOAD16_BYTE( "rt_r-l0-b.3b", 0x00000, 0x10000, CRC(a1928df0) SHA1(3001c1b87cd1d441ba1226fb5b9dd6268458c0e8) )
	ROM_LOAD16_BYTE( "rt_r-h1-b.1c", 0x20001, 0x10000, CRC(a9d71eca) SHA1(008d1dc289df2ae2ba8f93d319c2b2c108cb9b89) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "rt_r-l1-b.3c", 0x20000, 0x10000, CRC(0df3573d) SHA1(0144c846fd0bdb3e4d790f6cb7bb64829e931b76) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Roms located on the M72-ROM-C rom board */
	ROM_LOAD( "rt_r-00.1h", 0x00000, 0x10000, CRC(dad53bc0) SHA1(1e3bc498861946278a0b1fe24259f5d224e265d7) )    /* sprites */
	ROM_LOAD( "rt_r-01.1j", 0x10000, 0x08000, CRC(5e441e7f) SHA1(6741eb7f2d9d985b5a89eefc73ea44c3e38de6f7) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "rt_r-10.1k", 0x20000, 0x10000, CRC(d6a66298) SHA1(d2873d05aa3b257e7699c188880ac3daad672fa5) )
	ROM_LOAD( "rt_r-11.1l", 0x30000, 0x08000, CRC(791df4f8) SHA1(5239a97222212ac9c019177771cb2b5096b7bc17) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "rt_r-20.3h", 0x40000, 0x10000, CRC(fc247c8a) SHA1(01cf0a60f47fa5e2ed430a3f075e69e6cb762a48) )
	ROM_LOAD( "rt_r-21.3j", 0x50000, 0x08000, CRC(ed793841) SHA1(7e55a9a11fcd989db39bce6be48821b747c7d97f) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "rt_r-30.3k", 0x60000, 0x10000, CRC(eb02a1cb) SHA1(60a394ab53afdcbbf9e88083b8dbe8c897170d77) )
	ROM_LOAD( "rt_r-31.3l", 0x70000, 0x08000, CRC(8558355d) SHA1(b5467d1f22f6e5f90c5d8a8ac2d55974f287d589) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* Roms located on the M72-B-D rom board */
	ROM_LOAD( "rt_b-a0.3c", 0x00000, 0x08000, CRC(4e212fb0) SHA1(687061ecade2ebd0bd1343c9c4a831791853f79c) )    /* tiles #1 */
	ROM_LOAD( "rt_b-a1.3d", 0x08000, 0x08000, CRC(8a65bdff) SHA1(130bf6af521f13247a739a95eab4bdaa24b2ac10) )
	ROM_LOAD( "rt_b-a2.3a", 0x10000, 0x08000, CRC(5a4ae5b9) SHA1(95c3b64f50e6f673b2bf9b40642c152da5009d25) )
	ROM_LOAD( "rt_b-a3.3e", 0x18000, 0x08000, CRC(73327606) SHA1(9529ecdedd30e2a0400fb1083117992cc18b5158) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Roms located on the M72-B-D rom board */
	ROM_LOAD( "rt_b-b0.3j", 0x00000, 0x08000, CRC(a7b17491) SHA1(5b390770e56ba2d35e108534d7eda8dca996fdf7) )    /* tiles #2 */
	ROM_LOAD( "rt_b-b1.3k", 0x08000, 0x08000, CRC(b9709686) SHA1(700905a3e9661e0874939f54da2909e1396ce596) )
	ROM_LOAD( "rt_b-b2.3h", 0x10000, 0x08000, CRC(433b229a) SHA1(14222eaa3e67e5a7f80eafcf22bac4eb2d485a9a) )
	ROM_LOAD( "rt_b-b3.3f", 0x18000, 0x08000, CRC(ad89b072) SHA1(e2683d0e7415f3abd147e518bf6c87e44744cd4f) )
ROM_END

ROM_START( rtypej )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Roms located on the M72-ROM-C rom board */
	ROM_LOAD16_BYTE( "rt_r-h0-.1b", 0x00001, 0x10000, CRC(c2940df2) SHA1(cbccd205ef81a0e39990a34d46e3f7d52b62e385) )
	ROM_LOAD16_BYTE( "rt_r-l0-.3b", 0x00000, 0x10000, CRC(858cc0f6) SHA1(7a256fe3aa3a96e161dd485a90b18c421b61458b) )
	ROM_LOAD16_BYTE( "rt_r-h1-.1c", 0x20001, 0x10000, CRC(5bcededa) SHA1(4ada3fd207fa57751f8e3d885bc91b374e27035d) )
	ROM_RELOAD(                     0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "rt_r-l1-.3c", 0x20000, 0x10000, CRC(4821141c) SHA1(df6cf04c3ecd04b6f27a96871848904575414dae) )
	ROM_RELOAD(                     0xe0000, 0x10000 )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Roms located on the M72-ROM-C rom board */
	ROM_LOAD( "rt_r-00.1h", 0x00000, 0x10000, CRC(dad53bc0) SHA1(1e3bc498861946278a0b1fe24259f5d224e265d7) )    /* sprites */
	ROM_LOAD( "rt_r-01.1j", 0x10000, 0x08000, CRC(5e441e7f) SHA1(6741eb7f2d9d985b5a89eefc73ea44c3e38de6f7) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "rt_r-10.1k", 0x20000, 0x10000, CRC(d6a66298) SHA1(d2873d05aa3b257e7699c188880ac3daad672fa5) )
	ROM_LOAD( "rt_r-11.1l", 0x30000, 0x08000, CRC(791df4f8) SHA1(5239a97222212ac9c019177771cb2b5096b7bc17) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "rt_r-20.3h", 0x40000, 0x10000, CRC(fc247c8a) SHA1(01cf0a60f47fa5e2ed430a3f075e69e6cb762a48) )
	ROM_LOAD( "rt_r-21.3j", 0x50000, 0x08000, CRC(ed793841) SHA1(7e55a9a11fcd989db39bce6be48821b747c7d97f) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "rt_r-30.3k", 0x60000, 0x10000, CRC(eb02a1cb) SHA1(60a394ab53afdcbbf9e88083b8dbe8c897170d77) )
	ROM_LOAD( "rt_r-31.3l", 0x70000, 0x08000, CRC(8558355d) SHA1(b5467d1f22f6e5f90c5d8a8ac2d55974f287d589) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* Roms located on the M72-B-D rom board */
	ROM_LOAD( "rt_b-a0.3c", 0x00000, 0x08000, CRC(4e212fb0) SHA1(687061ecade2ebd0bd1343c9c4a831791853f79c) )    /* tiles #1 */
	ROM_LOAD( "rt_b-a1.3d", 0x08000, 0x08000, CRC(8a65bdff) SHA1(130bf6af521f13247a739a95eab4bdaa24b2ac10) )
	ROM_LOAD( "rt_b-a2.3a", 0x10000, 0x08000, CRC(5a4ae5b9) SHA1(95c3b64f50e6f673b2bf9b40642c152da5009d25) )
	ROM_LOAD( "rt_b-a3.3e", 0x18000, 0x08000, CRC(73327606) SHA1(9529ecdedd30e2a0400fb1083117992cc18b5158) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Roms located on the M72-B-D rom board */
	ROM_LOAD( "rt_b-b0.3j", 0x00000, 0x08000, CRC(a7b17491) SHA1(5b390770e56ba2d35e108534d7eda8dca996fdf7) )    /* tiles #2 */
	ROM_LOAD( "rt_b-b1.3k", 0x08000, 0x08000, CRC(b9709686) SHA1(700905a3e9661e0874939f54da2909e1396ce596) )
	ROM_LOAD( "rt_b-b2.3h", 0x10000, 0x08000, CRC(433b229a) SHA1(14222eaa3e67e5a7f80eafcf22bac4eb2d485a9a) )
	ROM_LOAD( "rt_b-b3.3f", 0x18000, 0x08000, CRC(ad89b072) SHA1(e2683d0e7415f3abd147e518bf6c87e44744cd4f) )
ROM_END

ROM_START( rtypejp )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Roms located on the M72-ROM-C rom board */
	ROM_LOAD16_BYTE( "db_b1.bin", 0x00001, 0x10000, CRC(c1865141) SHA1(3302b6529aa903d81eb2196d745eb4f7f8316857) )
	ROM_LOAD16_BYTE( "db_a1.bin", 0x00000, 0x10000, CRC(5ad2bd90) SHA1(0937dbbdf0cbce2e81cecf4d770bbd8c6bd82801) )
	ROM_LOAD16_BYTE( "db_b2.bin", 0x20001, 0x10000, CRC(b4f6407e) SHA1(4a00d8e104c580900b4feb318dd162b77b71d0a5) )
	ROM_RELOAD(                   0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "db_a2.bin", 0x20000, 0x10000, CRC(6098d86f) SHA1(c6c9c1c2c30d5f190c40e000004bd21606efb8b0) )
	ROM_RELOAD(                   0xe0000, 0x10000 )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Roms located on the M72-ROM-C rom board */
	ROM_LOAD( "rt_r-00.1h", 0x00000, 0x10000, CRC(dad53bc0) SHA1(1e3bc498861946278a0b1fe24259f5d224e265d7) )    /* sprites */
	ROM_LOAD( "rt_r-01.1j", 0x10000, 0x08000, CRC(5e441e7f) SHA1(6741eb7f2d9d985b5a89eefc73ea44c3e38de6f7) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "rt_r-10.1k", 0x20000, 0x10000, CRC(d6a66298) SHA1(d2873d05aa3b257e7699c188880ac3daad672fa5) )
	ROM_LOAD( "rt_r-11.1l", 0x30000, 0x08000, CRC(791df4f8) SHA1(5239a97222212ac9c019177771cb2b5096b7bc17) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "rt_r-20.3h", 0x40000, 0x10000, CRC(fc247c8a) SHA1(01cf0a60f47fa5e2ed430a3f075e69e6cb762a48) )
	ROM_LOAD( "rt_r-21.3j", 0x50000, 0x08000, CRC(ed793841) SHA1(7e55a9a11fcd989db39bce6be48821b747c7d97f) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "rt_r-30.3k", 0x60000, 0x10000, CRC(eb02a1cb) SHA1(60a394ab53afdcbbf9e88083b8dbe8c897170d77) )
	ROM_LOAD( "rt_r-31.3l", 0x70000, 0x08000, CRC(8558355d) SHA1(b5467d1f22f6e5f90c5d8a8ac2d55974f287d589) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* Roms located on the M72-B-D rom board */
	ROM_LOAD( "rt_b-a0.3c", 0x00000, 0x08000, CRC(4e212fb0) SHA1(687061ecade2ebd0bd1343c9c4a831791853f79c) )    /* tiles #1 */
	ROM_LOAD( "rt_b-a1.3d", 0x08000, 0x08000, CRC(8a65bdff) SHA1(130bf6af521f13247a739a95eab4bdaa24b2ac10) )
	ROM_LOAD( "rt_b-a2.3a", 0x10000, 0x08000, CRC(5a4ae5b9) SHA1(95c3b64f50e6f673b2bf9b40642c152da5009d25) )
	ROM_LOAD( "rt_b-a3.3e", 0x18000, 0x08000, CRC(73327606) SHA1(9529ecdedd30e2a0400fb1083117992cc18b5158) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Roms located on the M72-B-D rom board */
	ROM_LOAD( "rt_b-b0.3j", 0x00000, 0x08000, CRC(a7b17491) SHA1(5b390770e56ba2d35e108534d7eda8dca996fdf7) )    /* tiles #2 */
	ROM_LOAD( "rt_b-b1.3k", 0x08000, 0x08000, CRC(b9709686) SHA1(700905a3e9661e0874939f54da2909e1396ce596) )
	ROM_LOAD( "rt_b-b2.3h", 0x10000, 0x08000, CRC(433b229a) SHA1(14222eaa3e67e5a7f80eafcf22bac4eb2d485a9a) )
	ROM_LOAD( "rt_b-b3.3f", 0x18000, 0x08000, CRC(ad89b072) SHA1(e2683d0e7415f3abd147e518bf6c87e44744cd4f) )
ROM_END

ROM_START( rtypeu )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Roms located on the M72-ROM-C rom board */
	ROM_LOAD16_BYTE( "rt_r-h0-a.1b", 0x00001, 0x10000, CRC(36008a4e) SHA1(832006cb14a34e1671e305cc8ae606c3c6185a6a) )
	ROM_LOAD16_BYTE( "rt_r-l0-a.3b", 0x00000, 0x10000, CRC(4aaa668e) SHA1(87059460b59f43f2ca8cd959d76f721facd9de96) )
	ROM_LOAD16_BYTE( "rt_r-h1-a.1c", 0x20001, 0x10000, CRC(7ebb2a53) SHA1(1466df19888c3374847eb77f702060647e49d6ad) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "rt_r-l1-a.3c", 0x20000, 0x10000, CRC(c28b103b) SHA1(f294a23c3917b97812eb4c7f3a99253fd0cbb7ea) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Roms located on the M72-ROM-C rom board */
	ROM_LOAD( "rt_r-00.1h", 0x00000, 0x10000, CRC(dad53bc0) SHA1(1e3bc498861946278a0b1fe24259f5d224e265d7) )    /* sprites */
	ROM_LOAD( "rt_r-01.1j", 0x10000, 0x08000, CRC(5e441e7f) SHA1(6741eb7f2d9d985b5a89eefc73ea44c3e38de6f7) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "rt_r-10.1k", 0x20000, 0x10000, CRC(d6a66298) SHA1(d2873d05aa3b257e7699c188880ac3daad672fa5) )
	ROM_LOAD( "rt_r-11.1l", 0x30000, 0x08000, CRC(791df4f8) SHA1(5239a97222212ac9c019177771cb2b5096b7bc17) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "rt_r-20.3h", 0x40000, 0x10000, CRC(fc247c8a) SHA1(01cf0a60f47fa5e2ed430a3f075e69e6cb762a48) )
	ROM_LOAD( "rt_r-21.3j", 0x50000, 0x08000, CRC(ed793841) SHA1(7e55a9a11fcd989db39bce6be48821b747c7d97f) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "rt_r-30.3k", 0x60000, 0x10000, CRC(eb02a1cb) SHA1(60a394ab53afdcbbf9e88083b8dbe8c897170d77) )
	ROM_LOAD( "rt_r-31.3l", 0x70000, 0x08000, CRC(8558355d) SHA1(b5467d1f22f6e5f90c5d8a8ac2d55974f287d589) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* Roms located on the M72-B-D rom board */
	ROM_LOAD( "rt_b-a0.3c", 0x00000, 0x08000, CRC(4e212fb0) SHA1(687061ecade2ebd0bd1343c9c4a831791853f79c) )    /* tiles #1 */
	ROM_LOAD( "rt_b-a1.3d", 0x08000, 0x08000, CRC(8a65bdff) SHA1(130bf6af521f13247a739a95eab4bdaa24b2ac10) )
	ROM_LOAD( "rt_b-a2.3a", 0x10000, 0x08000, CRC(5a4ae5b9) SHA1(95c3b64f50e6f673b2bf9b40642c152da5009d25) )
	ROM_LOAD( "rt_b-a3.3e", 0x18000, 0x08000, CRC(73327606) SHA1(9529ecdedd30e2a0400fb1083117992cc18b5158) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Roms located on the M72-B-D rom board */
	ROM_LOAD( "rt_b-b0.3j", 0x00000, 0x08000, CRC(a7b17491) SHA1(5b390770e56ba2d35e108534d7eda8dca996fdf7) )    /* tiles #2 */
	ROM_LOAD( "rt_b-b1.3k", 0x08000, 0x08000, CRC(b9709686) SHA1(700905a3e9661e0874939f54da2909e1396ce596) )
	ROM_LOAD( "rt_b-b2.3h", 0x10000, 0x08000, CRC(433b229a) SHA1(14222eaa3e67e5a7f80eafcf22bac4eb2d485a9a) )
	ROM_LOAD( "rt_b-b3.3f", 0x18000, 0x08000, CRC(ad89b072) SHA1(e2683d0e7415f3abd147e518bf6c87e44744cd4f) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Located on M72-A-C CPU/Sound board */
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) /* TBP24S10 */
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) /* TBP24S10 */

	ROM_REGION( 0x0003, "plds", 0 )
	/* Located on M72-ROM-C rom board */
	ROM_LOAD( "m72_r-3a-.bin",  0x0000, 0x0001, NO_DUMP ) /* PAL16L8 at 3A */
	/* Located on M72-A-C CPU/Sound board */
	ROM_LOAD( "m72_a-3d-.bin",  0x0000, 0x0001, NO_DUMP ) /* PAL16L8 at IC11 */
	ROM_LOAD( "m72_a-4d-.bin",  0x0000, 0x0001, NO_DUMP ) /* PAL16L8 at IC19 */
ROM_END

ROM_START( rtypeb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Roms located on the M72-ROM-C rom board */
	ROM_LOAD16_BYTE( "7.512", 0x00001, 0x10000, CRC(eacc8024) SHA1(6bcf1d4ea182b7341eac736d2a5d5f70deec0758) )
	ROM_LOAD16_BYTE( "1.512", 0x00000, 0x10000, CRC(2e5fe27b) SHA1(a3364be5ab9c67aaa2152baf39ea12c571eca3cc) )
	ROM_LOAD16_BYTE( "8.512", 0x20001, 0x10000, CRC(22cc4950) SHA1(ada5cffc13c38391a334411632237166a6be4938) )
	ROM_RELOAD(               0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "2.512", 0x20000, 0x10000, CRC(ada7b90e) SHA1(c9d2caed95b95d1c1718a10766bc88b2f8f51619) )
	ROM_RELOAD(               0xe0000, 0x10000 )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Roms located on the M72-ROM-C rom board */
	ROM_LOAD( "rt_r-00.1h", 0x00000, 0x10000, CRC(dad53bc0) SHA1(1e3bc498861946278a0b1fe24259f5d224e265d7) )    /* sprites */
	ROM_LOAD( "rt_r-01.1j", 0x10000, 0x08000, CRC(5e441e7f) SHA1(6741eb7f2d9d985b5a89eefc73ea44c3e38de6f7) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "rt_r-10.1k", 0x20000, 0x10000, CRC(d6a66298) SHA1(d2873d05aa3b257e7699c188880ac3daad672fa5) )
	ROM_LOAD( "rt_r-11.1l", 0x30000, 0x08000, CRC(791df4f8) SHA1(5239a97222212ac9c019177771cb2b5096b7bc17) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "rt_r-20.3h", 0x40000, 0x10000, CRC(fc247c8a) SHA1(01cf0a60f47fa5e2ed430a3f075e69e6cb762a48) )
	ROM_LOAD( "rt_r-21.3j", 0x50000, 0x08000, CRC(ed793841) SHA1(7e55a9a11fcd989db39bce6be48821b747c7d97f) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "rt_r-30.3k", 0x60000, 0x10000, CRC(eb02a1cb) SHA1(60a394ab53afdcbbf9e88083b8dbe8c897170d77) )
	ROM_LOAD( "rt_r-31.3l", 0x70000, 0x08000, CRC(8558355d) SHA1(b5467d1f22f6e5f90c5d8a8ac2d55974f287d589) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* Roms located on the M72-B-D rom board */
	ROM_LOAD( "rt_b-a0.3c", 0x00000, 0x08000, CRC(4e212fb0) SHA1(687061ecade2ebd0bd1343c9c4a831791853f79c) )    /* tiles #1 */
	ROM_LOAD( "rt_b-a1.3d", 0x08000, 0x08000, CRC(8a65bdff) SHA1(130bf6af521f13247a739a95eab4bdaa24b2ac10) )
	ROM_LOAD( "rt_b-a2.3a", 0x10000, 0x08000, CRC(5a4ae5b9) SHA1(95c3b64f50e6f673b2bf9b40642c152da5009d25) )
	ROM_LOAD( "rt_b-a3.3e", 0x18000, 0x08000, CRC(73327606) SHA1(9529ecdedd30e2a0400fb1083117992cc18b5158) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Roms located on the M72-B-D rom board */
	ROM_LOAD( "rt_b-b0.3j", 0x00000, 0x08000, CRC(a7b17491) SHA1(5b390770e56ba2d35e108534d7eda8dca996fdf7) )    /* tiles #2 */
	ROM_LOAD( "rt_b-b1.3k", 0x08000, 0x08000, CRC(b9709686) SHA1(700905a3e9661e0874939f54da2909e1396ce596) )
	ROM_LOAD( "rt_b-b2.3h", 0x10000, 0x08000, CRC(433b229a) SHA1(14222eaa3e67e5a7f80eafcf22bac4eb2d485a9a) )
	ROM_LOAD( "rt_b-b3.3f", 0x18000, 0x08000, CRC(ad89b072) SHA1(e2683d0e7415f3abd147e518bf6c87e44744cd4f) )
ROM_END

ROM_START( bchopper )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c-h0-b.rom",   0x00001, 0x10000, CRC(f2feab16) SHA1(03ee874658e0f59957f8425e1ebf9c938737cc19) )
	ROM_LOAD16_BYTE( "c-l0-b.rom",   0x00000, 0x10000, CRC(9f887096) SHA1(4f41ef29580fc026ea91d110ec6b2e6af83dbd9a) )
	ROM_LOAD16_BYTE( "c-h1-b.rom",   0x20001, 0x10000, CRC(a995d64f) SHA1(43eb2eb11e6875298a6ef2b18f0f5e587f1bba16) )
	ROM_LOAD16_BYTE( "c-l1-b.rom",   0x20000, 0x10000, CRC(41dda999) SHA1(4d07a399aaf16bc37b5488e3e4bb60e78811a099) )
	ROM_LOAD16_BYTE( "c-h3-b.rom",   0x60001, 0x10000, CRC(ab9451ca) SHA1(ec0e0ad592d8b21bb4e6927a452e3b7964cda015) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "c-l3-b.rom",   0x60000, 0x10000, CRC(11562221) SHA1(a2f136a487fb6f30350e8d1e26c0729eb0686c7d) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "bchopper_i8751.mcu",  0x00000, 0x10000, NO_DUMP ) // read protected

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "c-00-a.rom",   0x00000, 0x10000, CRC(f6e6e660) SHA1(e066e5ed37719cf2b6fd36e0117f11325bb06f9c) )  /* sprites */
	ROM_LOAD( "c-01-b.rom",   0x10000, 0x10000, CRC(708cdd37) SHA1(24f3fcd381422f0d75410c2af7a56744e3b4a699) )
	ROM_LOAD( "c-10-a.rom",   0x20000, 0x10000, CRC(292c8520) SHA1(c552090d295ee1c1ca611b0cddee356e509e2045) )
	ROM_LOAD( "c-11-b.rom",   0x30000, 0x10000, CRC(20904cf3) SHA1(71fe505f2da53c2eb445b7b758d257d6af42e6f1) )
	ROM_LOAD( "c-20-a.rom",   0x40000, 0x10000, CRC(1ab50c23) SHA1(43e2f11e5bbf157c47764e04e372f40ed68bab59) )
	ROM_LOAD( "c-21-b.rom",   0x50000, 0x10000, CRC(c823d34c) SHA1(47383214b6a60e0b1b70208b00c291f8ffed36bc) )
	ROM_LOAD( "c-30-a.rom",   0x60000, 0x10000, CRC(11f6c56b) SHA1(39a2a674698b044c84fea65ae41a9e003a50b639) )
	ROM_LOAD( "c-31-b.rom",   0x70000, 0x10000, CRC(23134ec5) SHA1(43453f8a13b51310e04729dc828d391ca9c04da2) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "b-a0-b.rom",   0x00000, 0x10000, CRC(e46ed7bf) SHA1(75abb5f40629f7c40a610a44e068b6c4e3a5126e) )  /* tiles #1 */
	ROM_LOAD( "b-a1-b.rom",   0x10000, 0x10000, CRC(590605ff) SHA1(fbb5c0cebd28b08d4ce39db4055d6343620e0f1c) )
	ROM_LOAD( "b-a2-b.rom",   0x20000, 0x10000, CRC(f8158226) SHA1(bb3a8686cd89bb8265b6b9e03682cc0bf6533793) )
	ROM_LOAD( "b-a3-b.rom",   0x30000, 0x10000, CRC(0f07b9b7) SHA1(63dbec17097f07eb39299372b736fbbc1b11b65e) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "b-b0-.rom",    0x00000, 0x10000, CRC(b5b95776) SHA1(4685b56071b916ce712c45f24da8068dd7e40ed1) )  /* tiles #2 */
	ROM_LOAD( "b-b1-.rom",    0x10000, 0x10000, CRC(74ca16ee) SHA1(7984bc9a0b46e1b4a8ecac7528d57606305aad73) )
	ROM_LOAD( "b-b2-.rom",    0x20000, 0x10000, CRC(b82cca04) SHA1(c12b95be311205181b01d15021bcf9f01ed3e0a3) )
	ROM_LOAD( "b-b3-.rom",    0x30000, 0x10000, CRC(a7afc920) SHA1(92c75463ada39184e731b82ef2883ae6f1f67482) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "c-v0-b.rom",   0x00000, 0x10000, CRC(d0c27e58) SHA1(fec76217cc0c04c723989c3ec127a2bd33d64c60) )
ROM_END

ROM_START( mrheli )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mh-c-h0.bin",  0x00001, 0x10000, CRC(e2ca5646) SHA1(9f4fe2f0a45233325bd9336cabb925a1f625453b) )
	ROM_LOAD16_BYTE( "mh-c-l0.bin",  0x00000, 0x10000, CRC(643e23cd) SHA1(66998a6dfc7ef538540986b61d2414a5ef250d0d) )
	ROM_LOAD16_BYTE( "mh-c-h1.bin",  0x20001, 0x10000, CRC(8974e84d) SHA1(39e05c80e805dde45f2fc5fc429b75f9b599089c) )
	ROM_LOAD16_BYTE( "mh-c-l1.bin",  0x20000, 0x10000, CRC(5f8bda69) SHA1(48629d617bd48c9de9c6a567fb203258a56fdbbd) )
	ROM_LOAD16_BYTE( "mh-c-h3.bin",  0x60001, 0x10000, CRC(143f596e) SHA1(f9d444eebcd53dac925d14b7a2858803b7fd9ce2) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "mh-c-l3.bin",  0x60000, 0x10000, CRC(c0982536) SHA1(45399f8d0577c6e2a277a69303954ce5d2de7c07) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "mh-c-pr.bin",  0x00000, 0x1000, CRC(897dc4ee) SHA1(05a24bf76e8fa9ca96ba9376cbf44d299df04138) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mh-c-00.bin",  0x00000, 0x20000, CRC(dec4e121) SHA1(92169b523f1600e994e016dc1959a52958e1d89d) )  /* sprites */
	ROM_LOAD( "mh-c-10.bin",  0x20000, 0x20000, CRC(7aaa151e) SHA1(efd980bb2eed7084354b7a4aa2f733cd2f876741) )
	ROM_LOAD( "mh-c-20.bin",  0x40000, 0x20000, CRC(eae0de74) SHA1(3a2469c0eeb18131f989807afb50228f57ccea30) )
	ROM_LOAD( "mh-c-30.bin",  0x60000, 0x20000, CRC(01d5052f) SHA1(5d5e70913bb7af48193c70209595f27a64fa6cac) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "mh-b-a0.bin",  0x00000, 0x10000, CRC(6a0db256) SHA1(fa3a2dc03da5bbe06a9c9b3d4ed4fddb47c469ac) )  /* tiles #1 */
	ROM_LOAD( "mh-b-a1.bin",  0x10000, 0x10000, CRC(14ec9795) SHA1(4842e076115efe9daf00dab8f61516d28c19baae) )
	ROM_LOAD( "mh-b-a2.bin",  0x20000, 0x10000, CRC(dfcb510e) SHA1(2387cde4ec0bae176486e1f7541103fd557fe255) )
	ROM_LOAD( "mh-b-a3.bin",  0x30000, 0x10000, CRC(957e329b) SHA1(9d48a0b84915e1cef0b0311a3581991dc83ee199) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "b-b0-.rom",    0x00000, 0x10000, CRC(b5b95776) SHA1(4685b56071b916ce712c45f24da8068dd7e40ed1) )  /* tiles #2 */
	ROM_LOAD( "b-b1-.rom",    0x10000, 0x10000, CRC(74ca16ee) SHA1(7984bc9a0b46e1b4a8ecac7528d57606305aad73) )
	ROM_LOAD( "b-b2-.rom",    0x20000, 0x10000, CRC(b82cca04) SHA1(c12b95be311205181b01d15021bcf9f01ed3e0a3) )
	ROM_LOAD( "b-b3-.rom",    0x30000, 0x10000, CRC(a7afc920) SHA1(92c75463ada39184e731b82ef2883ae6f1f67482) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "c-v0-b.rom",   0x00000, 0x10000, CRC(d0c27e58) SHA1(fec76217cc0c04c723989c3ec127a2bd33d64c60) )
ROM_END

/*

Ninja Spirit
Irem, 1988

This game runs on Irem M72 hardware.


Top Board
---------

M72-C
|-------------------------------|
| 8MHz  J3 NIN_C-L3.6A   J4  J5 |
|     J1   NIN_C-L2.6B NIN-V0.7A|
|8751 J2   NIN_C-L1.6C   J12    |
|          NIN_C-L0.6D   J6     |
|                     NIN-R30.7D|
|      NIN_C-3F.3F              |
|            4364     NIN-R20.7F|
|            4364 J8          J9|
|                   J10         |
| LM358     NIN_C-H0.H6         |
|           NIN_C-H1.J6         |
|----|   MB8431     J11         |
     | CN3                  CN4 |
     |   MB8421       NIN-R10.7J|
     |     NIN_C-H2.6L NIN-R00.7M
     |     NIN_C-H3.6M   J7     |
     |--------------------------|
Notes:
      NIN_C-3F.3F - Ti TBP16L8 PAL
      All other NIN_C* - 27C512 EPROM
      NIN-R* - 28-pin 1Mb maskROM
      J1   - Jumper set to A
      J2   - Jumper set to A
      J3   - Jumper set to A
      J4   - Jumper set to A
      J5   - Jumper set to B
      J6   - Jumper set to A
      J7   - Jumper set to A
      J8   - Open
      J9   - Jumper set to A
      J10  - Jumper set to A
      J11  - Jumper set to B
      J12  - Jumper set to A
      8751 - MCU with label 'NIN C-PR' at location 1C. Clock input 8MHz
      4364 - 8kb x8-bit SRAM
      MB8431 - Fujitsu 2k x 8-bit CMOS Dual-Port SRAM
      MB8421 - Fujitsu 2k x 8-bit CMOS Dual-Port SRAM
      LM358  - Low power dual operational amplifier IC


Middle Board
------------

M72-A-C
|-----------------------------------------------------------|
|M51516L     YM2151                                         |
|VOL         Y3014B                3.579545KHz             |--|
|                                               43256      |  |
|         CN3                                              |  |
|           M72_A-3D.3D  M72_A-4D.4D   D780     43256      |  |
|                                                          |  |
|                                                          |  |
|J   DSW1(8)                                               |  |
|A                 2016                                    |  |
|M   DSW2(8)                               D71011   D71088 |--|
|M                 2016                                     |
|A                                       V30     D71059    |--|
|                  2018   KNA70H016(12)                    |  |
|                                          M72_A-8L.8L     |  |
|                  2018                         M72_A-9L.9L|  |
|                              2018                        |  |
|         KNA71H010(14)        2018           KNA70H015(11)|  |
|                              2018                        |  |
|         CN4                  2018         32MHz          |  |
|         KNA71H009(13)                                    |--|
|KNA71H010(15)                    KNA65005(17)  KNA91H014   |
|-----------------------------------------------------------|
Notes:
      KNA*     - NANAO custom chips
      M72*.*L  - Bipolar PROMs type TBP24S10 (==82S129)
      M72*.*D  - Ti TBP16L8 PALs
      2016     - 2kb x8-bit SRAM
      2018     - 2kb x8-bit SRAM
      43256    - 32kb x8-bit SRAM
      CN3/CN4  - Joining connectors for top board


Bottom Board
------------

M72-B-D
|-----------------------------------------------------------|
|                                                           |
|            NIN_B-A2.4B                                   |--|
|                                                          |  |
| KNA6034201             J2                                |  |
|            NIN_B-A0.4C J3                                |  |
|                                                          |  |
|                                                          |  |
|            NIN_B-A1.4D  4364                             |  |
|                                                          |  |
|            NIN_B-A3.4E  4364                             |--|
|                                                           |
|            B3.4F        4364                             |--|
|                                                          |  |
|            B2.4H        4364                             |  |
|                                                          |  |
| KNA6034201 B0.4J                                         |  |
|                                                          |  |
|                        J4                                |  |
|            B1.4K       J5                                |  |
| KNA91H014                                                |--|
|                                                           |
|-----------------------------------------------------------|
Notes:
      KNA* - NANAO custom chips
      NIN* - 27C512 EPROMs
      B*   - 512kb mask ROMs (no labels, just B and a number)
      4364 - 8kb x8-bit SRAM
      J*   - 3-pin jumpers. All set to the B position
*/

ROM_START( nspirit )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "nin_c-h0.6h", 0x00001, 0x10000, CRC(035692fa) SHA1(d5ab54488344bf405063737ed55d68ff1e64b55f) )
	ROM_LOAD16_BYTE( "nin_c-l0.6d", 0x00000, 0x10000, CRC(9a405898) SHA1(b28d71c1a6410720a37e6b6518b3cc66d4c32972) )
	ROM_LOAD16_BYTE( "nin_c-h1.6j", 0x20001, 0x10000, CRC(cbc10586) SHA1(9b1935ea9ebb21fe42ee3a57d6c10f1e8516f23c) )
	ROM_LOAD16_BYTE( "nin_c-l1.6c", 0x20000, 0x10000, CRC(b75c9a4d) SHA1(03c28896cbe0c9f778c259d59d2e69796902daa8) )
	ROM_LOAD16_BYTE( "nin_c-h2.6l", 0x40001, 0x10000, CRC(8ad818fa) SHA1(dd25e79b656b7fc6c31d1f8971fd0916295ccdb0) )
	ROM_LOAD16_BYTE( "nin_c-l2.6b", 0x40000, 0x10000, CRC(c52ca78c) SHA1(2b40cce5a1f5c588b49634e7fd4bc28c9160fe43) )
	ROM_LOAD16_BYTE( "nin_c-h3.6m", 0x60001, 0x10000, CRC(501104ef) SHA1(e44e060c072affd359e52bf6606b1dd565368d44) )
	ROM_RELOAD(                     0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "nin_c-l3.6a", 0x60000, 0x10000, CRC(fd7408b8) SHA1(3cbe72835a561c50265a047f0f5cd62db48378fd) )
	ROM_RELOAD(                     0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "nin_c-pr.1c", 0x00000, 0x01000, NO_DUMP ) // sldh - read protected

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "nin-r00.7m",  0x00000, 0x20000, CRC(5f61d30b) SHA1(7754697e43f6117fa604f50885b76014b1dc5760) )  /* sprites */
	ROM_LOAD( "nin-r10.7j",  0x20000, 0x20000, CRC(0caad107) SHA1(c4eff00327313e05ac8f7c6dbee3a0de1c83fadd) )
	ROM_LOAD( "nin-r20.7f",  0x40000, 0x20000, CRC(ef3617d3) SHA1(16c175cf45559aacdea6e4002dd8a87f16817cfb) )
	ROM_LOAD( "nin-r30.7d",  0x60000, 0x20000, CRC(175d2a24) SHA1(d1887efd4d8e74c38c53dbbc541ca8d17f29eb59) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "nin_b-a0.4c", 0x00000, 0x10000, CRC(63f8f658) SHA1(82c02d0f7a2d95dfd8d300c46312d511524775ce) )  /* tiles #1 */
	ROM_LOAD( "nin_b-a1.4d", 0x10000, 0x10000, CRC(75eb8306) SHA1(2abc359a0bb2863759a68ed60e730761b9751829) )
	ROM_LOAD( "nin_b-a2.4b", 0x20000, 0x10000, CRC(df532172) SHA1(58b5a79a57e71405b3e1abd41d54cf6a4d12873a) )
	ROM_LOAD( "nin_b-a3.4e", 0x30000, 0x10000, CRC(4dedd64c) SHA1(8a5c73a024d95e6fe3ab70daafcd5b235418ad36) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "b0.4j",       0x00000, 0x10000, CRC(1b0e08a6) SHA1(892686594970c264babbe8673c258929a5e480f6) )  /* tiles #2 */
	ROM_LOAD( "b1.4k",       0x10000, 0x10000, CRC(728727f0) SHA1(2f594c77a847ebee71c9da8a644f83ea2a1313d7) )
	ROM_LOAD( "b2.4h",       0x20000, 0x10000, CRC(f87efd75) SHA1(16474c7ab57b4fbb5cb50799ea6a2326c66706b5) )
	ROM_LOAD( "b3.4f",       0x30000, 0x10000, CRC(98856cb4) SHA1(aa4fbae972d2e827c75650a71ab4ef73a33cd018) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "nin-v0.7a",   0x00000, 0x10000, CRC(a32e8caf) SHA1(63d56ad3a63fb089056e4a170159120287594ea8) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Located on M72-A-C CPU/Sound board */
	ROM_LOAD( "m72_a-8l.8l", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) /* TBP24S10 */
	ROM_LOAD( "m72_a-9l.9l", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) /* TBP24S10 */

	ROM_REGION( 0x0300, "plds", 0 )
	/* Located on M72-ROM-C rom board */
	ROM_LOAD( "nin_c-3f.3f", 0x0000, 0x0100, CRC(5402fc07) SHA1(fa4284710b31be62ab99b0e1d60844db9a8d843e) ) /* TBP16L8 */
	/* Located on M72-A-C CPU/Sound board */
	ROM_LOAD( "m72_a-3d.3d", 0x0100, 0x0100, CRC(de85dac3) SHA1(af83b0325f28fbb1bcc424c1b58ff0f4b49f6b67) ) /* TBP16L8 */
	ROM_LOAD( "m72_a-4d.4d", 0x0200, 0x0100, CRC(59676de1) SHA1(fcf35f5463c14a4b06d58684c47ea9de5216d1da) ) /* TBP16L8 */

ROM_END

ROM_START( nspiritj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c-h0",        0x00001, 0x10000, CRC(8603fab2) SHA1(2c5bc97b6c9648156969b4a9f139081dca19fa24) )
	ROM_LOAD16_BYTE( "c-l0",        0x00000, 0x10000, CRC(e520fa35) SHA1(05f7e5a1a5ada95809ffd941080fb2c2b54363b7) )
	ROM_LOAD16_BYTE( "nin_c-h1.6j", 0x20001, 0x10000, CRC(cbc10586) SHA1(9b1935ea9ebb21fe42ee3a57d6c10f1e8516f23c) )
	ROM_LOAD16_BYTE( "nin_c-l1.6c", 0x20000, 0x10000, CRC(b75c9a4d) SHA1(03c28896cbe0c9f778c259d59d2e69796902daa8) )
	ROM_LOAD16_BYTE( "nin_c-h2.6l", 0x40001, 0x10000, CRC(8ad818fa) SHA1(dd25e79b656b7fc6c31d1f8971fd0916295ccdb0) )
	ROM_LOAD16_BYTE( "nin_c-l2.6b", 0x40000, 0x10000, CRC(c52ca78c) SHA1(2b40cce5a1f5c588b49634e7fd4bc28c9160fe43) )
	ROM_LOAD16_BYTE( "c-h3",        0x60001, 0x10000, CRC(95b63a61) SHA1(bd5ec35fffe6d4898e6712eb6add7c51077b58d2) )
	ROM_RELOAD(                     0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "c-l3",        0x60000, 0x10000, CRC(e754a87a) SHA1(9951d972ed13a0415c827beff122bc7ddb078447) )
	ROM_RELOAD(                     0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "nin_c-pr.1c", 0x00000, 0x01000, CRC(802d440a) SHA1(45b844b831aa6d5d002e3960e17fb5a058b02a29) )  /* checksum correct for Japan version only (see test mode) */

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "nin-r00.7m",  0x00000, 0x20000, CRC(5f61d30b) SHA1(7754697e43f6117fa604f50885b76014b1dc5760) )  /* sprites */
	ROM_LOAD( "nin-r10.7j",  0x20000, 0x20000, CRC(0caad107) SHA1(c4eff00327313e05ac8f7c6dbee3a0de1c83fadd) )
	ROM_LOAD( "nin-r20.7f",  0x40000, 0x20000, CRC(ef3617d3) SHA1(16c175cf45559aacdea6e4002dd8a87f16817cfb) )
	ROM_LOAD( "nin-r30.7d",  0x60000, 0x20000, CRC(175d2a24) SHA1(d1887efd4d8e74c38c53dbbc541ca8d17f29eb59) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "nin_b-a0.4c", 0x00000, 0x10000, CRC(63f8f658) SHA1(82c02d0f7a2d95dfd8d300c46312d511524775ce) )  /* tiles #1 */
	ROM_LOAD( "nin_b-a1.4d", 0x10000, 0x10000, CRC(75eb8306) SHA1(2abc359a0bb2863759a68ed60e730761b9751829) )
	ROM_LOAD( "nin_b-a2.4b", 0x20000, 0x10000, CRC(df532172) SHA1(58b5a79a57e71405b3e1abd41d54cf6a4d12873a) )
	ROM_LOAD( "nin_b-a3.4e", 0x30000, 0x10000, CRC(4dedd64c) SHA1(8a5c73a024d95e6fe3ab70daafcd5b235418ad36) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "b0.4j",       0x00000, 0x10000, CRC(1b0e08a6) SHA1(892686594970c264babbe8673c258929a5e480f6) )  /* tiles #2 */
	ROM_LOAD( "b1.4k",       0x10000, 0x10000, CRC(728727f0) SHA1(2f594c77a847ebee71c9da8a644f83ea2a1313d7) )
	ROM_LOAD( "b2.4h",       0x20000, 0x10000, CRC(f87efd75) SHA1(16474c7ab57b4fbb5cb50799ea6a2326c66706b5) )
	ROM_LOAD( "b3.4f",       0x30000, 0x10000, CRC(98856cb4) SHA1(aa4fbae972d2e827c75650a71ab4ef73a33cd018) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "nin-v0.7a",   0x00000, 0x10000, CRC(a32e8caf) SHA1(63d56ad3a63fb089056e4a170159120287594ea8) )
ROM_END

ROM_START( imgfight )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "if-c-h0-a.bin", 0x00001, 0x10000, CRC(f5c94464) SHA1(5964a00d21ebb358eecc0f10f6221fb684f284df) )
	ROM_LOAD16_BYTE( "if-c-l0-a.bin", 0x00000, 0x10000, CRC(87c534fe) SHA1(10c231a2b3046a711a1fdcc6c1631a7378295f2f) )
	ROM_LOAD16_BYTE( "if-c-h3.bin",   0x40001, 0x20000, CRC(ea030541) SHA1(ee4c12773ecced2d755443ce0ca78fb2b2c04805) )
	ROM_RELOAD(                       0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "if-c-l3.bin",   0x40000, 0x20000, CRC(c66ae348) SHA1(eca5096ebd5bffc6e68f3fc9969cda9679bd921f) )
	ROM_RELOAD(                       0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "imgfight_i8751h.bin",  0x00000, 0x01000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "if-c-00.bin",  0x00000, 0x20000, CRC(745e6638) SHA1(43fb1f9da4190fea67eee3aee8caf4219becc21b) )  /* sprites */
	ROM_LOAD( "if-c-10.bin",  0x20000, 0x20000, CRC(b7108449) SHA1(1f41ebe7164fab86958caaf6749b99425e682657) )
	ROM_LOAD( "if-c-20.bin",  0x40000, 0x20000, CRC(aef33cba) SHA1(2d8a8458207d0c790c81b1285366463c8540d190) )
	ROM_LOAD( "if-c-30.bin",  0x60000, 0x20000, CRC(1f98e695) SHA1(5fddcfb17523f8e96f4b85f0cb15d837b81f2bd4) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "if-a-a0.bin",  0x00000, 0x10000, CRC(34ee2d77) SHA1(38826e0318aa8da893fa4c93f217288c015df606) )  /* tiles #1 */
	ROM_LOAD( "if-a-a1.bin",  0x10000, 0x10000, CRC(6bd2845b) SHA1(149cf14f919590da88b9a8e254690da010709862) )
	ROM_LOAD( "if-a-a2.bin",  0x20000, 0x10000, CRC(090d50e5) SHA1(4f2a7c76320b3f8dafae90a246187e034fe7562b) )
	ROM_LOAD( "if-a-a3.bin",  0x30000, 0x10000, CRC(3a8e3083) SHA1(8a75d556790b6bea41ead1a5f95589dd293bdf4e) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "if-a-b0.bin",  0x00000, 0x10000, CRC(b425c829) SHA1(0ccd487dba00bb7cb0ff5d1c67f8fee3e68df5d8) )  /* tiles #2 */
	ROM_LOAD( "if-a-b1.bin",  0x10000, 0x10000, CRC(e9bfe23e) SHA1(f97a68dbdce7e06d07faab19acf7625cdc8eeaa8) )
	ROM_LOAD( "if-a-b2.bin",  0x20000, 0x10000, CRC(256e50f2) SHA1(9e9fda4f1f1449548942c0da4478f61fe0d263d1) )
	ROM_LOAD( "if-a-b3.bin",  0x30000, 0x10000, CRC(4c682785) SHA1(f61f1227e0ad629fdfca106306b17a9f6a9959e3) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "if-c-v0.bin",  0x00000, 0x10000, CRC(cb64a194) SHA1(940fad6b9147bccc8290e112f5973f8ea062b52f) )
	ROM_LOAD( "if-c-v1.bin",  0x10000, 0x10000, CRC(45b68bf5) SHA1(2fb28793019ca85b3b6d7c4c31eedff1d71f2d83) )
ROM_END

ROM_START( imgfightj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "if-c-h0.bin",  0x00001, 0x10000, CRC(592d2d80) SHA1(d54916a9bfe4b65a972b62202af706135e73518d) )
	ROM_LOAD16_BYTE( "if-c-l0.bin",  0x00000, 0x10000, CRC(61f89056) SHA1(3e0724dbc2b00a30193ea6cfac8b4331055d4fd4) )
	ROM_LOAD16_BYTE( "if-c-h3.bin",  0x40001, 0x20000, CRC(ea030541) SHA1(ee4c12773ecced2d755443ce0ca78fb2b2c04805) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "if-c-l3.bin",  0x40000, 0x20000, CRC(c66ae348) SHA1(eca5096ebd5bffc6e68f3fc9969cda9679bd921f) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "imgfightj_i8751h.bin",  0x00000, 0x01000, CRC(ef0d5098) SHA1(068b73937588e16a318a094dfe2fb1293b1a1711) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "if-c-00.bin",  0x00000, 0x20000, CRC(745e6638) SHA1(43fb1f9da4190fea67eee3aee8caf4219becc21b) )  /* sprites */
	ROM_LOAD( "if-c-10.bin",  0x20000, 0x20000, CRC(b7108449) SHA1(1f41ebe7164fab86958caaf6749b99425e682657) )
	ROM_LOAD( "if-c-20.bin",  0x40000, 0x20000, CRC(aef33cba) SHA1(2d8a8458207d0c790c81b1285366463c8540d190) )
	ROM_LOAD( "if-c-30.bin",  0x60000, 0x20000, CRC(1f98e695) SHA1(5fddcfb17523f8e96f4b85f0cb15d837b81f2bd4) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "if-a-a0.bin",  0x00000, 0x10000, CRC(34ee2d77) SHA1(38826e0318aa8da893fa4c93f217288c015df606) )  /* tiles #1 */
	ROM_LOAD( "if-a-a1.bin",  0x10000, 0x10000, CRC(6bd2845b) SHA1(149cf14f919590da88b9a8e254690da010709862) )
	ROM_LOAD( "if-a-a2.bin",  0x20000, 0x10000, CRC(090d50e5) SHA1(4f2a7c76320b3f8dafae90a246187e034fe7562b) )
	ROM_LOAD( "if-a-a3.bin",  0x30000, 0x10000, CRC(3a8e3083) SHA1(8a75d556790b6bea41ead1a5f95589dd293bdf4e) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "if-a-b0.bin",  0x00000, 0x10000, CRC(b425c829) SHA1(0ccd487dba00bb7cb0ff5d1c67f8fee3e68df5d8) )  /* tiles #2 */
	ROM_LOAD( "if-a-b1.bin",  0x10000, 0x10000, CRC(e9bfe23e) SHA1(f97a68dbdce7e06d07faab19acf7625cdc8eeaa8) )
	ROM_LOAD( "if-a-b2.bin",  0x20000, 0x10000, CRC(256e50f2) SHA1(9e9fda4f1f1449548942c0da4478f61fe0d263d1) )
	ROM_LOAD( "if-a-b3.bin",  0x30000, 0x10000, CRC(4c682785) SHA1(f61f1227e0ad629fdfca106306b17a9f6a9959e3) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "if-c-v0.bin",  0x00000, 0x10000, CRC(cb64a194) SHA1(940fad6b9147bccc8290e112f5973f8ea062b52f) )
	ROM_LOAD( "if-c-v1.bin",  0x10000, 0x10000, CRC(45b68bf5) SHA1(2fb28793019ca85b3b6d7c4c31eedff1d71f2d83) )
ROM_END

ROM_START( loht )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tom_c-h0.rom", 0x00001, 0x20000, CRC(a63204b6) SHA1(d217bc70650a1a1bbe0cf536ec3bb678f670718d) )
	ROM_LOAD16_BYTE( "tom_c-l0.rom", 0x00000, 0x20000, CRC(e788002f) SHA1(35f509976b342fd47e645453381faa3d86645876) )
	ROM_LOAD16_BYTE( "tom_c-h3-",    0x40001, 0x20000, CRC(714778b5) SHA1(e2eaa35d6b5fa5df5163fe0d7b45fa66667f9947) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "tom_c-l3-",    0x40000, 0x20000, CRC(2f049b03) SHA1(21047cb10912b1fc23795673af3ea7de249328b7) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "loht_i8751.mcu",  0x00000, 0x10000, NO_DUMP ) // read protected

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "tom_m53.rom",  0x00000, 0x20000, CRC(0b83265f) SHA1(b31918d6442b79c9fe4f20410189788b050a994e) )  /* sprites */
	ROM_LOAD( "tom_m51.rom",  0x20000, 0x20000, CRC(8ec5f6f3) SHA1(210f2753f5eeb06396758d21ab1778d459add247) )
	ROM_LOAD( "tom_m49.rom",  0x40000, 0x20000, CRC(a41d3bfd) SHA1(536fb7c0321dbbc1a8b73e9647fba9c53a253fcc) )
	ROM_LOAD( "tom_m47.rom",  0x60000, 0x20000, CRC(9d81a25b) SHA1(a354537c2fbba85f06485aa8487d7583a7133357) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "tom_m21.rom",  0x00000, 0x10000, CRC(3ca3e771) SHA1(be052e01c5429ee89057c9d408794f2c7744047c) )  /* tiles #1 */
	ROM_LOAD( "tom_m22.rom",  0x10000, 0x10000, CRC(7a05ee2f) SHA1(7d1ca5db9a5a85610129e3bc6c640ade036fe7f9) )
	ROM_LOAD( "tom_m20.rom",  0x20000, 0x10000, CRC(79aa2335) SHA1(6b70c79d800a7b755aa7c9a368c4ea74029aaa1e) )
	ROM_LOAD( "tom_m23.rom",  0x30000, 0x10000, CRC(789e8b24) SHA1(e957cd25c3c155ca295ab1aea03d610f91562cfb) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "tom_m26.rom",  0x00000, 0x10000, CRC(44626bf6) SHA1(571ef74d42d30a272ff0fb33f830652b4a4bad29) )  /* tiles #2 */
	ROM_LOAD( "tom_m27.rom",  0x10000, 0x10000, CRC(464952cf) SHA1(6b99360b6ba1ed5a72c257f51291f9f7a1ddf363) )
	ROM_LOAD( "tom_m25.rom",  0x20000, 0x10000, CRC(3db9b2c7) SHA1(02a318ffc459c494b7f40827eff5f89b41ac0426) )
	ROM_LOAD( "tom_m24.rom",  0x30000, 0x10000, CRC(f01fe899) SHA1(c5ab967b7af55a757638bcdc9975f4b15064022d) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "tom_m44.rom",  0x00000, 0x10000, CRC(3ed51d1f) SHA1(84f3aa17d640df91387e5f1f5b5971cf8dcd4e17) )
ROM_END

/*

Legend of Hero TONMA
(c)1989 Irem

M72 System
Horizontal Freq. = 15.625KHz
H.Period         = 64.0us
H.Blank          = 16.0us
H.Sync Pulse     = 5.0us
Vertical Freq.   = 55.02Hz
V.Period         = 18.176ms
V.Blank          = 1.792ms
V.Sync Pulse     = 384us

ROMs:
on M72-C mainboard
set jumper pins
J1:A
J2:A
J3:A
J4:A
J5:B
J6:A
J7:A
J9:A
J10:A
J11:B
J12:A

TOM_C-H0- (M5M27C101K, main programs)
TOM_C-L0-
TOM_C-H3-
TOM_C-H0-

R200 (28pin 1Mbit mask, read as 531000)
R210
R220
R230

082 - Samples

TOM_C-PR- (i8751H, read protected, not dumped) <-- Since decapped, deprotected and read
TOM_C-3F- (PAL, read protected, not dumped)


on M72-B-B or M72-B-C or M72-B-D
set jumper pins (J2, J3, J4, J5) to "B"
R2A0.A0 (27C512)
R2A1.A1
R2A2.A2
R2A3.A3

078.B0
079.B1
080.B2
081.B3

*/

ROM_START( lohtj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tom_c-h0-", 0x00001, 0x20000, CRC(2a752998) SHA1(a88c3c75a1106665c94ddd0945bfaa7696a21b75) )
	ROM_LOAD16_BYTE( "tom_c-l0-", 0x00000, 0x20000, CRC(a224d928) SHA1(c4744f6ca19ce60b0c03415be979f2a824235a1c) )
	ROM_LOAD16_BYTE( "tom_c-h3-", 0x40001, 0x20000, CRC(714778b5) SHA1(e2eaa35d6b5fa5df5163fe0d7b45fa66667f9947) )
	ROM_RELOAD(                   0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "tom_c-l3-", 0x40000, 0x20000, CRC(2f049b03) SHA1(21047cb10912b1fc23795673af3ea7de249328b7) )
	ROM_RELOAD(                   0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "tom_c-pr.bin",  0x00000, 0x01000, CRC(9fa9b496) SHA1(b529bcd7bf123894e11f2a8df8826932122e375a) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "r200",     0x00000, 0x20000, CRC(0b83265f) SHA1(b31918d6442b79c9fe4f20410189788b050a994e) )  /* sprites */
	ROM_LOAD( "r210",     0x20000, 0x20000, CRC(8ec5f6f3) SHA1(210f2753f5eeb06396758d21ab1778d459add247) )
	ROM_LOAD( "r220",     0x40000, 0x20000, CRC(a41d3bfd) SHA1(536fb7c0321dbbc1a8b73e9647fba9c53a253fcc) )
	ROM_LOAD( "r230",     0x60000, 0x20000, CRC(9d81a25b) SHA1(a354537c2fbba85f06485aa8487d7583a7133357) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "r2a0.a0",  0x00000, 0x10000, CRC(3ca3e771) SHA1(be052e01c5429ee89057c9d408794f2c7744047c) )  /* tiles #1 */
	ROM_LOAD( "r2a1.a1",  0x10000, 0x10000, CRC(7a05ee2f) SHA1(7d1ca5db9a5a85610129e3bc6c640ade036fe7f9) )
	ROM_LOAD( "r2a2.a2",  0x20000, 0x10000, CRC(79aa2335) SHA1(6b70c79d800a7b755aa7c9a368c4ea74029aaa1e) )
	ROM_LOAD( "r2a3.a3",  0x30000, 0x10000, CRC(789e8b24) SHA1(e957cd25c3c155ca295ab1aea03d610f91562cfb) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "078.b0",   0x00000, 0x10000, CRC(44626bf6) SHA1(571ef74d42d30a272ff0fb33f830652b4a4bad29) )  /* tiles #2 */
	ROM_LOAD( "079.b1",   0x10000, 0x10000, CRC(464952cf) SHA1(6b99360b6ba1ed5a72c257f51291f9f7a1ddf363) )
	ROM_LOAD( "080.b2",   0x20000, 0x10000, CRC(3db9b2c7) SHA1(02a318ffc459c494b7f40827eff5f89b41ac0426) )
	ROM_LOAD( "081.b3",   0x30000, 0x10000, CRC(f01fe899) SHA1(c5ab967b7af55a757638bcdc9975f4b15064022d) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "082",      0x00000, 0x10000, CRC(3ed51d1f) SHA1(84f3aa17d640df91387e5f1f5b5971cf8dcd4e17) )
ROM_END

/*
CPU cpu: nec v30
cpu: z80 (sharp LH0080B z80b-cpu)
cpu: 2x YM2203C

quarzi: 16mhz vicino al v30 e 28mhz vicino allo z80 e ym2203c
*/

ROM_START( lohtb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lohtb03.b", 0x00001, 0x20000, CRC(8b845a70) SHA1(902c623310cd77b25592496745f9c6121149b516) )
	ROM_LOAD16_BYTE( "lohtb05.d", 0x00000, 0x20000, CRC(e90f7623) SHA1(e6e2f66a39286b2d7c03fc267beb2024913fb4ca) )
	ROM_LOAD16_BYTE( "lohtb02.a", 0x40001, 0x20000, CRC(714778b5) SHA1(e2eaa35d6b5fa5df5163fe0d7b45fa66667f9947) )
	ROM_RELOAD(                   0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "lohtb04.c", 0x40000, 0x20000, CRC(2f049b03) SHA1(21047cb10912b1fc23795673af3ea7de249328b7) )
	ROM_RELOAD(                   0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Sound CPU program (Z80) + Samples*/
	ROM_LOAD( "lohtb01.02",  0x00000, 0x10000, CRC(e4bd8f03) SHA1(69fe41a978db92daa912cb345c2c7bafd2a6eb93) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "lohtb14.11",  0x00000, 0x10000, CRC(df5ac5ee) SHA1(5b45417ada402047d97dfb6cee6545686ad26e37) )
	ROM_LOAD( "lohtb15.12",  0x20000, 0x10000, CRC(45220b01) SHA1(83715cf155f91c82067d69f14b3b01ed77777b7d) )
	ROM_LOAD( "lohtb16.13",  0x40000, 0x10000, CRC(25b85cfc) SHA1(c7a9962165379193dc6553ed1f977795a79e0f78) )
	ROM_LOAD( "lohtb17.14",  0x60000, 0x10000, CRC(763fa4ec) SHA1(2d72b1b41f24ae299fde23869942c0b6bbb82363) )
	ROM_LOAD( "lohtb18.15",  0x10000, 0x10000, CRC(d7ecf849) SHA1(ab86a88eae21e054d4e8a740a60c7c6c198232d4) )
	ROM_LOAD( "lohtb19.16",  0x30000, 0x10000, CRC(35d1a808) SHA1(9378ff000104ecfb842b3b884197be82c43a01b4))
	ROM_LOAD( "lohtb20.17",  0x50000, 0x10000, CRC(464d8579) SHA1(b5981f4865ee5439f0e330091927e6d97d29933f) )
	ROM_LOAD( "lohtb21.18",  0x70000, 0x10000, CRC(a73568c7) SHA1(8fe1867256708cc1ed76d1bed5566b1852b47c40) )

	ROM_REGION( 0x040000, "gfx2", ROMREGION_INVERT ) /* tiles #1 */
	ROM_LOAD( "lohtb13.10",  0x00000, 0x10000, CRC(359f17d4) SHA1(2875ba48395e7faa1a58404475be936dcca45ed1) )
	ROM_LOAD( "lohtb11.08",  0x10000, 0x10000, CRC(73391e8a) SHA1(53ca89b8a10895f817ecdb9fa5eef462edb94ae6) )
	ROM_LOAD( "lohtb09.06",  0x20000, 0x10000, CRC(7096d390) SHA1(f4a16bf8aef7a1a65619ab022cbdb67d2f191888) )
	ROM_LOAD( "lohtb07.04",  0x30000, 0x10000, CRC(71a27b81) SHA1(d8fe72d15bbcd5b170d1123d8f4c58874cefdca3) )

	ROM_REGION( 0x040000, "gfx3", ROMREGION_INVERT ) /* tiles #2 */
	ROM_LOAD( "lohtb12.09",  0x00000, 0x10000, CRC(4d5e9b53) SHA1(3e3977bab7a66ed0171afcd555d181960e338749) )
	ROM_LOAD( "lohtb10.07",  0x10000, 0x10000, CRC(4f75a26a) SHA1(79c09a1ad3a6f9cfbd07cb527bbd89d2478ce582) )
	ROM_LOAD( "lohtb08.05",  0x20000, 0x10000, CRC(34854262) SHA1(37436c12579fb41d22a1596b495f065959c14a26) )
	ROM_LOAD( "lohtb06.03",  0x30000, 0x10000, CRC(f923183c) SHA1(a6b578191864aefa81e0cad3ba12a2ca491c91cf) )

	ROM_REGION( 0x10000, "samples", ROMREGION_ERASEFF ) /* -- no sample roms on bootleg, included with z80 code */
ROM_END

ROM_START( lohtb2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "loht-a2.bin",  0x00001, 0x10000, CRC(ccc90e54) SHA1(860da001d9b0782adc25cfc3b453383225253d9e) )
	ROM_LOAD16_BYTE( "loht-a3.bin",  0x20001, 0x10000, CRC(ff8a98de) SHA1(ccb8275241bea81abc01dc36e62557712c1b5a8c) )
	ROM_LOAD16_BYTE( "loht-a10.bin", 0x00000, 0x10000, CRC(3aa06730) SHA1(483b135f8ee0fc54b1953c7c28e909a88aa2fa2e) )
	ROM_LOAD16_BYTE( "loht-a11.bin", 0x20000, 0x10000, CRC(eab1d7bc) SHA1(ec50fe89f05ae46e91b9f2f3d4e4383aa764e71d) )

	ROM_LOAD16_BYTE( "loht-a5.bin",  0x40001, 0x10000, CRC(79e007ec) SHA1(b2e4cc4a47f5f127ba9a1a00eaaf067464314ea0) )
	ROM_RELOAD(                      0xc0001, 0x10000 )
	ROM_LOAD16_BYTE( "loht-a4.bin",  0x60001, 0x10000, CRC(254ea4d5) SHA1(07277bbe2ea6678f0de1f28e40be794880b3faff) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "loht-a13.bin", 0x40000, 0x10000, CRC(b951346e) SHA1(82fa3c4a09a86b74b98c31aaea5c0629ddff83a0) )
	ROM_RELOAD(                      0xc0000, 0x10000 )
	ROM_LOAD16_BYTE( "loht-a12.bin", 0x60000, 0x10000, CRC(cfb0390d) SHA1(4acc61a51a7ae681bd8d835e2644b44c4d6d7bcb) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "loht-a26.bin",  0x00000, 0x02000, CRC(ac901e17) SHA1(70a73288d594c78ad2aca78ce55a699cb040bede) ) // unprotected??

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "loht-a16.bin",  0x00000, 0x10000, CRC(df5ac5ee) SHA1(5b45417ada402047d97dfb6cee6545686ad26e37) )
	ROM_LOAD( "loht-a17.bin",  0x10000, 0x10000, CRC(d7ecf849) SHA1(ab86a88eae21e054d4e8a740a60c7c6c198232d4) )
	ROM_LOAD( "loht-a8.bin",   0x20000, 0x10000, CRC(45220b01) SHA1(83715cf155f91c82067d69f14b3b01ed77777b7d) )
	ROM_LOAD( "loht-a9.bin",   0x30000, 0x10000, CRC(4af9bb3c) SHA1(04f66caae5b3ae985451002293ad8f609a8d9377) )
	ROM_LOAD( "loht-a14.bin",  0x40000, 0x10000, CRC(25b85cfc) SHA1(c7a9962165379193dc6553ed1f977795a79e0f78) )
	ROM_LOAD( "loht-a15.bin",  0x50000, 0x10000, CRC(464d8579) SHA1(b5981f4865ee5439f0e330091927e6d97d29933f) )
	ROM_LOAD( "loht-a6.bin",   0x60000, 0x10000, CRC(763fa4ec) SHA1(2d72b1b41f24ae299fde23869942c0b6bbb82363) )
	ROM_LOAD( "loht-a7.bin",   0x70000, 0x10000, CRC(a73568c7) SHA1(8fe1867256708cc1ed76d1bed5566b1852b47c40) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "loht-a19.bin",  0x00000, 0x10000, CRC(3ca3e771) SHA1(be052e01c5429ee89057c9d408794f2c7744047c) ) /* tiles #1 */
//  ROM_LOAD( "loht-a20.bin",  0x10000, 0x10000, BAD_DUMP CRC(db2e3d77) SHA1(3f0758f74490b084321d8b2da29525dd1f19da09) )
	ROM_LOAD( "loht-a20.bin",  0x10000, 0x10000, CRC(7a05ee2f) SHA1(7d1ca5db9a5a85610129e3bc6c640ade036fe7f9) )
	ROM_LOAD( "loht-a18.bin",  0x20000, 0x10000, CRC(79aa2335) SHA1(6b70c79d800a7b755aa7c9a368c4ea74029aaa1e) )
	ROM_LOAD( "loht-a21.bin",  0x30000, 0x10000, CRC(789e8b24) SHA1(e957cd25c3c155ca295ab1aea03d610f91562cfb) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "loht-a24.bin",  0x00000, 0x10000, CRC(44626bf6) SHA1(571ef74d42d30a272ff0fb33f830652b4a4bad29) ) /* tiles #2 */
	ROM_LOAD( "loht-a25.bin",  0x10000, 0x10000, CRC(464952cf) SHA1(6b99360b6ba1ed5a72c257f51291f9f7a1ddf363) )
	ROM_LOAD( "loht-a23.bin",  0x20000, 0x10000, CRC(3db9b2c7) SHA1(02a318ffc459c494b7f40827eff5f89b41ac0426) )
	ROM_LOAD( "loht-a22.bin",  0x30000, 0x10000, CRC(f01fe899) SHA1(c5ab967b7af55a757638bcdc9975f4b15064022d) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "loht-a1.bin",   0x00000, 0x10000, CRC(3ed51d1f) SHA1(84f3aa17d640df91387e5f1f5b5971cf8dcd4e17) )
ROM_END

ROM_START( xmultipl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "xm-a-h1-.ic58", 0x00001, 0x20000, CRC(449048cf) SHA1(871b588177fb018937d143f76eda18aa53b0f6c4) )
	ROM_LOAD16_BYTE( "xm-a-l1-.ic67", 0x00000, 0x20000, CRC(26ce39b0) SHA1(18ae2e8c2c826c6ecfa66f7af5afdeeac3936543) )
	ROM_LOAD16_BYTE( "xm-a-h0-.ic59", 0x40001, 0x10000, CRC(509bc970) SHA1(44bb4ecedf8f127792e9a8da70b3a42c8ff30ad2) )
	ROM_RELOAD(                       0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "xm-a-l0-.ic68", 0x40000, 0x10000, CRC(490a9ebc) SHA1(55d9d3a4f82f120faabca78c2e47922831f62a5d) )
	ROM_RELOAD(                       0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "xm-a-sp-.ic14", 0x00000, 0x10000, CRC(006eef56) SHA1(917b26b200fa4c1692d4c7ca0ea0f7897e3e3b7b) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "t44.00",        0x00000, 0x20000, CRC(db45186e) SHA1(8c8edeb4b7e6b0516f2597823dc27eba9c5d9528) ) /* sprites */
	ROM_LOAD( "t45.01",        0x20000, 0x20000, CRC(4d0764d4) SHA1(4942333336a110b033f16ac1afa06ffef7b2dad6) )
	ROM_LOAD( "t46.10",        0x40000, 0x20000, CRC(f0c465a4) SHA1(69c107c860d4e8736431fd86b6821b70a8367eb3) )
	ROM_LOAD( "t47.11",        0x60000, 0x20000, CRC(1263b24b) SHA1(0445a5381df3a868bed6967c8e5de7169e4be6a3) )
	ROM_LOAD( "t48.20",        0x80000, 0x20000, CRC(4129944f) SHA1(988b072032d1667c3ac0731fada32fb6978505dc) )
	ROM_LOAD( "t49.21",        0xa0000, 0x20000, CRC(2346e6f9) SHA1(b3de017dd0353e04d279f57e151c47f5fcc70e9c) )
	ROM_LOAD( "t50.30",        0xc0000, 0x20000, CRC(e322543e) SHA1(b4c3a7f202d81485d5f0a7b7668ee89fc1edb215) )
	ROM_LOAD( "t51.31",        0xe0000, 0x20000, CRC(229bf7b1) SHA1(ae42c7efbb6278dd3fa56842361138391f2d49ca) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "t53.a0",       0x00000, 0x20000, CRC(1a082494) SHA1(63a3a84a262833d2cafab41e35df8f10a5e317b1) )  /* tiles #1 */
	ROM_LOAD( "t54.a1",       0x20000, 0x20000, CRC(076c16c5) SHA1(4be858806b916953d59aceee550e721eaf3996a6) )
	ROM_LOAD( "t55.a2",       0x40000, 0x20000, CRC(25d877a5) SHA1(48c948bf714c432f534c098123c8f50d5561756f) )
	ROM_LOAD( "t56.a3",       0x60000, 0x20000, CRC(5b1213f5) SHA1(87782aa0bd04d4378c4ba78b63028ae2709da2f1) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "t57.b0",       0x00000, 0x20000, CRC(0a84e0c7) SHA1(67ad181a7d2c431cb4bf45955e09754549a03576) )  /* tiles #2 */
	ROM_LOAD( "t58.b1",       0x20000, 0x20000, CRC(a874121d) SHA1(1351d5901d55059c6472a4588a2e560396903861) )
	ROM_LOAD( "t59.b2",       0x40000, 0x20000, CRC(69deb990) SHA1(1eed3183efbe576376661b45152a0a21240ecfc8) )
	ROM_LOAD( "t60.b3",       0x60000, 0x20000, CRC(14c69f99) SHA1(4bea72f8bd421ef3ca559363f7473ce2e7038699) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "t52.v0",        0x00000, 0x20000, CRC(2db1bd80) SHA1(657006d0642ec7fb949bb52821d78fe51a599415) )

	ROM_REGION( 0x0200, "proms", 0 )    /* proms */
	ROM_LOAD( "m81_a-9l-.ic72", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) )
	ROM_LOAD( "m81_a-9p-.ic74", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) )
ROM_END

ROM_START( xmultiplm72 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ch3.h3",       0x00001, 0x20000, CRC(20685021) SHA1(92f4216320bf525045223b9454fb5bb224c536d8) )
	ROM_LOAD16_BYTE( "cl3.l3",       0x00000, 0x20000, CRC(93fdd200) SHA1(dd4244ba0ce6c621136b0648374179da44363c01) )
	ROM_LOAD16_BYTE( "ch0.h0",       0x40001, 0x10000, CRC(9438dd8a) SHA1(dc85789c47d31a96300b4236dc43f065e1b01e4a) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "cl0.l0",       0x40000, 0x10000, CRC(06a9e213) SHA1(9831c110814642703d6e71d49848d854095b7d3a) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "xmultipl_i8751h.bin",  0x00000, 0x01000, CRC(c8ceb3cd) SHA1(e5d20a3a9d7f0919604543c97643a03434d80130) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "t44.00",       0x00000, 0x20000, CRC(db45186e) SHA1(8c8edeb4b7e6b0516f2597823dc27eba9c5d9528) )  /* sprites */
	ROM_LOAD( "t45.01",       0x20000, 0x20000, CRC(4d0764d4) SHA1(4942333336a110b033f16ac1afa06ffef7b2dad6) )
	ROM_LOAD( "t46.10",       0x40000, 0x20000, CRC(f0c465a4) SHA1(69c107c860d4e8736431fd86b6821b70a8367eb3) )
	ROM_LOAD( "t47.11",       0x60000, 0x20000, CRC(1263b24b) SHA1(0445a5381df3a868bed6967c8e5de7169e4be6a3) )
	ROM_LOAD( "t48.20",       0x80000, 0x20000, CRC(4129944f) SHA1(988b072032d1667c3ac0731fada32fb6978505dc) )
	ROM_LOAD( "t49.21",       0xa0000, 0x20000, CRC(2346e6f9) SHA1(b3de017dd0353e04d279f57e151c47f5fcc70e9c) )
	ROM_LOAD( "t50.30",       0xc0000, 0x20000, CRC(e322543e) SHA1(b4c3a7f202d81485d5f0a7b7668ee89fc1edb215) )
	ROM_LOAD( "t51.31",       0xe0000, 0x20000, CRC(229bf7b1) SHA1(ae42c7efbb6278dd3fa56842361138391f2d49ca) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "t53.a0",       0x00000, 0x20000, CRC(1a082494) SHA1(63a3a84a262833d2cafab41e35df8f10a5e317b1) )  /* tiles #1 */
	ROM_LOAD( "t54.a1",       0x20000, 0x20000, CRC(076c16c5) SHA1(4be858806b916953d59aceee550e721eaf3996a6) )
	ROM_LOAD( "t55.a2",       0x40000, 0x20000, CRC(25d877a5) SHA1(48c948bf714c432f534c098123c8f50d5561756f) )
	ROM_LOAD( "t56.a3",       0x60000, 0x20000, CRC(5b1213f5) SHA1(87782aa0bd04d4378c4ba78b63028ae2709da2f1) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "t57.b0",       0x00000, 0x20000, CRC(0a84e0c7) SHA1(67ad181a7d2c431cb4bf45955e09754549a03576) )  /* tiles #2 */
	ROM_LOAD( "t58.b1",       0x20000, 0x20000, CRC(a874121d) SHA1(1351d5901d55059c6472a4588a2e560396903861) )
	ROM_LOAD( "t59.b2",       0x40000, 0x20000, CRC(69deb990) SHA1(1eed3183efbe576376661b45152a0a21240ecfc8) )
	ROM_LOAD( "t60.b3",       0x60000, 0x20000, CRC(14c69f99) SHA1(4bea72f8bd421ef3ca559363f7473ce2e7038699) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "t52.v0",       0x00000, 0x20000, CRC(2db1bd80) SHA1(657006d0642ec7fb949bb52821d78fe51a599415) )
ROM_END

ROM_START( dbreed )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "db-a-h0-.59",   0x00001, 0x10000, CRC(e1177267) SHA1(f226f34ce85305870e659dd4f519bee30936af9a) )
	ROM_CONTINUE(                     0x60001, 0x10000 )
	ROM_RELOAD(                       0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "db-a-l0-.68",   0x00000, 0x10000, CRC(d82b167e) SHA1(f9ccb152feb31971230f61371a906bd900ef34e8) )
	ROM_CONTINUE(                     0x60000, 0x10000 )
	ROM_RELOAD(                       0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "db-a-sp-.14", 0x00000, 0x10000, CRC(54a61560) SHA1(e5fccfcedcadbab1667900f98370043c1907dd89) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "db_k800m.00", 0x00000, 0x20000, CRC(c027a8cf) SHA1(534dc416b8f5587168c7f644d3f9438c8a190491) )   /* sprites */
	ROM_LOAD( "db_k801m.10", 0x20000, 0x20000, CRC(093faf33) SHA1(2704f644cdce87daf975984f143b1d55ba731c3f) )
	ROM_LOAD( "db_k802m.20", 0x40000, 0x20000, CRC(055b4c59) SHA1(71315dd7476612f138cb64b905648791d44eb7da) )
	ROM_LOAD( "db_k803m.30", 0x60000, 0x20000, CRC(8ed63922) SHA1(51daa8a23e637f6b4394598ff4a1d26f65b59c8b) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "db_k804m.a0", 0x00000, 0x20000, CRC(4c83e92e) SHA1(6dade027435c48ab48bd4516d16a9961d4dd6fad) )   /* tiles */
	ROM_LOAD( "db_k805m.a1", 0x20000, 0x20000, CRC(835ef268) SHA1(89d0bb15201440dffad3ef745970f95505d7ab03) )
	ROM_LOAD( "db_k806m.a2", 0x40000, 0x20000, CRC(5117f114) SHA1(a401a3e638209b32d4101a5c2e2a8b4612eaa21b) )
	ROM_LOAD( "db_k807m.a3", 0x60000, 0x20000, CRC(8eb0c978) SHA1(7fc55bbe4d0923db88492bb7160a89de34e11cd6) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "db_a-v0.rom", 0x00000, 0x20000, CRC(312f7282) SHA1(742d56980b4618180e9a0e02051c5aec4d5cdae4) )
ROM_END

ROM_START( dbreedm72 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "db_c-h3.rom",  0x00001, 0x20000, CRC(4bf3063c) SHA1(3f970c9ece2ac700738e217e0b31b3aba2848ab2) )
	ROM_LOAD16_BYTE( "db_c-l3.rom",  0x00000, 0x20000, CRC(e4b89b79) SHA1(c312925940633e60fb5d0f05044c6e73e4f7fd54) )
	ROM_LOAD16_BYTE( "db_c-h0.rom",  0x60001, 0x10000, CRC(5aa79fb2) SHA1(b7b862699ddccf90cf18d3822703078668aa1dc7) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "db_c-l0.rom",  0x60000, 0x10000, CRC(ed0f5e06) SHA1(9030840b15e83c18d59c884ed08c93c05fa70c5b) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "dbreedm72_i8751.mcu", 0x00000, 0x10000, NO_DUMP ) // read protected

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "db_k800m.00", 0x00000, 0x20000, CRC(c027a8cf) SHA1(534dc416b8f5587168c7f644d3f9438c8a190491) )   /* sprites */
	ROM_LOAD( "db_k801m.10", 0x20000, 0x20000, CRC(093faf33) SHA1(2704f644cdce87daf975984f143b1d55ba731c3f) )
	ROM_LOAD( "db_k802m.20", 0x40000, 0x20000, CRC(055b4c59) SHA1(71315dd7476612f138cb64b905648791d44eb7da) )
	ROM_LOAD( "db_k803m.30", 0x60000, 0x20000, CRC(8ed63922) SHA1(51daa8a23e637f6b4394598ff4a1d26f65b59c8b) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "db_k804m.a0", 0x00000, 0x20000, CRC(4c83e92e) SHA1(6dade027435c48ab48bd4516d16a9961d4dd6fad) )   /* tiles #1 */
	ROM_LOAD( "db_k805m.a1", 0x20000, 0x20000, CRC(835ef268) SHA1(89d0bb15201440dffad3ef745970f95505d7ab03) )
	ROM_LOAD( "db_k806m.a2", 0x40000, 0x20000, CRC(5117f114) SHA1(a401a3e638209b32d4101a5c2e2a8b4612eaa21b) )
	ROM_LOAD( "db_k807m.a3", 0x60000, 0x20000, CRC(8eb0c978) SHA1(7fc55bbe4d0923db88492bb7160a89de34e11cd6) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "db_k804m.b0", 0x00000, 0x20000, CRC(4c83e92e) SHA1(6dade027435c48ab48bd4516d16a9961d4dd6fad) )   /* tiles #2 */
	ROM_LOAD( "db_k805m.b1", 0x20000, 0x20000, CRC(835ef268) SHA1(89d0bb15201440dffad3ef745970f95505d7ab03) )
	ROM_LOAD( "db_k806m.b2", 0x40000, 0x20000, CRC(5117f114) SHA1(a401a3e638209b32d4101a5c2e2a8b4612eaa21b) )
	ROM_LOAD( "db_k807m.b3", 0x60000, 0x20000, CRC(8eb0c978) SHA1(7fc55bbe4d0923db88492bb7160a89de34e11cd6) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "db_c-v0.rom",  0x00000, 0x20000, CRC(312f7282) SHA1(742d56980b4618180e9a0e02051c5aec4d5cdae4) )
ROM_END

ROM_START( rtype2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rt2-a-h0-d.54", 0x00001, 0x20000, CRC(d8ece6f4) SHA1(f7bb246fe8b75af24716d419bb3c6e7d9cd0971e) )
	ROM_LOAD16_BYTE( "rt2-a-l0-d.60", 0x00000, 0x20000, CRC(32cfb2e4) SHA1(d4b44a40e2933040eddb2b09de7bfe28d76c5f25) )
	ROM_LOAD16_BYTE( "rt2-a-h1-d.53", 0x40001, 0x20000, CRC(4f6e9b15) SHA1(ef733c2615951f54691877ad3e84d08107723324) )
	ROM_RELOAD(                       0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "rt2-a-l1-d.59", 0x40000, 0x20000, CRC(0fd123bf) SHA1(1133163f6716e9a4bbb437b3a471477d0bd97051) )
	ROM_RELOAD(                       0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic17.4f",      0x00000, 0x10000, CRC(73ffecb4) SHA1(4795bf0d6263060c3d3759b659bdb189a4087600) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "ic31.6l",      0x00000, 0x20000, CRC(2cd8f913) SHA1(a53752b35da95b420dd29a09176d265d292b3938) )  /* sprites */
	ROM_LOAD( "ic21.4l",      0x20000, 0x20000, CRC(5033066d) SHA1(e125127f0610c63f9e59a585db547be5d49ed863) )
	ROM_LOAD( "ic32.6m",      0x40000, 0x20000, CRC(ec3a0450) SHA1(632bdd397f1bc67f6970faf7d09ab8d911e105fe) )
	ROM_LOAD( "ic22.4m",      0x60000, 0x20000, CRC(db6176fc) SHA1(1eaf72af0322490c98461aded202288e387caac1) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "ic50.7s",      0x00000, 0x20000, CRC(f3f8736e) SHA1(37872b30459ad05b2981d4ac84983f3b52d0d2d6) )  /* tiles */
	ROM_LOAD( "ic51.7u",      0x20000, 0x20000, CRC(b4c543af) SHA1(56042eba711160fc701021c8787414dcaddcdecb) )
	ROM_LOAD( "ic56.8s",      0x40000, 0x20000, CRC(4cb80d66) SHA1(31c5496c14b277e428a2f22195fe1742d6a577d4) )
	ROM_LOAD( "ic57.8u",      0x60000, 0x20000, CRC(bee128e0) SHA1(b149dae5f8f67a329d6df033fadf50ad75c0a57a) )
	ROM_LOAD( "ic65.9r",      0x80000, 0x20000, CRC(2dc9c71a) SHA1(124e89c17f3af034d5a387ff3eab906d289c27f7) )
	ROM_LOAD( "ic66.9u",      0xa0000, 0x20000, CRC(7533c428) SHA1(ba435cfb6c3c49fcc4d716dcecf8f17545b8eec6) )
	ROM_LOAD( "ic63.9m",      0xc0000, 0x20000, CRC(a6ad67f2) SHA1(b005b037ce8b3c932089982ecfbccdc922278fe3) )
	ROM_LOAD( "ic64.9p",      0xe0000, 0x20000, CRC(3686d555) SHA1(d03754d9b8a6a3bfd4a85eeddacc35a36af197bd) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "ic14.4c",      0x00000, 0x20000, CRC(637172d5) SHA1(9dd0dc409306287238826bf301e2a7a12d6cd9ce) )
ROM_END

ROM_START( rtype2j )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rt2-a-h0.54",  0x00001, 0x20000, CRC(7857ccf6) SHA1(9f6774a8128ee2dbb5b6c42289095275337bc73e) )
	ROM_LOAD16_BYTE( "rt2-a-l0.60",  0x00000, 0x20000, CRC(cb22cd6e) SHA1(a877cffbac9f55bca8932b12540a4686ba975684) )
	ROM_LOAD16_BYTE( "rt2-a-h1.53",  0x40001, 0x20000, CRC(49e75d28) SHA1(956bafaaa6711a8a13f2bffe43e8d05d51d8a3c9) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "rt2-a-l1.59",  0x40000, 0x20000, CRC(12ec1676) SHA1(10cee9a87dd954444b0e64fad7f15a5ae529890d) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic17.4f",      0x00000, 0x10000, CRC(73ffecb4) SHA1(4795bf0d6263060c3d3759b659bdb189a4087600) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "ic31.6l",      0x00000, 0x20000, CRC(2cd8f913) SHA1(a53752b35da95b420dd29a09176d265d292b3938) )  /* sprites */
	ROM_LOAD( "ic21.4l",      0x20000, 0x20000, CRC(5033066d) SHA1(e125127f0610c63f9e59a585db547be5d49ed863) )
	ROM_LOAD( "ic32.6m",      0x40000, 0x20000, CRC(ec3a0450) SHA1(632bdd397f1bc67f6970faf7d09ab8d911e105fe) )
	ROM_LOAD( "ic22.4m",      0x60000, 0x20000, CRC(db6176fc) SHA1(1eaf72af0322490c98461aded202288e387caac1) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "ic50.7s",      0x00000, 0x20000, CRC(f3f8736e) SHA1(37872b30459ad05b2981d4ac84983f3b52d0d2d6) )  /* tiles */
	ROM_LOAD( "ic51.7u",      0x20000, 0x20000, CRC(b4c543af) SHA1(56042eba711160fc701021c8787414dcaddcdecb) )
	ROM_LOAD( "ic56.8s",      0x40000, 0x20000, CRC(4cb80d66) SHA1(31c5496c14b277e428a2f22195fe1742d6a577d4) )
	ROM_LOAD( "ic57.8u",      0x60000, 0x20000, CRC(bee128e0) SHA1(b149dae5f8f67a329d6df033fadf50ad75c0a57a) )
	ROM_LOAD( "ic65.9r",      0x80000, 0x20000, CRC(2dc9c71a) SHA1(124e89c17f3af034d5a387ff3eab906d289c27f7) )
	ROM_LOAD( "ic66.9u",      0xa0000, 0x20000, CRC(7533c428) SHA1(ba435cfb6c3c49fcc4d716dcecf8f17545b8eec6) )
	ROM_LOAD( "ic63.9m",      0xc0000, 0x20000, CRC(a6ad67f2) SHA1(b005b037ce8b3c932089982ecfbccdc922278fe3) )
	ROM_LOAD( "ic64.9p",      0xe0000, 0x20000, CRC(3686d555) SHA1(d03754d9b8a6a3bfd4a85eeddacc35a36af197bd) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "ic14.4c",      0x00000, 0x20000, CRC(637172d5) SHA1(9dd0dc409306287238826bf301e2a7a12d6cd9ce) )
ROM_END

ROM_START( rtype2jc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rt2-a-h0-c.54",  0x00001, 0x20000, CRC(ef9a9990) SHA1(fdf8fb7cb2b16f3cc58592da5c696ccff85f87e0) )
	ROM_LOAD16_BYTE( "rt2-a-l0-c.60",  0x00000, 0x20000, CRC(d8b9da64) SHA1(ce19fc95d0adefa57c2161d7b0d756ecf799c707) )
	ROM_LOAD16_BYTE( "rt2-a-h1-c.53",  0x40001, 0x20000, CRC(1b1870f4) SHA1(9a98b146980a87d088b7157da7a64c99902ddd54) )
	ROM_RELOAD(                        0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "rt2-a-l1-c.59",  0x40000, 0x20000, CRC(60fdff35) SHA1(9c89682deebfa88864b5af9cf0f05944d5c2212f) )
	ROM_RELOAD(                        0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic17.4f",      0x00000, 0x10000, CRC(73ffecb4) SHA1(4795bf0d6263060c3d3759b659bdb189a4087600) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "ic31.6l",      0x00000, 0x20000, CRC(2cd8f913) SHA1(a53752b35da95b420dd29a09176d265d292b3938) )  /* sprites */
	ROM_LOAD( "ic21.4l",      0x20000, 0x20000, CRC(5033066d) SHA1(e125127f0610c63f9e59a585db547be5d49ed863) )
	ROM_LOAD( "ic32.6m",      0x40000, 0x20000, CRC(ec3a0450) SHA1(632bdd397f1bc67f6970faf7d09ab8d911e105fe) )
	ROM_LOAD( "ic22.4m",      0x60000, 0x20000, CRC(db6176fc) SHA1(1eaf72af0322490c98461aded202288e387caac1) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "ic50.7s",      0x00000, 0x20000, CRC(f3f8736e) SHA1(37872b30459ad05b2981d4ac84983f3b52d0d2d6) )  /* tiles */
	ROM_LOAD( "ic51.7u",      0x20000, 0x20000, CRC(b4c543af) SHA1(56042eba711160fc701021c8787414dcaddcdecb) )
	ROM_LOAD( "ic56.8s",      0x40000, 0x20000, CRC(4cb80d66) SHA1(31c5496c14b277e428a2f22195fe1742d6a577d4) )
	ROM_LOAD( "ic57.8u",      0x60000, 0x20000, CRC(bee128e0) SHA1(b149dae5f8f67a329d6df033fadf50ad75c0a57a) )
	ROM_LOAD( "ic65.9r",      0x80000, 0x20000, CRC(2dc9c71a) SHA1(124e89c17f3af034d5a387ff3eab906d289c27f7) )
	ROM_LOAD( "ic66.9u",      0xa0000, 0x20000, CRC(7533c428) SHA1(ba435cfb6c3c49fcc4d716dcecf8f17545b8eec6) )
	ROM_LOAD( "ic63.9m",      0xc0000, 0x20000, CRC(a6ad67f2) SHA1(b005b037ce8b3c932089982ecfbccdc922278fe3) )
	ROM_LOAD( "ic64.9p",      0xe0000, 0x20000, CRC(3686d555) SHA1(d03754d9b8a6a3bfd4a85eeddacc35a36af197bd) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "ic14.4c",      0x00000, 0x20000, CRC(637172d5) SHA1(9dd0dc409306287238826bf301e2a7a12d6cd9ce) )
ROM_END

ROM_START( majtitle )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mt_h0-a.bin",    0x00001, 0x20000, CRC(36aadb67) SHA1(11cb9f190431ef7b68bcad691191c810b00452be) )
	ROM_LOAD16_BYTE( "mt_l0-a.bin",    0x00000, 0x20000, CRC(2e1b6242) SHA1(a1d36c1bad7eb874e6b37a372d0ff95452b09315) )
	ROM_LOAD16_BYTE( "mt_h1-a.bin",    0x40001, 0x20000, CRC(e1402a22) SHA1(97ecf3cf5438dd3aad65554b672a4e401917dc34) )
	ROM_RELOAD(                        0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "mt_l1-a.bin",    0x40000, 0x20000, CRC(0efa409a) SHA1(2b4fd62705398c7ac1647f6ea821f722b4f7496f) )
	ROM_RELOAD(                        0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "mt_sp.bin",    0x00000, 0x10000, CRC(e44260a9) SHA1(a2512033c8cca9a8064eae1ada721202edf06e8e) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mt_n0.bin",    0x00000, 0x40000, CRC(5618cddc) SHA1(16d34b431ab9b72067fa669d694e635c88aeb261) )  /* sprites #1 */
	ROM_LOAD( "mt_n1.bin",    0x40000, 0x40000, CRC(483b873b) SHA1(654efd67b2102521e8c46cd57cefa2cc64cf4fd3) )
	ROM_LOAD( "mt_n2.bin",    0x80000, 0x40000, CRC(4f5d665b) SHA1(f539d0f5c738ffabfac16121706abe3bb3b2a1fa) )
	ROM_LOAD( "mt_n3.bin",    0xc0000, 0x40000, CRC(83571549) SHA1(ce0b89aa4b3e3e1cf6ec6136f956577267cdd9d3) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "mt_c0.bin",    0x00000, 0x20000, CRC(780e7a02) SHA1(9776ecb8b5d86636061f8360464001a63bec0842) )  /* tiles */
	ROM_LOAD( "mt_c1.bin",    0x20000, 0x20000, CRC(45ad1381) SHA1(de281398dcd1c547bde9fa86f8ca409dd8d4aa6c) )
	ROM_LOAD( "mt_c2.bin",    0x40000, 0x20000, CRC(5df5856d) SHA1(f16163f672de6701b411315c9956ddb74c8464ce) )
	ROM_LOAD( "mt_c3.bin",    0x60000, 0x20000, CRC(f5316cc8) SHA1(123892d4a7e8d98582ea736afe659afdba8c5f87) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "mt_f0.bin",    0x00000, 0x20000, CRC(2d5e05d5) SHA1(18bdc9c561dbf0f91642161ca985d2154bd58b5d) )  /* sprites #2 */
	ROM_LOAD( "mt_f1.bin",    0x20000, 0x20000, CRC(c68cd65f) SHA1(8999b558b4af0f453ada9e4ef705163df96844e6) )
	ROM_LOAD( "mt_f2.bin",    0x40000, 0x20000, CRC(a71feb2d) SHA1(47e366b422772bed08ee4d1c338970687d6c3b4c) )
	ROM_LOAD( "mt_f3.bin",    0x60000, 0x20000, CRC(179f7562) SHA1(6d28b199daffc62e8fa9009878ac0bb976ccbb2a) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "mt_vo.bin",    0x00000, 0x20000, CRC(eb24bb2c) SHA1(9fca04fba0249e8213dd164eb6829e1a5acbee65) )
ROM_END

ROM_START( majtitlej )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mt_h0.bin",    0x00001, 0x20000, CRC(b9682c70) SHA1(b979c0a630397f2a2eb73709cf12c5262c973782) )
	ROM_LOAD16_BYTE( "mt_l0.bin",    0x00000, 0x20000, CRC(702c9fd6) SHA1(84a5e9e64f4bf235d115f5648b4a108f710ade1d) )
	ROM_LOAD16_BYTE( "mt_h1.bin",    0x40001, 0x20000, CRC(d9e97c30) SHA1(97f59b614eeeced0a414f8a1693590525a58f788) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "mt_l1.bin",    0x40000, 0x20000, CRC(8dbd91b5) SHA1(2bd01f3fba0fa1ca4b6f8ff57e7dc4434c42ce48) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "mt_sp.bin",    0x00000, 0x10000, CRC(e44260a9) SHA1(a2512033c8cca9a8064eae1ada721202edf06e8e) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mt_n0.bin",    0x00000, 0x40000, CRC(5618cddc) SHA1(16d34b431ab9b72067fa669d694e635c88aeb261) )  /* sprites #1 */
	ROM_LOAD( "mt_n1.bin",    0x40000, 0x40000, CRC(483b873b) SHA1(654efd67b2102521e8c46cd57cefa2cc64cf4fd3) )
	ROM_LOAD( "mt_n2.bin",    0x80000, 0x40000, CRC(4f5d665b) SHA1(f539d0f5c738ffabfac16121706abe3bb3b2a1fa) )
	ROM_LOAD( "mt_n3.bin",    0xc0000, 0x40000, CRC(83571549) SHA1(ce0b89aa4b3e3e1cf6ec6136f956577267cdd9d3) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "mt_c0.bin",    0x00000, 0x20000, CRC(780e7a02) SHA1(9776ecb8b5d86636061f8360464001a63bec0842) )  /* tiles */
	ROM_LOAD( "mt_c1.bin",    0x20000, 0x20000, CRC(45ad1381) SHA1(de281398dcd1c547bde9fa86f8ca409dd8d4aa6c) )
	ROM_LOAD( "mt_c2.bin",    0x40000, 0x20000, CRC(5df5856d) SHA1(f16163f672de6701b411315c9956ddb74c8464ce) )
	ROM_LOAD( "mt_c3.bin",    0x60000, 0x20000, CRC(f5316cc8) SHA1(123892d4a7e8d98582ea736afe659afdba8c5f87) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "mt_f0.bin",    0x00000, 0x20000, CRC(2d5e05d5) SHA1(18bdc9c561dbf0f91642161ca985d2154bd58b5d) )  /* sprites #2 */
	ROM_LOAD( "mt_f1.bin",    0x20000, 0x20000, CRC(c68cd65f) SHA1(8999b558b4af0f453ada9e4ef705163df96844e6) )
	ROM_LOAD( "mt_f2.bin",    0x40000, 0x20000, CRC(a71feb2d) SHA1(47e366b422772bed08ee4d1c338970687d6c3b4c) )
	ROM_LOAD( "mt_f3.bin",    0x60000, 0x20000, CRC(179f7562) SHA1(6d28b199daffc62e8fa9009878ac0bb976ccbb2a) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "mt_vo.bin",    0x00000, 0x20000, CRC(eb24bb2c) SHA1(9fca04fba0249e8213dd164eb6829e1a5acbee65) )
ROM_END

ROM_START( hharry )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a-h0-v.rom",   0x00001, 0x20000, CRC(c52802a5) SHA1(7180189c886aebe8d3e7fd38922916cecfddae32) )
	ROM_LOAD16_BYTE( "a-l0-v.rom",   0x00000, 0x20000, CRC(f463074c) SHA1(aca86345610e65848c276ab278092d35ba215916) )
	ROM_LOAD16_BYTE( "a-h1-0.rom",   0x60001, 0x10000, CRC(3ae21335) SHA1(780d7a0c5bebe4b914ea5b3741e30630f8c29a4f) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "a-l1-0.rom",   0x60000, 0x10000, CRC(bc6ac5f9) SHA1(c6afba4967a8055f6b63827697425eac743f5a75) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "a-sp-0.rom",   0x00000, 0x10000, CRC(80e210e7) SHA1(66cff58fb37c52e1d8e0567e13b774253e862585) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "hh_00.rom",    0x00000, 0x20000, CRC(ec5127ef) SHA1(014ac8ad7b19cd9b475b72a0f42a4991119501c4) )  /* sprites */
	ROM_LOAD( "hh_10.rom",    0x20000, 0x20000, CRC(def65294) SHA1(23f5d99fa9f604fde37cb52113bff233d9be1d25) )
	ROM_LOAD( "hh_20.rom",    0x40000, 0x20000, CRC(bb0d6ad4) SHA1(4ab617fadfc32efad90ed7f0555513f167b0c43a) )
	ROM_LOAD( "hh_30.rom",    0x60000, 0x20000, CRC(4351044e) SHA1(0d3ce3f4f1473fd997e70de91e7b5b5a5ec60ad4) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "hh_a0.rom",    0x00000, 0x20000, CRC(c577ba5f) SHA1(c882e58cf64deca8eee6f14f3df43ecc932488fc) )  /* tiles */
	ROM_LOAD( "hh_a1.rom",    0x20000, 0x20000, CRC(429d12ab) SHA1(ccba25eab981fc4e664f76e06a2964066f2ae2e8) )
	ROM_LOAD( "hh_a2.rom",    0x40000, 0x20000, CRC(b5b163b0) SHA1(82a708fea4953a7c4dcd1d4a1b07f302221ba30b) )
	ROM_LOAD( "hh_a3.rom",    0x60000, 0x20000, CRC(8ef566a1) SHA1(3afb020a7317efe89c18b2a7773894ce28499d49) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "a-v0-0.rom",   0x00000, 0x20000, CRC(faaacaff) SHA1(ea3a3920255c07aa9c0a7e0191eae257a9f7f558) )
ROM_END

ROM_START( hharryu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a-ho-u.8d",    0x00001, 0x20000, CRC(ede7f755) SHA1(adcec83d6b936ab1a14d039792b9375e9f803a08) )
	ROM_LOAD16_BYTE( "a-lo-u.9d",    0x00000, 0x20000, CRC(df0726ae) SHA1(7ef163d2e8c14a14328d4365705bb31540bdc7cb) )
	ROM_LOAD16_BYTE( "a-h1-f.8b",    0x60001, 0x10000, CRC(31b741c5) SHA1(46c1c4cea09477cc4989f3e06e08851d02743e62) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "a-l1-f.9b",    0x60000, 0x10000, CRC(b23e966c) SHA1(f506f6d1f4f7874070e91d1df8f141cca031ce29) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "a-sp-0.rom",   0x00000, 0x10000, CRC(80e210e7) SHA1(66cff58fb37c52e1d8e0567e13b774253e862585) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "hh_00.rom",    0x00000, 0x20000, CRC(ec5127ef) SHA1(014ac8ad7b19cd9b475b72a0f42a4991119501c4) )  /* sprites */
	ROM_LOAD( "hh_10.rom",    0x20000, 0x20000, CRC(def65294) SHA1(23f5d99fa9f604fde37cb52113bff233d9be1d25) )
	ROM_LOAD( "hh_20.rom",    0x40000, 0x20000, CRC(bb0d6ad4) SHA1(4ab617fadfc32efad90ed7f0555513f167b0c43a) )
	ROM_LOAD( "hh_30.rom",    0x60000, 0x20000, CRC(4351044e) SHA1(0d3ce3f4f1473fd997e70de91e7b5b5a5ec60ad4) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "hh_a0.rom",    0x00000, 0x20000, CRC(c577ba5f) SHA1(c882e58cf64deca8eee6f14f3df43ecc932488fc) )  /* tiles */
	ROM_LOAD( "hh_a1.rom",    0x20000, 0x20000, CRC(429d12ab) SHA1(ccba25eab981fc4e664f76e06a2964066f2ae2e8) )
	ROM_LOAD( "hh_a2.rom",    0x40000, 0x20000, CRC(b5b163b0) SHA1(82a708fea4953a7c4dcd1d4a1b07f302221ba30b) )
	ROM_LOAD( "hh_a3.rom",    0x60000, 0x20000, CRC(8ef566a1) SHA1(3afb020a7317efe89c18b2a7773894ce28499d49) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "a-v0-0.rom",   0x00000, 0x20000, CRC(faaacaff) SHA1(ea3a3920255c07aa9c0a7e0191eae257a9f7f558) )
ROM_END

ROM_START( dkgensan )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gen-a-h0.bin", 0x00001, 0x20000, CRC(07a45f6d) SHA1(8ffbd395aad244747d9f87062d2b062f41a4829c) )
	ROM_LOAD16_BYTE( "gen-a-l0.bin", 0x00000, 0x20000, CRC(46478fea) SHA1(fd4ff544588535333c1b98fbc08446ef49b11212) )
	ROM_LOAD16_BYTE( "gen-a-h1.bin", 0x60001, 0x10000, CRC(54e5b73c) SHA1(5664f6e0a931b1c139e82dc98fcc9e38acd14616) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "gen-a-l1.bin", 0x60000, 0x10000, CRC(894f8a9f) SHA1(57a0885c52a094def03b129a450cc891e6c075c6) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "gen-a-sp.bin", 0x00000, 0x10000, CRC(e83cfc2c) SHA1(3193bdd06a9712fc499e6fc90a33140463ef59fe) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "hh_00.rom",    0x00000, 0x20000, CRC(ec5127ef) SHA1(014ac8ad7b19cd9b475b72a0f42a4991119501c4) )  /* sprites */
	ROM_LOAD( "hh_10.rom",    0x20000, 0x20000, CRC(def65294) SHA1(23f5d99fa9f604fde37cb52113bff233d9be1d25) )
	ROM_LOAD( "hh_20.rom",    0x40000, 0x20000, CRC(bb0d6ad4) SHA1(4ab617fadfc32efad90ed7f0555513f167b0c43a) )
	ROM_LOAD( "hh_30.rom",    0x60000, 0x20000, CRC(4351044e) SHA1(0d3ce3f4f1473fd997e70de91e7b5b5a5ec60ad4) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "hh_a0.rom",    0x00000, 0x20000, CRC(c577ba5f) SHA1(c882e58cf64deca8eee6f14f3df43ecc932488fc) )  /* tiles */
	ROM_LOAD( "hh_a1.rom",    0x20000, 0x20000, CRC(429d12ab) SHA1(ccba25eab981fc4e664f76e06a2964066f2ae2e8) )
	ROM_LOAD( "hh_a2.rom",    0x40000, 0x20000, CRC(b5b163b0) SHA1(82a708fea4953a7c4dcd1d4a1b07f302221ba30b) )
	ROM_LOAD( "hh_a3.rom",    0x60000, 0x20000, CRC(8ef566a1) SHA1(3afb020a7317efe89c18b2a7773894ce28499d49) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "gen-vo.bin",   0x00000, 0x20000, CRC(d8595c66) SHA1(97920c9947fbac609fb901415e5471c6e4ca066c) )
ROM_END

ROM_START( dkgensanm72 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ge72-h0.bin",  0x00001, 0x20000, CRC(a0ad992c) SHA1(6de4105d8454c4e4e62762fdd7e22829acc2442b) )
	ROM_LOAD16_BYTE( "ge72-l0.bin",  0x00000, 0x20000, CRC(996396f0) SHA1(1a2501ba46bcbc607f772765e8614bc442154a18) )
	ROM_LOAD16_BYTE( "ge72-h3.bin",  0x60001, 0x10000, CRC(d8b86005) SHA1(dd626cfe50a823066c54cc24d9fdaaf03d61d1e7) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "ge72-l3.bin",  0x60000, 0x10000, CRC(23d303a5) SHA1(b62010f34d71afb590deae458493454f9af38f7c) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "dkgenm72_i8751.mcu",  0x00000, 0x10000, NO_DUMP ) // read protected

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "hh_00.rom",    0x00000, 0x20000, CRC(ec5127ef) SHA1(014ac8ad7b19cd9b475b72a0f42a4991119501c4) )  /* sprites */
	ROM_LOAD( "hh_10.rom",    0x20000, 0x20000, CRC(def65294) SHA1(23f5d99fa9f604fde37cb52113bff233d9be1d25) )
	ROM_LOAD( "hh_20.rom",    0x40000, 0x20000, CRC(bb0d6ad4) SHA1(4ab617fadfc32efad90ed7f0555513f167b0c43a) )
	ROM_LOAD( "hh_30.rom",    0x60000, 0x20000, CRC(4351044e) SHA1(0d3ce3f4f1473fd997e70de91e7b5b5a5ec60ad4) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "ge72b-a0.bin", 0x00000, 0x10000, CRC(f5f56b2a) SHA1(4ef6602052fa70e765d6d7747e672b7108b44f59) )  /* tiles #1 */
	ROM_LOAD( "ge72-a1.bin",  0x10000, 0x10000, CRC(d194ea08) SHA1(0270897049cd256472df42f3dda856ee707535cd) )
	ROM_LOAD( "ge72-a2.bin",  0x20000, 0x10000, CRC(2b06bcc3) SHA1(36378a4a69f3c3da96d2dc8df48916af8de50009) )
	ROM_LOAD( "ge72-a3.bin",  0x30000, 0x10000, CRC(94b96bfa) SHA1(33c1e9045e7a984097f3fe4954b20d954cffbafa) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "ge72-b0.bin",  0x00000, 0x10000, CRC(208796b3) SHA1(38b90732c8d5c77ee84053364a8a7e3daaaabe66) )  /* tiles #2 */
	ROM_LOAD( "ge72-b1.bin",  0x10000, 0x10000, CRC(b4a7f490) SHA1(851b40650fc8920b49f43f9cc6f19e845a25e945) )
	ROM_LOAD( "ge72b-b2.bin", 0x20000, 0x10000, CRC(34fe8f7f) SHA1(fbf8839b26be55ad83ad4db538ba3e196c1ab945) )
	ROM_LOAD( "ge72b-b3.bin", 0x30000, 0x10000, CRC(4b0e92f4) SHA1(16ad9220ca6708028cea18c1c4b57e2b6eb425b4) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "gen-vo.bin",   0x00000, 0x20000, CRC(d8595c66) SHA1(97920c9947fbac609fb901415e5471c6e4ca066c) )
ROM_END

ROM_START( poundfor )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ppa-h0-b.9e",  0x00001, 0x20000, CRC(50d4a2d8) SHA1(7fd62c6613cb58b512c6c3670fa66a5b9906e6a1) )
	ROM_LOAD16_BYTE( "ppa-l0-b.9d",  0x00000, 0x20000, CRC(bd997942) SHA1(da484afe3b79e09e323c768a0b2165e6283971a7) )
	ROM_LOAD16_BYTE( "ppa-h1.9f",    0x40001, 0x20000, CRC(f6c82f48) SHA1(b38a2f9f0f6439b2cf453fec87ca11d959777ee6) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "ppa-l1.9c",    0x40000, 0x20000, CRC(5b07b087) SHA1(04a2403eb8c443cb92b880edc612542acdbcafa4) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ppa-sp.4j",    0x00000, 0x10000, CRC(3f458a5b) SHA1(d73740b2a548bf8a895909da0841f18d9ed32668) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ppb-n0.bin",   0x00000, 0x40000, CRC(951a41f8) SHA1(59b64f63ea2452c2b42ff7ebf1ff6fc4e7879ce3) )  /* sprites */
	ROM_LOAD( "ppb-n1.bin",   0x40000, 0x40000, CRC(c609b7f2) SHA1(1da3550c7e4d2a26d75d143934680d9177ba5c35) )
	ROM_LOAD( "ppb-n2.bin",   0x80000, 0x40000, CRC(318c0b5f) SHA1(1d4cd17dc2f8fc4e523eaf679f21d83e1bfade4e) )
	ROM_LOAD( "ppb-n3.bin",   0xc0000, 0x40000, CRC(93dc9490) SHA1(3df4d57a7bf19443f5aa6a416bcee968f81d9059) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "ppa-g00.bin",  0x00000, 0x20000, CRC(8a88a174) SHA1(d360b9014aec31960538ee488894496248a820dc) )  /* tiles */
	ROM_LOAD( "ppa-g10.bin",  0x20000, 0x20000, CRC(e48a66ac) SHA1(49b33db6a922d6f1d1417e28714a67431b7c0217) )
	ROM_LOAD( "ppa-g20.bin",  0x40000, 0x20000, CRC(12b93e79) SHA1(f3d2b76a30874827c8998c1d13a55a3990b699b7) )
	ROM_LOAD( "ppa-g30.bin",  0x60000, 0x20000, CRC(faa39aee) SHA1(9cc1a468b304437766c04189054d3b8f7ff1f958) )

	ROM_REGION( 0x40000, "samples", 0 ) /* samples */
	ROM_LOAD( "ppa-v0.bin",   0x00000, 0x40000, CRC(03321664) SHA1(51f2b2b712385c1cd55fd069829efac01838d603) )
ROM_END

ROM_START( poundforj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ppa-h0-.9e",  0x00001, 0x20000, CRC(f0165e3b) SHA1(a0482b34c0d05d8f48d1b16f2bc2d5d9ec465dc8) )
	ROM_LOAD16_BYTE( "ppa-l0-.9d",  0x00000, 0x20000, CRC(f954f99f) SHA1(6e7a9718dc63e595403bfc0f1ceae4a71dc75133) )
	ROM_LOAD16_BYTE( "ppa-h1.9f",   0x40001, 0x20000, CRC(f6c82f48) SHA1(b38a2f9f0f6439b2cf453fec87ca11d959777ee6) )
	ROM_RELOAD(                     0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "ppa-l1.9c",   0x40000, 0x20000, CRC(5b07b087) SHA1(04a2403eb8c443cb92b880edc612542acdbcafa4) )
	ROM_RELOAD(                     0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ppa-sp.4j",    0x00000, 0x10000, CRC(3f458a5b) SHA1(d73740b2a548bf8a895909da0841f18d9ed32668) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ppb-n0.bin",   0x00000, 0x40000, CRC(951a41f8) SHA1(59b64f63ea2452c2b42ff7ebf1ff6fc4e7879ce3) )  /* sprites */
	ROM_LOAD( "ppb-n1.bin",   0x40000, 0x40000, CRC(c609b7f2) SHA1(1da3550c7e4d2a26d75d143934680d9177ba5c35) )
	ROM_LOAD( "ppb-n2.bin",   0x80000, 0x40000, CRC(318c0b5f) SHA1(1d4cd17dc2f8fc4e523eaf679f21d83e1bfade4e) )
	ROM_LOAD( "ppb-n3.bin",   0xc0000, 0x40000, CRC(93dc9490) SHA1(3df4d57a7bf19443f5aa6a416bcee968f81d9059) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "ppa-g00.bin",  0x00000, 0x20000, CRC(8a88a174) SHA1(d360b9014aec31960538ee488894496248a820dc) )  /* tiles */
	ROM_LOAD( "ppa-g10.bin",  0x20000, 0x20000, CRC(e48a66ac) SHA1(49b33db6a922d6f1d1417e28714a67431b7c0217) )
	ROM_LOAD( "ppa-g20.bin",  0x40000, 0x20000, CRC(12b93e79) SHA1(f3d2b76a30874827c8998c1d13a55a3990b699b7) )
	ROM_LOAD( "ppa-g30.bin",  0x60000, 0x20000, CRC(faa39aee) SHA1(9cc1a468b304437766c04189054d3b8f7ff1f958) )

	ROM_REGION( 0x40000, "samples", 0 ) /* samples */
	ROM_LOAD( "ppa-v0.bin",   0x00000, 0x40000, CRC(03321664) SHA1(51f2b2b712385c1cd55fd069829efac01838d603) )
ROM_END

ROM_START( poundforu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ppa-ho-a.9e",  0x00001, 0x20000, CRC(ff4c83a4) SHA1(1b7791c784bf7c4774e3200b76d65ab0bf0ff93b) )
	ROM_LOAD16_BYTE( "ppa-lo-a.9d",  0x00000, 0x20000, CRC(3374ce8f) SHA1(7455f8339aeed0ef3d0567baa804b62ca3615283) )
	ROM_LOAD16_BYTE( "ppa-h1.9f",    0x40001, 0x20000, CRC(f6c82f48) SHA1(b38a2f9f0f6439b2cf453fec87ca11d959777ee6) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "ppa-l1.9c",    0x40000, 0x20000, CRC(5b07b087) SHA1(04a2403eb8c443cb92b880edc612542acdbcafa4) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ppa-sp.4j",    0x00000, 0x10000, CRC(3f458a5b) SHA1(d73740b2a548bf8a895909da0841f18d9ed32668) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ppb-n0.bin",   0x00000, 0x40000, CRC(951a41f8) SHA1(59b64f63ea2452c2b42ff7ebf1ff6fc4e7879ce3) )  /* sprites */
	ROM_LOAD( "ppb-n1.bin",   0x40000, 0x40000, CRC(c609b7f2) SHA1(1da3550c7e4d2a26d75d143934680d9177ba5c35) )
	ROM_LOAD( "ppb-n2.bin",   0x80000, 0x40000, CRC(318c0b5f) SHA1(1d4cd17dc2f8fc4e523eaf679f21d83e1bfade4e) )
	ROM_LOAD( "ppb-n3.bin",   0xc0000, 0x40000, CRC(93dc9490) SHA1(3df4d57a7bf19443f5aa6a416bcee968f81d9059) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "ppa-g00.bin",  0x00000, 0x20000, CRC(8a88a174) SHA1(d360b9014aec31960538ee488894496248a820dc) )  /* tiles */
	ROM_LOAD( "ppa-g10.bin",  0x20000, 0x20000, CRC(e48a66ac) SHA1(49b33db6a922d6f1d1417e28714a67431b7c0217) )
	ROM_LOAD( "ppa-g20.bin",  0x40000, 0x20000, CRC(12b93e79) SHA1(f3d2b76a30874827c8998c1d13a55a3990b699b7) )
	ROM_LOAD( "ppa-g30.bin",  0x60000, 0x20000, CRC(faa39aee) SHA1(9cc1a468b304437766c04189054d3b8f7ff1f958) )

	ROM_REGION( 0x40000, "samples", 0 ) /* samples */
	ROM_LOAD( "ppa-v0.bin",   0x00000, 0x40000, CRC(03321664) SHA1(51f2b2b712385c1cd55fd069829efac01838d603) )
ROM_END

ROM_START( airduel )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ad-c-h0.bin",  0x00001, 0x20000, CRC(12140276) SHA1(f218c5f2e6795b6295dea064817d7d6b1a7762b6) )
	ROM_LOAD16_BYTE( "ad-c-l0.bin",  0x00000, 0x20000, CRC(4ac0b91d) SHA1(97e2f633181cd5c25927fd0e2988af2acdb3f388) )
	ROM_LOAD16_BYTE( "ad-c-h3.bin",  0x40001, 0x20000, CRC(9f7cfca3) SHA1(becf827aa7749c54f1c435ea224e1fd9c8b3f5f9) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "ad-c-l3.bin",  0x40000, 0x20000, CRC(9dd343f7) SHA1(9f499936b6d3807aa5b5c18e9811c73c9a2c99f9) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "airduel_i8751.mcu",  0x00000, 0x10000, NO_DUMP ) // read protected

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "ad-00.bin",    0x00000, 0x20000, CRC(2f0d599b) SHA1(a966f806b5e25bb98cc63c46c49e0e676a62afcf) )  /* sprites */
	ROM_LOAD( "ad-10.bin",    0x20000, 0x20000, CRC(9865856b) SHA1(b18a06899ae29d45e2351594df544220f3f4485a) )
	ROM_LOAD( "ad-20.bin",    0x40000, 0x20000, CRC(d392aef2) SHA1(0f639a07066cadddc3884eb490885a8745571567) )
	ROM_LOAD( "ad-30.bin",    0x60000, 0x20000, CRC(923240c3) SHA1(f587a83329087a715a3e42110f74f104e8c8ef1f) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "ad-a0.bin",    0x00000, 0x20000, CRC(ce134b47) SHA1(841358cc222c81b8a91edc262f355310d50b4dbb) )  /* tiles #1 */
	ROM_LOAD( "ad-a1.bin",    0x20000, 0x20000, CRC(097fd853) SHA1(8e08f4f4a747c899bb8e21b347635e26af9edc2d) )
	ROM_LOAD( "ad-a2.bin",    0x40000, 0x20000, CRC(6a94c1b9) SHA1(55174acbac54236e5fc1b80d120cd6da9fe5524c) )
	ROM_LOAD( "ad-a3.bin",    0x60000, 0x20000, CRC(6637c349) SHA1(27cb7c89ab73292b43f8ae3c0d803a01ef3d3936) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "ad-b0.bin",    0x00000, 0x20000, CRC(ce134b47) SHA1(841358cc222c81b8a91edc262f355310d50b4dbb) )  /* tiles #2 */
	ROM_LOAD( "ad-b1.bin",    0x20000, 0x20000, CRC(097fd853) SHA1(8e08f4f4a747c899bb8e21b347635e26af9edc2d) )
	ROM_LOAD( "ad-b2.bin",    0x40000, 0x20000, CRC(6a94c1b9) SHA1(55174acbac54236e5fc1b80d120cd6da9fe5524c) )
	ROM_LOAD( "ad-b3.bin",    0x60000, 0x20000, CRC(6637c349) SHA1(27cb7c89ab73292b43f8ae3c0d803a01ef3d3936) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "ad-v0.bin",    0x00000, 0x20000, CRC(339f474d) SHA1(a81bb52598a0e31b2ed6a538755237c5d14d1844) )
ROM_END

ROM_START( cosmccop )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cc-d-h0b.bin", 0x00001, 0x40000, CRC(38958b01) SHA1(7d7e217742e33a1fe096adf5bbc93d63ddcfb375) )
	ROM_RELOAD(                      0x80001, 0x40000 )
	ROM_LOAD16_BYTE( "cc-d-l0b.bin", 0x00000, 0x40000, CRC(eff87f70) SHA1(61f49b8738cf31546d4182680b761705274b01bf) )
	ROM_RELOAD(                      0x80000, 0x40000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cc-d-sp.bin", 0x00000, 0x10000, CRC(3e3ace60) SHA1(d89b1b84de2887598bb7bcb17b1df1ec8d1862a9) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "cc-c-00.bin", 0x00000, 0x20000, CRC(9d99deaa) SHA1(acf16bea0f482306107d2a305c568406b6c21e9a) )   // cc-b-n0
	ROM_LOAD( "cc-c-10.bin", 0x20000, 0x20000, CRC(7eb083ed) SHA1(31fa7d532fd46e861c3d19d5b08661653f685a49) )   // cc-b-n1
	ROM_LOAD( "cc-c-20.bin", 0x40000, 0x20000, CRC(9421489e) SHA1(e43d042bf8b4ebed93558d74ec479ec60a01ca5c) )   // cc-b-n2
	ROM_LOAD( "cc-c-30.bin", 0x60000, 0x20000, CRC(920ec735) SHA1(2d0949b43dddce7317c45910d6e4868ddf010806) )   // cc-b-n3

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "cc-d-g00.bin", 0x00000, 0x20000, CRC(e7f3d772) SHA1(c7f0bc42e8dde7bae334c7974c3d0ddba3856144) ) /* tiles */
	ROM_LOAD( "cc-d-g10.bin", 0x20000, 0x20000, CRC(418b4e4c) SHA1(1191f12741ee7a360240f706534c9c83be8d5c2d) )
	ROM_LOAD( "cc-d-g20.bin", 0x40000, 0x20000, CRC(a4b558eb) SHA1(0babf725de0065dbeca73fa170bd33565305d129) )
	ROM_LOAD( "cc-d-g30.bin", 0x60000, 0x20000, CRC(f64a3166) SHA1(1661db2a37c76e6b4552e48c04966dbbccab8926) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "cc-c-v0.bin", 0x00000, 0x20000, CRC(6247bade) SHA1(4bc9f86acd09908c74b1ab0e7817c4ff1cad6f0b) )   // cc-d-v0
ROM_END

ROM_START( gallop )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cc-c-h0.bin",  0x00001, 0x20000, CRC(2217dcd0) SHA1(9485b6c3eec99e720439e69dcbe0e55798bbff1c) )
	ROM_LOAD16_BYTE( "cc-c-l0.bin",  0x00000, 0x20000, CRC(ff39d7fb) SHA1(fad95f76050fce04464268b5edff6622b2cb798f) )
	ROM_LOAD16_BYTE( "cc-c-h3.bin",  0x40001, 0x20000, CRC(9b2bbab9) SHA1(255d4dda55be667f5f1f4324e9e66111738e79b3) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "cc-c-l3.bin",  0x40000, 0x20000, CRC(acd3278e) SHA1(83d7ddfbdb4bc9548a179b728351a21b3b0ac134) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "gallop_i8751.mcu",  0x00000, 0x10000, NO_DUMP ) // read protected (only used for sample triggering, not supplying code / warning screens)

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "cc-c-00.bin",  0x00000, 0x20000, CRC(9d99deaa) SHA1(acf16bea0f482306107d2a305c568406b6c21e9a) )  /* sprites */
	ROM_LOAD( "cc-c-10.bin",  0x20000, 0x20000, CRC(7eb083ed) SHA1(31fa7d532fd46e861c3d19d5b08661653f685a49) )
	ROM_LOAD( "cc-c-20.bin",  0x40000, 0x20000, CRC(9421489e) SHA1(e43d042bf8b4ebed93558d74ec479ec60a01ca5c) )
	ROM_LOAD( "cc-c-30.bin",  0x60000, 0x20000, CRC(920ec735) SHA1(2d0949b43dddce7317c45910d6e4868ddf010806) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "cc-b-a0.bin",  0x00000, 0x10000, CRC(a33472bd) SHA1(962047fe3dd1fb996285ecef615a8ebdb529adef) )  /* tiles #1 */
	ROM_LOAD( "cc-b-a1.bin",  0x10000, 0x10000, CRC(118b1f2d) SHA1(7413ccc67a8aa9dae156e6ee122b1ca5beeb9a76) )
	ROM_LOAD( "cc-b-a2.bin",  0x20000, 0x10000, CRC(83cebf48) SHA1(12847827ecbf6b493eb9dbddd0a469729d87a451) )
	ROM_LOAD( "cc-b-a3.bin",  0x30000, 0x10000, CRC(572903fc) SHA1(03305301bcf939e97044e746594736b1ca1d7c0a) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "cc-b-b0.bin",  0x00000, 0x10000, CRC(0df5b439) SHA1(0775cf92139a111542c8b5f940da0f7f43020982) )  /* tiles #2 */
	ROM_LOAD( "cc-b-b1.bin",  0x10000, 0x10000, CRC(010b778f) SHA1(cc5bfeb0fbe0ed2fe513458c5785ec0ce5b02f53) )
	ROM_LOAD( "cc-b-b2.bin",  0x20000, 0x10000, CRC(bda9f6fb) SHA1(a6b655ae5bff0568c1fb56ee8a3874fc6524052c) )
	ROM_LOAD( "cc-b-b3.bin",  0x30000, 0x10000, CRC(d361ba3f) SHA1(7348fdae03e997e05187a2726eb221edb92553df) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "cc-c-v0.bin",  0x00000, 0x20000, CRC(6247bade) SHA1(4bc9f86acd09908c74b1ab0e7817c4ff1cad6f0b) )
ROM_END

ROM_START( kengo )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ken_d-h0.rom", 0x00001, 0x20000, CRC(f4ddeea5) SHA1(bcf016e40886e11c171f2f50de39ac0d8cabcdd1) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "ken_d-l0.rom", 0x00000, 0x20000, CRC(04dc0f81) SHA1(b296529f0bc26d53b344449dfa5a08eca70f30d8) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ken_d-sp.rom", 0x00000, 0x10000, CRC(233ca1cf) SHA1(4ebb6162773bd586a10016ccd77998a9b880f474) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "ken_m31.rom",  0x00000, 0x20000, CRC(e00b95a6) SHA1(6efcd8d58f8ebe3a42c60a0aa790b42c0e132777) )  /* sprites */
	ROM_LOAD( "ken_m21.rom",  0x20000, 0x20000, CRC(d7722f87) SHA1(8606a53b8630934d2b5dfc986bd92ac4142f67e2) )
	ROM_LOAD( "ken_m32.rom",  0x40000, 0x20000, CRC(30a844c4) SHA1(72b2caba3ee7a229ca56f004516dea8d3f0a7ba6) )
	ROM_LOAD( "ken_m22.rom",  0x60000, 0x20000, CRC(a00dac85) SHA1(0c1ed852795046926f62843f6b256cbeecf9ebcf) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "ken_m51.rom",  0x00000, 0x20000, CRC(1646cf4f) SHA1(d240cb2bad3e766128e8e40aa7b1bf4f3b9a5559) )  /* tiles */
	ROM_LOAD( "ken_m57.rom",  0x20000, 0x20000, CRC(a9f88d90) SHA1(c8d4a96fe55fed4b7499550f3c74b03d10306757) )
	ROM_LOAD( "ken_m66.rom",  0x40000, 0x20000, CRC(e9d17645) SHA1(fbe18d6691686a1c458d4a91169c9850698b5ca7) )
	ROM_LOAD( "ken_m64.rom",  0x60000, 0x20000, CRC(df46709b) SHA1(e7c2cd752e765bf7b8ff24637305d61031ce0baa) )

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "ken_m14.rom",  0x00000, 0x20000, CRC(6651e9b7) SHA1(c42009f986c9a9f35732d5cd717d548536469b1c) )
ROM_END

// in the case of the i8751 protected games the World and Japan sets should be using different MCU roms.
// the MCU roms provide checksum information needed for the sets, so using the wrong ROM will result in
// the program roms failing their tests.  This is why we still have simulation code for many games
// despite having Japanese version MCU roms for several of them.  See notes next to the sets

GAME( 1987, rtype,       0,        rtype,       rtype,    driver_device, 0,           ROT0,   "Irem", "R-Type (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, rtypej,      rtype,    rtype,       rtype,    driver_device, 0,           ROT0,   "Irem", "R-Type (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, rtypejp,     rtype,    rtype,       rtypep,   driver_device, 0,           ROT0,   "Irem", "R-Type (Japan prototype)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, rtypeu,      rtype,    rtype,       rtype,    driver_device, 0,           ROT0,   "Irem (Nintendo of America license)", "R-Type (US)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, rtypeb,      rtype,    rtype,       rtype,    driver_device, 0,           ROT0,   "bootleg", "R-Type (World bootleg)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1987, bchopper,    0,        m72,         bchopper, m72_state,     bchopper,    ROT0,   "Irem", "Battle Chopper", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, mrheli,      bchopper, m72_8751,    bchopper, m72_state,     m72_8751,    ROT0,   "Irem", "Mr. HELI no Daibouken (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1988, nspirit,     0,        m72,         nspirit,  m72_state,     nspirit,     ROT0,   "Irem", "Ninja Spirit", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )                 // doesn't wait / check for japan warning string.. fails rom check if used with japanese mcu rom (World version?)
GAME( 1988, nspiritj,    nspirit,  m72_8751,    nspirit,  m72_state,     m72_8751,    ROT0,   "Irem", "Saigo no Nindou (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )      // waits for japan warning screen, works with our mcu dump, corrupt warning screen due to priority / mixing errors (Japan Version)

GAME( 1988, imgfight,    0,        m72,         imgfight, m72_state,     imgfight,    ROT270, "Irem", "Image Fight (World, revision A)", MACHINE_SUPPORTS_SAVE )             // doesn't wait / check for japan warning string.. fails rom check if used with japanese mcu rom (World version?)
GAME( 1988, imgfightj,   imgfight, m72_8751,    imgfight, m72_state,     m72_8751,    ROT270, "Irem", "Image Fight (Japan)", MACHINE_SUPPORTS_SAVE )                         // waits for japan warning screen, works with our mcu dump, can't actually see warning screen due to priority / mixing errors, check tilemap viewer (Japan Version)

GAME( 1989, loht,        0,        m72,         loht,     m72_state,     loht,        ROT0,   "Irem", "Legend of Hero Tonma", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )         // fails rom check if used with Japan MCU rom (World version?)
GAME( 1989, lohtj,       loht,     m72_8751,    loht,     m72_state,     m72_8751,    ROT0,   "Irem", "Legend of Hero Tonma (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // waits for japan warning screen, works with our mcu dump (Japan Version)
GAME( 1989, lohtb2,      loht,     m72_8751,    loht,     m72_state,     m72_8751,    ROT0,   "bootleg", "Legend of Hero Tonma (Japan, bootleg with i8751)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // works like above, mcu code is the same as the real code, probably just an alt revision on a bootleg board
GAME( 1989, lohtb,       loht,     m72,         loht,     driver_device, 0,           ROT0,   "bootleg", "Legend of Hero Tonma (unprotected bootleg)", MACHINE_NOT_WORKING| MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1989, xmultipl,    0,        xmultipl,    xmultipl, driver_device, 0,           ROT0,   "Irem", "X Multiply (World, M81)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, xmultiplm72, xmultipl, xmultiplm72, xmultipl, m72_state,     m72_8751,    ROT0,   "Irem", "X Multiply (Japan, M72)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1989, dbreed,      0,        dbreed,      dbreed,   driver_device, 0,           ROT0,   "Irem", "Dragon Breed (M81 PCB version)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, dbreedm72,   dbreed,   dbreedm72,   dbreed,   m72_state,     dbreedm72,   ROT0,   "Irem", "Dragon Breed (M72 PCB version)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1989, rtype2,      0,        rtype2,      rtype2,   driver_device, 0,           ROT0,   "Irem", "R-Type II", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, rtype2j,     rtype2,   rtype2,      rtype2,   driver_device, 0,           ROT0,   "Irem", "R-Type II (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, rtype2jc,    rtype2,   rtype2,      rtype2,   driver_device, 0,           ROT0,   "Irem", "R-Type II (Japan, revision C)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1990, majtitle,    0,        majtitle,    rtype2,   driver_device, 0,           ROT0,   "Irem", "Major Title (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, majtitlej,   majtitle, majtitle,    rtype2,   driver_device, 0,           ROT0,   "Irem", "Major Title (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1990, hharry,      0,        hharry,      hharry,   driver_device, 0,           ROT0,   "Irem", "Hammerin' Harry (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, hharryu,     hharry,   hharryu,     hharry,   driver_device, 0,           ROT0,   "Irem America", "Hammerin' Harry (US)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, dkgensan,    hharry,   hharryu,     hharry,   driver_device, 0,           ROT0,   "Irem", "Daiku no Gensan (Japan, M82)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, dkgensanm72, hharry,   dkgenm72,    hharry,   m72_state,     dkgenm72,    ROT0,   "Irem", "Daiku no Gensan (Japan, M72)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1990, poundfor,    0,        poundfor,    poundfor, driver_device, 0,           ROT270, "Irem", "Pound for Pound (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, poundforj,   poundfor, poundfor,    poundfor, driver_device, 0,           ROT270, "Irem", "Pound for Pound (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, poundforu,   poundfor, poundfor,    poundfor, driver_device, 0,           ROT270, "Irem America", "Pound for Pound (US)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1990, airduel,     0,        m72,         airduel,  m72_state,     airduel,     ROT270, "Irem", "Air Duel (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, cosmccop,    0,        cosmccop,    gallop,   driver_device, 0,           ROT0,   "Irem", "Cosmic Cop (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, gallop,      cosmccop, m72,         gallop,   m72_state,     gallop,      ROT0,   "Irem", "Gallop - Armed Police Unit (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1991, kengo,       0,        kengo,       kengo,    driver_device, 0,           ROT0,   "Irem", "Ken-Go", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
