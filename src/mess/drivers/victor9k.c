/***************************************************************************

    Victor 9000

    - very exciting hardware, disk controller is a direct descendant
      of the Commodore drives (designed by Chuck Peddle)

    Skeleton driver

***************************************************************************/

/*

    TODO:

    - floppy 8048
    - keyboard
    - hires graphics
    - character attributes
    - brightness/contrast
    - MC6852
    - codec sound
    - hard disk (Tandon TM502, TM603SE)

*/

enum
{
	LED_A = 0,
	LED_B
};

#include "includes/victor9k.h"

// Memory Maps

static ADDRESS_MAP_START( victor9k_mem, AS_PROGRAM, 8, victor9k_state )
//  AM_RANGE(0x00000, 0xdffff) AM_RAM
	AM_RANGE(0xe0000, 0xe0001) AM_DEVREADWRITE_LEGACY(I8259A_TAG, pic8259_r, pic8259_w)
	AM_RANGE(0xe0020, 0xe0023) AM_DEVREADWRITE_LEGACY(I8253_TAG, pit8253_r, pit8253_w)
	AM_RANGE(0xe0040, 0xe0043) AM_DEVREADWRITE(UPD7201_TAG, upd7201_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0xe8000, 0xe8000) AM_DEVREADWRITE(HD46505S_TAG, mc6845_device, status_r, address_w)
	AM_RANGE(0xe8001, 0xe8001) AM_DEVREADWRITE(HD46505S_TAG, mc6845_device, register_r, register_w)
	AM_RANGE(0xe8020, 0xe802f) AM_DEVREADWRITE(M6522_1_TAG, via6522_device, read, write)
	AM_RANGE(0xe8040, 0xe804f) AM_DEVREADWRITE(M6522_2_TAG, via6522_device, read, write)
	AM_RANGE(0xe8060, 0xe8061) AM_DEVREADWRITE(MC6852_TAG, mc6852_device, read, write)
	AM_RANGE(0xe8080, 0xe808f) AM_DEVREADWRITE(M6522_3_TAG, via6522_device, read, write)
	AM_RANGE(0xe80a0, 0xe80af) AM_DEVREADWRITE(M6522_4_TAG, via6522_device, read, write)
	AM_RANGE(0xe80c0, 0xe80cf) AM_DEVREADWRITE(M6522_6_TAG, via6522_device, read, write)
	AM_RANGE(0xe80e0, 0xe80ef) AM_DEVREADWRITE(M6522_5_TAG, via6522_device, read, write)
	AM_RANGE(0xf0000, 0xf0fff) AM_MIRROR(0x1000) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION(I8088_TAG, 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( floppy_io, AS_IO, 8, victor9k_state )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_NOP
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_NOP
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_NOP
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_NOP
ADDRESS_MAP_END

// Input Ports

static INPUT_PORTS_START( victor9k )
	// defined in machine/victor9kb.c
INPUT_PORTS_END

// Video

#define CODE_NON_DISPLAY    0x1000
#define CODE_UNDERLINE      0x2000
#define CODE_LOW_INTENSITY  0x4000
#define CODE_REVERSE_VIDEO  0x8000

static MC6845_UPDATE_ROW( victor9k_update_row )
{
	victor9k_state *state = device->machine().driver_data<victor9k_state>();
	address_space &program = state->m_maincpu->space(AS_PROGRAM);
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());

	if (BIT(ma, 13))
	{
		fatalerror("Graphics mode not supported!\n");
	}
	else
	{
		UINT16 video_ram_addr = (ma & 0xfff) << 1;

		for (int sx = 0; sx < x_count; sx++)
		{
			UINT16 code = (state->m_video_ram[video_ram_addr + 1] << 8) | state->m_video_ram[video_ram_addr];
			UINT32 char_ram_addr = (BIT(ma, 12) << 16) | ((code & 0xff) << 5) | (ra << 1);
			UINT16 data = program.read_word(char_ram_addr);

			for (int x = 0; x <= 10; x++)
			{
				int color = BIT(data, x);

				if (sx == cursor_x) color = 1;

				bitmap.pix32(y, x + sx*10) = palette[color];
			}

			video_ram_addr += 2;
			video_ram_addr &= 0xfff;
		}
	}
}


static MC6845_INTERFACE( hd46505s_intf )
{
	SCREEN_TAG,
	false,
	10,
	NULL,
	victor9k_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE(I8259A_TAG, pic8259_ir7_w),
	NULL
};

// Intel 8253 Interface

WRITE_LINE_MEMBER(victor9k_state::mux_serial_b_w)
{
}

WRITE_LINE_MEMBER(victor9k_state::mux_serial_a_w)
{
}

static const struct pit8253_config pit_intf =
{
	{
		{
			2500000,
			DEVCB_LINE_VCC,
			DEVCB_DRIVER_LINE_MEMBER(victor9k_state,mux_serial_b_w)
		}, {
			2500000,
			DEVCB_LINE_VCC,
			DEVCB_DRIVER_LINE_MEMBER(victor9k_state,mux_serial_a_w)
		}, {
			100000,
			DEVCB_LINE_VCC,
			DEVCB_DEVICE_LINE(I8259A_TAG, pic8259_ir2_w)
		}
	}
};

// Intel 8259 Interfaces

/*

    pin     signal      description

    IR0     SYN         sync detect
    IR1     COMM        serial communications (7201)
    IR2     TIMER       8253 timer
    IR3     PARALLEL    all 6522 IRQ (including disk)
    IR4     IR4         expansion IR4
    IR5     IR5         expansion IR5
    IR6     KBINT       keyboard data ready
    IR7     VINT        vertical sync or nonspecific interrupt

*/

static const struct pic8259_interface pic_intf =
{
	DEVCB_CPU_INPUT_LINE(I8088_TAG, INPUT_LINE_IRQ0),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};

// NEC uPD7201 Interface

static UPD7201_INTERFACE( mpsc_intf )
{
	DEVCB_DEVICE_LINE(I8259A_TAG, pic8259_ir1_w), // interrupt
	{
		{
			0,                  // receive clock
			0,                  // transmit clock
			DEVCB_NULL,         // receive DRQ
			DEVCB_NULL,         // transmit DRQ
			DEVCB_NULL,         // receive data
			DEVCB_NULL,         // transmit data
			DEVCB_NULL,         // clear to send
			DEVCB_NULL,         // data carrier detect
			DEVCB_NULL,         // ready to send
			DEVCB_NULL,         // data terminal ready
			DEVCB_NULL,         // wait
			DEVCB_NULL          // sync output
		}, {
			0,                  // receive clock
			0,                  // transmit clock
			DEVCB_NULL,         // receive DRQ
			DEVCB_NULL,         // transmit DRQ
			DEVCB_NULL,         // receive data
			DEVCB_NULL,         // transmit data
			DEVCB_NULL,         // clear to send
			DEVCB_NULL,         // data carrier detect
			DEVCB_NULL,         // ready to send
			DEVCB_NULL,         // data terminal ready
			DEVCB_NULL,         // wait
			DEVCB_NULL          // sync output
		}
	}
};

// MC6852 Interface

WRITE_LINE_MEMBER( victor9k_state::ssda_irq_w )
{
	m_ssda_irq = state;

	pic8259_ir3_w(m_pic, m_ssda_irq | m_via1_irq | m_via2_irq | m_via3_irq | m_via4_irq | m_via5_irq | m_via6_irq);
}

static MC6852_INTERFACE( ssda_intf )
{
	0,
	0,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE(HC55516_TAG, hc55516_digit_w),
	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, ssda_irq_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

// M6522 Interface

WRITE8_MEMBER( victor9k_state::via1_pa_w )
{
	/*

	    bit     description

	    PA0     DIO1
	    PA1     DIO2
	    PA2     DIO3
	    PA3     DIO4
	    PA4     DIO5
	    PA5     DIO6
	    PA6     DIO7
	    PA7     DIO8

	*/

	m_ieee488->dio_w(data);
}

READ8_MEMBER( victor9k_state::via1_pb_r )
{
	/*

	    bit     description

	    PB0     DAV
	    PB1     EOI
	    PB2     REN
	    PB3     ATN
	    PB4     IFC
	    PB5     SRQ
	    PB6     NRFD
	    PB7     NDAC

	*/

	UINT8 data = 0;

	// data valid
	data |= m_ieee488->dav_r();

	// end or identify
	data |= m_ieee488->eoi_r() << 1;

	// remote enable
	data |= m_ieee488->ren_r() << 2;

	// attention
	data |= m_ieee488->atn_r() << 3;

	// interface clear
	data |= m_ieee488->ifc_r() << 4;

	// service request
	data |= m_ieee488->srq_r() << 5;

	// not ready for data
	data |= m_ieee488->nrfd_r() << 6;

	// data not accepted
	data |= m_ieee488->ndac_r() << 7;

	return data;
}

WRITE8_MEMBER( victor9k_state::via1_pb_w )
{
	/*

	    bit     description

	    PB0     DAV
	    PB1     EOI
	    PB2     REN
	    PB3     ATN
	    PB4     IFC
	    PB5     SRQ
	    PB6     NRFD
	    PB7     NDAC

	*/

	// data valid
	m_ieee488->dav_w(BIT(data, 0));

	// end or identify
	m_ieee488->eoi_w(BIT(data, 1));

	// remote enable
	m_ieee488->ren_w(BIT(data, 2));

	// attention
	m_ieee488->atn_w(BIT(data, 3));

	// interface clear
	m_ieee488->ifc_w(BIT(data, 4));

	// service request
	m_ieee488->srq_w(BIT(data, 5));

	// not ready for data
	m_ieee488->nrfd_w(BIT(data, 6));

	// data not accepted
	m_ieee488->ndac_w(BIT(data, 7));
}

WRITE_LINE_MEMBER( victor9k_state::codec_vol_w )
{
}

WRITE_LINE_MEMBER( victor9k_state::via1_irq_w )
{
	m_via1_irq = state;

	pic8259_ir3_w(m_pic, m_ssda_irq | m_via1_irq | m_via2_irq | m_via3_irq | m_via4_irq | m_via5_irq | m_via6_irq);
}

static const via6522_interface via1_intf =
{
	DEVCB_DEVICE_MEMBER(IEEE488_TAG, ieee488_device, dio_r),
	DEVCB_DRIVER_MEMBER(victor9k_state, via1_pb_r),
	DEVCB_DEVICE_LINE_MEMBER(IEEE488_TAG, ieee488_device, nrfd_r),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(IEEE488_TAG, ieee488_device, ndac_r),
	DEVCB_NULL,

	DEVCB_DRIVER_MEMBER(victor9k_state, via1_pa_w),
	DEVCB_DRIVER_MEMBER(victor9k_state, via1_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, codec_vol_w),

	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, via1_irq_w)
};

READ8_MEMBER( victor9k_state::via2_pa_r )
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2     RIA
	    PA3     DSRA
	    PA4     RIB
	    PA5     DSRB
	    PA6     KBDATA
	    PA7     VERT

	*/

	UINT8 data = 0;

	// keyboard data
	data |= m_kb->kbdata_r() << 6;

	// vertical sync
	data |= m_crtc->vsync_r() << 7;

	return data;
}

WRITE8_MEMBER( victor9k_state::via2_pa_w )
{
	/*

	    bit     description

	    PA0     _INT/EXTA
	    PA1     _INT/EXTB
	    PA2
	    PA3
	    PA4
	    PA5
	    PA6
	    PA7

	*/
}

WRITE8_MEMBER( victor9k_state::via2_pb_w )
{
	/*

	    bit     description

	    PB0     TALK/LISTEN
	    PB1     KBACKCTL
	    PB2     BRT0
	    PB3     BRT1
	    PB4     BRT2
	    PB5     CONT0
	    PB6     CONT1
	    PB7     CONT2

	*/

	// keyboard acknowledge
	m_kb->kback_w(BIT(data, 1));

	// brightness
	m_brt = (data >> 2) & 0x07;

	// contrast
	m_cont = data >> 5;
}

WRITE_LINE_MEMBER( victor9k_state::via2_irq_w )
{
	m_via2_irq = state;

	pic8259_ir3_w(m_pic, m_ssda_irq | m_via1_irq | m_via2_irq | m_via3_irq | m_via4_irq | m_via5_irq | m_via6_irq);
}

static const via6522_interface via2_intf =
{
	DEVCB_DRIVER_MEMBER(victor9k_state, via2_pa_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(VICTOR9K_KEYBOARD_TAG, victor9k_keyboard_device, kbrdy_r),
	DEVCB_NULL, // SRQ/BUSY
	DEVCB_DEVICE_LINE_MEMBER(VICTOR9K_KEYBOARD_TAG, victor9k_keyboard_device, kbdata_r),

	DEVCB_DRIVER_MEMBER(victor9k_state, via2_pa_w),
	DEVCB_DRIVER_MEMBER(victor9k_state, via2_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, via2_irq_w)
};

READ8_MEMBER( victor9k_state::via3_pa_r )
{
	/*

	    bit     description

	    PA0     J5-16
	    PA1     J5-18
	    PA2     J5-20
	    PA3     J5-22
	    PA4     J5-24
	    PA5     J5-26
	    PA6     J5-28
	    PA7     J5-30

	*/

	return 0;
}

WRITE8_MEMBER( victor9k_state::via3_pa_w )
{
	/*

	    bit     description

	    PA0     J5-16
	    PA1     J5-18
	    PA2     J5-20
	    PA3     J5-22
	    PA4     J5-24
	    PA5     J5-26
	    PA6     J5-28
	    PA7     J5-30

	*/
}

READ8_MEMBER( victor9k_state::via3_pb_r )
{
	/*

	    bit     description

	    PB0     J5-32
	    PB1     J5-34
	    PB2     J5-36
	    PB3     J5-38
	    PB4     J5-40
	    PB5     J5-42
	    PB6     J5-44
	    PB7     J5-46

	*/

	return 0;
}

WRITE8_MEMBER( victor9k_state::via3_pb_w )
{
	/*

	    bit     description

	    PB0     J5-32
	    PB1     J5-34
	    PB2     J5-36
	    PB3     J5-38
	    PB4     J5-40
	    PB5     J5-42
	    PB6     J5-44
	    PB7     J5-46

	*/

	// codec clock output
	m_ssda->rx_clk_w(BIT(data, 7));
	m_ssda->tx_clk_w(BIT(data, 7));
}

WRITE_LINE_MEMBER( victor9k_state::via3_irq_w )
{
	m_via3_irq = state;

	pic8259_ir3_w(m_pic, m_ssda_irq | m_via1_irq | m_via2_irq | m_via3_irq | m_via4_irq | m_via5_irq | m_via6_irq);
}

static const via6522_interface via3_intf =
{
	DEVCB_DRIVER_MEMBER(victor9k_state, via3_pa_r),
	DEVCB_DRIVER_MEMBER(victor9k_state, via3_pb_r),
	DEVCB_NULL, // J5-12
	DEVCB_NULL, // J5-48
	DEVCB_NULL, // J5-14
	DEVCB_NULL, // J5-50

	DEVCB_DRIVER_MEMBER(victor9k_state, via3_pa_w),
	DEVCB_DRIVER_MEMBER(victor9k_state, via3_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, via3_irq_w)
};

WRITE8_MEMBER( victor9k_state::via4_pa_w )
{
	/*

	    bit     description

	    PA0     L0MS0
	    PA1     L0MS1
	    PA2     L0MS2
	    PA3     L0MS3
	    PA4     ST0A
	    PA5     ST0B
	    PA6     ST0C
	    PA7     ST0D

	*/

	m_st[0] = data >> 4;
}

WRITE8_MEMBER( victor9k_state::via4_pb_w )
{
	/*

	    bit     description

	    PB0     L1MS0
	    PB1     L1MS1
	    PB2     L1MS2
	    PB3     L1MS3
	    PB4     ST1A
	    PB5     ST1B
	    PB6     ST1C
	    PB7     ST1D

	*/

	m_st[1] = data >> 4;
}

WRITE_LINE_MEMBER( victor9k_state::mode_w )
{
}

WRITE_LINE_MEMBER( victor9k_state::via4_irq_w )
{
	m_via4_irq = state;

	pic8259_ir3_w(m_pic, m_ssda_irq | m_via1_irq | m_via2_irq | m_via3_irq | m_via4_irq | m_via5_irq | m_via6_irq);
}

static const via6522_interface via4_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL, // DS0
	DEVCB_NULL, // DS1
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DRIVER_MEMBER(victor9k_state, via4_pa_w),
	DEVCB_DRIVER_MEMBER(victor9k_state, via4_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, mode_w),
	DEVCB_NULL,

	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, via4_irq_w)
};

READ8_MEMBER( victor9k_state::via5_pa_r )
{
	/*

	    bit     description

	    PA0     E0
	    PA1     E1
	    PA2     I1
	    PA3     E2
	    PA4     E4
	    PA5     E5
	    PA6     I7
	    PA7     E6

	*/

	return 0;
}

WRITE8_MEMBER( victor9k_state::via5_pb_w )
{
	/*

	    bit     description

	    PB0     WD0
	    PB1     WD1
	    PB2     WD2
	    PB3     WD3
	    PB4     WD4
	    PB5     WD5
	    PB6     WD6
	    PB7     WD7

	*/
}

WRITE_LINE_MEMBER( victor9k_state::via5_irq_w )
{
	m_via5_irq = state;

	pic8259_ir3_w(m_pic, m_ssda_irq | m_via1_irq | m_via2_irq | m_via3_irq | m_via4_irq | m_via5_irq | m_via6_irq);
}

static const via6522_interface via5_intf =
{
	DEVCB_DRIVER_MEMBER(victor9k_state, via5_pa_r),
	DEVCB_NULL,
	DEVCB_NULL, // BRDY
	DEVCB_NULL,
	DEVCB_NULL, // RDY0
	DEVCB_NULL, // RDY1

	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(victor9k_state, via5_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, via5_irq_w)
};

READ8_MEMBER( victor9k_state::via6_pa_r )
{
	/*

	    bit     description

	    PA0
	    PA1     TRK0D0
	    PA2
	    PA3     TRK0D1
	    PA4
	    PA5
	    PA6     WPS
	    PA7     SYNC

	*/

	UINT8 data = 0;

	// track 0 drive A sense
	data |= floppy_tk00_r(m_floppy0) << 1;

	// track 0 drive B sense
	data |= floppy_tk00_r(m_floppy1) << 3;

	// write protect sense
	data |= floppy_wpt_r(m_drive ? m_floppy1 : m_floppy0) << 6;

	// disk sync detect

	return data;
}

WRITE8_MEMBER( victor9k_state::via6_pa_w )
{
	/*

	    bit     description

	    PA0     LED0A
	    PA1
	    PA2     LED1A
	    PA3
	    PA4     SIDE SELECT
	    PA5     DRIVE SELECT
	    PA6
	    PA7

	*/

	// LED, drive A
	output_set_led_value(LED_A, BIT(data, 0));

	// LED, drive B
	output_set_led_value(LED_B, BIT(data, 2));

	// dual side select
	m_side = BIT(data, 4);

	// select drive A/B
	m_drive = BIT(data, 5);
}

READ8_MEMBER( victor9k_state::via6_pb_r )
{
	/*

	    bit     description

	    PB0     RDY0
	    PB1     RDY1
	    PB2
	    PB3     _DS1
	    PB4     _DS0
	    PB5     SINGLE/_DOUBLE SIDED
	    PB6
	    PB7

	*/

	UINT8 data = 0;

	// motor speed status, drive A

	// motor speed status, drive B

	// door B sense

	// door A sense

	// single/double sided
	data |= floppy_twosid_r(m_drive ? m_floppy1 : m_floppy0) << 5;

	return data;
}

WRITE8_MEMBER( victor9k_state::via6_pb_w )
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2     SCRESET
	    PB3
	    PB4
	    PB5
	    PB6     stepper enable A
	    PB7     stepper enable B

	*/

	// motor speed controller reset
	m_fdc_cpu->set_input_line(INPUT_LINE_RESET, BIT(data, 2));

	// stepper enable A
	m_se[0] = BIT(data, 6);

	// stepper enable B
	m_se[1] = BIT(data, 7);
}

WRITE_LINE_MEMBER( victor9k_state::drw_w )
{
}

WRITE_LINE_MEMBER( victor9k_state::erase_w )
{
}

WRITE_LINE_MEMBER( victor9k_state::via6_irq_w )
{
	m_via6_irq = state;

	pic8259_ir3_w(m_pic, m_ssda_irq | m_via1_irq | m_via2_irq | m_via3_irq | m_via4_irq | m_via5_irq | m_via6_irq);
}

static const via6522_interface via6_intf =
{
	DEVCB_DRIVER_MEMBER(victor9k_state, via6_pa_r),
	DEVCB_DRIVER_MEMBER(victor9k_state, via6_pb_r),
	DEVCB_NULL, // GCRERR
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DRIVER_MEMBER(victor9k_state, via6_pa_w),
	DEVCB_DRIVER_MEMBER(victor9k_state, via6_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, drw_w),
	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, erase_w),

	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, via6_irq_w)
};

// Keyboard

WRITE_LINE_MEMBER( victor9k_state::kbrdy_w )
{
	m_via2->write_cb1(state);

	pic8259_ir6_w(m_pic, state ? CLEAR_LINE : ASSERT_LINE);
}

static VICTOR9K_KEYBOARD_INTERFACE( kb_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(victor9k_state, kbrdy_w)
};

// Floppy Configuration

static const floppy_interface victor9k_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSQD,
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	"floppy_5_25",
	NULL
};

// IEEE-488 Interface

static IEEE488_INTERFACE( ieee488_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(M6522_1_TAG, via6522_device, write_ca1),
	DEVCB_DEVICE_LINE_MEMBER(M6522_1_TAG, via6522_device, write_ca2),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

// Machine Initialization

static IRQ_CALLBACK( victor9k_irq_callback )
{
	victor9k_state *state = device->machine().driver_data<victor9k_state>();

	return pic8259_acknowledge(state->m_pic);
}

void victor9k_state::machine_start()
{
	// set interrupt callback
	m_maincpu->set_irq_acknowledge_callback(victor9k_irq_callback);

	// memory banking
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_ram(0x00000, m_ram->size() - 1, m_ram->pointer());
}

// Machine Driver

static MACHINE_CONFIG_START( victor9k, victor9k_state )
	// basic machine hardware
	MCFG_CPU_ADD(I8088_TAG, I8088, XTAL_30MHz/6)
	MCFG_CPU_PROGRAM_MAP(victor9k_mem)

	MCFG_CPU_ADD(I8048_TAG, I8048, XTAL_30MHz/6)
	MCFG_CPU_IO_MAP(floppy_io)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_UPDATE_DEVICE(HD46505S_TAG, hd6845_device, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(monochrome_green)

	MCFG_MC6845_ADD(HD46505S_TAG, HD6845, 1000000, hd46505s_intf) // HD6845 == HD46505S

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(HC55516_TAG, HC55516, 100000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_IEEE488_BUS_ADD(ieee488_intf)
	MCFG_PIC8259_ADD(I8259A_TAG, pic_intf)
	MCFG_PIT8253_ADD(I8253_TAG, pit_intf)
	MCFG_UPD7201_ADD(UPD7201_TAG, XTAL_30MHz/30, mpsc_intf)
	MCFG_MC6852_ADD(MC6852_TAG, XTAL_30MHz/30, ssda_intf)
	MCFG_VIA6522_ADD(M6522_1_TAG, XTAL_30MHz/30, via1_intf)
	MCFG_VIA6522_ADD(M6522_2_TAG, XTAL_30MHz/30, via2_intf)
	MCFG_VIA6522_ADD(M6522_3_TAG, XTAL_30MHz/30, via3_intf)
	MCFG_VIA6522_ADD(M6522_4_TAG, XTAL_30MHz/30, via4_intf)
	MCFG_VIA6522_ADD(M6522_5_TAG, XTAL_30MHz/30, via5_intf)
	MCFG_VIA6522_ADD(M6522_6_TAG, XTAL_30MHz/30, via6_intf)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(victor9k_floppy_interface)
	MCFG_VICTOR9K_KEYBOARD_ADD(kb_intf)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("256K,384K,512K,640K,768K,896K")
MACHINE_CONFIG_END

// ROMs

ROM_START( victor9k )
	ROM_REGION( 0x2000, I8088_TAG, 0 )
	ROM_DEFAULT_BIOS( "univ" )
	ROM_SYSTEM_BIOS( 0, "old", "Older" )
	ROMX_LOAD( "102320.7j", 0x0000, 0x1000, CRC(3d615fd7) SHA1(b22f7e5d66404185395d8effbf57efded0079a92), ROM_BIOS(1) )
	ROMX_LOAD( "102322.8j", 0x1000, 0x1000, CRC(9209df0e) SHA1(3ee8e0c15186bbd5768b550ecc1fa3b6b1dbb928), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "univ", "Universal" )
	ROMX_LOAD( "v9000 univ. fe f3f7 13db.7j", 0x0000, 0x1000, CRC(25c7a59f) SHA1(8784e9aa7eb9439f81e18b8e223c94714e033911), ROM_BIOS(2) )
	ROMX_LOAD( "v9000 univ. ff f3f7 39fe.8j", 0x1000, 0x1000, CRC(496c7467) SHA1(eccf428f62ef94ab85f4a43ba59ae6a066244a66), ROM_BIOS(2) )

	ROM_REGION( 0x400, I8048_TAG, 0)
	ROM_LOAD( "36080.5d", 0x000, 0x400, CRC(9bf49f7d) SHA1(b3a11bb65105db66ae1985b6f482aab6ea1da38b) )

	ROM_REGION( 0x800, "gcr", 0 )
	ROM_LOAD( "100836-001.4k", 0x000, 0x800, CRC(adc601bd) SHA1(6eeff3d2063ae2d97452101aa61e27ef83a467e5) )
ROM_END

// System Drivers

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     INIT  COMPANY                     FULLNAME        FLAGS
COMP( 1982, victor9k, 0,      0,      victor9k, victor9k, driver_device, 0,    "Victor Business Products", "Victor 9000",   GAME_NOT_WORKING )
