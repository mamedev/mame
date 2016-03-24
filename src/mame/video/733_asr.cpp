// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    733 ASR emulation

    We are emulating a TI Model 733 ASR ("Silent 700") data terminal,
    interfaced through a TI asynchronous EIA/TTY interface module.

    The ASR features a printer, a keyboard and a tape unit (which is not
    emulated).  The ASR is attached to the computer with a serial interface.

    References:
    945401-9701 Model 990/4 Computer System Field Maintenance Manual p. C-1,
    945250-9701 990 Computer Family Systems Handbook pp. 5-9 through 5-16,
    0943442-9701 Model 990 Computer Reference Manual Preliminary pp. 3-13
    through 3-21 and 3-39 through 3-44.

    TODO:
    * separate ASR emulation from EIA interface emulation?
    * implement tape interface?

    Raphael Nabet 2003

    Rewritten as class
    Michael Zapf, 2014
*/

#include "emu.h"
#include "733_asr.h"

enum
{
	/*ASROutQueueSize = 32,*/

	asr_window_offset_x = 0,
	asr_window_offset_y = 0,
	asr_window_width = 640,
	asr_window_height = 480,
	asr_scroll_step = 8
};

enum
{
	AS_wrq_mask = 1 << 3,
	AS_rrq_mask = 1 << 4,
	AS_dsr_mask = 1 << 6,
	AS_int_mask = 1 << 7,

	AM_dtr_mask = 1 << 1,
	AM_rts_mask = 1 << 2,
	AM_enint_mask = 1 << 6
};

enum
{
	asrfontdata_size = 96/*128*/*8
};

static const gfx_layout fontlayout =
{
	6, 8,           /* 6*8 characters */
	/*96*/128,              /* 96 characters */
	1,              /* 1 bit per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, /* straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( asr733 )
	GFXDECODE_ENTRY( asr733_chr_region, 0, fontlayout, 0, 1 )
GFXDECODE_END

PALETTE_INIT_MEMBER(asr733_device, asr733)
{
	palette.set_pen_color(0,rgb_t::white); /* white */
	palette.set_pen_color(1,rgb_t::black); /* black */
}


const device_type ASR733 = &device_creator<asr733_device>;

asr733_device::asr733_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ASR733, "733 ASR", tag, owner, clock, "asr733", __FILE__),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_keyint_line(*this),
		m_lineint_line(*this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void asr733_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void asr733_device::device_start()
{
	screen_device *screen = machine().first_screen();
	int width = screen->width();
	int height = screen->height();
	const rectangle &visarea = screen->visible_area();

	m_last_key_pressed = 0x80;
	m_bitmap = std::make_unique<bitmap_ind16>(width, height);

	m_bitmap->fill(0, visarea);

	m_keyint_line.resolve();
	m_lineint_line.resolve();

	m_line_timer = timer_alloc(0);

	UINT8 *dst;

	static const unsigned char fontdata6x8[asrfontdata_size] =
	{   /* ASCII characters */
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00,
		0x50,0x50,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0xf8,0x50,0xf8,0x50,0x00,0x00,
		0x20,0x70,0xc0,0x70,0x18,0xf0,0x20,0x00,0x40,0xa4,0x48,0x10,0x20,0x48,0x94,0x08,
		0x60,0x90,0xa0,0x40,0xa8,0x90,0x68,0x00,0x10,0x20,0x40,0x00,0x00,0x00,0x00,0x00,
		0x20,0x40,0x40,0x40,0x40,0x40,0x20,0x00,0x10,0x08,0x08,0x08,0x08,0x08,0x10,0x00,
		0x20,0xa8,0x70,0xf8,0x70,0xa8,0x20,0x00,0x00,0x20,0x20,0xf8,0x20,0x20,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60,0x00,0x00,0x00,0xf8,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00,
		0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x10,0x30,0x10,0x10,0x10,0x10,0x10,0x00,
		0x70,0x88,0x08,0x10,0x20,0x40,0xf8,0x00,0x70,0x88,0x08,0x30,0x08,0x88,0x70,0x00,
		0x10,0x30,0x50,0x90,0xf8,0x10,0x10,0x00,0xf8,0x80,0xf0,0x08,0x08,0x88,0x70,0x00,
		0x70,0x80,0xf0,0x88,0x88,0x88,0x70,0x00,0xf8,0x08,0x08,0x10,0x20,0x20,0x20,0x00,
		0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00,0x70,0x88,0x88,0x88,0x78,0x08,0x70,0x00,
		0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x00,0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x60,
		0x10,0x20,0x40,0x80,0x40,0x20,0x10,0x00,0x00,0x00,0xf8,0x00,0xf8,0x00,0x00,0x00,
		0x40,0x20,0x10,0x08,0x10,0x20,0x40,0x00,0x70,0x88,0x08,0x10,0x20,0x00,0x20,0x00,
		0x70,0x88,0xb8,0xa8,0xb8,0x80,0x70,0x00,0x70,0x88,0x88,0xf8,0x88,0x88,0x88,0x00,
		0xf0,0x88,0x88,0xf0,0x88,0x88,0xf0,0x00,0x70,0x88,0x80,0x80,0x80,0x88,0x70,0x00,
		0xf0,0x88,0x88,0x88,0x88,0x88,0xf0,0x00,0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8,0x00,
		0xf8,0x80,0x80,0xf0,0x80,0x80,0x80,0x00,0x70,0x88,0x80,0x98,0x88,0x88,0x70,0x00,
		0x88,0x88,0x88,0xf8,0x88,0x88,0x88,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,
		0x08,0x08,0x08,0x08,0x88,0x88,0x70,0x00,0x88,0x90,0xa0,0xc0,0xa0,0x90,0x88,0x00,
		0x80,0x80,0x80,0x80,0x80,0x80,0xf8,0x00,0x88,0xd8,0xa8,0x88,0x88,0x88,0x88,0x00,
		0x88,0xc8,0xa8,0x98,0x88,0x88,0x88,0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,
		0xf0,0x88,0x88,0xf0,0x80,0x80,0x80,0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x08,
		0xf0,0x88,0x88,0xf0,0x88,0x88,0x88,0x00,0x70,0x88,0x80,0x70,0x08,0x88,0x70,0x00,
		0xf8,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00,
		0x88,0x88,0x88,0x88,0x88,0x50,0x20,0x00,0x88,0x88,0x88,0x88,0xa8,0xd8,0x88,0x00,
		0x88,0x50,0x20,0x20,0x20,0x50,0x88,0x00,0x88,0x88,0x88,0x50,0x20,0x20,0x20,0x00,
		0xf8,0x08,0x10,0x20,0x40,0x80,0xf8,0x00,0x30,0x20,0x20,0x20,0x20,0x20,0x30,0x00,
		0x40,0x40,0x20,0x20,0x10,0x10,0x08,0x08,0x30,0x10,0x10,0x10,0x10,0x10,0x30,0x00,
		0x20,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,
		0x40,0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x08,0x78,0x88,0x78,0x00,
		0x80,0x80,0xf0,0x88,0x88,0x88,0xf0,0x00,0x00,0x00,0x70,0x88,0x80,0x80,0x78,0x00,
		0x08,0x08,0x78,0x88,0x88,0x88,0x78,0x00,0x00,0x00,0x70,0x88,0xf8,0x80,0x78,0x00,
		0x18,0x20,0x70,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x70,
		0x80,0x80,0xf0,0x88,0x88,0x88,0x88,0x00,0x20,0x00,0x20,0x20,0x20,0x20,0x20,0x00,
		0x20,0x00,0x20,0x20,0x20,0x20,0x20,0xc0,0x80,0x80,0x90,0xa0,0xe0,0x90,0x88,0x00,
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xf0,0xa8,0xa8,0xa8,0xa8,0x00,
		0x00,0x00,0xb0,0xc8,0x88,0x88,0x88,0x00,0x00,0x00,0x70,0x88,0x88,0x88,0x70,0x00,
		0x00,0x00,0xf0,0x88,0x88,0xf0,0x80,0x80,0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x08,
		0x00,0x00,0xb0,0xc8,0x80,0x80,0x80,0x00,0x00,0x00,0x78,0x80,0x70,0x08,0xf0,0x00,
		0x20,0x20,0x70,0x20,0x20,0x20,0x18,0x00,0x00,0x00,0x88,0x88,0x88,0x98,0x68,0x00,
		0x00,0x00,0x88,0x88,0x88,0x50,0x20,0x00,0x00,0x00,0xa8,0xa8,0xa8,0xa8,0x50,0x00,
		0x00,0x00,0x88,0x50,0x20,0x50,0x88,0x00,0x00,0x00,0x88,0x88,0x88,0x78,0x08,0x70,
		0x00,0x00,0xf8,0x10,0x20,0x40,0xf8,0x00,0x08,0x10,0x10,0x20,0x10,0x10,0x08,0x00,
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x40,0x20,0x20,0x10,0x20,0x20,0x40,0x00,
		0x00,0x68,0xb0,0x00,0x00,0x00,0x00,0x00,0x20,0x50,0x20,0x50,0xa8,0x50,0x00,0x00
	};

	dst = machine().root_device().memregion(asr733_chr_region)->base();

	memcpy(dst, fontdata6x8, asrfontdata_size);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void asr733_device::device_reset()
{
	/*m_OutQueueLen = 0;*/

	m_status = AS_dsr_mask | AS_wrq_mask;
	m_mode = 0;
	m_line_timer->adjust(attotime::from_msec(0), 0, attotime::from_hz(60));

	set_interrupt_line();
}

void asr733_device::set_interrupt_line()
{
	if ((m_mode & AM_enint_mask) && (m_new_status_flag))  /* right??? */
	{
		m_status |= AS_int_mask;
		m_keyint_line(ASSERT_LINE);
	}
	else
	{
		m_status &= ~AS_int_mask;
		m_keyint_line(CLEAR_LINE);
	}
}

/* write a single char on screen */
void asr733_device::draw_char(int character, int x, int y, int color)
{
		m_gfxdecode->gfx(0)->opaque(*m_bitmap, m_bitmap->cliprect(), character-32, color, 0, 0, x+1, y);
}

void asr733_device::linefeed()
{
	UINT8 buf[asr_window_width];

	for (int y=asr_window_offset_y; y<asr_window_offset_y+asr_window_height-asr_scroll_step; y++)
	{
		extract_scanline8(*m_bitmap, asr_window_offset_x, y+asr_scroll_step, asr_window_width, buf);
		draw_scanline8(*m_bitmap, asr_window_offset_x, y, asr_window_width, buf, m_palette->pens());
	}

	const rectangle asr_scroll_clear_window(
		asr_window_offset_x,                                    /* min_x */
		asr_window_offset_x+asr_window_width-1,                 /* max_x */
		asr_window_offset_y+asr_window_height-asr_scroll_step,  /* min_y */
		asr_window_offset_y+asr_window_height-1                 /* max_y */
	);
	m_bitmap->fill(0, asr_scroll_clear_window);
}

void asr733_device::transmit(UINT8 data)
{
	switch (data)
	{
	/* aux device control chars */
	case 0x05:
		/* ENQ -> "WRU": ??? */
		break;

	case 0x11:
		/* DC1 -> "X-ON": transmit on??? */
		break;

	case 0x12:
		/* DC2 -> "X-OFF": transmit off??? */
		break;

	case 0x13:
		/* DC3 -> "TAPE": tape on??? */
		break;

	case 0x14:
		/* DC4 -> "-T-A-P-E-" ("TAPE" with overstrike): tape off??? */
		break;


	/* printer control chars */
	case 0x07:
		/* BELL: 250ms beep */
		break;

	case 0x08:
		/* BS: backspace */
		if (m_x > 0)
			m_x--;
		break;

	case 0x0A:
		/* LF: line feed */
		linefeed();
		break;

	case 0x0D:
		/* CR: carriage return */
		m_x = 0;
		break;


	default:
		if ((data < 0x20) || (data == 0x7f) || (data >= 0x80))
			/* ignore control characters */
			break;

		if (m_x == 80)
		{
			m_x = 0;
			linefeed();
		}
		draw_char(data, asr_window_offset_x + m_x*8, asr_window_offset_y+asr_window_height-8, 0);
		m_x++;
		break;
	}

	m_status |= AS_wrq_mask;
	m_new_status_flag = 1;   /* right??? */
	set_interrupt_line();
}

#if 0
void asr733_device::receive_callback(int dummy)
{
	(void) dummy;

	m_recv_buf = m_OutQueue[m_OutQueueHead];
	m_OutQueueHead = (m_OutQueueHead + 1) % ASROutQueueSize;
	m_OutQueueLen--;

	m_status |= AS_rrq_mask;
	m_new_status_flag = 1;   /* right??? */
	set_interrupt_line();
}
#endif

/*
    0-7: receive buffer
    8: XMITING transmit in progress, 1 if transmitting
    9: TIMERR timing error, 1 if error
    10: RCR reverse channel receive, not used
        "ASR733/33 ID" 1 -> TTY (???) (2270509-9701 pp. G-9 & G-10)
    11: WRQ write request, 1 if ready to transmit
    12: RRQ read request, 1 if ready to receive
    13: DCD data carrier detect, not used
    14: DSR data set ready, 1 if online
    15: INT interrupt, 1 if interrupt
*/
READ8_MEMBER( asr733_device::cru_r )
{
	int reply = 0;

	switch (offset)
	{
	case 0:
		/* receive buffer */
		reply = m_recv_buf;
		break;

	case 1:
		/* status register */
		reply = m_status;
		break;
	}

	return reply;
}

/*
    0-7: transmit buffer
    8: not used
    9: DTR data terminal ready (set to 1)
    10: RTS request to send (set to 1)
    11: CLRWRQ clear write request (write any value to execute)
    12: CLRRRQ clear read request (write any value to execute)
    13: CLRNSF clear new status flag - clear DSR/DCD interrupts (write any value to execute)
    14: enable interrupts, 1 to enable interrupts
    15: diagnostic mode, 0 for normal mode
*/
WRITE8_MEMBER( asr733_device::cru_w )
{
	switch (offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		/* transmit buffer */
		if (data)
			m_xmit_buf |= 1 << offset;
		else
			m_xmit_buf &= ~ (1 << offset);
		if ((offset == 7) && (m_mode & AM_dtr_mask) && (m_mode & AM_rts_mask))    /* right??? */
			transmit(m_xmit_buf);
		break;

	case 8:     /* not used */
		break;

	case 9:     /* data terminal ready (set to 1) */
	case 10:    /* request to send (set to 1) */
	case 14:    /* enable interrupts, 1 to enable interrupts */
	case 15:    /* diagnostic mode, 0 for normal mode */
		if (data)
			m_mode |= 1 << (offset - 8);
		else
			m_mode &= ~ (1 << (offset - 8));
		if (offset == 14)
			set_interrupt_line();
		break;

	case 11:    /* clear write request (write any value to execute) */
	case 12:    /* clear read request (write any value to execute) */
		m_status &= ~ (1 << (offset - 8));
		set_interrupt_line();
		break;

	case 13:    /* clear new status flag - whatever it means (write any value to execute) */
		m_new_status_flag = 0;
		set_interrupt_line();
		break;
	}
}

/*
    Video refresh
*/
void asr733_device::refresh(bitmap_ind16 &bitmap, int x, int y)
{
	copybitmap(bitmap, *m_bitmap, 0, 0, x, y, m_bitmap->cliprect());
}


/*
    Time callbacks
*/
void asr733_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	check_keyboard();
	m_lineint_line(ASSERT_LINE);
	m_lineint_line(CLEAR_LINE);
}

static const unsigned char key_translate[3][51] =
{
	{   /* unshifted */
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'0',
		':',
		'-',

		0x1b,
		'Q',
		'W',
		'E',
		'R',
		'T',
		'Y',
		'U',
		'I',
		'O',
		'P',
		0x0a,
		0x0d,

		0,
		'A',
		'S',
		'D',
		'F',
		'G',
		'H',
		'J',
		'K',
		'L',
		';',
		0x08,
		0,

		0,
		'Z',
		'X',
		'C',
		'V',
		'B',
		'N',
		'M',
		',',
		'.',
		'/',
		0,

		' '
	},
	{   /* shifted */
		'!',
		'"',
		'#',
		'$',
		'%',
		'&',
		'^',
		'(',
		')',
		' ',
		'*',
		'=',

		0x1b,
		'Q',
		'W',
		'E',
		'R',
		'T',
		'Y',
		'U',
		'I',
		'_',
		'@',
		0x0a,
		0x0d,

		0,
		'A',
		'S',
		'D',
		'F',
		'G',
		'H',
		'J',
		0,
		'/',
		'+',
		0x08,
		0,

		0,
		'Z',
		'X',
		'C',
		'V',
		'B',
		'^',
		'|',
		'<',
		'>',
		'?',
		0,

		' '
	},
	{   /* control */
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'0',
		':',
		'-',

		0x1b,
		0x11,
		0x17,
		0x05,
		0x12,
		0x14,
		0x19,
		0x15,
		0x09,
		0x0f,
		0x10,
		0x0a,
		0x0d,

		0,
		0x01,
		0x13,
		0x04,
		0x06,
		0x07,
		0x08,
		0x0a,
		0x0b,
		0x0c,
		';',
		0x08,
		0,

		0,
		0x1a,
		0x18,
		0x03,
		0x16,
		0x02,
		0x0e,
		0x0d,
		',',
		'.',
		'/',
		0,

		' '
	}
};


/*
    keyboard handler: should be called regularly by machine code, for instance
    every Video Blank Interrupt.
*/
void asr733_device::check_keyboard()
{
	enum modifier_state_t
	{
		/* key modifier states */
		unshifted = 0, shift, control,
		/* special value to stop repeat if the modifier state changes */
		special_debounce = -1
	} ;

	enum { repeat_delay = 5 /* approx. 1/10s */ };

	//UINT16 key_buf[6];
	UINT16 key_buf[4];
	int i, j;
	modifier_state_t modifier_state;
	int repeat_mode;

	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3" };

	/* read current key state */
	/* 2008-05 FP: in 733_asr.h there are only 4 input ports defined... */
	/* for (i = 0; i < 6; i++) */
	for (i = 0; i < 4; i++)
	{
		key_buf[i] = ioport(keynames[i])->read();
	}

	/* process key modifiers */
	if (key_buf[1] & 0x0200)
		modifier_state = control;
	else if ((key_buf[2] & 0x0040) || (key_buf[3] & 0x0002))
		modifier_state = shift;
	else
		modifier_state = unshifted;

	/* test repeat key */
	repeat_mode = key_buf[2] & 0x0020;

	/* remove modifier keys */
	key_buf[1] &= ~0x0200;
	key_buf[2] &= ~0x0060;
	key_buf[3] &= ~0x0002;

	if (! repeat_mode)
		/* reset REPEAT timer if the REPEAT key is not pressed */
		m_repeat_timer = 0;

	if ( !(m_last_key_pressed & 0x80) && (key_buf[m_last_key_pressed >> 4] & (1 << (m_last_key_pressed & 0xf))))
	{
		/* last key has not been released */
		if (modifier_state == m_last_modifier_state)
		{
			/* handle REPEAT mode if applicable */
			if ((repeat_mode) && (++m_repeat_timer == repeat_delay))
			{
				if (m_status & AS_rrq_mask)
				{   /* keyboard buffer full */
					m_repeat_timer--;
				}
				else
				{   /* repeat current key */
					m_status |= AS_rrq_mask;
					m_new_status_flag = 1;   /* right??? */
					set_interrupt_line();
					m_repeat_timer = 0;
				}
			}
		}
		else
		{
			m_repeat_timer = 0;
			m_last_modifier_state = special_debounce;
		}
	}
	else
	{
		m_last_key_pressed = 0x80;

		if (m_status & AS_rrq_mask)
		{   /* keyboard buffer full */
			/* do nothing */
		}
		else
		{
			//for (i=0; i<6; i++)
			for (i=0; i<4; i++)
			{
				for (j=0; j<16; j++)
				{
					if (key_buf[i] & (1 << j))
					{
						m_last_key_pressed = (i << 4) | j;
						m_last_modifier_state = modifier_state;

						m_recv_buf = (int)key_translate[modifier_state][m_last_key_pressed];
						m_status |= AS_rrq_mask;
						m_new_status_flag = 1;   /* right??? */
						set_interrupt_line();
						return;
					}
				}
			}
		}
	}
}

UINT32 asr733_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	refresh(bitmap, 0, 0);
	return 0;
}

static MACHINE_CONFIG_FRAGMENT( asr733 )
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", asr733)

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(asr733_device, asr733)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, asr733_device, screen_update)

	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_PALETTE("palette")
MACHINE_CONFIG_END

INPUT_PORTS_START( asr733 )
	PORT_START("KEY0")  /* keys 1-16 */                                                                 \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)      \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)      \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)      \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)      \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)      \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)      \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)      \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)      \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)      \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)      \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_MINUS)  \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_EQUALS) \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)      \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)      \
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)      \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)      \
																								\
	PORT_START("KEY1")  /* keys 17-32 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)      \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)      \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)      \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)      \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)      \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)      \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)      \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_CLOSEBRACE) \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)    \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)      \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)      \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)      \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)      \
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)      \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)      \
																								\
	PORT_START("KEY2")  /* keys 33-48 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)      \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)      \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)      \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)  \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RUB OUT") PORT_CODE(KEYCODE_BACKSPACE)    \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REPEAT") PORT_CODE(KEYCODE_RALT)  \
		/* hack for my mac that does not disciminate the right ALT key */                       \
		/* PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REPEAT") PORT_CODE(KEYCODE_LALT) */    \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)      \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)      \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)      \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)      \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)      \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)      \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)      \
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)  \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)   \
																								\
	PORT_START("KEY3")  /* keys 49-51 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)  \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_RSHIFT) \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(SPACE)") PORT_CODE(KEYCODE_SPACE)
INPUT_PORTS_END

ioport_constructor asr733_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( asr733 );
}

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor asr733_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( asr733 );
}
