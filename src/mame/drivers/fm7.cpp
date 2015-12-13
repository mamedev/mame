// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***********************************************************************************************

    Fujitsu Micro 7 (FM-7)

    12/05/2009 Skeleton driver.

    Computers in this series:

                 | Release |    Main CPU    |  Sub CPU  |              RAM              |
    =====================================================================================
    FM-8         | 1981-05 | M68A09 @ 1MHz  |  M6809    |    64K (main) + 48K (VRAM)    |
    FM-7         | 1982-11 | M68B09 @ 2MHz  |  M68B09   |    64K (main) + 48K (VRAM)    |
    FM-NEW7      | 1984-05 | M68B09 @ 2MHz  |  M68B09   |    64K (main) + 48K (VRAM)    |
    FM-77        | 1984-05 | M68B09 @ 2MHz  |  M68B09E  |  64/256K (main) + 48K (VRAM)  |
    FM-77AV      | 1985-10 | M68B09E @ 2MHz |  M68B09   | 128/192K (main) + 96K (VRAM)  |
    FM-77AV20    | 1986-10 | M68B09E @ 2MHz |  M68B09   | 128/192K (main) + 96K (VRAM)  |
    FM-77AV40    | 1986-10 | M68B09E @ 2MHz |  M68B09   | 192/448K (main) + 144K (VRAM) |
    FM-77AV20EX  | 1987-11 | M68B09E @ 2MHz |  M68B09   | 128/192K (main) + 96K (VRAM)  |
    FM-77AV40EX  | 1987-11 | M68B09E @ 2MHz |  M68B09   | 192/448K (main) + 144K (VRAM) |
    FM-77AV40SX  | 1988-11 | M68B09E @ 2MHz |  M68B09   | 192/448K (main) + 144K (VRAM) |

    Note: FM-77AV dumps probably come from a FM-77AV40SX. Shall we confirm that both computers
    used the same BIOS components?

    memory map info from http://www.nausicaa.net/~lgreenf/fm7page.htm
    see also http://retropc.net/ryu/xm7/xm7.shtml


    Known issues:
     - Beeper is not implemented
     - Keyboard repeat is not implemented
     - Optional Kanji ROM use is not implemented
     - Other optional hardware is not implemented (RS232, Z80 card...)
     - FM-77AV20 and later aren't working (extra features not yet implemented)

************************************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/2203intf.h"
#include "sound/wave.h"
#include "sound/beep.h"

#include "imagedev/cassette.h"
#include "formats/fm7_cas.h"
#include "imagedev/flopdrv.h"
#include "bus/centronics/dsjoy.h"
#include "includes/fm7.h"
#include "softlist.h"

/* key scancode conversion table
 * The FM-7 expects different scancodes when shift,ctrl or graph is held, or
 * when kana is active.
 */
	// TODO: fill in shift,ctrl,graph and kana code
static const UINT16 fm7_key_list[0x60][7] =
{ // norm  shift ctrl  graph kana  sh.kana scan
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x01},  // ESC
	{0x31, 0x21, 0xf9, 0xf9, 0xc7, 0x00, 0x02},  // 1
	{0x32, 0x22, 0xfa, 0xfa, 0xcc, 0x00, 0x03},  // 2
	{0x33, 0x23, 0xfb, 0xfb, 0xb1, 0xa7, 0x04},
	{0x34, 0x24, 0xfc, 0xfc, 0xb3, 0xa9, 0x05},
	{0x35, 0x25, 0xf2, 0xf2, 0xb4, 0xaa, 0x06},
	{0x36, 0x26, 0xf3, 0xf3, 0xb5, 0xab, 0x07},
	{0x37, 0x27, 0xf4, 0xf4, 0xd4, 0xac, 0x08},
	{0x38, 0x28, 0xf5, 0xf5, 0xd5, 0xad, 0x09},
	{0x39, 0x29, 0xf6, 0xf6, 0xd6, 0xae, 0x0a},  // 9
	{0x30, 0x00, 0xf7, 0xf7, 0xdc, 0xa6, 0x0b},  // 0
	{0x2d, 0x3d, 0x1e, 0x8c, 0xce, 0x00, 0x0c},  // -
	{0x5e, 0x7e, 0x1c, 0x8b, 0xcd, 0x00, 0x0d},  // ^
	{0x5c, 0x7c, 0xf1, 0xf1, 0xb0, 0x00, 0x0e},  // Yen
	{0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f},  // Backspace
	{0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x10},  // Tab
	{0x71, 0x51, 0x11, 0xfd, 0xc0, 0x00, 0x11},  // Q
	{0x77, 0x57, 0x17, 0xf8, 0xc3, 0x00, 0x12},  // W
	{0x65, 0x45, 0x05, 0xe4, 0xb2, 0xa8, 0x13},  // E
	{0x72, 0x52, 0x12, 0xe5, 0xbd, 0x00, 0x14},
	{0x74, 0x54, 0x14, 0x9c, 0xb6, 0x00, 0x15},
	{0x79, 0x59, 0x19, 0x9d, 0xdd, 0x00, 0x16},
	{0x75, 0x55, 0x15, 0xf0, 0xc5, 0x00, 0x17},
	{0x69, 0x49, 0x09, 0xe8, 0xc6, 0x00, 0x18},
	{0x6f, 0x4f, 0x0f, 0xe9, 0xd7, 0x00, 0x19},
	{0x70, 0x50, 0x10, 0x8d, 0xbe, 0x00, 0x1a},  // P
	{0x40, 0x60, 0x00, 0x8a, 0xde, 0x00, 0x1b},  // @
	{0x5b, 0x7b, 0x1b, 0xed, 0xdf, 0xa2, 0x1c},  // [
	{0x0d, 0x0d, 0x00, 0x0d, 0x0d, 0x0d, 0x1d},  // Return
	{0x61, 0x41, 0x01, 0x95, 0xc1, 0x00, 0x1e},  // A
	{0x73, 0x53, 0x13, 0x96, 0xc4, 0x00, 0x1f},  // S

	{0x64, 0x44, 0x04, 0xe6, 0xbc, 0x00, 0x20},  // D
	{0x66, 0x46, 0x06, 0xe7, 0xca, 0x00, 0x21},
	{0x67, 0x47, 0x07, 0x9e, 0xb7, 0x00, 0x22},
	{0x68, 0x48, 0x08, 0x9f, 0xb8, 0x00, 0x23},
	{0x6a, 0x4a, 0x0a, 0xea, 0xcf, 0x00, 0x24},
	{0x6b, 0x4b, 0x0b, 0xeb, 0xc9, 0x00, 0x25},
	{0x6c, 0x4c, 0x0c, 0x8e, 0xd8, 0x00, 0x26},  // L
	{0x3b, 0x2b, 0x00, 0x99, 0xda, 0x00, 0x27},  // ;
	{0x3a, 0x2a, 0x00, 0x94, 0xb9, 0x00, 0x28},  // :
	{0x5d, 0x7d, 0x1d, 0xec, 0xd1, 0xa3, 0x29},  // ]
	{0x7a, 0x5a, 0x1a, 0x80, 0xc2, 0xaf, 0x2a},  // Z
	{0x78, 0x58, 0x18, 0x81, 0xbb, 0x00, 0x2b},  // X
	{0x63, 0x43, 0x03, 0x82, 0xbf, 0x00, 0x2c},  // C
	{0x76, 0x56, 0x16, 0x83, 0xcb, 0x00, 0x2d},
	{0x62, 0x42, 0x02, 0x84, 0xba, 0x00, 0x2e},
	{0x6e, 0x4e, 0x0e, 0x85, 0xd0, 0x00, 0x2f},
	{0x6d, 0x4d, 0x0d, 0x86, 0xd3, 0x00, 0x30},  // M
	{0x2c, 0x3c, 0x00, 0x87, 0xc8, 0xa4, 0x31},  // <
	{0x2e, 0x3e, 0x00, 0x88, 0xd9, 0xa1, 0x32},  // >
	{0x2f, 0x3f, 0x00, 0x97, 0xd2, 0xa5, 0x33},  // /
	{0x22, 0x5f, 0x1f, 0xe0, 0xdb, 0x00, 0x34},  // "
	{0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x35},  // Space
	{0x2a, 0x2a, 0x00, 0x98, 0x2a, 0x2a, 0x36},  // Tenkey
	{0x2f, 0x2f, 0x00, 0x91, 0x2f, 0x2f, 0x37},
	{0x2b, 0x2b, 0x00, 0x99, 0x2b, 0x2b, 0x38},
	{0x2d, 0x2d, 0x00, 0xee, 0x2d, 0x2d, 0x39},
	{0x37, 0x37, 0x00, 0xe1, 0x37, 0x37, 0x3a},
	{0x38, 0x38, 0x00, 0xe2, 0x38, 0x38, 0x3b},
	{0x39, 0x39, 0x00, 0xe3, 0x39, 0x39, 0x3c},
	{0x3d, 0x3d, 0x00, 0xef, 0x3d, 0x3d, 0x3d},  // Tenkey =
	{0x34, 0x34, 0x00, 0x93, 0x34, 0x34, 0x3e},
	{0x35, 0x35, 0x00, 0x8f, 0x35, 0x35, 0x3f},

	{0x36, 0x36, 0x00, 0x92, 0x36, 0x36, 0x40},
	{0x2c, 0x2c, 0x00, 0x00, 0x2c, 0x2c, 0x41},
	{0x31, 0x31, 0x00, 0x9a, 0x31, 0x31, 0x42},
	{0x32, 0x32, 0x00, 0x90, 0x32, 0x32, 0x43},
	{0x33, 0x33, 0x00, 0x9b, 0x33, 0x33, 0x44},
	{0x0d, 0x0d, 0x00, 0x0d, 0x0d, 0x0d, 0x45},
	{0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x46},
	{0x2e, 0x2e, 0x00, 0x2e, 0x2e, 0x2e, 0x47},
	{0x12, 0x12, 0x00, 0x12, 0x12, 0x12, 0x48}, // INS
	{0x05, 0x05, 0x00, 0x05, 0x05, 0x05, 0x49},  // EL
	{0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x0c, 0x4a},  // CLS
	{0x7f, 0x7f, 0x00, 0x7f, 0x7f, 0x7f, 0x4b},  // DEL
	{0x11, 0x11, 0x00, 0x11, 0x11, 0x11, 0x4c},  // DUP
	{0x1e, 0x19, 0x00, 0x1e, 0x1e, 0x19, 0x4d},  // Cursor Up
	{0x0b, 0x0b, 0x00, 0x0b, 0x0b, 0x0b, 0x4e},  // HOME
	{0x1d, 0x02, 0x00, 0x1d, 0x1d, 0x02, 0x4f},  // Cursor Left
	{0x1f, 0x1a, 0x00, 0x1f, 0x1f, 0x1a, 0x50},  // Cursor Down
	{0x1c, 0x06, 0x00, 0x1c, 0x1c, 0x16, 0x51},  // Cursor Right
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c},  // BREAK
	{0x101, 0x00, 0x101, 0x101, 0x101, 0x00, 0x5d},  // PF1
	{0x102, 0x00, 0x102, 0x102, 0x102, 0x00, 0x5e},
	{0x103, 0x00, 0x103, 0x103, 0x103, 0x00, 0x5f},
	{0x104, 0x00, 0x104, 0x104, 0x104, 0x00, 0x60},
	{0x105, 0x00, 0x105, 0x105, 0x105, 0x00, 0x61},
	{0x106, 0x00, 0x106, 0x106, 0x106, 0x00, 0x62},
	{0x107, 0x00, 0x107, 0x107, 0x107, 0x00, 0x63},
	{0x108, 0x00, 0x108, 0x108, 0x108, 0x00, 0x64},
	{0x109, 0x00, 0x109, 0x109, 0x109, 0x00, 0x65},
	{0x10a, 0x00, 0x10a, 0x10a, 0x10a, 0x00, 0x66},  // PF10
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};


void fm7_state::main_irq_set_flag(UINT8 flag)
{
	m_irq_flags |= flag;

	if(m_irq_flags != 0)
		m_maincpu->set_input_line(M6809_IRQ_LINE,ASSERT_LINE);
}

void fm7_state::main_irq_clear_flag(UINT8 flag)
{
	m_irq_flags &= ~flag;

	if(m_irq_flags == 0)
		m_maincpu->set_input_line(M6809_IRQ_LINE,CLEAR_LINE);
}


void fm7_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_FM7_BEEPER_OFF:
		fm7_beeper_off(ptr, param);
		break;
	case TIMER_FM77AV_ENCODER_ACK:
		fm77av_encoder_ack(ptr, param);
		break;
	case TIMER_FM7_IRQ:
		fm7_timer_irq(ptr, param);
		break;
	case TIMER_FM7_SUBTIMER_IRQ:
		fm7_subtimer_irq(ptr, param);
		break;
	case TIMER_FM7_KEYBOARD_POLL:
		fm7_keyboard_poll(ptr, param);
		break;
	case TIMER_FM77AV_ALU_TASK_END:
		fm77av_alu_task_end(ptr, param);
		break;
	case TIMER_FM77AV_VSYNC:
		fm77av_vsync(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in fm7_state::device_timer");
	}
}


/*
 * Main CPU: I/O port 0xfd02
 *
 * On read: returns cassette data (bit 7) and printer status (bits 0-5)
 * On write: sets IRQ masks
 *   bit 0 - keypress
 *   bit 1 - printer
 *   bit 2 - timer
 *   bit 3 - not used
 *   bit 4 - MFD
 *   bit 5 - TXRDY
 *   bit 6 - RXRDY
 *   bit 7 - SYNDET
 *
 */
WRITE8_MEMBER(fm7_state::fm7_irq_mask_w)
{
	m_irq_mask = data;
	logerror("IRQ mask set: 0x%02x\n",m_irq_mask);
}

/*
 * Main CPU: I/O port 0xfd03
 *
 * On read: returns which IRQ is currently active (typically read by IRQ handler)
 *   bit 0 - keypress
 *   bit 1 - printer
 *   bit 2 - timer
 *   bit 3 - ???
 * On write: Buzzer/Speaker On/Off
 *   bit 0 - speaker on/off
 *   bit 6 - buzzer on for 205ms
 *   bit 7 - buzzer on/off
 */
READ8_MEMBER(fm7_state::fm7_irq_cause_r)
{
	UINT8 ret = ~m_irq_flags;

	// Timer and Printer IRQ flags are cleared when this port is read
	// Keyboard IRQ flag is cleared when the scancode is read from
	// either keyboard data port (main CPU 0xfd01 or sub CPU 0xd401)
	if(m_irq_flags & 0x04)
		main_irq_clear_flag(IRQ_FLAG_TIMER);
	if(m_irq_flags & 0x02)
		main_irq_clear_flag(IRQ_FLAG_PRINTER);

	logerror("IRQ flags read: 0x%02x\n",ret);
	return ret;
}

TIMER_CALLBACK_MEMBER(fm7_state::fm7_beeper_off)
{
	m_beeper->set_state(0);
	logerror("timed beeper off\n");
}

WRITE8_MEMBER(fm7_state::fm7_beeper_w)
{
	m_speaker_active = data & 0x01;

	if(!m_speaker_active)  // speaker not active, disable all beeper sound
	{
		m_beeper->set_state(0);
		return;
	}

	if(data & 0x80)
	{
		if(m_speaker_active)
			m_beeper->set_state(1);
	}
	else
		m_beeper->set_state(0);

	if(data & 0x40)
	{
		if(m_speaker_active)
		{
			m_beeper->set_state(1);
			logerror("timed beeper on\n");
			timer_set(attotime::from_msec(205), TIMER_FM7_BEEPER_OFF);
		}
	}
	logerror("beeper state: %02x\n",data);
}


/*
 *  Sub CPU: port 0xd403 (read-only)
 *  On read: timed buzzer sound
 */
READ8_MEMBER(fm7_state::fm7_sub_beeper_r)
{
	if(m_speaker_active)
	{
		m_beeper->set_state(1);
		logerror("timed beeper on\n");
		timer_set(attotime::from_msec(205), TIMER_FM7_BEEPER_OFF);
	}
	return 0xff;
}

READ8_MEMBER(fm7_state::vector_r)
{
	UINT8* RAM = memregion("maincpu")->base();
	UINT8* ROM = memregion("init")->base();
	UINT32 init_size = memregion("init")->bytes();

	if(m_init_rom_en)
		return ROM[(init_size-0x10)+offset];
	else
	{
		if(m_type == SYS_FM7)
			return RAM[0xfff0+offset];
		else
			return RAM[0x3fff0+offset];
	}
}

WRITE8_MEMBER(fm7_state::vector_w)
{
	UINT8* RAM = memregion("maincpu")->base();

	if(m_type == SYS_FM7)
		RAM[0xfff0+offset] = data;
	else
		RAM[0x3fff0+offset] = data;
}

/*
 * Main CPU: I/O port 0xfd04
 *
 *  bit 0 - attention IRQ active, clears flag when read.
 *  bit 1 - break key active
 */
READ8_MEMBER(fm7_state::fm7_fd04_r)
{
	UINT8 ret = 0xff;

	if(m_video.attn_irq != 0)
	{
		ret &= ~0x01;
		m_video.attn_irq = 0;
	}
	if(m_break_flag != 0)
	{
		ret &= ~0x02;
	}
	return ret;
}

/*
 *  Main CPU: I/O port 0xfd0f
 *
 *  On read, enables BASIC ROM at 0x8000 (default)
 *  On write, disables BASIC ROM, enables RAM (if more than 32kB)
 */
READ8_MEMBER(fm7_state::fm7_rom_en_r)
{
	if(!space.debugger_access())
	{
		UINT8* RAM = memregion("maincpu")->base();

		m_basic_rom_en = 1;
		if(m_type == SYS_FM7)
		{
			membank("bank1")->set_base(RAM+0x38000);
		}
		else
			fm7_mmr_refresh(space);
		logerror("BASIC ROM enabled\n");
	}
	return 0x00;
}

WRITE8_MEMBER(fm7_state::fm7_rom_en_w)
{
	UINT8* RAM = memregion("maincpu")->base();

	m_basic_rom_en = 0;
	if(m_type == SYS_FM7)
	{
		membank("bank1")->set_base(RAM+0x8000);
	}
	else
		fm7_mmr_refresh(space);
	logerror("BASIC ROM disabled\n");
}

/*
 *  Main CPU: port 0xfd10
 *  Initiate ROM enable. (FM-77AV and later only)
 *  Port is write-only.  Initiate ROM is on by default.
 *
 */
WRITE8_MEMBER(fm7_state::fm7_init_en_w)
{
	if(data & 0x02)
	{
		m_init_rom_en = 0;
		fm7_mmr_refresh(space);
	}
	else
	{
		m_init_rom_en = 1;
		fm7_mmr_refresh(space);
	}
}

/*
 *  Main CPU: I/O ports 0xfd18 - 0xfd1f
 *  Floppy Disk Controller (MB8877A)
 */
WRITE_LINE_MEMBER(fm7_state::fm7_fdc_intrq_w)
{
	m_fdc_irq_flag = state;
}

WRITE_LINE_MEMBER(fm7_state::fm7_fdc_drq_w)
{
	m_fdc_drq_flag = state;
}

READ8_MEMBER(fm7_state::fm7_fdc_r)
{
	UINT8 ret = 0;

	switch(offset)
	{
		case 0:
			return m_fdc->status_r(space, offset);
		case 1:
			return m_fdc->track_r(space, offset);
		case 2:
			return m_fdc->sector_r(space, offset);
		case 3:
			return m_fdc->data_r(space, offset);
		case 4:
			return m_fdc_side | 0xfe;
		case 5:
			return m_fdc_drive;
		case 6:
			// FM-7 always returns 0xff for this register
			return 0xff;
		case 7:
			if(m_fdc_irq_flag != 0)
				ret |= 0x40;
			if(m_fdc_drq_flag != 0)
				ret |= 0x80;
			return ret;
	}
	logerror("FDC: read from 0x%04x\n",offset+0xfd18);

	return 0x00;
}

WRITE8_MEMBER(fm7_state::fm7_fdc_w)
{
	switch(offset)
	{
		case 0:
			m_fdc->cmd_w(space, offset,data);
			break;
		case 1:
			m_fdc->track_w(space, offset,data);
			break;
		case 2:
			m_fdc->sector_w(space, offset,data);
			break;
		case 3:
			m_fdc->data_w(space, offset,data);
			break;
		case 4:
			m_fdc_side = data & 0x01;
			if (m_floppy)
				m_floppy->ss_w(data & 0x01);
			logerror("FDC: wrote %02x to 0x%04x (side)\n",data,offset+0xfd18);
			break;
		case 5:
			m_fdc_drive = data;
			if((data & 0x03) > 0x01)
			{
				m_fdc_drive = 0;
			}
			else
			{
				switch (data & 0x01)
				{
				case 0: m_floppy = m_floppy0->get_device(); break;
				case 1: m_floppy = m_floppy1->get_device(); break;
				}

				m_fdc->set_floppy(m_floppy);

				if (m_floppy)
					m_floppy->mon_w(!BIT(data, 7));

				logerror("FDC: wrote %02x to 0x%04x (drive)\n",data,offset+0xfd18);
			}
			break;
		case 6:
			// FM77AV and later only. FM-7 returns 0xff;
			// bit 6 = 320k(1)/640k(0) FDD
			// bits 2,3 = logical drive
			logerror("FDC: mode write - %02x\n",data);
			break;
		default:
			logerror("FDC: wrote %02x to 0x%04x\n",data,offset+0xfd18);
	}
}

/*
 *  Main CPU: I/O ports 0xfd00-0xfd01
 *  Sub CPU: I/O ports 0xd400-0xd401
 *
 *  The scancode of the last key pressed is stored in fd/d401, with the 9th
 *  bit (MSB) in bit 7 of fd/d400.  0xfd00 also holds a flag for the main
 *  CPU clock speed in bit 0 (0 = 1.2MHz, 1 = 2MHz)
 *  Clears keyboard IRQ flag
 */
READ8_MEMBER(fm7_state::fm7_keyboard_r)
{
	UINT8 ret;
	switch(offset)
	{
		case 0:
			ret = (m_current_scancode >> 1) & 0x80;
			ret |= 0x01; // 1 = 2MHz, 0 = 1.2MHz
			return ret;
		case 1:
			main_irq_clear_flag(IRQ_FLAG_KEY);
			return m_current_scancode & 0xff;
		default:
			return 0x00;
	}
}

READ8_MEMBER(fm7_state::fm7_sub_keyboard_r)
{
	UINT8 ret;
	switch(offset)
	{
		case 0:
			ret = (m_current_scancode >> 1) & 0x80;
			return ret;
		case 1:
			main_irq_clear_flag(IRQ_FLAG_KEY);
			return m_current_scancode & 0xff;
		default:
			return 0x00;
	}
}

/*
 *  Sub CPU: port 0xd431, 0xd432
 *  Keyboard encoder
 *
 *  d431 (R/W): Data register (8 bit)
 *  d432 (R/O): Status register
 *              bit 0 - ACK
 *              bit 7 - LATCH (0 if ready to receive)
 *
 *  Encoder commands:
 *      00 xx    : Set scancode format (FM-7, FM16B(?), Scan(Make/Break))
 *      01       : Get scancode format
 *      02 xx    : Set LED status
 *      03       : Get LED status
 *      04 xx    : Enable/Disable key repeat
 *      05 xx xx : Set repeat rate and time
 *      80 00    : Get RTC
 *      80 01 xx xx xx xx xx xx xx : Set RTC
 *      81 xx    : Video digitise
 *      82 xx    : Set video mode(?)
 *      83       : Get video mode(?)
 *      84 xx    : Video brightness (monitor?)
 *
 *  ACK is received after 5us.
 */
READ8_MEMBER(fm7_state::fm77av_key_encoder_r)
{
	UINT8 ret = 0xff;
	switch(offset)
	{
		case 0x00:  // data register
			if(m_encoder.rx_count > 0)
			{
				ret = m_encoder.buffer[m_encoder.position];
				m_encoder.position++;
				m_encoder.rx_count--;
				m_encoder.latch = 0;
			}
			if(m_encoder.rx_count > 0)
				m_encoder.latch = 1;  // more data to receive
			break;
		case 0x01:  // status register
			if(m_encoder.latch != 0)
				ret &= ~0x80;
			if(m_encoder.ack == 0)
				ret &= ~0x01;
			break;
	}

	return ret;
}

void fm7_state::fm77av_encoder_setup_command()
{
	switch(m_encoder.buffer[0])
	{
		case 0:  // set scancode format
			m_encoder.tx_count = 2;
			break;
		case 1:  // get scancode format
			m_encoder.tx_count = 1;
			break;
		case 2:  // set LED
			m_encoder.tx_count = 2;
			break;
		case 3:  // get LED
			m_encoder.tx_count = 1;
			break;
		case 4:  // enable repeat
			m_encoder.tx_count = 2;
			break;
		case 5:  // set repeat rate
			m_encoder.tx_count = 3;
			break;
		case 0x80:  // get/set RTC (at least two bytes, 9 if byte two = 0x01)
			m_encoder.tx_count = 2;
			break;
		case 0x81:  // digitise
			m_encoder.tx_count = 2;
			break;
		case 0x82:  // set screen mode
			m_encoder.tx_count = 2;
			break;
		case 0x83:  // get screen mode
			m_encoder.tx_count = 1;
			break;
		case 0x84:  // set monitor brightness
			m_encoder.tx_count = 2;
			break;
		default:
			m_encoder.tx_count = 0;
			m_encoder.rx_count = 0;
			m_encoder.position = 0;
			logerror("ENC: Unknown command 0x%02x sent, ignoring\n",m_encoder.buffer[0]);
	}
}

TIMER_CALLBACK_MEMBER(fm7_state::fm77av_encoder_ack)
{
	m_encoder.ack = 1;
}

void fm7_state::fm77av_encoder_handle_command()
{
	switch(m_encoder.buffer[0])
	{
		case 0:  // set keyboard scancode mode
			m_key_scan_mode = m_encoder.buffer[1];
			m_encoder.rx_count = 0;
			logerror("ENC: Keyboard set to mode %i\n",m_encoder.buffer[1]);
			break;
		case 1:  // get keyboard scancode mode
			m_encoder.buffer[0] = m_key_scan_mode;
			m_encoder.rx_count = 1;
			logerror("ENC: Command %02x received\n",m_encoder.buffer[0]);
			break;
		case 2:  // set LEDs
			m_encoder.rx_count = 0;
			logerror("ENC: Command %02x received\n",m_encoder.buffer[0]);
			break;
		case 3:  // get LEDs
			m_encoder.rx_count = 1;
			logerror("ENC: Command %02x received\n",m_encoder.buffer[0]);
			break;
		case 4:  // enable key repeat
			m_encoder.rx_count = 0;
			logerror("ENC: Command %02x received\n",m_encoder.buffer[0]);
			break;
		case 5:  // set key repeat rate
			m_key_repeat = m_encoder.buffer[2] * 10;
			m_key_delay = m_encoder.buffer[1] * 10;
			m_encoder.rx_count = 0;
			logerror("ENC: Keyboard repeat rate set to %i/%i\n",m_encoder.buffer[1],m_encoder.buffer[2]);
			break;
		case 0x80:  // get/set RTC
			if(m_encoder.buffer[1] == 0x01)
				m_encoder.rx_count = 0;
			else
				m_encoder.rx_count = 7;
			logerror("ENC: Command %02x %02x received\n",m_encoder.buffer[0],m_encoder.buffer[1]);
			break;
		case 0x81:  // digitise
			m_encoder.rx_count = 0;
			logerror("ENC: Command %02x received\n",m_encoder.buffer[0]);
			break;
		case 0x82:  // set screen mode
			m_encoder.rx_count = 0;
			logerror("ENC: Command %02x received\n",m_encoder.buffer[0]);
			break;
		case 0x83:  // get screen mode
			m_encoder.rx_count = 1;
			logerror("ENC: Command %02x received\n",m_encoder.buffer[0]);
			break;
		case 0x84:  // set monitor brightness
			m_encoder.rx_count = 0;
			logerror("ENC: Command %02x received\n",m_encoder.buffer[0]);
			break;
	}
	m_encoder.position = 0;
}

WRITE8_MEMBER(fm7_state::fm77av_key_encoder_w)
{
	m_encoder.ack = 0;
	if(offset == 0) // data register
	{
		if(m_encoder.position == 0)  // first byte
		{
			fm77av_encoder_setup_command();
		}
		if(m_encoder.position == 1)  // second byte
		{
			if(m_encoder.buffer[0] == 0x80 || m_encoder.buffer[1] == 0x01)
			{
				m_encoder.tx_count = 8; // 80 01 command is 9 bytes
			}
		}
		m_encoder.buffer[m_encoder.position] = data;
		m_encoder.position++;
		m_encoder.tx_count--;
		if(m_encoder.tx_count == 0)  // last byte
			fm77av_encoder_handle_command();

		// wait 5us to set ACK flag
		timer_set(attotime::from_usec(5), TIMER_FM77AV_ENCODER_ACK);

		//logerror("ENC: write 0x%02x to data register, moved to pos %i\n",data,m_encoder.position);
	}
}

WRITE_LINE_MEMBER(fm7_state::write_centronics_busy)
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER(fm7_state::write_centronics_fault)
{
	m_centronics_fault = state;
}

WRITE_LINE_MEMBER(fm7_state::write_centronics_ack)
{
	m_centronics_ack = state;
}

WRITE_LINE_MEMBER(fm7_state::write_centronics_perror)
{
	m_centronics_perror = state;
}

READ8_MEMBER(fm7_state::fm7_cassette_printer_r)
{
	// bit 7: cassette input
	// bit 5: printer DET2
	// bit 4: printer DTT1
	// bit 3: printer PE
	// bit 2: printer acknowledge
	// bit 1: printer error
	// bit 0: printer busy
	UINT8 ret = 0;

	if(m_cassette->input() > 0.03)
		ret |= 0x80;

	if(m_cassette->get_state() & CASSETTE_MOTOR_DISABLED)
		ret |= 0x80;  // cassette input is high when not in use.

	ret |= 0x70;
	ret |= m_centronics_perror << 3;
	ret |= m_centronics_ack << 2;
	ret |= m_centronics_fault << 1;
	ret |= m_centronics_busy;

	return ret;
}

WRITE8_MEMBER(fm7_state::fm7_cassette_printer_w)
{
	switch(offset)
	{
		case 0:
		// bit 7: SLCTIN (select?)
		// bit 6: printer strobe
		// bit 1: cassette motor
		// bit 0: cassette output
			if((data & 0x01) != (m_cp_prev & 0x01))
				m_cassette->output((data & 0x01) ? +1.0 : -1.0);
			if((data & 0x02) != (m_cp_prev & 0x02))
				m_cassette->change_state((data & 0x02) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);

			m_centronics->write_strobe(!BIT(data,6));
			m_centronics->write_select_in(!BIT(data,7));
			m_cp_prev = data;
			break;
		case 1:
		// Printer data
			m_cent_data_out->write(space, 0, data);
			break;
	}
}

/*
 *  Main CPU: 0xfd0b
 *   - bit 0: Boot mode: 0=BASIC, 1=DOS
 */
READ8_MEMBER(fm7_state::fm77av_boot_mode_r)
{
	UINT8 ret = 0xff;

	if(m_dsw->read() & 0x02)
		ret &= ~0x01;

	return ret;
}

/*
 *  Main CPU: I/O ports 0xfd0d-0xfd0e
 *  PSG AY-3-891x (FM-7), YM2203 - (FM-77AV and later)
 *  0xfd0d - function select (bit 1 = BDIR, bit 0 = BC1)
 *  0xfd0e - data register
 *  AY I/O ports are not connected to anything.
 */
void fm7_state::fm7_update_psg()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if(m_type == SYS_FM7)
	{
		switch(m_psg_regsel)
		{
			case 0x00:
				// High impedance
				break;
			case 0x01:
				// Data read
				m_psg_data = m_psg->data_r(space, 0);
				break;
			case 0x02:
				// Data write
				m_psg->data_w(space, 0,m_psg_data);
				break;
			case 0x03:
				// Address latch
				m_psg->address_w(space, 0,m_psg_data);
				break;
		}
	}
	else
	{   // FM-77AV and later use a YM2203
		switch(m_psg_regsel)
		{
			case 0x00:
				// High impedance
				break;
			case 0x01:
				// Data read
				m_psg_data = m_ym->read(space, 1);
				break;
			case 0x02:
				// Data write
				m_ym->write(space, 1,m_psg_data);
				logerror("YM: data write 0x%02x\n",m_psg_data);
				break;
			case 0x03:
				// Address latch
				m_ym->write(space, 0,m_psg_data);
				logerror("YM: address latch 0x%02x\n",m_psg_data);
				break;
			case 0x04:
				// Status register
				m_psg_data = m_ym->read(space, 0);
				break;
			case 0x09:
				// Joystick port read
				m_psg_data = m_joy1->read();
				break;
		}
	}
}

READ8_MEMBER(fm7_state::fm7_psg_select_r)
{
	return 0xff;
}

WRITE8_MEMBER(fm7_state::fm7_psg_select_w)
{
	m_psg_regsel = data & 0x03;
	fm7_update_psg();
}

WRITE8_MEMBER(fm7_state::fm77av_ym_select_w)
{
	m_psg_regsel = data & 0x0f;
	fm7_update_psg();
}

READ8_MEMBER(fm7_state::fm7_psg_data_r)
{
//  fm7_update_psg();
	return m_psg_data;
}

WRITE8_MEMBER(fm7_state::fm7_psg_data_w)
{
	m_psg_data = data;
//  fm7_update_psg();
}

WRITE8_MEMBER(fm7_state::fm77av_bootram_w)
{
	if(!(m_mmr.mode & 0x01))
		return;
	m_boot_ram[offset] = data;
}

// Shared RAM is only usable on the main CPU if the sub CPU is halted
READ8_MEMBER(fm7_state::fm7_main_shared_r)
{
	if(m_video.sub_halt != 0)
		return m_shared_ram[offset];
	else
		return 0xff;
}

WRITE8_MEMBER(fm7_state::fm7_main_shared_w)
{
	if(m_video.sub_halt != 0)
		m_shared_ram[offset] = data;
}

READ8_MEMBER(fm7_state::fm7_fmirq_r)
{
	UINT8 ret = 0xff;

	if(m_fm77av_ym_irq != 0)
		ret &= ~0x08;

	return ret;
}

READ8_MEMBER(fm7_state::fm77av_joy_1_r)
{
	return m_joy1->read();
}

READ8_MEMBER(fm7_state::fm77av_joy_2_r)
{
	return m_joy2->read();
}

READ8_MEMBER(fm7_state::fm7_unknown_r)
{
	// Port 0xFDFC is read by Dig Dug.  Controller port, perhaps?
	// Must return 0xff for it to read the keyboard.
	// Mappy uses ports FD15 and FD16.  On the FM77AV, this is the YM2203,
	// but on the FM-7, this is nothing, so we return 0xff for it to
	// read the keyboard correctly.
	return 0xff;
}

/*
 * Memory Management Register
 * Main CPU: 0xfd80 - 0xfd93  (FM-77L4, FM-77AV and later only)
 *
 * fd80-fd8f (R/W): RAM bank select for current segment (A19-A12)
 * fd90 (W/O): segment select register (3-bit, default = 0)
 * fd92 (W/O): window offset register (OA15-OA8)
 * fd93 (R/W): mode select register
 *              - bit 7: MMR enable/disable
 *              - bit 6: window enable/disable
 *              - bit 0: boot RAM read-write/read-only
 *
 */
READ8_MEMBER(fm7_state::fm7_mmr_r)
{
	if(offset < 0x10)
	{
		return m_mmr.bank_addr[m_mmr.segment][offset];
	}
	if(offset == 0x13)
		return m_mmr.mode;
	return 0xff;
}

void fm7_state::fm7_update_bank(address_space & space, int bank, UINT8 physical)
{
	address_map_bank_device* avbank[16] = { m_avbank1, m_avbank2, m_avbank3, m_avbank4, m_avbank5, m_avbank6, m_avbank7
			, m_avbank8, m_avbank9, m_avbank10, m_avbank11, m_avbank12, m_avbank13, m_avbank14, m_avbank15, m_avbank16 };

	avbank[bank]->set_bank(physical);
/*  UINT8* RAM = memregion("maincpu")->base();
    UINT16 size = 0xfff;
    char bank_name[10];

    if(bank == 15)
        size = 0xbff;

    sprintf(bank_name,"bank%d",bank+1);

    if(physical >= 0x10 && physical <= 0x1b)
    {
        switch(physical)
        {
            case 0x10:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vram0_r),this),write8_delegate(FUNC(fm7_state::fm7_vram0_w),this));
                break;
            case 0x11:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vram1_r),this),write8_delegate(FUNC(fm7_state::fm7_vram1_w),this));
                break;
            case 0x12:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vram2_r),this),write8_delegate(FUNC(fm7_state::fm7_vram2_w),this));
                break;
            case 0x13:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vram3_r),this),write8_delegate(FUNC(fm7_state::fm7_vram3_w),this));
                break;
            case 0x14:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vram4_r),this),write8_delegate(FUNC(fm7_state::fm7_vram4_w),this));
                break;
            case 0x15:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vram5_r),this),write8_delegate(FUNC(fm7_state::fm7_vram5_w),this));
                break;
            case 0x16:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vram6_r),this),write8_delegate(FUNC(fm7_state::fm7_vram6_w),this));
                break;
            case 0x17:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vram7_r),this),write8_delegate(FUNC(fm7_state::fm7_vram7_w),this));
                break;
            case 0x18:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vram8_r),this),write8_delegate(FUNC(fm7_state::fm7_vram8_w),this));
                break;
            case 0x19:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vram9_r),this),write8_delegate(FUNC(fm7_state::fm7_vram9_w),this));
                break;
            case 0x1a:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vramA_r),this),write8_delegate(FUNC(fm7_state::fm7_vramA_w),this));
                break;
            case 0x1b:
                space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_vramB_r),this),write8_delegate(FUNC(fm7_state::fm7_vramB_w),this));
                break;
        }
//      membank(bank+1)->set_base(RAM+(physical<<12)-0x10000);
        return;
    }
    if(physical == 0x1c)
    {
        space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_console_ram_banked_r),this),write8_delegate(FUNC(fm7_state::fm7_console_ram_banked_w),this));
        return;
    }
    if(physical == 0x1d)
    {
        space.install_readwrite_handler(bank*0x1000,(bank*0x1000)+size,read8_delegate(FUNC(fm7_state::fm7_sub_ram_ports_banked_r),this),write8_delegate(FUNC(fm7_state::fm7_sub_ram_ports_banked_w),this));
        return;
    }
    if(physical == 0x35)
    {
        if(m_init_rom_en && (m_type == SYS_FM11 || m_type == SYS_FM16))
        {
            RAM = memregion("init")->base();
            space.install_read_bank(bank*0x1000,(bank*0x1000)+size,bank_name);
            space.nop_write(bank*0x1000,(bank*0x1000)+size);
            membank(bank_name)->set_base(RAM+(physical<<12)-0x35000);
            return;
        }
    }
    if(physical == 0x36 || physical == 0x37)
    {
        if(m_init_rom_en && (m_type != SYS_FM11 && m_type != SYS_FM16))
        {
            RAM = memregion("init")->base();
            space.install_read_bank(bank*0x1000,(bank*0x1000)+size,bank_name);
            space.nop_write(bank*0x1000,(bank*0x1000)+size);
            membank(bank_name)->set_base(RAM+(physical<<12)-0x36000);
            return;
        }
    }
    if(physical > 0x37 && physical <= 0x3f)
    {
        if(m_basic_rom_en && (m_type != SYS_FM11 && m_type != SYS_FM16))
        {
            RAM = memregion("fbasic")->base();
            space.install_read_bank(bank*0x1000,(bank*0x1000)+size,bank_name);
            space.nop_write(bank*0x1000,(bank*0x1000)+size);
            membank(bank_name)->set_base(RAM+(physical<<12)-0x38000);
            return;
        }
    }
    space.install_readwrite_bank(bank*0x1000,(bank*0x1000)+size,bank_name);
    membank(bank_name)->set_base(RAM+(physical<<12));
    */
}

void fm7_state::fm7_mmr_refresh(address_space& space)
{
	int x;
	UINT16 window_addr;
	UINT8* RAM = memregion("maincpu")->base();

	if(m_mmr.enabled)
	{
		for(x=0;x<16;x++)
			fm7_update_bank(space,x,m_mmr.bank_addr[m_mmr.segment][x]);
	}
	else
	{
		// when MMR is disabled, 0x30000-0x3ffff is banked in
		for(x=0;x<16;x++)
			fm7_update_bank(space,x,0x30+x);
	}

	if(m_mmr.mode & 0x40)
	{
		// Handle window offset - 0x7c00-0x7fff will show the area of extended
		// memory (0x00000-0x0ffff) defined by the window address register
		// 0x00 = 0x07c00, 0x04 = 0x08000 ... 0xff = 0x07400.
		window_addr = ((m_mmr.window_offset << 8) + 0x7c00) & 0xffff;
//      if(window_addr < 0xfc00)
		{
			space.install_readwrite_bank(0x7c00,0x7fff,"bank24");
			membank("bank24")->set_base(RAM+window_addr);
		}
	}
	else
	{
		space.install_readwrite_handler(0x7000,0x7fff,read8_delegate(FUNC(address_map_bank_device::read8),(address_map_bank_device*)m_avbank8),write8_delegate(FUNC(address_map_bank_device::write8),(address_map_bank_device*)m_avbank8));
	}
	if(m_init_rom_en)
	{
		UINT8* ROM = memregion("init")->base();
		membank("init_bank_r")->set_base(ROM);
	}
	else
	{
		membank("init_bank_r")->set_base(RAM+0x36000);
	}
	if(m_basic_rom_en)
	{
		UINT8* ROM = memregion("fbasic")->base();
		if(ROM != nullptr)
			membank("fbasic_bank_r")->set_base(ROM);
	}
	else
	{
		membank("fbasic_bank_r")->set_base(RAM+0x38000);
	}
}

WRITE8_MEMBER(fm7_state::fm7_mmr_w)
{
	if(offset < 0x10)
	{
		m_mmr.bank_addr[m_mmr.segment][offset] = data;
		if(m_mmr.enabled)
			fm7_update_bank(space,offset,data);
		logerror("MMR: Segment %i, bank %i, set to  0x%02x\n",m_mmr.segment,offset,data);
		return;
	}
	switch(offset)
	{
		case 0x10:
			m_mmr.segment = data & 0x07;
			fm7_mmr_refresh(space);
			logerror("MMR: Active segment set to %i\n",m_mmr.segment);
			break;
		case 0x12:
			m_mmr.window_offset = data;
			fm7_mmr_refresh(space);
			logerror("MMR: Window offset set to %02x\n",data);
			break;
		case 0x13:
			m_mmr.mode = data;
			m_mmr.enabled = data & 0x80;
			fm7_mmr_refresh(space);
			logerror("MMR: Mode register set to %02x\n",data);
			break;
	}
}

/*
 *  Main CPU: ports 0xfd20-0xfd23
 *  Kanji ROM read ports
 *  FD20 (W/O): ROM address high
 *  FD21 (W/O): ROM address low
 *  FD22 (R/O): Kanji data high
 *  FD23 (R/O): Kanji data low
 *
 *  Kanji ROM is visible at 0x20000 (first half only?)
 */
READ8_MEMBER(fm7_state::fm7_kanji_r)
{
	UINT8* KROM = m_kanji->base();
	UINT32 addr = m_kanji_address << 1;

	switch(offset)
	{
		case 0:
		case 1:
			logerror("KANJI: read from invalid register %i\n",offset);
			return 0xff;  // write-only
		case 2:
			return KROM[addr];
		case 3:
			return KROM[addr+1];
		default:
			logerror("KANJI: read from invalid register %i\n",offset);
			return 0xff;
	}
}

WRITE8_MEMBER(fm7_state::fm7_kanji_w)
{
	UINT16 addr;

	switch(offset)
	{
		case 0:
			addr = ((data & 0xff) << 8) | (m_kanji_address & 0x00ff);
			m_kanji_address = addr;
			break;
		case 1:
			addr = (data & 0xff) | (m_kanji_address & 0xff00);
			m_kanji_address = addr;
			break;
		case 2:
		case 3:
		default:
			logerror("KANJI: write to invalid register %i\n",offset);
	}
}

TIMER_CALLBACK_MEMBER(fm7_state::fm7_timer_irq)
{
	if(m_irq_mask & IRQ_FLAG_TIMER)
	{
		main_irq_set_flag(IRQ_FLAG_TIMER);
	}
}

TIMER_CALLBACK_MEMBER(fm7_state::fm7_subtimer_irq)
{
	if(m_video.nmi_mask == 0 && m_video.sub_halt == 0)
		m_sub->set_input_line(INPUT_LINE_NMI,PULSE_LINE);
}

// When a key is pressed or released (in scan mode only), an IRQ is generated on the main CPU,
// or an FIRQ on the sub CPU, if masked.  Both CPUs have ports to read keyboard data.
// Scancodes are 9 bits in FM-7 mode, 8 bits in scan mode.
void fm7_state::key_press(UINT16 scancode)
{
	m_current_scancode = scancode;

	if(scancode == 0)
		return;

	if(m_irq_mask & IRQ_FLAG_KEY)
	{
		main_irq_set_flag(IRQ_FLAG_KEY);
	}
	else
	{
		m_sub->set_input_line(M6809_FIRQ_LINE,ASSERT_LINE);
	}
	logerror("KEY: sent scancode 0x%03x\n",scancode);
}

void fm7_state::fm7_keyboard_poll_scan()
{
	ioport_port* portnames[3] = { m_key1, m_key2, m_key3 };
	int bit = 0;
	int x,y;
	UINT32 keys;
	UINT32 modifiers = m_keymod->read();
	static const UINT16 modscancodes[6] = { 0x52, 0x53, 0x54, 0x55, 0x56, 0x5a };

	for(x=0;x<3;x++)
	{
		keys = portnames[x]->read();

		for(y=0;y<32;y++)  // loop through each bit in the port
		{
			if((keys & (1<<y)) != 0 && (m_key_data[x] & (1<<y)) == 0)
			{
				key_press(fm7_key_list[bit][6]); // key press
			}
			if((keys & (1<<y)) == 0 && (m_key_data[x] & (1<<y)) != 0)
			{
				key_press(fm7_key_list[bit][6] | 0x80); // key release
			}
			bit++;
		}

		m_key_data[x] = keys;
	}
	// check modifier keys
	bit = 0;
	for(y=0;x<7;x++)
	{
		if((modifiers & (1<<y)) != 0 && (m_mod_data & (1<<y)) == 0)
		{
			key_press(modscancodes[bit]); // key press
		}
		if((modifiers & (1<<y)) == 0 && (m_mod_data & (1<<y)) != 0)
		{
			key_press(modscancodes[bit] | 0x80); // key release
		}
		bit++;
	}
	m_mod_data = modifiers;
}

TIMER_CALLBACK_MEMBER(fm7_state::fm7_keyboard_poll)
{
	ioport_port* portnames[3] = { m_key1, m_key2, m_key3 };
	int x,y;
	int bit = 0;
	int mod = 0;
	UINT32 keys;
	UINT32 modifiers = m_keymod->read();

	if(m_key3->read() & 0x40000)
	{
		m_break_flag = 1;
		m_maincpu->set_input_line(M6809_FIRQ_LINE,ASSERT_LINE);
	}
	else
		m_break_flag = 0;

	if(m_key_scan_mode == KEY_MODE_SCAN)
	{
		// handle scancode mode
		fm7_keyboard_poll_scan();
		return;
	}

	// check key modifiers (Shift, Ctrl, Kana, etc...)
	if(modifiers & 0x02 || modifiers & 0x04)
		mod = 1;  // shift
	if(modifiers & 0x10)
		mod = 3;  // Graph  (shift has no effect with graph)
	if(modifiers & 0x01)
		mod = 2;  // ctrl (overrides shift, if also pressed)
	if(modifiers & 0x20)
		mod = 4;  // kana (overrides all)
	if((modifiers & 0x22) == 0x22 || (modifiers & 0x24) == 0x24)
		mod = 5;  // shifted kana

	for(x=0;x<3;x++)
	{
		keys = portnames[x]->read();

		for(y=0;y<32;y++)  // loop through each bit in the port
		{
			if((keys & (1<<y)) != 0 && (m_key_data[x] & (1<<y)) == 0)
			{
				key_press(fm7_key_list[bit][mod]); // key press
			}
			bit++;
		}

		m_key_data[x] = keys;
	}
}

IRQ_CALLBACK_MEMBER(fm7_state::fm7_irq_ack)
{
	if(irqline == M6809_FIRQ_LINE)
		m_maincpu->set_input_line(irqline,CLEAR_LINE);
	return -1;
}

IRQ_CALLBACK_MEMBER(fm7_state::fm7_sub_irq_ack)
{
	m_sub->set_input_line(irqline,CLEAR_LINE);
	return -1;
}

WRITE_LINE_MEMBER(fm7_state::fm77av_fmirq)
{
	if(state == 1)
	{
		// cannot be masked
		main_irq_set_flag(IRQ_FLAG_OTHER);
		m_fm77av_ym_irq = 1;
		logerror("YM: IRQ on\n");
	}
	else
	{
		main_irq_clear_flag(IRQ_FLAG_OTHER);
		m_fm77av_ym_irq = 0;
		logerror("YM: IRQ off\n");
	}
}

/*
   0000 - 7FFF: (RAM) BASIC working area, user's area
   8000 - FBFF: (ROM) F-BASIC ROM, extra user RAM
   FC00 - FC7F: more RAM, if 64kB is installed
   FC80 - FCFF: Shared RAM between main and sub CPU, available only when sub CPU is halted
   FD00 - FDFF: I/O space (6809 uses memory-mapped I/O)
   FE00 - FFEF: Boot rom
   FFF0 - FFFF: Interrupt vector table
*/
// The FM-7 has only 64kB RAM, so we'll worry about banking when we do the later models
static ADDRESS_MAP_START( fm7_mem, AS_PROGRAM, 8, fm7_state )
	AM_RANGE(0x0000,0x7fff) AM_RAM
	AM_RANGE(0x8000,0xfbff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2") // also F-BASIC ROM, when enabled
	AM_RANGE(0xfc00,0xfc7f) AM_RAM
	AM_RANGE(0xfc80,0xfcff) AM_READWRITE(fm7_main_shared_r,fm7_main_shared_w)
	// I/O space (FD00-FDFF)
	AM_RANGE(0xfd00,0xfd01) AM_READWRITE(fm7_keyboard_r,fm7_cassette_printer_w)
	AM_RANGE(0xfd02,0xfd02) AM_READWRITE(fm7_cassette_printer_r,fm7_irq_mask_w)  // IRQ mask
	AM_RANGE(0xfd03,0xfd03) AM_READWRITE(fm7_irq_cause_r,fm7_beeper_w)  // IRQ flags
	AM_RANGE(0xfd04,0xfd04) AM_READ(fm7_fd04_r)
	AM_RANGE(0xfd05,0xfd05) AM_READWRITE(fm7_subintf_r,fm7_subintf_w)
	AM_RANGE(0xfd06,0xfd0c) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd0d,0xfd0d) AM_READWRITE(fm7_psg_select_r,fm7_psg_select_w)
	AM_RANGE(0xfd0e,0xfd0e) AM_READWRITE(fm7_psg_data_r, fm7_psg_data_w)
	AM_RANGE(0xfd0f,0xfd0f) AM_READWRITE(fm7_rom_en_r,fm7_rom_en_w)
	AM_RANGE(0xfd10,0xfd17) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd18,0xfd1f) AM_READWRITE(fm7_fdc_r,fm7_fdc_w)
	AM_RANGE(0xfd20,0xfd23) AM_READWRITE(fm7_kanji_r,fm7_kanji_w)
	AM_RANGE(0xfd24,0xfd36) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd37,0xfd37) AM_WRITE(fm7_multipage_w)
	AM_RANGE(0xfd38,0xfd3f) AM_READWRITE(fm7_palette_r,fm7_palette_w)
	AM_RANGE(0xfd40,0xfdff) AM_READ(fm7_unknown_r)
	// Boot ROM
	AM_RANGE(0xfe00,0xffdf) AM_ROMBANK("bank17")
	AM_RANGE(0xffe0,0xffef) AM_RAM
	AM_RANGE(0xfff0,0xffff) AM_READWRITE(vector_r,vector_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fm8_mem, AS_PROGRAM, 8, fm7_state )
	AM_RANGE(0x0000,0x7fff) AM_RAM
	AM_RANGE(0x8000,0xfbff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2") // also F-BASIC ROM, when enabled
	AM_RANGE(0xfc00,0xfc7f) AM_RAM
	AM_RANGE(0xfc80,0xfcff) AM_READWRITE(fm7_main_shared_r,fm7_main_shared_w)
	// I/O space (FD00-FDFF)
	AM_RANGE(0xfd00,0xfd01) AM_READWRITE(fm7_keyboard_r,fm7_cassette_printer_w)
	AM_RANGE(0xfd02,0xfd02) AM_READWRITE(fm7_cassette_printer_r,fm7_irq_mask_w)  // IRQ mask
	AM_RANGE(0xfd03,0xfd03) AM_READWRITE(fm7_irq_cause_r,fm7_beeper_w)  // IRQ flags
	AM_RANGE(0xfd04,0xfd04) AM_READ(fm7_fd04_r)
	AM_RANGE(0xfd05,0xfd05) AM_READWRITE(fm7_subintf_r,fm7_subintf_w)
	AM_RANGE(0xfd06,0xfd0c) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd0f,0xfd0f) AM_READWRITE(fm7_rom_en_r,fm7_rom_en_w)
	AM_RANGE(0xfd10,0xfd17) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd18,0xfd1f) AM_READWRITE(fm7_fdc_r,fm7_fdc_w)
	AM_RANGE(0xfd20,0xfd23) AM_READWRITE(fm7_kanji_r,fm7_kanji_w)
	AM_RANGE(0xfd24,0xfd36) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd37,0xfd37) AM_WRITE(fm7_multipage_w)
	AM_RANGE(0xfd38,0xfd3f) AM_READWRITE(fm7_palette_r,fm7_palette_w)
	AM_RANGE(0xfd40,0xfdff) AM_READ(fm7_unknown_r)
	// Boot ROM
	AM_RANGE(0xfe00,0xffdf) AM_ROMBANK("bank17")
	AM_RANGE(0xffe0,0xffef) AM_RAM
	AM_RANGE(0xfff0,0xffff) AM_READWRITE(vector_r,vector_w)
ADDRESS_MAP_END

/*
   0000 - 3FFF: Video RAM bank 0 (Blue plane)
   4000 - 7FFF: Video RAM bank 1 (Red plane)
   8000 - BFFF: Video RAM bank 2 (Green plane)
   D000 - D37F: (RAM) working area
   D380 - D3FF: Shared RAM between main and sub CPU
   D400 - D4FF: I/O ports
   D800 - FFDF: (ROM) Graphics command code
   FFF0 - FFFF: Interrupt vector table
*/

static ADDRESS_MAP_START( fm7_sub_mem, AS_PROGRAM, 8, fm7_state )
	AM_RANGE(0x0000,0xbfff) AM_READWRITE(fm7_vram_r,fm7_vram_w) // VRAM
	AM_RANGE(0xc000,0xcfff) AM_RAM // Console RAM
	AM_RANGE(0xd000,0xd37f) AM_RAM // Work RAM
	AM_RANGE(0xd380,0xd3ff) AM_RAM AM_SHARE("shared_ram")
	// I/O space (D400-D4FF)
	AM_RANGE(0xd400,0xd401) AM_READ(fm7_sub_keyboard_r)
	AM_RANGE(0xd402,0xd402) AM_READ(fm7_cancel_ack)
	AM_RANGE(0xd403,0xd403) AM_READ(fm7_sub_beeper_r)
	AM_RANGE(0xd404,0xd404) AM_READ(fm7_attn_irq_r)
	AM_RANGE(0xd408,0xd408) AM_READWRITE(fm7_crt_r,fm7_crt_w)
	AM_RANGE(0xd409,0xd409) AM_READWRITE(fm7_vram_access_r,fm7_vram_access_w)
	AM_RANGE(0xd40a,0xd40a) AM_READWRITE(fm7_sub_busyflag_r,fm7_sub_busyflag_w)
	AM_RANGE(0xd40e,0xd40f) AM_WRITE(fm7_vram_offset_w)
	AM_RANGE(0xd800,0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fm11_mem, AS_PROGRAM, 8, fm7_state )
	AM_RANGE(0x0000,0x0fff) AM_DEVREADWRITE("av_bank1", address_map_bank_device, read8, write8)
	AM_RANGE(0x1000,0x1fff) AM_DEVREADWRITE("av_bank2", address_map_bank_device, read8, write8)
	AM_RANGE(0x2000,0x2fff) AM_DEVREADWRITE("av_bank3", address_map_bank_device, read8, write8)
	AM_RANGE(0x3000,0x3fff) AM_DEVREADWRITE("av_bank4", address_map_bank_device, read8, write8)
	AM_RANGE(0x4000,0x4fff) AM_DEVREADWRITE("av_bank5", address_map_bank_device, read8, write8)
	AM_RANGE(0x5000,0x5fff) AM_DEVREADWRITE("av_bank6", address_map_bank_device, read8, write8)
	AM_RANGE(0x6000,0x6fff) AM_DEVREADWRITE("av_bank7", address_map_bank_device, read8, write8)
	AM_RANGE(0x7000,0x7fff) AM_DEVREADWRITE("av_bank8", address_map_bank_device, read8, write8)
	AM_RANGE(0x8000,0x8fff) AM_DEVREADWRITE("av_bank9", address_map_bank_device, read8, write8)
	AM_RANGE(0x9000,0x9fff) AM_DEVREADWRITE("av_bank10", address_map_bank_device, read8, write8)
	AM_RANGE(0xa000,0xafff) AM_DEVREADWRITE("av_bank11", address_map_bank_device, read8, write8)
	AM_RANGE(0xb000,0xbfff) AM_DEVREADWRITE("av_bank12", address_map_bank_device, read8, write8)
	AM_RANGE(0xc000,0xcfff) AM_DEVREADWRITE("av_bank13", address_map_bank_device, read8, write8)
	AM_RANGE(0xd000,0xdfff) AM_DEVREADWRITE("av_bank14", address_map_bank_device, read8, write8)
	AM_RANGE(0xe000,0xefff) AM_DEVREADWRITE("av_bank15", address_map_bank_device, read8, write8)
	AM_RANGE(0xf000,0xfbff) AM_DEVREADWRITE("av_bank16", address_map_bank_device, read8, write8)
	AM_RANGE(0xfc00,0xfc7f) AM_RAM
	AM_RANGE(0xfc80,0xfcff) AM_READWRITE(fm7_main_shared_r,fm7_main_shared_w)
	// I/O space (FD00-FDFF)
	AM_RANGE(0xfd00,0xfd01) AM_READWRITE(fm7_keyboard_r,fm7_cassette_printer_w)
	AM_RANGE(0xfd02,0xfd02) AM_READWRITE(fm7_cassette_printer_r,fm7_irq_mask_w)  // IRQ mask
	AM_RANGE(0xfd03,0xfd03) AM_READWRITE(fm7_irq_cause_r,fm7_beeper_w)  // IRQ flags
	AM_RANGE(0xfd04,0xfd04) AM_READ(fm7_fd04_r)
	AM_RANGE(0xfd05,0xfd05) AM_READWRITE(fm7_subintf_r,fm7_subintf_w)
	AM_RANGE(0xfd06,0xfd0a) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd0b,0xfd0b) AM_READ(fm77av_boot_mode_r)
	AM_RANGE(0xfd0c,0xfd0c) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd0f,0xfd0f) AM_READWRITE(fm7_rom_en_r,fm7_rom_en_w)
	AM_RANGE(0xfd10,0xfd10) AM_WRITE(fm7_init_en_w)
	AM_RANGE(0xfd11,0xfd11) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd12,0xfd12) AM_READWRITE(fm77av_sub_modestatus_r,fm77av_sub_modestatus_w)
	AM_RANGE(0xfd13,0xfd13) AM_WRITE(fm77av_sub_bank_w)
	AM_RANGE(0xfd14,0xfd14) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd17,0xfd17) AM_READ(fm7_fmirq_r)
	AM_RANGE(0xfd18,0xfd1f) AM_READWRITE(fm7_fdc_r,fm7_fdc_w)
	AM_RANGE(0xfd20,0xfd23) AM_READWRITE(fm7_kanji_r,fm7_kanji_w)
	AM_RANGE(0xfd24,0xfd2b) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd30,0xfd34) AM_WRITE(fm77av_analog_palette_w)
	AM_RANGE(0xfd35,0xfd36) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd37,0xfd37) AM_WRITE(fm7_multipage_w)
	AM_RANGE(0xfd38,0xfd3f) AM_READWRITE(fm7_palette_r,fm7_palette_w)
	AM_RANGE(0xfd40,0xfd7f) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd80,0xfd93) AM_READWRITE(fm7_mmr_r,fm7_mmr_w)
	AM_RANGE(0xfd94,0xfdff) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfe00,0xffdf) AM_RAM_WRITE(fm77av_bootram_w) AM_SHARE("boot_ram")
	AM_RANGE(0xffe0,0xffef) AM_RAM
	AM_RANGE(0xfff0,0xffff) AM_READWRITE(vector_r,vector_w)
ADDRESS_MAP_END

// Much of this is guesswork at the moment
static ADDRESS_MAP_START( fm11_sub_mem, AS_PROGRAM, 8, fm7_state )
	AM_RANGE(0x0000,0x7fff) AM_READWRITE(fm7_vram_r,fm7_vram_w) // VRAM
	AM_RANGE(0x8000,0x8fff) AM_RAM // Console RAM(?)
	AM_RANGE(0x9000,0x9f7f) AM_RAM // Work RAM(?)
	AM_RANGE(0x9f80,0x9fff) AM_RAM AM_SHARE("shared_ram")
	AM_RANGE(0xafe0,0xafe3) AM_RAM
//  AM_RANGE(0xafe4,0xafe4) AM_READWRITE(fm7_sub_busyflag_r,fm7_sub_busyflag_w)
	AM_RANGE(0xafe6,0xafe6) AM_READWRITE(fm77av_video_flags_r,fm77av_video_flags_w)
	AM_RANGE(0xaff0,0xaff0) AM_READWRITE(fm7_sub_busyflag_r,fm7_sub_busyflag_w)
	AM_RANGE(0xc000,0xffff) AM_ROM // sybsystem ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fm11_x86_mem, AS_PROGRAM, 8, fm7_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0xfefff) AM_RAM
	AM_RANGE(0xff000,0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fm11_x86_io, AS_IO, 8, fm7_state )
	AM_RANGE(0xfd00,0xfd01) AM_READWRITE(fm7_keyboard_r,fm7_cassette_printer_w)
	AM_RANGE(0xfd02,0xfd02) AM_READWRITE(fm7_cassette_printer_r,fm7_irq_mask_w)  // IRQ mask
	AM_RANGE(0xfd03,0xfd03) AM_READWRITE(fm7_irq_cause_r,fm7_beeper_w)  // IRQ flags
	AM_RANGE(0xfd04,0xfd04) AM_READ(fm7_fd04_r)
	AM_RANGE(0xfd05,0xfd05) AM_READWRITE(fm7_subintf_r,fm7_subintf_w)
	AM_RANGE(0xfd06,0xfd0c) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd0f,0xfd0f) AM_READWRITE(fm7_rom_en_r,fm7_rom_en_w)
	AM_RANGE(0xfd10,0xfd17) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd18,0xfd1f) AM_READWRITE(fm7_fdc_r,fm7_fdc_w)
	AM_RANGE(0xfd20,0xfd23) AM_READWRITE(fm7_kanji_r,fm7_kanji_w)
	AM_RANGE(0xfd24,0xfd36) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd37,0xfd37) AM_WRITE(fm7_multipage_w)
	AM_RANGE(0xfd38,0xfd3f) AM_READWRITE(fm7_palette_r,fm7_palette_w)
	AM_RANGE(0xfd40,0xfdff) AM_READ(fm7_unknown_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fm16_mem, AS_PROGRAM, 16, fm7_state )
	AM_RANGE(0x00000,0xfbfff) AM_RAM
	AM_RANGE(0xfc000,0xfffff) AM_ROM // IPL
ADDRESS_MAP_END

static ADDRESS_MAP_START( fm16_io, AS_IO, 16, fm7_state )
	AM_RANGE(0xfd00,0xfd01) AM_READWRITE8(fm7_keyboard_r,fm7_cassette_printer_w,0xffff)
	AM_RANGE(0xfd02,0xfd03) AM_READWRITE8(fm7_cassette_printer_r,fm7_irq_mask_w,0x00ff)  // IRQ mask
	AM_RANGE(0xfd02,0xfd03) AM_READWRITE8(fm7_irq_cause_r,fm7_beeper_w,0xff00)  // IRQ flags
	AM_RANGE(0xfd04,0xfd05) AM_READ8(fm7_fd04_r,0x00ff)
	AM_RANGE(0xfd04,0xfd05) AM_READWRITE8(fm7_subintf_r,fm7_subintf_w,0xff00)
//  AM_RANGE(0xfd06,0xfd0c) AM_READ8(fm7_unknown_r,0xffff)
	AM_RANGE(0xfd0e,0xfd0f) AM_READWRITE8(fm7_rom_en_r,fm7_rom_en_w,0xff00)
//  AM_RANGE(0xfd10,0xfd17) AM_READ8(fm7_unknown_r,0xffff)
	AM_RANGE(0xfd18,0xfd1f) AM_READWRITE8(fm7_fdc_r,fm7_fdc_w,0xffff)
	AM_RANGE(0xfd20,0xfd23) AM_READWRITE8(fm7_kanji_r,fm7_kanji_w,0xffff)
//  AM_RANGE(0xfd24,0xfd36) AM_READ8(fm7_unknown_r,0xffff)
	AM_RANGE(0xfd36,0xfd37) AM_WRITE8(fm7_multipage_w,0xff00)
	AM_RANGE(0xfd38,0xfd3f) AM_READWRITE8(fm7_palette_r,fm7_palette_w,0xffff)
//  AM_RANGE(0xfd40,0xfdff) AM_READ8(fm7_unknown_r,0xffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fm16_sub_mem, AS_PROGRAM, 8, fm7_state )
	AM_RANGE(0x0000,0xafff) AM_READWRITE(fm7_vram_r,fm7_vram_w) // VRAM
	AM_RANGE(0xb000,0xffff) AM_ROM // subsystem ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fm77av_mem, AS_PROGRAM, 8, fm7_state )
	AM_RANGE(0x0000,0x0fff) AM_DEVREADWRITE("av_bank1", address_map_bank_device, read8, write8)
	AM_RANGE(0x1000,0x1fff) AM_DEVREADWRITE("av_bank2", address_map_bank_device, read8, write8)
	AM_RANGE(0x2000,0x2fff) AM_DEVREADWRITE("av_bank3", address_map_bank_device, read8, write8)
	AM_RANGE(0x3000,0x3fff) AM_DEVREADWRITE("av_bank4", address_map_bank_device, read8, write8)
	AM_RANGE(0x4000,0x4fff) AM_DEVREADWRITE("av_bank5", address_map_bank_device, read8, write8)
	AM_RANGE(0x5000,0x5fff) AM_DEVREADWRITE("av_bank6", address_map_bank_device, read8, write8)
	AM_RANGE(0x6000,0x6fff) AM_DEVREADWRITE("av_bank7", address_map_bank_device, read8, write8)
	AM_RANGE(0x7000,0x7fff) AM_DEVREADWRITE("av_bank8", address_map_bank_device, read8, write8)
	AM_RANGE(0x8000,0x8fff) AM_DEVREADWRITE("av_bank9", address_map_bank_device, read8, write8)
	AM_RANGE(0x9000,0x9fff) AM_DEVREADWRITE("av_bank10", address_map_bank_device, read8, write8)
	AM_RANGE(0xa000,0xafff) AM_DEVREADWRITE("av_bank11", address_map_bank_device, read8, write8)
	AM_RANGE(0xb000,0xbfff) AM_DEVREADWRITE("av_bank12", address_map_bank_device, read8, write8)
	AM_RANGE(0xc000,0xcfff) AM_DEVREADWRITE("av_bank13", address_map_bank_device, read8, write8)
	AM_RANGE(0xd000,0xdfff) AM_DEVREADWRITE("av_bank14", address_map_bank_device, read8, write8)
	AM_RANGE(0xe000,0xefff) AM_DEVREADWRITE("av_bank15", address_map_bank_device, read8, write8)
	AM_RANGE(0xf000,0xfbff) AM_DEVREADWRITE("av_bank16", address_map_bank_device, read8, write8)
	AM_RANGE(0xfc00,0xfc7f) AM_RAM
	AM_RANGE(0xfc80,0xfcff) AM_READWRITE(fm7_main_shared_r,fm7_main_shared_w)
	// I/O space (FD00-FDFF)
	AM_RANGE(0xfd00,0xfd01) AM_READWRITE(fm7_keyboard_r,fm7_cassette_printer_w)
	AM_RANGE(0xfd02,0xfd02) AM_READWRITE(fm7_cassette_printer_r,fm7_irq_mask_w)  // IRQ mask
	AM_RANGE(0xfd03,0xfd03) AM_READWRITE(fm7_irq_cause_r,fm7_beeper_w)  // IRQ flags
	AM_RANGE(0xfd04,0xfd04) AM_READ(fm7_fd04_r)
	AM_RANGE(0xfd05,0xfd05) AM_READWRITE(fm7_subintf_r,fm7_subintf_w)
	AM_RANGE(0xfd06,0xfd0a) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd0b,0xfd0b) AM_READ(fm77av_boot_mode_r)
	AM_RANGE(0xfd0c,0xfd0c) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd0d,0xfd0d) AM_READWRITE(fm7_psg_select_r,fm7_psg_select_w)
	AM_RANGE(0xfd0e,0xfd0e) AM_READWRITE(fm7_psg_data_r, fm7_psg_data_w)
	AM_RANGE(0xfd0f,0xfd0f) AM_READWRITE(fm7_rom_en_r,fm7_rom_en_w)
	AM_RANGE(0xfd10,0xfd10) AM_WRITE(fm7_init_en_w)
	AM_RANGE(0xfd11,0xfd11) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd12,0xfd12) AM_READWRITE(fm77av_sub_modestatus_r,fm77av_sub_modestatus_w)
	AM_RANGE(0xfd13,0xfd13) AM_WRITE(fm77av_sub_bank_w)
	AM_RANGE(0xfd14,0xfd14) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd15,0xfd15) AM_READWRITE(fm7_psg_select_r,fm77av_ym_select_w)
	AM_RANGE(0xfd16,0xfd16) AM_READWRITE(fm7_psg_data_r,fm7_psg_data_w)
	AM_RANGE(0xfd17,0xfd17) AM_READ(fm7_fmirq_r)
	AM_RANGE(0xfd18,0xfd1f) AM_READWRITE(fm7_fdc_r,fm7_fdc_w)
	AM_RANGE(0xfd20,0xfd23) AM_READWRITE(fm7_kanji_r,fm7_kanji_w)
	AM_RANGE(0xfd24,0xfd2b) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd30,0xfd34) AM_WRITE(fm77av_analog_palette_w)
	AM_RANGE(0xfd35,0xfd36) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd37,0xfd37) AM_WRITE(fm7_multipage_w)
	AM_RANGE(0xfd38,0xfd3f) AM_READWRITE(fm7_palette_r,fm7_palette_w)
	AM_RANGE(0xfd40,0xfd7f) AM_READ(fm7_unknown_r)
	AM_RANGE(0xfd80,0xfd93) AM_READWRITE(fm7_mmr_r,fm7_mmr_w)
	AM_RANGE(0xfd94,0xfdff) AM_READ(fm7_unknown_r)
	// Boot ROM (RAM on FM77AV and later)
	AM_RANGE(0xfe00,0xffdf) AM_RAM_WRITE(fm77av_bootram_w) AM_SHARE("boot_ram")
	AM_RANGE(0xffe0,0xffef) AM_RAM
	AM_RANGE(0xfff0,0xffff) AM_READWRITE(vector_r,vector_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fm77av_sub_mem, AS_PROGRAM, 8, fm7_state )
	AM_RANGE(0x0000,0xbfff) AM_READWRITE(fm7_vram_r,fm7_vram_w) // VRAM
	AM_RANGE(0xc000,0xcfff) AM_RAM AM_REGION("maincpu",0x1c000) // Console RAM
	AM_RANGE(0xd000,0xd37f) AM_RAM AM_REGION("maincpu",0x1d000) // Work RAM
	AM_RANGE(0xd380,0xd3ff) AM_RAM AM_SHARE("shared_ram")
	// I/O space (D400-D4FF)
	AM_RANGE(0xd400,0xd401) AM_READ(fm7_sub_keyboard_r)
	AM_RANGE(0xd402,0xd402) AM_READ(fm7_cancel_ack)
	AM_RANGE(0xd403,0xd403) AM_READ(fm7_sub_beeper_r)
	AM_RANGE(0xd404,0xd404) AM_READ(fm7_attn_irq_r)
	AM_RANGE(0xd408,0xd408) AM_READWRITE(fm7_crt_r,fm7_crt_w)
	AM_RANGE(0xd409,0xd409) AM_READWRITE(fm7_vram_access_r,fm7_vram_access_w)
	AM_RANGE(0xd40a,0xd40a) AM_READWRITE(fm7_sub_busyflag_r,fm7_sub_busyflag_w)
	AM_RANGE(0xd40e,0xd40f) AM_WRITE(fm7_vram_offset_w)
	AM_RANGE(0xd410,0xd42b) AM_READWRITE(fm77av_alu_r, fm77av_alu_w)
	AM_RANGE(0xd430,0xd430) AM_READWRITE(fm77av_video_flags_r,fm77av_video_flags_w)
	AM_RANGE(0xd431,0xd432) AM_READWRITE(fm77av_key_encoder_r,fm77av_key_encoder_w)
	AM_RANGE(0xd500,0xd7ff) AM_RAM AM_REGION("maincpu",0x1d500) // Work RAM
	AM_RANGE(0xd800,0xdfff) AM_ROMBANK("bank20")
	AM_RANGE(0xe000,0xffff) AM_ROMBANK("bank21")
ADDRESS_MAP_END

static ADDRESS_MAP_START( fm7_banked_mem, AS_PROGRAM, 8, fm7_state)
	// Extended RAM
	AM_RANGE(0x00000,0x0ffff) AM_RAM AM_REGION("maincpu",0x00000)

	// Sub CPU space
	AM_RANGE(0x10000,0x1bfff) AM_READWRITE(fm7_vram_r,fm7_vram_w) // VRAM
	AM_RANGE(0x1c000,0x1cfff) AM_RAM AM_REGION("maincpu",0x1c000) // Console RAM
	AM_RANGE(0x1d000,0x1d37f) AM_RAM AM_REGION("maincpu",0x1d000) // Work RAM
	AM_RANGE(0x1d380,0x1d3ff) AM_RAM AM_SHARE("shared_ram")
	// I/O space (D400-D4FF)
	AM_RANGE(0x1d400,0x1d401) AM_READ(fm7_sub_keyboard_r)
	AM_RANGE(0x1d402,0x1d402) AM_READ(fm7_cancel_ack)
	AM_RANGE(0x1d403,0x1d403) AM_READ(fm7_sub_beeper_r)
	AM_RANGE(0x1d404,0x1d404) AM_READ(fm7_attn_irq_r)
	AM_RANGE(0x1d408,0x1d408) AM_READWRITE(fm7_crt_r,fm7_crt_w)
	AM_RANGE(0x1d409,0x1d409) AM_READWRITE(fm7_vram_access_r,fm7_vram_access_w)
	AM_RANGE(0x1d40a,0x1d40a) AM_READWRITE(fm7_sub_busyflag_r,fm7_sub_busyflag_w)
	AM_RANGE(0x1d40e,0x1d40f) AM_WRITE(fm7_vram_offset_w)
	AM_RANGE(0x1d410,0x1d42b) AM_READWRITE(fm77av_alu_r, fm77av_alu_w)
	AM_RANGE(0x1d430,0x1d430) AM_READWRITE(fm77av_video_flags_r,fm77av_video_flags_w)
	AM_RANGE(0x1d431,0x1d432) AM_READWRITE(fm77av_key_encoder_r,fm77av_key_encoder_w)
	AM_RANGE(0x1d500,0x1d7ff) AM_RAM AM_REGION("maincpu",0x1d500) // Work RAM
	AM_RANGE(0x1d800,0x1dfff) AM_ROMBANK("bank20")
	AM_RANGE(0x1e000,0x1ffff) AM_ROMBANK("bank21")

	// more RAM?
	AM_RANGE(0x20000,0x2ffff) AM_RAM AM_REGION("maincpu",0x20000)

	// Main CPU space
	AM_RANGE(0x30000,0x35fff) AM_RAM AM_REGION("maincpu",0x30000)
	AM_RANGE(0x36000,0x37fff) AM_READ_BANK("init_bank_r") AM_WRITE_BANK("init_bank_w")
	AM_RANGE(0x38000,0x3fbff) AM_READ_BANK("fbasic_bank_r") AM_WRITE_BANK("fbasic_bank_w")
	AM_RANGE(0x3fc00,0x3ffff) AM_RAM AM_REGION("maincpu",0x3fc00)

ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( fm7_keyboard )
	PORT_START("key1")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(27)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')  PORT_CHAR('=')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xEF\xBF\xA5")
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')

	PORT_START("key2")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_")  PORT_CHAR('_')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey *") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey /") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey =")
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 5") PORT_CODE(KEYCODE_5_PAD)

	PORT_START("key3")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey ,")
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("EL") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CLS") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DUP") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF8") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF9") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF10") PORT_CODE(KEYCODE_F10)

	PORT_START("key_modifiers")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CAP") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kana") PORT_CODE(KEYCODE_RCONTROL) PORT_TOGGLE

	PORT_START("joy1")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_JOYSTICK_UP) PORT_NAME("1P Joystick Up") PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_JOYSTICK_DOWN) PORT_NAME("1P Joystick Down") PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_JOYSTICK_LEFT) PORT_NAME("1P Joystick Left") PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_JOYSTICK_RIGHT) PORT_NAME("1P Joystick Right") PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_BUTTON2) PORT_NAME("1P Joystick Button 2") PORT_PLAYER(1)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_BUTTON1) PORT_NAME("1P Joystick Button 1") PORT_PLAYER(1)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_UNUSED)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("joy2")
	PORT_BIT(0x01,IP_ACTIVE_LOW,IPT_JOYSTICK_UP) PORT_NAME("2P Joystick Up") PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x02,IP_ACTIVE_LOW,IPT_JOYSTICK_DOWN) PORT_NAME("2P Joystick Down") PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x04,IP_ACTIVE_LOW,IPT_JOYSTICK_LEFT) PORT_NAME("2P Joystick Left") PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x08,IP_ACTIVE_LOW,IPT_JOYSTICK_RIGHT) PORT_NAME("2P Joystick Right") PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x10,IP_ACTIVE_LOW,IPT_BUTTON2) PORT_NAME("2P Joystick Button 2") PORT_PLAYER(2)
	PORT_BIT(0x20,IP_ACTIVE_LOW,IPT_BUTTON1) PORT_NAME("2P Joystick Button 1") PORT_PLAYER(2)
	PORT_BIT(0x40,IP_ACTIVE_LOW,IPT_UNUSED)
	PORT_BIT(0x80,IP_ACTIVE_LOW,IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( fm7 )
	PORT_INCLUDE( fm7_keyboard )

	PORT_START("DSW")
	PORT_DIPNAME(0x01,0x01,"Switch A") PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x01,DEF_STR( On ))
	PORT_DIPNAME(0x02,0x02,"Boot mode") PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(0x00,"DOS")
	PORT_DIPSETTING(0x02,"BASIC")
	PORT_DIPNAME(0x04,0x00,"Switch C") PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x04,DEF_STR( On ))
	PORT_DIPNAME(0x08,0x00,"FM-8 Compatibility mode") PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x08,DEF_STR( On ))
INPUT_PORTS_END

static INPUT_PORTS_START( fm8 )
	PORT_INCLUDE( fm7_keyboard )

	PORT_START("DSW")
	PORT_DIPNAME(0x02,0x02,"Boot mode") PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(0x00,"DOS")
	PORT_DIPSETTING(0x02,"BASIC")
INPUT_PORTS_END

DRIVER_INIT_MEMBER(fm7_state,fm7)
{
//  m_shared_ram = auto_alloc_array(machine(),UINT8,0x80);
	m_video_ram = auto_alloc_array(machine(),UINT8,0x18000);  // 2 pages on some systems
	m_timer = timer_alloc(TIMER_FM7_IRQ);
	m_subtimer = timer_alloc(TIMER_FM7_SUBTIMER_IRQ);
	m_keyboard_timer = timer_alloc(TIMER_FM7_KEYBOARD_POLL);
	m_fm77av_vsync_timer = timer_alloc(TIMER_FM77AV_VSYNC);
}

MACHINE_START_MEMBER(fm7_state,fm7)
{
	// The FM-7 has no initialisation ROM, and no other obvious
	// way to set the reset vector, so for now this will have to do.
	UINT8* RAM = memregion("maincpu")->base();

	RAM[0xfffe] = 0xfe;
	RAM[0xffff] = 0x00;

	memset(m_shared_ram,0xff,0x80);
	m_type = SYS_FM7;

	m_beeper->set_frequency(1200);
	m_beeper->set_state(0);
}

MACHINE_START_MEMBER(fm7_state,fm77av)
{
	UINT8* RAM = memregion("maincpu")->base();
	UINT8* ROM = memregion("init")->base();

	memset(m_shared_ram,0xff,0x80);

	// last part of Initiate ROM is visible at the end of RAM too (interrupt vectors)
	memcpy(RAM+0x3fff0,ROM+0x1ff0,16);

	m_video.subrom = 0;  // default sub CPU ROM is type C.
	RAM = memregion("subsyscg")->base();
	membank("bank20")->set_base(RAM);
	RAM = memregion("subsys_c")->base();
	membank("bank21")->set_base(RAM+0x800);

	m_type = SYS_FM77AV;
	m_beeper->set_frequency(1200);
	m_beeper->set_state(0);
}

MACHINE_START_MEMBER(fm7_state,fm11)
{
	UINT8* RAM = memregion("maincpu")->base();
	UINT8* ROM = memregion("init")->base();

	memset(m_shared_ram,0xff,0x80);
	m_type = SYS_FM11;
	m_beeper->set_frequency(1200);
	m_beeper->set_state(0);
	// last part of Initiate ROM is visible at the end of RAM too (interrupt vectors)
	memcpy(RAM+0x3fff0,ROM+0x0ff0,16);
}

MACHINE_START_MEMBER(fm7_state,fm16)
{
	m_type = SYS_FM16;
	m_beeper->set_frequency(1200);
	m_beeper->set_state(0);
}

void fm7_state::machine_reset()
{
	UINT8* RAM = memregion("maincpu")->base();
	UINT8* ROM = memregion("init")->base();

	m_timer->adjust(attotime::from_nsec(2034500),0,attotime::from_nsec(2034500));
	m_subtimer->adjust(attotime::from_msec(20),0,attotime::from_msec(20));
	m_keyboard_timer->adjust(attotime::zero,0,attotime::from_msec(10));
	if(m_type == SYS_FM77AV || m_type == SYS_FM77AV40EX || m_type == SYS_FM11)
		m_fm77av_vsync_timer->adjust(machine().first_screen()->time_until_vblank_end());

	m_irq_mask = 0x00;
	m_irq_flags = 0x00;
	m_video.attn_irq = 0;
	m_video.sub_busy = 0x80;  // busy at reset
	m_basic_rom_en = 1;  // enabled at reset, if in BASIC mode
	if(m_type == SYS_FM11 || m_type == SYS_FM16)
		m_basic_rom_en = 0;  // all FM11/16 systems have no BASIC ROM except for the FM-11 ST
	if(m_type == SYS_FM77AV || m_type == SYS_FM77AV40EX)
	{
		m_init_rom_en = 1;
		// last part of Initiate ROM is visible at the end of RAM too (interrupt vectors)
		memcpy(RAM+0x3fff0,ROM+0x1ff0,16);
	}
	else if (m_type == SYS_FM11)
	{
		m_init_rom_en = 1;
		// last part of Initiate ROM is visible at the end of RAM too (interrupt vectors)
		memcpy(RAM+0x3fff0,ROM+0x0ff0,16);
	}
	else
		m_init_rom_en = 0;
	if(m_type == SYS_FM7)
	{
		if(!(m_dsw->read() & 0x02))
		{
			m_basic_rom_en = 0;  // disabled for DOS mode
			membank("bank1")->set_base(RAM+0x08000);
		}
		else
			membank("bank1")->set_base(RAM+0x38000);
		membank("bank2")->set_base(RAM+0x08000);
	}
	m_key_delay = 700;  // 700ms on FM-7
	m_key_repeat = 70;  // 70ms on FM-7
	m_break_flag = 0;
	m_key_scan_mode = KEY_MODE_FM7;
	m_psg_regsel = 0;
	m_psg_data = 0;
	m_fdc_side = 0;
	m_fdc_drive = 0;
	m_mmr.mode = 0;
	m_mmr.segment = 0;
	m_mmr.enabled = 0;
	m_fm77av_ym_irq = 0;
	m_encoder.latch = 1;
	m_encoder.ack = 1;
	// set boot mode (FM-7 only, AV and later has boot RAM instead)
	if(m_type == SYS_FM7)
	{
		if(!(m_dsw->read() & 0x02))
		{  // DOS mode
			membank("bank17")->set_base(memregion("dos")->base());
		}
		else
		{  // BASIC mode
			membank("bank17")->set_base(memregion("basic")->base());
		}
	}
	if(m_type == SYS_FM77AV || m_type == SYS_FM77AV40EX || m_type == SYS_FM11)
	{
		fm7_mmr_refresh(m_maincpu->space(AS_PROGRAM));
		membank("fbasic_bank_w")->set_base(RAM+0x38000);
		membank("init_bank_w")->set_base(RAM+0x36000);
	}
	if(m_type == SYS_FM11)
	{
		// Probably best to halt the 8088, I'm pretty sure it and the main 6809 should not be running at the same time
		m_x86->set_input_line(INPUT_LINE_HALT,ASSERT_LINE);
	}

	memset(m_video_ram, 0, sizeof(UINT8) * 0x18000);
}


static SLOT_INTERFACE_START( fm7_floppies )
	SLOT_INTERFACE("qd", FLOPPY_525_QD)
SLOT_INTERFACE_END


#define MCFG_ADDRESS_BANK(tag) \
MCFG_DEVICE_ADD(tag, ADDRESS_MAP_BANK, 0) \
MCFG_DEVICE_PROGRAM_MAP(fm7_banked_mem) \
MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE) \
MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8) \
MCFG_ADDRESS_MAP_BANK_STRIDE(0x1000)


static MACHINE_CONFIG_START( fm7, fm7_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(fm7_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(fm7_state,fm7_irq_ack)
	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_CPU_ADD("sub", M6809, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(fm7_sub_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(fm7_state,fm7_sub_irq_ack)
	MCFG_QUANTUM_PERFECT_CPU("sub")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("psg", AY8910, XTAL_4_9152MHz / 4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono", 1.00)
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono", 0.50)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono", 0.25)

	MCFG_MACHINE_START_OVERRIDE(fm7_state,fm7)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(fm7_state, screen_update_fm7)

	MCFG_PALETTE_ADD_3BIT_BRG("palette")

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(fm7_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("fm7_cass")

	MCFG_SOFTWARE_LIST_ADD("cass_list","fm7_cass")

	MCFG_MB8877_ADD("fdc", XTAL_8MHz / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(fm7_state, fm7_fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(fm7_state, fm7_fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", fm7_floppies, "qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", fm7_floppies, "qd", floppy_image_device::default_floppy_formats)

	MCFG_SOFTWARE_LIST_ADD("flop_list","fm7_disk")

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_SLOT_OPTION_ADD( "dsjoy", DEMPA_SHINBUNSHA_JOYSTICK )
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(fm7_state, write_centronics_busy))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(fm7_state, write_centronics_fault))
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(fm7_state, write_centronics_ack))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(fm7_state, write_centronics_perror))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( fm8, fm7_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 1200000)  // 1.2MHz 68A09
	MCFG_CPU_PROGRAM_MAP(fm8_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(fm7_state,fm7_irq_ack)
	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_CPU_ADD("sub", M6809, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(fm7_sub_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(fm7_state,fm7_sub_irq_ack)
	MCFG_QUANTUM_PERFECT_CPU("sub")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.25)

	MCFG_MACHINE_START_OVERRIDE(fm7_state,fm7)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(fm7_state, screen_update_fm7)

	MCFG_PALETTE_ADD_3BIT_BRG("palette")

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(fm7_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("fm7_cass")

	MCFG_MB8877_ADD("fdc", XTAL_8MHz / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(fm7_state, fm7_fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(fm7_state, fm7_fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", fm7_floppies, "qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", fm7_floppies, "qd", floppy_image_device::default_floppy_formats)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(fm7_state, write_centronics_busy))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(fm7_state, write_centronics_fault))
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(fm7_state, write_centronics_ack))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(fm7_state, write_centronics_perror))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( fm77av, fm7_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_2MHz)  // actually MB68B09E, but the 6809E core runs too slowly
	MCFG_CPU_PROGRAM_MAP(fm77av_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(fm7_state,fm7_irq_ack)
	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_CPU_ADD("sub", M6809, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(fm77av_sub_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(fm7_state,fm7_sub_irq_ack)
	MCFG_QUANTUM_PERFECT_CPU("sub")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym", YM2203, XTAL_4_9152MHz / 4)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(fm7_state, fm77av_fmirq))
	MCFG_AY8910_PORT_A_READ_CB(READ8(fm7_state, fm77av_joy_1_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(fm7_state, fm77av_joy_2_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",1.0)
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.25)

	MCFG_MACHINE_START_OVERRIDE(fm7_state,fm77av)

	MCFG_ADDRESS_BANK("av_bank1")
	MCFG_ADDRESS_BANK("av_bank2")
	MCFG_ADDRESS_BANK("av_bank3")
	MCFG_ADDRESS_BANK("av_bank4")
	MCFG_ADDRESS_BANK("av_bank5")
	MCFG_ADDRESS_BANK("av_bank6")
	MCFG_ADDRESS_BANK("av_bank7")
	MCFG_ADDRESS_BANK("av_bank8")
	MCFG_ADDRESS_BANK("av_bank9")
	MCFG_ADDRESS_BANK("av_bank10")
	MCFG_ADDRESS_BANK("av_bank11")
	MCFG_ADDRESS_BANK("av_bank12")
	MCFG_ADDRESS_BANK("av_bank13")
	MCFG_ADDRESS_BANK("av_bank14")
	MCFG_ADDRESS_BANK("av_bank15")
	MCFG_ADDRESS_BANK("av_bank16")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(fm7_state, screen_update_fm7)

	MCFG_PALETTE_ADD_3BIT_BRG("palette")
	MCFG_PALETTE_ADD("av_palette", 4096)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(fm7_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("fm7_cass")

	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("cass_list", "fm7_cass")

	MCFG_MB8877_ADD("fdc", XTAL_8MHz / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(fm7_state, fm7_fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(fm7_state, fm7_fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", fm7_floppies, "qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", fm7_floppies, "qd", floppy_image_device::default_floppy_formats)

	MCFG_SOFTWARE_LIST_ADD("av_flop_list", "fm77av")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("flop_list", "fm7_disk")

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(fm7_state, write_centronics_busy))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(fm7_state, write_centronics_fault))
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(fm7_state, write_centronics_ack))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(fm7_state, write_centronics_perror))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( fm11, fm7_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_2MHz)  // 2MHz 68B09E
	MCFG_CPU_PROGRAM_MAP(fm11_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(fm7_state,fm7_irq_ack)
	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_CPU_ADD("sub", M6809, XTAL_2MHz)  // 2MHz 68B09
	MCFG_CPU_PROGRAM_MAP(fm11_sub_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(fm7_state,fm7_sub_irq_ack)
	MCFG_QUANTUM_PERFECT_CPU("sub")

	MCFG_CPU_ADD("x86", I8088, XTAL_8MHz)  // 8MHz i8088
	MCFG_CPU_PROGRAM_MAP(fm11_x86_mem)
	MCFG_CPU_IO_MAP(fm11_x86_io)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.25)

	MCFG_MACHINE_START_OVERRIDE(fm7_state,fm11)

	MCFG_ADDRESS_BANK("av_bank1")
	MCFG_ADDRESS_BANK("av_bank2")
	MCFG_ADDRESS_BANK("av_bank3")
	MCFG_ADDRESS_BANK("av_bank4")
	MCFG_ADDRESS_BANK("av_bank5")
	MCFG_ADDRESS_BANK("av_bank6")
	MCFG_ADDRESS_BANK("av_bank7")
	MCFG_ADDRESS_BANK("av_bank8")
	MCFG_ADDRESS_BANK("av_bank9")
	MCFG_ADDRESS_BANK("av_bank10")
	MCFG_ADDRESS_BANK("av_bank11")
	MCFG_ADDRESS_BANK("av_bank12")
	MCFG_ADDRESS_BANK("av_bank13")
	MCFG_ADDRESS_BANK("av_bank14")
	MCFG_ADDRESS_BANK("av_bank15")
	MCFG_ADDRESS_BANK("av_bank16")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(fm7_state, screen_update_fm7)

	MCFG_PALETTE_ADD_3BIT_BRG("palette")

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(fm7_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("fm7_cass")

	MCFG_MB8877_ADD("fdc", XTAL_8MHz / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(fm7_state, fm7_fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(fm7_state, fm7_fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", fm7_floppies, "qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", fm7_floppies, "qd", floppy_image_device::default_floppy_formats)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(fm7_state, write_centronics_busy))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(fm7_state, write_centronics_fault))
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(fm7_state, write_centronics_ack))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(fm7_state, write_centronics_perror))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( fm16beta, fm7_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, XTAL_8MHz)  // 8MHz i8086
	MCFG_CPU_PROGRAM_MAP(fm16_mem)
	MCFG_CPU_IO_MAP(fm16_io)
	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_CPU_ADD("sub", M6809, XTAL_2MHz)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(fm7_state,fm7_sub_irq_ack)
	MCFG_CPU_PROGRAM_MAP(fm16_sub_mem)
	MCFG_QUANTUM_PERFECT_CPU("sub")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.25)

	MCFG_MACHINE_START_OVERRIDE(fm7_state,fm16)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(fm7_state, screen_update_fm7)

	MCFG_PALETTE_ADD_3BIT_BRG("palette")

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(fm7_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("fm7_cass")

	MCFG_MB8877_ADD("fdc", XTAL_8MHz / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(fm7_state, fm7_fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(fm7_state, fm7_fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", fm7_floppies, "qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", fm7_floppies, "qd", floppy_image_device::default_floppy_formats)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(fm7_state, write_centronics_busy))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(fm7_state, write_centronics_fault))
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(fm7_state, write_centronics_ack))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(fm7_state, write_centronics_perror))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( fm8 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "fbasic10.rom", 0x38000,  0x7c00, CRC(e80ed96c) SHA1(f3fa8a6adb07224ad2a1def77d5dae9662de0867) )

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "subsys_8.rom", 0xd800,  0x2800, CRC(979f9046) SHA1(9c52052087bf3a41b83d437a51d89b9fcfec2515) )

	// either one of these boot ROMs are selectable via DIP switch
	ROM_REGION( 0x200, "basic", 0 )
	ROM_LOAD( "bootbas8.rom", 0x0000,  0x0200, CRC(8260267a) SHA1(fee6fb9c52d22dd7108c68d08c74e2f3ebcb9e4d) )

	ROM_REGION( 0x200, "dos", 0 )
	ROM_LOAD( "bootdos8.rom", 0x0000,  0x0200, CRC(1ed5a506) SHA1(966538fa92c32fc15034576dc480cfa4a339384d) )

	// optional Kanji ROM (same as for the FM-7?)
	ROM_REGION( 0x20000, "kanji1", 0 )
	ROM_LOAD_OPTIONAL( "kanji.rom", 0x0000, 0x20000, NO_DUMP )

ROM_END


ROM_START( fmnew7 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // at 0x7ba5 there is the ID string 0302840301, meaning it's v3.02 from 1984/03/01
	ROM_LOAD( "fbasic302.rom", 0x38000,  0x7c00, CRC(a96d19b6) SHA1(8d5f0cfe7e0d39bf2ab7e4c798a13004769c28b2) )

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "subsys_c.rom", 0xd800,  0x2800, CRC(24cec93f) SHA1(50b7283db6fe1342c6063fc94046283f4feddc1c) )

	// either one of these boot ROMs are selectable via DIP switch
	ROM_REGION( 0x200, "basic", 0 )
	ROM_LOAD( "boot_bas.rom", 0x0000,  0x0200, CRC(c70f0c74) SHA1(53b63a301cba7e3030e79c59a4d4291eab6e64b0) )

	ROM_REGION( 0x200, "dos", 0 )
	ROM_LOAD( "boot_dos.rom", 0x0000,  0x0200, CRC(198614ff) SHA1(037e5881bd3fed472a210ee894a6446965a8d2ef) )

	// optional Kanji ROM
	ROM_REGION( 0x20000, "kanji1", 0 )
	ROM_LOAD_OPTIONAL( "kanji.rom", 0x0000, 0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )

ROM_END

ROM_START( fm7 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // at 0x7ba5 there is the ID string 0300820920, meaning it's v3.00 from 1982/09/20
	ROM_LOAD( "fbasic300.rom", 0x38000,  0x7c00, CRC(87c98494) SHA1(d7e3603b0a2442c7632dad45f9704d9ad71968f5) )

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "subsys_c.rom", 0xd800,  0x2800, CRC(24cec93f) SHA1(50b7283db6fe1342c6063fc94046283f4feddc1c) )

	// either one of these boot ROMs are selectable via DIP switch
	ROM_REGION( 0x200, "basic", 0 )
	ROM_LOAD( "boot_bas.rom", 0x0000,  0x0200, CRC(c70f0c74) SHA1(53b63a301cba7e3030e79c59a4d4291eab6e64b0) )

	ROM_REGION( 0x200, "dos", 0 )
	ROM_LOAD( "boot_dos_a.rom", 0x0000,  0x0200, CRC(bf441864) SHA1(616c17155f84fb0e3731a31ef0eb0cbb664a5600) )

	// optional Kanji ROM
	ROM_REGION( 0x20000, "kanji1", 0 )
	ROM_LOAD_OPTIONAL( "kanji.rom", 0x0000, 0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )

ROM_END

ROM_START( fm77av )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_FILL(0x0000,0x40000,0xff)

	ROM_REGION( 0x2000, "init", 0 )
	ROM_LOAD( "initiate.rom", 0x0000,  0x2000, CRC(785cb06c) SHA1(b65987e98a9564a82c85eadb86f0204eee5a5c93) )

	ROM_REGION( 0x7c00, "fbasic", 0 )
	ROM_LOAD( "fbasic30.rom", 0x0000,  0x7c00, CRC(a96d19b6) SHA1(8d5f0cfe7e0d39bf2ab7e4c798a13004769c28b2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_FILL(0x0000,0x10000,0xff)

	// sub CPU ROMs
	ROM_REGION( 0x2800, "subsys_c", 0 )
	ROM_LOAD( "subsys_c.rom", 0x0000,  0x2800, CRC(24cec93f) SHA1(50b7283db6fe1342c6063fc94046283f4feddc1c) )
	ROM_REGION( 0x2000, "subsys_a", 0 )
	ROM_LOAD( "subsys_a.rom", 0x0000,  0x2000, CRC(e8014fbb) SHA1(038cb0b42aee9e933b20fccd6f19942e2f476c83) )
	ROM_REGION( 0x2000, "subsys_b", 0 )
	ROM_LOAD( "subsys_b.rom", 0x0000,  0x2000, CRC(9be69fac) SHA1(0305bdd44e7d9b7b6a17675aff0a3330a08d21a8) )
	ROM_REGION( 0x2000, "subsyscg", 0 )
	ROM_LOAD( "subsyscg.rom", 0x0000,  0x2000, CRC(e9f16c42) SHA1(8ab466b1546d023ba54987790a79e9815d2b7bb2) )

	ROM_REGION( 0x20000, "kanji1", 0 )
	ROM_LOAD( "kanji.rom", 0x0000, 0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )

	// optional dict rom?
ROM_END

ROM_START( fm7740sx )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_FILL(0x0000,0x40000,0xff)

	ROM_REGION( 0x2000, "init", 0 )
	ROM_LOAD( "initiate.rom", 0x0000,  0x2000, CRC(785cb06c) SHA1(b65987e98a9564a82c85eadb86f0204eee5a5c93) )

	ROM_REGION( 0x7c00, "fbasic", 0 )
	ROM_LOAD( "fbasic30.rom", 0x0000,  0x7c00, CRC(a96d19b6) SHA1(8d5f0cfe7e0d39bf2ab7e4c798a13004769c28b2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_FILL(0x0000,0x10000,0xff)

	// sub CPU ROMs
	ROM_REGION( 0x2800, "subsys_c", 0 )
	ROM_LOAD( "subsys_c.rom", 0x0000,  0x2800, CRC(24cec93f) SHA1(50b7283db6fe1342c6063fc94046283f4feddc1c) )
	ROM_REGION( 0x2000, "subsys_a", 0 )
	ROM_LOAD( "subsys_a.rom", 0x0000,  0x2000, CRC(e8014fbb) SHA1(038cb0b42aee9e933b20fccd6f19942e2f476c83) )
	ROM_REGION( 0x2000, "subsys_b", 0 )
	ROM_LOAD( "subsys_b.rom", 0x0000,  0x2000, CRC(9be69fac) SHA1(0305bdd44e7d9b7b6a17675aff0a3330a08d21a8) )
	ROM_REGION( 0x2000, "subsyscg", 0 )
	ROM_LOAD( "subsyscg.rom", 0x0000,  0x2000, CRC(e9f16c42) SHA1(8ab466b1546d023ba54987790a79e9815d2b7bb2) )

	ROM_REGION( 0x20000, "kanji1", 0 )
	ROM_LOAD( "kanji.rom", 0x0000, 0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )
	ROM_LOAD( "kanji2.rom", 0x0000, 0x20000, CRC(38644251) SHA1(ebfdc43c38e1380709ed08575c346b2467ad1592) )

	/* These should be loaded at 2e000-2ffff of maincpu, but I'm not sure if it is correct */
	ROM_REGION( 0x4c000, "additional", 0 )
	ROM_LOAD( "dicrom.rom", 0x00000, 0x40000, CRC(b142acbc) SHA1(fe9f92a8a2750bcba0a1d2895e75e83858e4f97f) )
	ROM_LOAD( "extsub.rom", 0x40000, 0x0c000, CRC(0f7fcce3) SHA1(a1304457eeb400b4edd3c20af948d66a04df255e) )

ROM_END

ROM_START( fm11 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_FILL(0x0000,0x40000,0xff)

	ROM_REGION( 0x1000, "init", 0 )
	ROM_LOAD( "boot6809.rom", 0x0000, 0x1000, CRC(447caa6f) SHA1(4aa30314994c256d37ee01d11ec2bf4df3bc8cde) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "subsys.rom", 0xc000, 0x4000, CRC(436c0618) SHA1(cd508e3e8f79737afb4384ea4b278eddf0ce935d) )

	ROM_REGION( 0x100000, "x86", 0 )
	ROM_LOAD( "boot8088.rom", 0xff000, 0x1000, CRC(d13096a6) SHA1(f9bd95b3b8184d0e04fba9b50f273dbb8823be77) )

	ROM_REGION( 0x10000, "subsys_e", 0 )
	ROM_LOAD( "subsys_e.rom", 0x00000, 0x1000, CRC(31d838aa) SHA1(86a275c27bc99985bef7a51bdab2e47d68e31c6c) )

	// optional Kanji ROM
	ROM_REGION( 0x20000, "kanji1", 0 )
	ROM_LOAD_OPTIONAL( "kanji.rom", 0x0000, 0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )

ROM_END

ROM_START( fm16beta )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ipl.rom", 0xfc000, 0x4000, CRC(25f618ea) SHA1(9c27d6ad283260e071d64a1bfca16f7d3ad61f96) )

//  ROM_REGION( 0x10000, "subsys", 0 )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "sub_cg.rom", 0xa000, 0x0f80, CRC(e7928bed) SHA1(68cf604aa7a5c2ec7bd0d612cf099302c7f8c442) )
	ROM_CONTINUE(0xff80,0x0080)
	ROM_LOAD( "subsys.rom", 0xb000, 0x4f80, CRC(1d878514) SHA1(4673879a81e39880655b380250c6b81137028727) )
ROM_END


/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT   INIT  COMPANY      FULLNAME        FLAGS */
COMP( 1981, fm8,      0,      0,      fm8,     fm8, fm7_state,    fm7,  "Fujitsu",   "FM-8",         0)
COMP( 1982, fm7,      0,      0,      fm7,     fm7, fm7_state,    fm7,  "Fujitsu",   "FM-7",         0)
COMP( 1984, fmnew7,   fm7,    0,      fm7,     fm7, fm7_state,    fm7,  "Fujitsu",   "FM-NEW7",      0)
COMP( 1985, fm77av,   fm7,    0,      fm77av,  fm7, fm7_state,    fm7,  "Fujitsu",   "FM-77AV",      MACHINE_IMPERFECT_GRAPHICS)
COMP( 1985, fm7740sx, fm7,    0,      fm77av,  fm7, fm7_state,    fm7,  "Fujitsu",   "FM-77AV40SX",  MACHINE_NOT_WORKING)

// These may be separated into a separate driver, depending on how different they are to the FM-8/FM-7
COMP( 1982, fm11,     0,      0,      fm11,     fm7, fm7_state,    fm7,       "Fujitsu",   "FM-11 EX",      MACHINE_NOT_WORKING)
COMP( 1982, fm16beta, 0,      0,      fm16beta, fm7, fm7_state,    fm7,       "Fujitsu",   "FM-16\xCE\xB2", MACHINE_NOT_WORKING)
