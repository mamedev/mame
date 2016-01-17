// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Konami FireBeat

    Driver by Ville Linde



    Hardware overview:

    GQ972 PWB(A2) 0000070609 Main board
    -----------------------------------
        OSC 64.00MHz
        IBM PowerPC 403GCX at 64MHz
        (2x) Konami 0000057714 (2D object processor)
        Yamaha YMZ280B (ADPCM sound chip)
        Epson RTC65271 RTC/NVRAM
        National Semiconductor PC16552DV (dual UART)

    GQ974 PWB(A2) 0000070140 Extend board
    -------------------------------------
        ADC0808CCN
        FDC37C665GT (floppy disk controller)
        National Semiconductor PC16552DV (dual UART)
        ADM223LJR (RS-232 driver/receiver)

    GQ971 PWB(A2) 0000070254 SPU
    ----------------------------
        Motorola MC68HC000FN16 at 16MHz (?)
        Xilinx XC9572 CPLD
        Ricoh RF5c400 sound chip

    GQ971 PWB(B2) 0000067784 Backplane
    ----------------------------------
        3x PCB Slots with 2x DIN96S connectors (for main and extend PCBs)
        40-pin ATA connector for each slot

    GQ986 PWB(A1) 0000073015 Backplane
    ----------------------------------
        2x PCB Slots with 2x DIN96S connectors (for main and extend PCBs)
        40-pin ATA connector for each slot

    GQ972 PWB(D2) Controller interface on Beatmania III (?)
    GQ972 PWB(G1) Sound Amp (?)
    GQ971 PWB(C) Sound Amp


    Hardware configurations:

    Beatmania III
    -------------
        GQ971 Backplane
        GQ972 Main Board
        GQ974 Extend Board
        GQ971 SPU
        GQ972 Controller Interface
        GQ972 Sound Amp
        Hard drive in Slot 1
        DVD drive in Slot 2

    Keyboardmania
    -------------
        GQ971 Backplane
        GQ972 Main Board
        GQ974 Extend Board
        Yamaha XT446 board (for keyboard sounds) (the board layout matches the Yamaha MU100 Tone Generator)
        GQ971 Sound Amp
        CD-ROM drive in Slot 1
        CD-ROM drive in Slot 2

    ParaParaParadise
    ----------------
        GQ986 Backplane
        GQ972 Main Board
        2x CD-ROM drive in Slot 1



    Games that run on this hardware:

    BIOS       Game ID        Year    Game
    ------------------------------------------------------------------
    GQ972      GQ972          2000    Beatmania III
    GQ972(?)   GCA21          2001    Beatmania III Append 6th Mix
    GQ972(?)   GCB07          2002    Beatmania III Append 7th Mix
    GQ972(?)   GCA05          2000    Beatmania III Append Core Remix
    GQ972(?)   GCC01          2003    Beatmania III The Final
    GQ974      GQ974          2000    Keyboardmania
    GQ974      GCA01          2000    Keyboardmania 2nd Mix
    GQ974      GCA12          2001    Keyboardmania 3rd Mix
    GQ977      GQ977          2000    Para Para Paradise
    GQ977      GQ977          2000    Para Para Dancing
    GQ977      GC977          2000    Para Para Paradise 1.1
    GQ977      GQA11          2000    Para Para Paradise 1st Mix+
    GQA02(?)   GQ986          2000    Pop'n Music 4
    ???        G?A04          2000    Pop'n Music 5
    ???        GQA16          2001    Pop'n Music 6
    GQA02      GCB00          2001    Pop'n Music 7
    ???        GQB30          2002    Pop'n Music 8
    ???        ?              2000    Pop'n Music Animelo
    ???        GEA02          2001    Pop'n Music Animelo 2
    ???        ?              2001    Pop'n Music Mickey Tunes

Dumpable pieces missing
-----------------------
Beatmania III - CD, HDD, EPROM on main board, EPROM on SPU board
Beatmania III Append 6th Mix - HDD, dongle, EPROM on main board, EPROM on SPU board
Para Para Paradise - dongle, program CD
Para Para Paradise 1.1 - dongle
Para Para Dancing - dongle, program CD
Pop'n Music Animelo - dongle, CD, DVD
Keyboard Mania  - dongle, program CD, audio CD
Keyboard Mania 2nd Mix - dongle, program CD, audio CD

    TODO:
        - The external Yamaha MIDI sound board is not emulated (no keyboard sounds).


        - Notes on how the video is supposed to work from Ville / Ian Patterson:

        There are four "display contexts" that are set up via registers 20-4E. They are
        basically just raw framebuffers. 40-4E sets the base framebuffer pointer, 30-3E
        sets the size, 20-2E may set the minimum x and y coordinates but I haven't seen
        them set to something other than 0 yet. One context is set as the one the RAMDAC
        outputs to the monitor (not sure how this is selected yet, probably the lower
        bits of register 12). Thestartup test in the popn BIOS checks all of VRAM, so
        it moves the currentdisplay address around so you don't see crazy colors, which
        is very helpful in figuring out how this part works.

        The other new part is that there are two VRAM write ports, managed by registers
        60+68+70 and 64+6A+74, with status read from the lower bits of reg 7A. Each context
        can either write to VRAM as currently emulated, or the port can be switched in to
        "immediate mode" via registers 68/6A. Immedate mode can be used to run GCU commands
        at any point during the frame. It's mainly used to call display lists, which is where
        the display list addresses come from. Some games use it to send other commands, so
        it appears to be a 4-dword FIFO or something along those lines.
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "machine/ataintf.h"
#include "machine/intelfsh.h"
#include "machine/rtc65271.h"
#include "machine/ins8250.h"
#include "machine/midikbd.h"
#include "sound/ymz280b.h"
#include "sound/cdda.h"
#include "sound/rf5c400.h"
#include "video/k057714.h"
#include "firebeat.lh"


struct IBUTTON_SUBKEY
{
	UINT8 identifier[8];
	UINT8 password[8];
	UINT8 data[0x30];
};

struct IBUTTON
{
	IBUTTON_SUBKEY subkey[3];
};


#define PRINT_SPU_MEM 0

class firebeat_state : public driver_device
{
public:
	firebeat_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_work_ram(*this, "work_ram"),
		m_flash_main(*this, "flash_main"),
		m_flash_snd1(*this, "flash_snd1"),
		m_flash_snd2(*this, "flash_snd2"),
		m_duart_midi(*this, "duart_midi"),
		m_duart_com(*this, "duart_com"),
		m_kbd0(*this, "kbd0"),
		m_kbd1(*this, "kbd1"),
		m_ata(*this, "ata"),
		m_gcu0(*this, "gcu0"),
		m_gcu1(*this, "gcu1"),
		m_spuata(*this, "spu_ata")
	{ }

	required_device<ppc4xx_device> m_maincpu;
	optional_device<m68000_device> m_audiocpu;
	required_shared_ptr<UINT32> m_work_ram;
	required_device<fujitsu_29f016a_device> m_flash_main;
	required_device<fujitsu_29f016a_device> m_flash_snd1;
	required_device<fujitsu_29f016a_device> m_flash_snd2;
	optional_device<pc16552_device> m_duart_midi;
	required_device<pc16552_device> m_duart_com;
	optional_device<midi_keyboard_device> m_kbd0;
	optional_device<midi_keyboard_device> m_kbd1;
	required_device<ata_interface_device> m_ata;
	required_device<k057714_device> m_gcu0;
	required_device<k057714_device> m_gcu1;
	optional_device<ata_interface_device> m_spuata;

	UINT8 m_extend_board_irq_enable;
	UINT8 m_extend_board_irq_active;
//  emu_timer *m_keyboard_timer;
	int m_tick;
	int m_layer;
	int m_cab_data_ptr;
	const int * m_cur_cab_data;
//  int m_keyboard_state[2];
	UINT8 m_spu_shared_ram[0x400];
	IBUTTON m_ibutton;
	int m_ibutton_state;
	int m_ibutton_read_subkey_ptr;
	UINT8 m_ibutton_subkey_data[0x40];

	DECLARE_READ8_MEMBER(soundram_r);
	DECLARE_DRIVER_INIT(ppd);
	DECLARE_DRIVER_INIT(kbm);
	DECLARE_DRIVER_INIT(ppp);
	DECLARE_MACHINE_START(firebeat);
	DECLARE_MACHINE_RESET(firebeat);
	DECLARE_VIDEO_START(firebeat);
	UINT32 screen_update_firebeat_0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_firebeat_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(firebeat_interrupt);
	DECLARE_READ32_MEMBER(input_r);
	DECLARE_READ32_MEMBER(sensor_r );
	DECLARE_READ32_MEMBER(flashram_r);
	DECLARE_WRITE32_MEMBER(flashram_w);
	DECLARE_READ32_MEMBER(soundflash_r);
	DECLARE_WRITE32_MEMBER(soundflash_w);
	DECLARE_WRITE_LINE_MEMBER(ata_interrupt);
	DECLARE_READ32_MEMBER(ata_command_r);
	DECLARE_WRITE32_MEMBER(ata_command_w);
	DECLARE_READ32_MEMBER(ata_control_r);
	DECLARE_WRITE32_MEMBER(ata_control_w);
//  DECLARE_READ32_MEMBER(comm_uart_r);
//  DECLARE_WRITE32_MEMBER(comm_uart_w);
	DECLARE_READ32_MEMBER(cabinet_r);
	DECLARE_READ32_MEMBER(keyboard_wheel_r);
	DECLARE_READ8_MEMBER(midi_uart_r);
	DECLARE_WRITE8_MEMBER(midi_uart_w);
	DECLARE_READ32_MEMBER(extend_board_irq_r);
	DECLARE_WRITE32_MEMBER(extend_board_irq_w);
	DECLARE_WRITE32_MEMBER(lamp_output_w);
	DECLARE_WRITE32_MEMBER(lamp_output_kbm_w);
	DECLARE_WRITE32_MEMBER(lamp_output_ppp_w);
	DECLARE_WRITE32_MEMBER(lamp_output2_w);
	DECLARE_WRITE32_MEMBER(lamp_output2_ppp_w);
	DECLARE_WRITE32_MEMBER(lamp_output3_w);
	DECLARE_WRITE32_MEMBER(lamp_output3_ppp_w);
	DECLARE_READ32_MEMBER(ppc_spu_share_r);
	DECLARE_WRITE32_MEMBER(ppc_spu_share_w);
	DECLARE_READ16_MEMBER(spu_unk_r);
	DECLARE_WRITE16_MEMBER(spu_irq_ack_w);
	DECLARE_WRITE16_MEMBER(spu_220000_w);
	DECLARE_WRITE16_MEMBER(spu_sdram_bank_w);
	DECLARE_READ16_MEMBER(m68k_spu_share_r);
	DECLARE_WRITE16_MEMBER(m68k_spu_share_w);
	DECLARE_WRITE_LINE_MEMBER(spu_ata_interrupt);
//  TIMER_CALLBACK_MEMBER(keyboard_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(spu_timer_callback);
	void set_ibutton(UINT8 *data);
	int ibutton_w(UINT8 data);
	DECLARE_WRITE8_MEMBER(security_w);
	void init_lights(write32_delegate out1, write32_delegate out2, write32_delegate out3);
	void init_firebeat();
	void init_keyboard();
	DECLARE_WRITE_LINE_MEMBER(sound_irq_callback);
	DECLARE_WRITE_LINE_MEMBER(midi_uart_ch0_irq_callback);
	DECLARE_WRITE_LINE_MEMBER(midi_uart_ch1_irq_callback);
	DECLARE_WRITE_LINE_MEMBER(gcu0_interrupt);
	DECLARE_WRITE_LINE_MEMBER(gcu1_interrupt);
};






VIDEO_START_MEMBER(firebeat_state,firebeat)
{
}

UINT32 firebeat_state::screen_update_firebeat_0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return m_gcu0->draw(screen, bitmap, cliprect); }
UINT32 firebeat_state::screen_update_firebeat_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return m_gcu1->draw(screen, bitmap, cliprect); }

/*****************************************************************************/

READ32_MEMBER(firebeat_state::input_r)
{
	UINT32 r = 0;

	if (ACCESSING_BITS_24_31)
	{
		r |= (ioport("IN0")->read() & 0xff) << 24;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= (ioport("IN1")->read() & 0xff) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= (ioport("IN2")->read() & 0xff);
	}

	return r;
}

READ32_MEMBER(firebeat_state::sensor_r )
{
	if (offset == 0)
	{
		return ioport("SENSOR1")->read() | 0x01000100;
	}
	else
	{
		return ioport("SENSOR2")->read() | 0x01000100;
	}
}

READ32_MEMBER(firebeat_state::flashram_r)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= (m_flash_main->read((offset*4)+0) & 0xff) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= (m_flash_main->read((offset*4)+1) & 0xff) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= (m_flash_main->read((offset*4)+2) & 0xff) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= (m_flash_main->read((offset*4)+3) & 0xff) << 0;
	}
	return r;
}

WRITE32_MEMBER(firebeat_state::flashram_w)
{
	if (ACCESSING_BITS_24_31)
	{
		m_flash_main->write((offset*4)+0, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		m_flash_main->write((offset*4)+1, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		m_flash_main->write((offset*4)+2, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		m_flash_main->write((offset*4)+3, (data >> 0) & 0xff);
	}
}

READ32_MEMBER(firebeat_state::soundflash_r)
{
	UINT32 r = 0;
	fujitsu_29f016a_device *chip;
	if (offset < 0x200000/4)
	{
		chip = m_flash_snd1;
	}
	else
	{
		chip = m_flash_snd2;
	}

	offset &= 0x7ffff;

	if (ACCESSING_BITS_24_31)
	{
		r |= (chip->read((offset*4)+0) & 0xff) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= (chip->read((offset*4)+1) & 0xff) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= (chip->read((offset*4)+2) & 0xff) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= (chip->read((offset*4)+3) & 0xff) << 0;
	}
	return r;
}

WRITE32_MEMBER(firebeat_state::soundflash_w)
{
	fujitsu_29f016a_device *chip;
	if (offset < 0x200000/4)
	{
		chip = m_flash_snd1;
	}
	else
	{
		chip = m_flash_snd2;
	}

	offset &= 0x7ffff;

	if (ACCESSING_BITS_24_31)
	{
		chip->write((offset*4)+0, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		chip->write((offset*4)+1, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		chip->write((offset*4)+2, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		chip->write((offset*4)+3, (data >> 0) & 0xff);
	}
}

/*****************************************************************************/
/* ATA Interface */

#define BYTESWAP16(x)   ((((x) >> 8) & 0xff) | (((x) << 8) & 0xff00))

READ32_MEMBER(firebeat_state::ata_command_r )
{
	UINT16 r;
//  printf("ata_command_r: %08X, %08X\n", offset, mem_mask);
	if (ACCESSING_BITS_16_31)
	{
		r = m_ata->read_cs0(space, offset*2, BYTESWAP16((mem_mask >> 16) & 0xffff));
		return BYTESWAP16(r) << 16;
	}
	else
	{
		r = m_ata->read_cs0(space, (offset*2) + 1, BYTESWAP16((mem_mask >> 0) & 0xffff));
		return BYTESWAP16(r) << 0;
	}
}

WRITE32_MEMBER(firebeat_state::ata_command_w )
{
//  printf("ata_command_w: %08X, %08X, %08X\n", data, offset, mem_mask);

	if (ACCESSING_BITS_16_31)
	{
		m_ata->write_cs0(space, offset*2, BYTESWAP16((data >> 16) & 0xffff), BYTESWAP16((mem_mask >> 16) & 0xffff));
	}
	else
	{
		m_ata->write_cs0(space, (offset*2) + 1, BYTESWAP16((data >> 0) & 0xffff), BYTESWAP16((mem_mask >> 0) & 0xffff));
	}
}


READ32_MEMBER(firebeat_state::ata_control_r )
{
	UINT16 r;
//  printf("ata_control_r: %08X, %08X\n", offset, mem_mask);

	if (ACCESSING_BITS_16_31)
	{
		r = m_ata->read_cs1(space, offset*2, BYTESWAP16((mem_mask >> 16) & 0xffff));
		return BYTESWAP16(r) << 16;
	}
	else
	{
		r = m_ata->read_cs1(space, (offset*2) + 1, BYTESWAP16((mem_mask >> 0) & 0xffff));
		return BYTESWAP16(r) << 0;
	}
}

WRITE32_MEMBER(firebeat_state::ata_control_w )
{
	if (ACCESSING_BITS_16_31)
	{
		m_ata->write_cs1(space, offset*2, BYTESWAP16(data >> 16) & 0xffff, BYTESWAP16((mem_mask >> 16) & 0xffff));
	}
	else
	{
		m_ata->write_cs1(space, (offset*2) + 1, BYTESWAP16(data >> 0) & 0xffff, BYTESWAP16((mem_mask >> 0) & 0xffff));
	}
}


/*****************************************************************************/
/*
READ32_MEMBER(firebeat_state::comm_uart_r )
{
    UINT32 r = 0;

    if (ACCESSING_BITS_24_31)
    {
        r |= pc16552d_0_r(space, (offset*4)+0) << 24;
    }
    if (ACCESSING_BITS_16_23)
    {
        r |= pc16552d_0_r(space, (offset*4)+1) << 16;
    }
    if (ACCESSING_BITS_8_15)
    {
        r |= pc16552d_0_r(space, (offset*4)+2) << 8;
    }
    if (ACCESSING_BITS_0_7)
    {
        r |= pc16552d_0_r(space, (offset*4)+3) << 0;
    }

    return r;
}

WRITE32_MEMBER(firebeat_state::comm_uart_w )
{
    if (ACCESSING_BITS_24_31)
    {
        pc16552d_0_w(space, (offset*4)+0, (data >> 24) & 0xff);
    }
    if (ACCESSING_BITS_16_23)
    {
        pc16552d_0_w(space, (offset*4)+1, (data >> 16) & 0xff);
    }
    if (ACCESSING_BITS_8_15)
    {
        pc16552d_0_w(space, (offset*4)+2, (data >> 8) & 0xff);
    }
    if (ACCESSING_BITS_0_7)
    {
        pc16552d_0_w(space, (offset*4)+3, (data >> 0) & 0xff);
    }
}

static void comm_uart_irq_callback(running_machine &machine, int channel, int value)
{
    // TODO
    //m_maincpu->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
}
*/

/*****************************************************************************/

static const int cab_data[2] = { 0x0, 0x8 };
static const int kbm_cab_data[2] = { 0x2, 0x8 };
static const int ppd_cab_data[2] = { 0x1, 0x9 };

READ32_MEMBER(firebeat_state::cabinet_r )
{
	UINT32 r = 0;

//  printf("cabinet_r: %08X, %08X\n", offset, mem_mask);

	switch (offset)
	{
		case 0:
		{
			r = m_cur_cab_data[m_cab_data_ptr & 1] << 28;
			m_cab_data_ptr++;
			return r;
		}
		case 2:     return 0x00000000;
		case 4:     return 0x00000000;
	}

	return 0;
}

/*****************************************************************************/

READ32_MEMBER(firebeat_state::keyboard_wheel_r )
{
	if (offset == 0)        // Keyboard Wheel (P1)
	{
		return ioport("WHEEL_P1")->read() << 24;
	}
	else if (offset == 2)   // Keyboard Wheel (P2)
	{
		return ioport("WHEEL_P2")->read() << 24;
	}

	return 0;
}

READ8_MEMBER(firebeat_state::midi_uart_r )
{
	return m_duart_midi->read(space, offset >> 6);
}

WRITE8_MEMBER(firebeat_state::midi_uart_w )
{
	m_duart_midi->write(space, offset >> 6, data);
}

WRITE_LINE_MEMBER(firebeat_state::midi_uart_ch0_irq_callback)
{
	if ((m_extend_board_irq_enable & 0x02) == 0 && state != CLEAR_LINE)
	{
		m_extend_board_irq_active |= 0x02;
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

WRITE_LINE_MEMBER(firebeat_state::midi_uart_ch1_irq_callback)
{
	if ((m_extend_board_irq_enable & 0x01) == 0 && state != CLEAR_LINE)
	{
		m_extend_board_irq_active |= 0x01;
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

/*
static const int keyboard_notes[24] =
{
    0x3c,   // C1
    0x3d,   // C1#
    0x3e,   // D1
    0x3f,   // D1#
    0x40,   // E1
    0x41,   // F1
    0x42,   // F1#
    0x43,   // G1
    0x44,   // G1#
    0x45,   // A1
    0x46,   // A1#
    0x47,   // B1
    0x48,   // C2
    0x49,   // C2#
    0x4a,   // D2
    0x4b,   // D2#
    0x4c,   // E2
    0x4d,   // F2
    0x4e,   // F2#
    0x4f,   // G2
    0x50,   // G2#
    0x51,   // A2
    0x52,   // A2#
    0x53,   // B2
};

TIMER_CALLBACK_MEMBER(firebeat_state::keyboard_timer_callback)
{
    static const int kb_uart_channel[2] = { 1, 0 };
    static const char *const keynames[] = { "KEYBOARD_P1", "KEYBOARD_P2" };
    int keyboard;
    int i;

    for (keyboard=0; keyboard < 2; keyboard++)
    {
        UINT32 kbstate = ioport(keynames[keyboard])->read();
        int uart_channel = kb_uart_channel[keyboard];

        if (kbstate != m_keyboard_state[keyboard])
        {
            for (i=0; i < 24; i++)
            {
                int kbnote = keyboard_notes[i];

                if ((m_keyboard_state[keyboard] & (1 << i)) != 0 && (kbstate & (1 << i)) == 0)
                {
                    // key was on, now off -> send Note Off message
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x80);
                    pc16552d_rx_data(machine(), 1, uart_channel, kbnote);
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x7f);
                }
                else if ((m_keyboard_state[keyboard] & (1 << i)) == 0 && (kbstate & (1 << i)) != 0)
                {
                    // key was off, now on -> send Note On message
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x90);
                    pc16552d_rx_data(machine(), 1, uart_channel, kbnote);
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x7f);
                }
            }
        }
        else
        {
            // no messages, send Active Sense message instead
            pc16552d_rx_data(machine(), 1, uart_channel, 0xfe);
        }

        m_keyboard_state[keyboard] = kbstate;
    }
}
*/
// Extend board IRQs
// 0x01: MIDI UART channel 2
// 0x02: MIDI UART channel 1
// 0x04: ?
// 0x08: ?
// 0x10: ?
// 0x20: ?

READ32_MEMBER(firebeat_state::extend_board_irq_r)
{
	UINT32 r = 0;

	if (ACCESSING_BITS_24_31)
	{
		r |= (~m_extend_board_irq_active) << 24;
	}

	return r;
}

WRITE32_MEMBER(firebeat_state::extend_board_irq_w )
{
//  printf("extend_board_irq_w: %08X, %08X, %08X\n", data, offset, mem_mask);

	if (ACCESSING_BITS_24_31)
	{
		m_extend_board_irq_active &= ~((data >> 24) & 0xff);

		m_extend_board_irq_enable = (data >> 24) & 0xff;
	}
}

/*****************************************************************************/

WRITE32_MEMBER(firebeat_state::lamp_output_w )
{
	// -------- -------- -------- xxxxxxxx   Status LEDs (active low)
	if (ACCESSING_BITS_0_7)
	{
		output().set_value("status_led_0", (data & 0x01) ? 0 : 1);
		output().set_value("status_led_1", (data & 0x02) ? 0 : 1);
		output().set_value("status_led_2", (data & 0x04) ? 0 : 1);
		output().set_value("status_led_3", (data & 0x08) ? 0 : 1);
		output().set_value("status_led_4", (data & 0x10) ? 0 : 1);
		output().set_value("status_led_5", (data & 0x20) ? 0 : 1);
		output().set_value("status_led_6", (data & 0x40) ? 0 : 1);
		output().set_value("status_led_7", (data & 0x80) ? 0 : 1);
	}

//  printf("lamp_output_w: %08X, %08X, %08X\n", data, offset, mem_mask);
}

WRITE32_MEMBER(firebeat_state::lamp_output_kbm_w )
{
	lamp_output_w(space, offset, data, mem_mask);

	if (ACCESSING_BITS_24_31)
	{
		output().set_value("door_lamp",   (data & 0x10000000) ? 1 : 0);
		output().set_value("start1p",     (data & 0x01000000) ? 1 : 0);
		output().set_value("start2p",     (data & 0x02000000) ? 1 : 0);
	}
	if (ACCESSING_BITS_8_15)
	{
		output().set_value("lamp1",       (data & 0x00000100) ? 1 : 0);
		output().set_value("lamp2",       (data & 0x00000200) ? 1 : 0);
		output().set_value("lamp3",       (data & 0x00000400) ? 1 : 0);
		output().set_value("neon",        (data & 0x00000800) ? 1 : 0);
	}
}

WRITE32_MEMBER(firebeat_state::lamp_output_ppp_w )
{
	lamp_output_w(space, offset, data, mem_mask);

	// ParaParaParadise lamps (active high)
	// 0x00000100 Left
	// 0x00000200 Right
	// 0x00000400 Door Lamp
	// 0x00000800 OK
	// 0x00008000 Slim
	// 0x01000000 Stage LED 0
	// 0x02000000 Stage LED 1
	// 0x04000000 Stage LED 2
	// 0x08000000 Stage LED 3
	// 0x00010000 Stage LED 4
	// 0x00020000 Stage LED 5
	// 0x00040000 Stage LED 6
	// 0x00080000 Stage LED 7
	if (ACCESSING_BITS_8_15)
	{
		output().set_value("left",            (data & 0x00000100) ? 1 : 0);
		output().set_value("right",           (data & 0x00000200) ? 1 : 0);
		output().set_value("door_lamp",       (data & 0x00000400) ? 1 : 0);
		output().set_value("ok",              (data & 0x00000800) ? 1 : 0);
		output().set_value("slim",            (data & 0x00008000) ? 1 : 0);
	}
	if (ACCESSING_BITS_24_31)
	{
		output().set_value("stage_led_0",     (data & 0x01000000) ? 1 : 0);
		output().set_value("stage_led_1",     (data & 0x02000000) ? 1 : 0);
		output().set_value("stage_led_2",     (data & 0x04000000) ? 1 : 0);
		output().set_value("stage_led_3",     (data & 0x08000000) ? 1 : 0);
	}
	if (ACCESSING_BITS_16_23)
	{
		output().set_value("stage_led_4",     (data & 0x00010000) ? 1 : 0);
		output().set_value("stage_led_5",     (data & 0x00020000) ? 1 : 0);
		output().set_value("stage_led_6",     (data & 0x00040000) ? 1 : 0);
		output().set_value("stage_led_7",     (data & 0x00080000) ? 1 : 0);
	}
}

WRITE32_MEMBER(firebeat_state::lamp_output2_w )
{
//  printf("lamp_output2_w: %08X, %08X, %08X\n", data, offset, mem_mask);
}

WRITE32_MEMBER(firebeat_state::lamp_output2_ppp_w )
{
	lamp_output2_w(space, offset, data, mem_mask);

	// ParaParaParadise lamps (active high)
	// 0x00010000 Top LED 0
	// 0x00020000 Top LED 1
	// 0x00040000 Top LED 2
	// 0x00080000 Top LED 3
	// 0x00000001 Top LED 4
	// 0x00000002 Top LED 5
	// 0x00000004 Top LED 6
	// 0x00000008 Top LED 7
	if (ACCESSING_BITS_16_23)
	{
		output().set_value("top_led_0",       (data & 0x00010000) ? 1 : 0);
		output().set_value("top_led_1",       (data & 0x00020000) ? 1 : 0);
		output().set_value("top_led_2",       (data & 0x00040000) ? 1 : 0);
		output().set_value("top_led_3",       (data & 0x00080000) ? 1 : 0);
	}
	if (ACCESSING_BITS_0_7)
	{
		output().set_value("top_led_4",       (data & 0x00000001) ? 1 : 0);
		output().set_value("top_led_5",       (data & 0x00000002) ? 1 : 0);
		output().set_value("top_led_6",       (data & 0x00000004) ? 1 : 0);
		output().set_value("top_led_7",       (data & 0x00000008) ? 1 : 0);
	}
}

WRITE32_MEMBER(firebeat_state::lamp_output3_w )
{
//  printf("lamp_output3_w: %08X, %08X, %08X\n", data, offset, mem_mask);
}

WRITE32_MEMBER(firebeat_state::lamp_output3_ppp_w )
{
	lamp_output3_w(space, offset, data, mem_mask);

	// ParaParaParadise lamps (active high)
	// 0x00010000 Lamp 0
	// 0x00040000 Lamp 1
	// 0x00100000 Lamp 2
	// 0x00400000 Lamp 3
	if (ACCESSING_BITS_16_23)
	{
		output().set_value("lamp_0",          (data & 0x00010000) ? 1 : 0);
		output().set_value("lamp_1",          (data & 0x00040000) ? 1 : 0);
		output().set_value("lamp_2",          (data & 0x00100000) ? 1 : 0);
		output().set_value("lamp_3",          (data & 0x00400000) ? 1 : 0);
	}
}

/*****************************************************************************/


READ32_MEMBER(firebeat_state::ppc_spu_share_r)
{
	UINT32 r = 0;

#if PRINT_SPU_MEM
	printf("ppc_spu_share_r: %08X, %08X at %08X\n", offset, mem_mask, space.device().safe_pc());
#endif

	if (ACCESSING_BITS_24_31)
	{
		r |= m_spu_shared_ram[(offset * 4) + 0] << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= m_spu_shared_ram[(offset * 4) + 1] << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= m_spu_shared_ram[(offset * 4) + 2] <<  8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= m_spu_shared_ram[(offset * 4) + 3] <<  0;

		if (offset == 0xff)     // address 0x3ff clears PPC interrupt
		{
			m_maincpu->set_input_line(INPUT_LINE_IRQ3, CLEAR_LINE);
		}
	}

	return r;
}

WRITE32_MEMBER(firebeat_state::ppc_spu_share_w)
{
#if PRINT_SPU_MEM
	printf("ppc_spu_share_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space.device().safe_pc());
#endif

	if (ACCESSING_BITS_24_31)
	{
		m_spu_shared_ram[(offset * 4) + 0] = (data >> 24) & 0xff;
	}
	if (ACCESSING_BITS_16_23)
	{
		m_spu_shared_ram[(offset * 4) + 1] = (data >> 16) & 0xff;
	}
	if (ACCESSING_BITS_8_15)
	{
		m_spu_shared_ram[(offset * 4) + 2] = (data >>  8) & 0xff;

		if (offset == 0xff)     // address 0x3fe triggers M68K interrupt
		{
			m_audiocpu->set_input_line(INPUT_LINE_IRQ4, ASSERT_LINE);

			printf("SPU command %02X%02X\n", m_spu_shared_ram[0], m_spu_shared_ram[1]);

			UINT16 cmd = ((UINT16)(m_spu_shared_ram[0]) << 8) | m_spu_shared_ram[1];
			if (cmd == 0x1110)
			{
				printf("   [%02X %02X %02X %02X %02X]\n", m_spu_shared_ram[0x10], m_spu_shared_ram[0x11], m_spu_shared_ram[0x12], m_spu_shared_ram[0x13], m_spu_shared_ram[0x14]);
			}
		}
	}
	if (ACCESSING_BITS_0_7)
	{
		m_spu_shared_ram[(offset * 4) + 3] = (data >>  0) & 0xff;
	}
}

/*  SPU board M68K IRQs

    IRQ1: ?

    IRQ2: Timer?

    IRQ4: Dual-port RAM mailbox (when PPC writes to 0x3FE)
          Handles commands from PPC (bytes 0x00 and 0x01)

    IRQ6: ATA
*/

READ16_MEMBER(firebeat_state::m68k_spu_share_r)
{
#if PRINT_SPU_MEM
	printf("m68k_spu_share_r: %08X, %08X\n", offset, mem_mask);
#endif
	UINT16 r = 0;

	if (ACCESSING_BITS_0_7)
	{
		r |= m_spu_shared_ram[offset];

		if (offset == 0x3fe)            // address 0x3fe clears M68K interrupt
		{
			m_audiocpu->set_input_line(INPUT_LINE_IRQ4, CLEAR_LINE);
		}
	}
	return r;
}

WRITE16_MEMBER(firebeat_state::m68k_spu_share_w)
{
#if PRINT_SPU_MEM
	printf("m68k_spu_share_w: %04X, %08X, %08X\n", data, offset, mem_mask);
#endif
	if (ACCESSING_BITS_0_7)
	{
		m_spu_shared_ram[offset] = data & 0xff;

		if (offset == 0x3ff)            // address 0x3ff triggers PPC interrupt
		{
			m_maincpu->set_input_line(INPUT_LINE_IRQ3, ASSERT_LINE);
		}
	}
}

READ16_MEMBER(firebeat_state::spu_unk_r)
{
	// dipswitches?

	UINT16 r = 0;
	r |= 0x80;      // if set, uses ATA PIO mode, otherwise DMA
	r |= 0x01;      // enable SDRAM test

	return r;
}

WRITE16_MEMBER(firebeat_state::spu_irq_ack_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 0x01)
			m_audiocpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
		if (data & 0x02)
			m_audiocpu->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
		if (data & 0x08)
			m_audiocpu->set_input_line(INPUT_LINE_IRQ6, CLEAR_LINE);
	}
}

WRITE16_MEMBER(firebeat_state::spu_220000_w)
{
	// IRQ2 handler 5 sets all bits
}

WRITE16_MEMBER(firebeat_state::spu_sdram_bank_w)
{
}

WRITE_LINE_MEMBER(firebeat_state::spu_ata_interrupt)
{
	m_audiocpu->set_input_line(INPUT_LINE_IRQ6, state);
}

TIMER_DEVICE_CALLBACK_MEMBER(firebeat_state::spu_timer_callback)
{
	m_audiocpu->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
}

/*****************************************************************************/

MACHINE_START_MEMBER(firebeat_state,firebeat)
{
	/* set conservative DRC options */
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x01ffffff, FALSE, m_work_ram);
}

static ADDRESS_MAP_START( firebeat_map, AS_PROGRAM, 32, firebeat_state )
	AM_RANGE(0x00000000, 0x01ffffff) AM_RAM AM_SHARE("work_ram")
	AM_RANGE(0x70000000, 0x70000fff) AM_READWRITE8(midi_uart_r, midi_uart_w, 0xff000000)
	AM_RANGE(0x70006000, 0x70006003) AM_WRITE(extend_board_irq_w)
	AM_RANGE(0x70008000, 0x7000800f) AM_READ(keyboard_wheel_r)
	AM_RANGE(0x7000a000, 0x7000a003) AM_READ(extend_board_irq_r)
	AM_RANGE(0x74000000, 0x740003ff) AM_READWRITE(ppc_spu_share_r, ppc_spu_share_w) // SPU shared RAM
	AM_RANGE(0x7d000200, 0x7d00021f) AM_READ(cabinet_r)
	AM_RANGE(0x7d000340, 0x7d000347) AM_READ(sensor_r)
	AM_RANGE(0x7d000400, 0x7d000403) AM_DEVREADWRITE8("ymz", ymz280b_device, read, write, 0xffff0000)
	AM_RANGE(0x7d000800, 0x7d000803) AM_READ(input_r)
	AM_RANGE(0x7d400000, 0x7d5fffff) AM_READWRITE(flashram_r, flashram_w)
	AM_RANGE(0x7d800000, 0x7dbfffff) AM_READWRITE(soundflash_r, soundflash_w)
	AM_RANGE(0x7dc00000, 0x7dc0000f) AM_DEVREADWRITE8("duart_com", pc16552_device, read, write, 0xffffffff)
	AM_RANGE(0x7e000000, 0x7e00003f) AM_DEVREADWRITE8("rtc", rtc65271_device, rtc_r, rtc_w, 0xffffffff)
	AM_RANGE(0x7e000100, 0x7e00013f) AM_DEVREADWRITE8("rtc", rtc65271_device, xram_r, xram_w, 0xffffffff)
	AM_RANGE(0x7e800000, 0x7e8000ff) AM_DEVREADWRITE("gcu0", k057714_device, read, write)
	AM_RANGE(0x7e800100, 0x7e8001ff) AM_DEVREADWRITE("gcu1", k057714_device, read, write)
	AM_RANGE(0x7fe00000, 0x7fe0000f) AM_READWRITE(ata_command_r, ata_command_w)
	AM_RANGE(0x7fe80000, 0x7fe8000f) AM_READWRITE(ata_control_r, ata_control_w)
	AM_RANGE(0x7ff80000, 0x7fffffff) AM_ROM AM_REGION("user1", 0)       /* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START( spu_map, AS_PROGRAM, 16, firebeat_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x13ffff) AM_RAM
	AM_RANGE(0x200000, 0x200001) AM_READ(spu_unk_r)
	AM_RANGE(0x220000, 0x220001) AM_WRITE(spu_220000_w)
	AM_RANGE(0x230000, 0x230001) AM_WRITE(spu_irq_ack_w)
	AM_RANGE(0x260000, 0x260001) AM_WRITE(spu_sdram_bank_w)
	AM_RANGE(0x280000, 0x2807ff) AM_READWRITE(m68k_spu_share_r, m68k_spu_share_w)
	AM_RANGE(0x300000, 0x30000f) AM_DEVREADWRITE("spu_ata", ata_interface_device, read_cs0, write_cs0)
	AM_RANGE(0x340000, 0x34000f) AM_DEVREADWRITE("spu_ata", ata_interface_device, read_cs1, write_cs1)
	AM_RANGE(0x400000, 0x400fff) AM_DEVREADWRITE("rf5c400", rf5c400_device, rf5c400_r, rf5c400_w)
	AM_RANGE(0x800000, 0x83ffff) AM_RAM         // SDRAM
	AM_RANGE(0xfc0000, 0xffffff) AM_RAM         // SDRAM
ADDRESS_MAP_END

/*****************************************************************************/

READ8_MEMBER(firebeat_state::soundram_r)
{
	offset &= 0x3fffff;
	if (offset < 0x200000)
		return m_flash_snd1->read(offset);
	else
		return m_flash_snd2->read(offset & 0x1fffff);
}

WRITE_LINE_MEMBER(firebeat_state::sound_irq_callback)
{
}

static INPUT_PORTS_START(ppp)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )            // Left
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )            // Right
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW)            // Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)      // Service
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )              // Coin
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )             // Start / Ok
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Dip switches */

	// ParaParaParadise has 24 sensors, grouped into groups of 3 for each sensor bar
	// Sensors 15...23 are only used by the Korean version of PPP, which has 8 sensor bars

	PORT_START("SENSOR1")
	PORT_BIT( 0x00070000, IP_ACTIVE_HIGH, IPT_BUTTON3 )     // Sensor 0, 1, 2  (Sensor bar 1)
	PORT_BIT( 0x00380000, IP_ACTIVE_HIGH, IPT_BUTTON4 )     // Sensor 3, 4, 5  (Sensor bar 2)
	PORT_BIT( 0x00c00001, IP_ACTIVE_HIGH, IPT_BUTTON5 )     // Sensor 6, 7, 8  (Sensor bar 3)
	PORT_BIT( 0x0000000e, IP_ACTIVE_HIGH, IPT_BUTTON6 )     // Sensor 9, 10,11 (Sensor bar 4)

	PORT_START("SENSOR2")
	PORT_BIT( 0x00070000, IP_ACTIVE_HIGH, IPT_BUTTON7 )     // Sensor 12,13,14 (Sensor bar 5)
	PORT_BIT( 0x00380000, IP_ACTIVE_HIGH, IPT_BUTTON8 )     // Sensor 15,16,17 (Sensor bar 6)   (unused by PPP)
	PORT_BIT( 0x00c00001, IP_ACTIVE_HIGH, IPT_BUTTON9 )     // Sensor 18,19,20 (Sensor bar 7)   (unused by PPP)
	PORT_BIT( 0x0000000e, IP_ACTIVE_HIGH, IPT_BUTTON10 )    // Sensor 21,22,23 (Sensor bar 8)   (unused by PPP)

INPUT_PORTS_END

static INPUT_PORTS_START(kbm)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )             // Start P1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )             // Start P2
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW)            // Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)      // Service
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )              // Coin
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )           // e-Amusement
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Dip switches */

	PORT_START("WHEEL_P1")          // Keyboard modulation wheel (P1)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0xff, 0x00) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("WHEEL_P2")          // Keyboard modulation wheel (P2)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE_V ) PORT_MINMAX(0xff, 0x00) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

/*
    PORT_START("KEYBOARD_P1")
    PORT_BIT( 0x000001, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C1") PORT_CODE(KEYCODE_Q)
    PORT_BIT( 0x000002, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C1#") PORT_CODE(KEYCODE_W)
    PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D1") PORT_CODE(KEYCODE_E)
    PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D1#") PORT_CODE(KEYCODE_R)
    PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 E1") PORT_CODE(KEYCODE_T)
    PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F1") PORT_CODE(KEYCODE_Y)
    PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F1#") PORT_CODE(KEYCODE_U)
    PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G1") PORT_CODE(KEYCODE_I)
    PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G1#") PORT_CODE(KEYCODE_O)
    PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A1") PORT_CODE(KEYCODE_A)
    PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A1#") PORT_CODE(KEYCODE_S)
    PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 B1") PORT_CODE(KEYCODE_D)
    PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C2") PORT_CODE(KEYCODE_F)
    PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C2#") PORT_CODE(KEYCODE_G)
    PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D2") PORT_CODE(KEYCODE_H)
    PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D2#") PORT_CODE(KEYCODE_J)
    PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 E2") PORT_CODE(KEYCODE_K)
    PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F2") PORT_CODE(KEYCODE_L)
    PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F2#") PORT_CODE(KEYCODE_Z)
    PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G2") PORT_CODE(KEYCODE_X)
    PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G2#") PORT_CODE(KEYCODE_C)
    PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A2") PORT_CODE(KEYCODE_V)
    PORT_BIT( 0x400000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A2#") PORT_CODE(KEYCODE_B)
    PORT_BIT( 0x800000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 B2") PORT_CODE(KEYCODE_N)

    PORT_START("KEYBOARD_P2")
    PORT_BIT( 0x000001, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C1") PORT_CODE(KEYCODE_Q)
    PORT_BIT( 0x000002, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C1#") PORT_CODE(KEYCODE_W)
    PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D1") PORT_CODE(KEYCODE_E)
    PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D1#") PORT_CODE(KEYCODE_R)
    PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 E1") PORT_CODE(KEYCODE_T)
    PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F1") PORT_CODE(KEYCODE_Y)
    PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F1#") PORT_CODE(KEYCODE_U)
    PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G1") PORT_CODE(KEYCODE_I)
    PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G1#") PORT_CODE(KEYCODE_O)
    PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A1") PORT_CODE(KEYCODE_A)
    PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A1#") PORT_CODE(KEYCODE_S)
    PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 B1") PORT_CODE(KEYCODE_D)
    PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C2") PORT_CODE(KEYCODE_F)
    PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C2#") PORT_CODE(KEYCODE_G)
    PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D2") PORT_CODE(KEYCODE_H)
    PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D2#") PORT_CODE(KEYCODE_J)
    PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 E2") PORT_CODE(KEYCODE_K)
    PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F2") PORT_CODE(KEYCODE_L)
    PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F2#") PORT_CODE(KEYCODE_Z)
    PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G2") PORT_CODE(KEYCODE_X)
    PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G2#") PORT_CODE(KEYCODE_C)
    PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A2") PORT_CODE(KEYCODE_V)
    PORT_BIT( 0x400000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A2#") PORT_CODE(KEYCODE_B)
    PORT_BIT( 0x800000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 B2") PORT_CODE(KEYCODE_N)
*/
INPUT_PORTS_END

static INPUT_PORTS_START(popn)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )            // Switch 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )            // Switch 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )            // Switch 3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )            // Switch 4
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )            // Switch 5
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )            // Switch 6
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 )            // Switch 7
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 )            // Switch 8

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 )            // Switch 9
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )              // Coin
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW)            // Test
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)      // Service
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Dip switches */

INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(firebeat_state::firebeat_interrupt)
{
	// IRQs
	// IRQ 0: VBlank
	// IRQ 1: Extend board IRQ
	// IRQ 2: Main board UART
	// IRQ 3: SPU mailbox interrupt
	// IRQ 4: ATA

	device.execute().set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

WRITE_LINE_MEMBER(firebeat_state::gcu0_interrupt)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

WRITE_LINE_MEMBER(firebeat_state::gcu1_interrupt)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

MACHINE_RESET_MEMBER(firebeat_state,firebeat)
{
	m_layer = 0;
}

WRITE_LINE_MEMBER( firebeat_state::ata_interrupt )
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ4, state);
}

static MACHINE_CONFIG_FRAGMENT( cdrom_config )
	MCFG_DEVICE_MODIFY("cdda")
	MCFG_SOUND_ROUTE(0, "^^^^lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "^^^^rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( firebeat, firebeat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC403GCX, XTAL_64MHz)
	MCFG_CPU_PROGRAM_MAP(firebeat_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", firebeat_state,  firebeat_interrupt)

	MCFG_MACHINE_START_OVERRIDE(firebeat_state,firebeat)
	MCFG_MACHINE_RESET_OVERRIDE(firebeat_state,firebeat)

	MCFG_DEVICE_ADD("rtc", RTC65271, 0)

	MCFG_FUJITSU_29F016A_ADD("flash_main")
	MCFG_FUJITSU_29F016A_ADD("flash_snd1")
	MCFG_FUJITSU_29F016A_ADD("flash_snd2")

	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "cdrom", "cdrom", true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(firebeat_state, ata_interrupt))

	MCFG_DEVICE_MODIFY("ata:1")
	MCFG_DEVICE_CARD_MACHINE_CONFIG( "cdrom", cdrom_config )

	/* video hardware */
	MCFG_PALETTE_ADD_RRRRRGGGGGBBBBB("palette")

	MCFG_DEVICE_ADD("gcu0", K057714, 0)
	MCFG_K057714_IRQ_CALLBACK(WRITELINE(firebeat_state, gcu0_interrupt))

	MCFG_DEVICE_ADD("gcu1", K057714, 0)
	MCFG_K057714_IRQ_CALLBACK(WRITELINE(firebeat_state, gcu1_interrupt))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(firebeat_state, screen_update_firebeat_0)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(firebeat_state,firebeat)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymz", YMZ280B, 16934400)
	MCFG_YMZ280B_IRQ_HANDLER(WRITELINE(firebeat_state, sound_irq_callback))
	MCFG_YMZ280B_EXT_READ_HANDLER(READ8(firebeat_state, soundram_r))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("duart_com", PC16552D, 0)  // pgmd to 9600baud
	MCFG_DEVICE_ADD("duart_com:chan0", NS16550, XTAL_19_6608MHz)
	MCFG_DEVICE_ADD("duart_com:chan1", NS16550, XTAL_19_6608MHz)
	MCFG_DEVICE_ADD("duart_midi", PC16552D, 0)  // in all memory maps, pgmd to 31250baud
	MCFG_DEVICE_ADD("duart_midi:chan0", NS16550, XTAL_24MHz)
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE(DEVICE_SELF_OWNER, firebeat_state, midi_uart_ch0_irq_callback))
	MCFG_DEVICE_ADD("duart_midi:chan1", NS16550, XTAL_24MHz)
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE(DEVICE_SELF_OWNER, firebeat_state, midi_uart_ch1_irq_callback))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( firebeat2, firebeat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC403GCX, XTAL_64MHz)
	MCFG_CPU_PROGRAM_MAP(firebeat_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", firebeat_state,  firebeat_interrupt)

	MCFG_MACHINE_START_OVERRIDE(firebeat_state,firebeat)
	MCFG_MACHINE_RESET_OVERRIDE(firebeat_state,firebeat)

	MCFG_DEVICE_ADD("rtc", RTC65271, 0)

	MCFG_FUJITSU_29F016A_ADD("flash_main")
	MCFG_FUJITSU_29F016A_ADD("flash_snd1")
	MCFG_FUJITSU_29F016A_ADD("flash_snd2")

	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "cdrom", "cdrom", true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(firebeat_state, ata_interrupt))

	MCFG_DEVICE_MODIFY("ata:1")
	MCFG_DEVICE_CARD_MACHINE_CONFIG( "cdrom", cdrom_config )

	/* video hardware */
	MCFG_PALETTE_ADD_RRRRRGGGGGBBBBB("palette")

	MCFG_DEVICE_ADD("gcu0", K057714, 0)
	MCFG_K057714_IRQ_CALLBACK(WRITELINE(firebeat_state, gcu0_interrupt))

	MCFG_DEVICE_ADD("gcu1", K057714, 0)
	MCFG_K057714_IRQ_CALLBACK(WRITELINE(firebeat_state, gcu1_interrupt))

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(firebeat_state, screen_update_firebeat_0)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(firebeat_state, screen_update_firebeat_1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(firebeat_state,firebeat)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymz", YMZ280B, 16934400)
	MCFG_YMZ280B_IRQ_HANDLER(WRITELINE(firebeat_state, sound_irq_callback))
	MCFG_YMZ280B_EXT_READ_HANDLER(READ8(firebeat_state, soundram_r))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("duart_com", PC16552D, 0)
	MCFG_DEVICE_ADD("duart_com:chan0", NS16550, XTAL_19_6608MHz)
	MCFG_DEVICE_ADD("duart_com:chan1", NS16550, XTAL_19_6608MHz)
	MCFG_DEVICE_ADD("duart_midi", PC16552D, 0)
	MCFG_DEVICE_ADD("duart_midi:chan0", NS16550, XTAL_24MHz)
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE(DEVICE_SELF_OWNER, firebeat_state, midi_uart_ch0_irq_callback))
	MCFG_DEVICE_ADD("duart_midi:chan1", NS16550, XTAL_24MHz)
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE(DEVICE_SELF_OWNER, firebeat_state, midi_uart_ch1_irq_callback))
	MCFG_MIDI_KBD_ADD("kbd0", DEVWRITELINE("duart_midi:chan0", ins8250_uart_device, rx_w), 31250)
	MCFG_MIDI_KBD_ADD("kbd1", DEVWRITELINE("duart_midi:chan1", ins8250_uart_device, rx_w), 31250)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( firebeat_spu, firebeat )

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(spu_map)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("sputimer", firebeat_state, spu_timer_callback, attotime::from_hz(1000));

	MCFG_RF5C400_ADD("rf5c400", XTAL_16_9344MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_ATA_INTERFACE_ADD("spu_ata", ata_devices, "cdrom", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(firebeat_state, spu_ata_interrupt))
MACHINE_CONFIG_END

/*****************************************************************************/
/* Security dongle is a Dallas DS1411 RS232 Adapter with a DS1991 Multikey iButton */

/* popn7 supports 8 different dongles:
   - Manufacture
   - Service
   - Event
   - Oversea
   - No Hardware
   - Rental
   - Debug
   - Normal
*/

enum
{
	DS1991_STATE_NORMAL,
	DS1991_STATE_READ_SUBKEY
};



void firebeat_state::set_ibutton(UINT8 *data)
{
	int i, j;

	for (i=0; i < 3; i++)
	{
		// identifier
		for (j=0; j < 8; j++)
		{
			m_ibutton.subkey[i].identifier[j] = *data++;
		}

		// password
		for (j=0; j < 8; j++)
		{
			m_ibutton.subkey[i].password[j] = *data++;
		}

		// data
		for (j=0; j < 48; j++)
		{
			m_ibutton.subkey[i].data[j] = *data++;
		}
	}
}

int firebeat_state::ibutton_w(UINT8 data)
{
	int r = -1;

	switch (m_ibutton_state)
	{
		case DS1991_STATE_NORMAL:
		{
			switch (data)
			{
				//
				// DS2408B Serial 1-Wire Line Driver with Load Sensor
				//
				case 0xc1:          // DS2480B reset
				{
					r = 0xcd;
					break;
				}
				case 0xe1:          // DS2480B set data mode
				{
					break;
				}
				case 0xe3:          // DS2480B set command mode
				{
					break;
				}

				//
				// DS1991 MultiKey iButton
				//
				case 0x66:          // DS1991 Read SubKey
				{
					r = 0x66;
					m_ibutton_state = DS1991_STATE_READ_SUBKEY;
					m_ibutton_read_subkey_ptr = 0;
					break;
				}
				case 0xcc:          // DS1991 skip rom
				{
					r = 0xcc;
					m_ibutton_state = DS1991_STATE_NORMAL;
					break;
				}
				default:
				{
					fatalerror("ibutton: unknown normal mode cmd %02X\n", data);
				}
			}
			break;
		}

		case DS1991_STATE_READ_SUBKEY:
		{
			if (m_ibutton_read_subkey_ptr == 0)      // Read SubKey, 2nd command byte
			{
				int subkey = (data >> 6) & 0x3;
		//      printf("iButton SubKey %d\n", subkey);
				r = data;

				if (subkey < 3)
				{
					memcpy(&m_ibutton_subkey_data[0],  m_ibutton.subkey[subkey].identifier, 8);
					memcpy(&m_ibutton_subkey_data[8],  m_ibutton.subkey[subkey].password, 8);
					memcpy(&m_ibutton_subkey_data[16], m_ibutton.subkey[subkey].data, 0x30);
				}
				else
				{
					memset(&m_ibutton_subkey_data[0], 0, 0x40);
				}
			}
			else if (m_ibutton_read_subkey_ptr == 1) // Read SubKey, 3rd command byte
			{
				r = data;
			}
			else
			{
				r = m_ibutton_subkey_data[m_ibutton_read_subkey_ptr-2];
			}
			m_ibutton_read_subkey_ptr++;
			if (m_ibutton_read_subkey_ptr >= 0x42)
			{
				m_ibutton_state = DS1991_STATE_NORMAL;
			}
			break;
		}
	}

	return r;
}

WRITE8_MEMBER(firebeat_state::security_w)
{
	int r = ibutton_w(data);
	if (r >= 0)
		m_maincpu->ppc4xx_spu_receive_byte(r);
}

/*****************************************************************************/

void firebeat_state::init_lights(write32_delegate out1, write32_delegate out2, write32_delegate out3)
{
	if(out1.isnull()) out1 = write32_delegate(FUNC(firebeat_state::lamp_output_w),this);
	if(out2.isnull()) out2 = write32_delegate(FUNC(firebeat_state::lamp_output2_w),this);
	if(out3.isnull()) out3 = write32_delegate(FUNC(firebeat_state::lamp_output3_w),this);

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x7d000804, 0x7d000807, out1);
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x7d000320, 0x7d000323, out2);
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x7d000324, 0x7d000327, out3);
}

void firebeat_state::init_firebeat()
{
	UINT8 *rom = memregion("user2")->base();

//  pc16552d_init(machine(), 0, 19660800, comm_uart_irq_callback, 0);     // Network UART
//  pc16552d_init(machine(), 1, 24000000, midi_uart_irq_callback, 0);     // MIDI UART

	m_extend_board_irq_enable = 0x3f;
	m_extend_board_irq_active = 0x00;

	m_cur_cab_data = cab_data;

	m_maincpu->ppc4xx_spu_set_tx_handler(write8_delegate(FUNC(firebeat_state::security_w), this));

	set_ibutton(rom);

	init_lights(write32_delegate(), write32_delegate(), write32_delegate());
}

DRIVER_INIT_MEMBER(firebeat_state,ppp)
{
	init_firebeat();
	init_lights(write32_delegate(FUNC(firebeat_state::lamp_output_ppp_w),this), write32_delegate(FUNC(firebeat_state::lamp_output2_ppp_w),this), write32_delegate(FUNC(firebeat_state::lamp_output3_ppp_w),this));
}

DRIVER_INIT_MEMBER(firebeat_state,ppd)
{
	init_firebeat();
	init_lights(write32_delegate(FUNC(firebeat_state::lamp_output_ppp_w),this), write32_delegate(FUNC(firebeat_state::lamp_output2_ppp_w),this), write32_delegate(FUNC(firebeat_state::lamp_output3_ppp_w),this));

	m_cur_cab_data = ppd_cab_data;
}

void firebeat_state::init_keyboard()
{
	// set keyboard timer
//  m_keyboard_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(firebeat_state::keyboard_timer_callback),this));
//  m_keyboard_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

DRIVER_INIT_MEMBER(firebeat_state,kbm)
{
	init_firebeat();
	init_lights(write32_delegate(FUNC(firebeat_state::lamp_output_kbm_w),this), write32_delegate(), write32_delegate());

	init_keyboard();

	m_cur_cab_data = kbm_cab_data;
}


/*****************************************************************************/

ROM_START( ppp )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc0, "user2", 0)    // Security dongle
	ROM_LOAD("gq977-ja", 0x00, 0xc0, BAD_DUMP CRC(55b5abdb) SHA1(d8da5bac005235480a1815bd0a79c3e8a63ebad1))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "977jaa01", 0, BAD_DUMP SHA1(59c03d8eb366167feef741d42d9d8b54bfeb3c1e) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "977jaa02", 0, SHA1(bd07c25ee3e1edc962997f6a5bb1700897231fb2) )
ROM_END

ROM_START( ppp1mp )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc0, "user2", 0)    // Security dongle
	ROM_LOAD( "gqa11-ja",     0x000000, 0x0000c0, CRC(2ed8e2ae) SHA1(b8c3410dab643111b2d2027068175ba018a0a67e) )

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a11jaa01", 0, SHA1(539ec6f1c1d198b0d6ce5543eadcbb4d9917fa42) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "a11jaa02", 0, SHA1(575069570cb4a2b58b199a1329d45b189a20fcc9) )
ROM_END

ROM_START( kbm )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq974-ja", 0x00, 0xc0, BAD_DUMP CRC(4578f29b) SHA1(faaeaf6357c1e86e898e7017566cfd2fc7ee3d6f))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "974jac01", 0, BAD_DUMP SHA1(c6145d7090e44c87f71ba626620d2ae2596a75ca) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "974jaa02", 1, BAD_DUMP SHA1(3b9946083239eb5687f66a49df24568bffa4fbbd) )
ROM_END

ROM_START( kbm2nd )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gca01-ja", 0x00, 0xc0, BAD_DUMP CRC(2bda339d) SHA1(031cb3f44e7a89cd62a9ba948f3d19d53a325abd))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a01jaa01", 0, BAD_DUMP SHA1(37bc3879719b3d3c6bc8a5691abd7aa4aec87d45) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "a01jaa02", 1, BAD_DUMP SHA1(a3fdeee0f85a7a9718c0fb1cc642ac22d3eff8db) )
ROM_END

ROM_START( kbm3rd )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))

	ROM_REGION(0xc8, "user2", 0)    // Security dongle
	ROM_LOAD("gca12-ja_gca12-aa.bin", 0x00, 0xc8, CRC(96b12482) SHA1(199f20d9fa53108b3fe02d91d6793af1554b0f6f))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a12jaa01", 0, SHA1(ea30bf1273bce772f09063bfc8a74df360c743a7) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "a12jaa02", 0, SHA1(6bf7adbd637a0ce0c19b57187d3e46fabea99363) )
ROM_END

ROM_START( popn4 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq986_gc986.bin", 0x00, 0xc8, CRC(5c11469a) SHA1(efbe2e4a8cbb1285122fad22e3cfe9d47310da02))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	ROM_REGION16_LE(0x1000000, "rf5c400", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gq986jaa01", 0, SHA1(e5368ac029b0bdf29943ae66677b5521ae1176e1) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE( "gq986jaa02", 0, SHA1(53367d3d5f91422fe386c42716492a0ae4332390) )
ROM_END

ROM_START( popn5 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP( "a02jaa03.21e", 0x000000, 0x080000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599) )

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gea04-ja_gca04-ja_gca04-jb.bin", 0x00, 0xc8, CRC(f48f62f7) SHA1(e28b3fd41fa4136cf9c271967d0e68f21a67ba1a))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP( "a02jaa04.3q",  0x000000, 0x080000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683) )

	ROM_REGION16_LE(0x1000000, "rf5c400", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a04jaa01", 0, SHA1(87136ddad1d786b4d5f04381fcbf679ab666e6c9) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "a04jaa02", 0, SHA1(49a017dde76f84829f6e99a678524c40665c3bfd) )
ROM_END

ROM_START( popn6 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gqa16-ja", 0x000000, 0x0000c0, CRC(a3393355) SHA1(6b28b972fe375e6ad0c614110c0ae3832cffccff) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	ROM_REGION16_LE(0x1000000, "rf5c400", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gqa16jaa01", 0, SHA1(7a7e475d06c74a273f821fdfde0743b33d566e4c) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE( "gqa16jaa02", 0, SHA1(e39067300e9440ff19cb98c1abc234fa3d5b26d1) )
ROM_END

ROM_START( popn7 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gcb00-ja", 0x00, 0xc0, CRC(d0a58c74) SHA1(fc1d8ad2f9d16743dc10b6e61a5a88ffa9c9dd2f))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	ROM_REGION16_LE(0x1000000, "rf5c400", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "b00jab01", 0, SHA1(259c733ca4d30281205b46b7bf8d60c9d01aa818) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "b00jaa02", 0, SHA1(c8ce2f8ee6aeeedef9c110a59e68fcec7b669ad6) )
ROM_END

ROM_START( popn8 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gqb30-ja", 0x000000, 0x0000c0, CRC(dbabb51b) SHA1(b53e971f544a654f0811e10eed40bee2e0393855) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	ROM_REGION16_LE(0x1000000, "rf5c400", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gqb30jaa01", 0, SHA1(0ff3e40e3717ce23337b3a2438bdaca01cba9e30) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gqb30jaa02", 0, SHA1(f067d502c23efe0267aada5706f5bc7a54605942) )
ROM_END

ROM_START( popnanm2 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gea02-ja", 0x000000, 0x0000c0, CRC(072f8624) SHA1(e869b85a891bf7f9c870fb581a9a2ddd70810e2c) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	ROM_REGION16_LE(0x1000000, "rf5c400", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gea02jaa01", 0, SHA1(e81203b6812336c4d00476377193340031ef11b1) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gea02jaa02", 0, SHA1(7212e399779f37a5dcb8317a8f635a3b3f620aa9) )
ROM_END

ROM_START( ppd )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq977-ko", 0x00, 0xc0, BAD_DUMP CRC(ee743323) SHA1(2042e45879795557ad3cc21b37962f6bf54da60d))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "977kaa01", 0, BAD_DUMP SHA1(7af9f4949ffa10ea5fc18b6c88c2abc710df3cf9) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "977kaa02", 1, SHA1(0feb5ac56269ad4a8401fcfe3bb98b01a0169177) )
ROM_END

ROM_START( ppp11 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq977-ja", 0x00, 0xc0, BAD_DUMP CRC(55b5abdb) SHA1(d8da5bac005235480a1815bd0a79c3e8a63ebad1))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gc977jaa01", 0, SHA1(7ed1f4b55105c93fec74468436bfb1d540bce944) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "gc977jaa02", 1, SHA1(74ce8c90575fd562807def7d561392d0f91f2bc6) )
ROM_END

// Beatmania III has a different BIOS and SPU program, and they aren't dumped yet
ROM_START( bm3core )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, BAD_DUMP CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))     // boots with KBM BIOS

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gca05-jc.bin", 0x00, 0xc8, CRC(a4c67c80) SHA1(a87609052fa879116350564353df7f5b70ef3ae5) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, BAD_DUMP CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	ROM_REGION16_LE(0x1000000, "rf5c400", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a05jca01", 0, SHA1(b89eced8a1325b087e3f875d1a643bebe9bad5c0) )

	DISK_REGION( "spu_ata:0:hdd:image" ) // HDD
	DISK_IMAGE_READONLY( "a05jca02", 0, NO_DUMP )
ROM_END

ROM_START( bm36th )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, BAD_DUMP CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))     // boots with KBM BIOS

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gca21-jc", 0x000000, 0x0000c8, NO_DUMP )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, BAD_DUMP CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	ROM_REGION16_LE(0x1000000, "rf5c400", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a21jca01", 0, SHA1(d1b888379cc0b2c2ab58fa2c5be49258043c3ea1) )

	DISK_REGION( "spu_ata:0:hdd:image" ) // HDD
	DISK_IMAGE_READONLY( "a21jca02", 0, NO_DUMP )
ROM_END

ROM_START( bm37th )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, BAD_DUMP CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))     // boots with KBM BIOS

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gcb07-jc", 0x000000, 0x0000c0, CRC(16115b6a) SHA1(dcb2a3346973941a946b2cdfd31a5a761f666ca3) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, BAD_DUMP CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	ROM_REGION16_LE(0x1000000, "rf5c400", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gcb07jca01", 0, SHA1(f906379bdebee314e2ca97c7756259c8c25897fd) )

	DISK_REGION( "spu_ata:0:hdd:image" ) // HDD
	DISK_IMAGE_READONLY( "gcb07jca02", 0, SHA1(6b8e17635825a6a43dc8d2721fe2eb0e0f39e940) )
ROM_END

ROM_START( bm3final )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, BAD_DUMP CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))     // boots with KBM BIOS

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gcc01-jc", 0x000000, 0x0000c0, CRC(9c49fed8) SHA1(212b87c1d25763117611ffb2a36ed568d429d2f4) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, BAD_DUMP CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	ROM_REGION16_LE(0x1000000, "rf5c400", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gcc01jca01", 0, SHA1(3e7af83670d791591ad838823422959987f7aab9) )

	DISK_REGION( "spu_ata:0:hdd:image" ) // HDD
	DISK_IMAGE_READONLY( "gcc01jca02", 0, SHA1(823e29bab11cb67069d822f5ffb2b90b9d3368d2) )
ROM_END

/*****************************************************************************/

GAME( 2000, ppp,      0,       firebeat,      ppp,  firebeat_state,   ppp,    ROT0,   "Konami",  "ParaParaParadise", MACHINE_NOT_WORKING)
GAME( 2000, ppd,      0,       firebeat,      ppp,  firebeat_state,   ppd,    ROT0,   "Konami",  "ParaParaDancing", MACHINE_NOT_WORKING)
GAME( 2000, ppp11,    0,       firebeat,      ppp,  firebeat_state,   ppp,    ROT0,   "Konami",  "ParaParaParadise v1.1", MACHINE_NOT_WORKING)
GAME( 2000, ppp1mp,   ppp,     firebeat,      ppp,  firebeat_state,   ppp,    ROT0,   "Konami",  "ParaParaParadise 1st Mix Plus", MACHINE_NOT_WORKING)
GAMEL(2000, kbm,      0,       firebeat2,     kbm,  firebeat_state,   kbm,    ROT270, "Konami",  "Keyboardmania", MACHINE_NOT_WORKING, layout_firebeat)
GAMEL(2000, kbm2nd,   0,       firebeat2,     kbm,  firebeat_state,   kbm,    ROT270, "Konami",  "Keyboardmania 2nd Mix", MACHINE_NOT_WORKING, layout_firebeat)
GAMEL(2001, kbm3rd,   0,       firebeat2,     kbm,  firebeat_state,   kbm,    ROT270, "Konami",  "Keyboardmania 3rd Mix", MACHINE_NOT_WORKING, layout_firebeat)
GAME( 2000, popn4,    0,       firebeat_spu,  popn, firebeat_state,   ppp,    ROT0,   "Konami",  "Pop'n Music 4", MACHINE_NOT_WORKING)
GAME( 2000, popn5,    0,       firebeat_spu,  popn, firebeat_state,   ppp,    ROT0,   "Konami",  "Pop'n Music 5", MACHINE_NOT_WORKING)
GAME( 2001, popn6,    0,       firebeat_spu,  popn, firebeat_state,   ppp,    ROT0,   "Konami",  "Pop'n Music 6", MACHINE_NOT_WORKING)
GAME( 2001, popn7,    0,       firebeat_spu,  popn, firebeat_state,   ppp,    ROT0,   "Konami",  "Pop'n Music 7", MACHINE_NOT_WORKING)
GAME( 2001, popnanm2, 0,       firebeat_spu,  popn, firebeat_state,   ppp,    ROT0,   "Konami",  "Pop'n Music Animelo 2", MACHINE_NOT_WORKING)
GAME( 2002, popn8,    0,       firebeat_spu,  popn, firebeat_state,   ppp,    ROT0,   "Konami",  "Pop'n Music 8", MACHINE_NOT_WORKING)
GAME( 2000, bm3core,  0,       firebeat_spu,  popn, firebeat_state,   ppp,    ROT0,   "Konami",  "Beatmania III Append Core Remix", MACHINE_NOT_WORKING)
GAME( 2001, bm36th,   0,       firebeat_spu,  popn, firebeat_state,   ppp,    ROT0,   "Konami",  "Beatmania III Append 6th Mix", MACHINE_NOT_WORKING)
GAME( 2002, bm37th,   0,       firebeat_spu,  popn, firebeat_state,   ppp,    ROT0,   "Konami",  "Beatmania III Append 7th Mix", MACHINE_NOT_WORKING)
GAME( 2003, bm3final, 0,       firebeat_spu,  popn, firebeat_state,   ppp,    ROT0,   "Konami",  "Beatmania III The Final", MACHINE_NOT_WORKING)
