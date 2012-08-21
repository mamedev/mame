/***************************************************************************

    includes/apple2.h

    Include file to handle emulation of the Apple II series.

***************************************************************************/

#ifndef APPLE2_H_
#define APPLE2_H_

#include "machine/a2bus.h"
#include "machine/a2eauxslot.h"
#include "machine/applefdc.h"
#include "machine/ram.h"

#define AUXSLOT_TAG "auxbus"

/***************************************************************************
    SOFTSWITCH VALUES
***************************************************************************/

#define VAR_80STORE		0x000001
#define VAR_RAMRD		0x000002
#define VAR_RAMWRT		0x000004
#define VAR_INTCXROM	0x000008
#define VAR_ALTZP		0x000010
#define VAR_SLOTC3ROM	0x000020
#define VAR_80COL		0x000040
#define VAR_ALTCHARSET	0x000080
#define VAR_TEXT		0x000100
#define VAR_MIXED		0x000200
#define VAR_PAGE2		0x000400
#define VAR_HIRES		0x000800
#define VAR_AN0			0x001000
#define VAR_AN1			0x002000
#define VAR_AN2			0x004000
#define VAR_AN3			0x008000
#define VAR_LCRAM		0x010000
#define VAR_LCRAM2		0x020000
#define VAR_LCWRITE		0x040000
#define VAR_ROMSWITCH	0x080000
#define VAR_TK2000RAM   0x100000        // ROM/RAM switch for TK2000

#define VAR_DHIRES		VAR_AN3


/***************************************************************************
    SPECIAL KEYS
***************************************************************************/

#define SPECIALKEY_CAPSLOCK		0x01
#define SPECIALKEY_SHIFT		0x06
#define SPECIALKEY_CONTROL		0x08
#define SPECIALKEY_BUTTON0		0x10	/* open apple */
#define SPECIALKEY_BUTTON1		0x20	/* closed apple */
#define SPECIALKEY_BUTTON2		0x40
#define SPECIALKEY_RESET		0x80


/***************************************************************************
    OTHER
***************************************************************************/

/* -----------------------------------------------------------------------
 * New Apple II memory manager
 * ----------------------------------------------------------------------- */

#define APPLE2_MEM_AUX		0x40000000
#define APPLE2_MEM_SLOT		0x80000000
#define APPLE2_MEM_ROM		0xC0000000
#define APPLE2_MEM_FLOATING	0xFFFFFFFF
#define APPLE2_MEM_MASK		0x00FFFFFF

typedef enum
{
    APPLE_II,           // Apple II/II+
    APPLE_IIEPLUS,      // Apple IIe/IIc/IIgs/IIc+
    TK2000,             // Microdigital TK2000
    LASER128,           // Laser 128/128EX/128EX2
    SPACE84             // "Space 84" with flipped text mode
} machine_type_t;

typedef enum
{
	A2MEM_IO		= 0,	/* this is always handlers; never banked memory */
	A2MEM_MONO		= 1,	/* this is a bank where read and write are always in unison */
	A2MEM_DUAL		= 2		/* this is a bank where read and write can go different places */
} bank_disposition_t;

typedef struct _apple2_meminfo apple2_meminfo;
struct _apple2_meminfo
{
	UINT32 read_mem;
	read8_delegate *read_handler;
	UINT32 write_mem;
	write8_delegate *write_handler;
};

typedef struct _apple2_memmap_entry apple2_memmap_entry;
struct _apple2_memmap_entry
{
	offs_t begin;
	offs_t end;
	void (*get_meminfo)(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo);
	bank_disposition_t bank_disposition;
};

typedef struct _apple2_memmap_config apple2_memmap_config;
struct _apple2_memmap_config
{
	int first_bank;
	UINT8 *auxmem;
	UINT32 auxmem_length;
	const apple2_memmap_entry *memmap;
};

class apple2_state : public driver_device
{
public:
	apple2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
        m_maincpu(*this, "maincpu"),
        m_ram(*this, RAM_TAG),
        m_a2bus(*this, "a2bus"),
        m_a2eauxslot(*this, AUXSLOT_TAG)
    { }

	required_device<cpu_device> m_maincpu;
    required_device<ram_device> m_ram;
    required_device<a2bus_device> m_a2bus;
    optional_device<a2eauxslot_device> m_a2eauxslot;

	UINT32 m_flags, m_flags_mask;
	INT32 m_a2_cnxx_slot;
	UINT32 m_a2_mask;
	UINT32 m_a2_set;
	int m_a2_speaker_state;
	double m_joystick_x1_time;
	double m_joystick_y1_time;
	double m_joystick_x2_time;
	double m_joystick_y2_time;
	apple2_memmap_config m_mem_config;
	apple2_meminfo *m_current_meminfo;
	int m_fdc_diskreg;
	unsigned int *m_ay3600_keys;
	UINT8 m_keycode;
	UINT8 m_keycode_unmodified;
	UINT8 m_keywaiting;
	UINT8 m_keystilldown;
	UINT8 m_keymodreg;
	int m_reset_flag;
	int m_last_key;
	int m_last_key_unmodified;
	unsigned int m_time_until_repeat;
	const UINT8 *m_a2_videoram;
	UINT32 m_a2_videomask;
	UINT32 m_old_a2;
	int m_fgcolor;
	int m_bgcolor;
	int m_flash;
	int m_alt_charset_value;
	UINT16 *m_hires_artifact_map;
	UINT16 *m_dhires_artifact_map;
    bool m_monochrome_dhr;
    int m_inh_slot;

    UINT8 *m_rambase;

    machine_type_t m_machinetype;

    READ8_MEMBER(apple2_c0xx_r);
    WRITE8_MEMBER(apple2_c0xx_w);
    READ8_MEMBER(apple2_c080_r);
    WRITE8_MEMBER(apple2_c080_w);

    READ8_MEMBER ( apple2_c00x_r );
    READ8_MEMBER ( apple2_c01x_r );
    READ8_MEMBER ( apple2_c02x_r );
    READ8_MEMBER ( apple2_c03x_r );
    READ8_MEMBER ( apple2_c05x_r );
    READ8_MEMBER ( apple2_c06x_r );
    READ8_MEMBER ( apple2_c07x_r );
    WRITE8_MEMBER ( apple2_c00x_w );
    WRITE8_MEMBER ( apple2_c01x_w );
    WRITE8_MEMBER ( apple2_c02x_w );
    WRITE8_MEMBER ( apple2_c03x_w );
    WRITE8_MEMBER ( apple2_c05x_w );
    WRITE8_MEMBER ( apple2_c07x_w );

    WRITE8_MEMBER ( apple2_mainram0400_w );
    WRITE8_MEMBER ( apple2_mainram2000_w );
    WRITE8_MEMBER ( apple2_auxram0400_w );
    WRITE8_MEMBER ( apple2_auxram2000_w );

    READ8_MEMBER ( apple2_c1xx_r );
    WRITE8_MEMBER ( apple2_c1xx_w );
    READ8_MEMBER ( apple2_c3xx_r );
    WRITE8_MEMBER ( apple2_c3xx_w );
    READ8_MEMBER ( apple2_c4xx_r );
    WRITE8_MEMBER ( apple2_c4xx_w );

    READ8_MEMBER ( apple2_c800_r );
    WRITE8_MEMBER ( apple2_c800_w );
    READ8_MEMBER ( apple2_ce00_r );
    WRITE8_MEMBER ( apple2_ce00_w );

    READ8_MEMBER ( apple2_inh_d000_r );
    WRITE8_MEMBER ( apple2_inh_d000_w );
    READ8_MEMBER ( apple2_inh_e000_r );
    WRITE8_MEMBER ( apple2_inh_e000_w );

    READ8_MEMBER(read_floatingbus);

    READ8_MEMBER(apple2_cfff_r);
    WRITE8_MEMBER(apple2_cfff_w);

    void apple2_refresh_delegates();

    read8_delegate read_delegates_master[4];
    write8_delegate write_delegates_master[3];
    write8_delegate write_delegates_0400[2];
    write8_delegate write_delegates_2000[2];
    read8_delegate rd_c000;
    write8_delegate wd_c000;
    read8_delegate rd_c080;
    write8_delegate wd_c080;
    read8_delegate rd_cfff;
    write8_delegate wd_cfff;
    read8_delegate rd_c800;
    write8_delegate wd_c800;
    read8_delegate rd_ce00;
    write8_delegate wd_ce00;
    read8_delegate rd_inh_d000;
    write8_delegate wd_inh_d000;
    read8_delegate rd_inh_e000;
    write8_delegate wd_inh_e000;
};


/*----------- defined in drivers/apple2.c -----------*/

INPUT_PORTS_EXTERN( apple2ep );
PALETTE_INIT( apple2 );


/*----------- defined in machine/apple2.c -----------*/

extern const applefdc_interface apple2_fdc_interface;

void apple2_iwm_setdiskreg(running_machine &machine, UINT8 data);
UINT8 apple2_iwm_getdiskreg(running_machine &machine);

void apple2_init_common(running_machine &machine);
MACHINE_START( apple2 );
MACHINE_START( apple2orig );
MACHINE_START( tk2000 );
MACHINE_START( laser128 );
MACHINE_START( space84 );
UINT8 apple2_getfloatingbusvalue(running_machine &machine);
READ8_HANDLER( apple2_c0xx_r );
WRITE8_HANDLER( apple2_c0xx_w );
READ8_HANDLER( apple2_c080_r );
WRITE8_HANDLER( apple2_c080_w );

TIMER_DEVICE_CALLBACK( apple2_interrupt );

INT8 apple2_slotram_r(running_machine &machine, int slotnum, int offset);

void apple2_setvar(running_machine &machine, UINT32 val, UINT32 mask);

int apple2_pressed_specialkey(running_machine &machine, UINT8 key);

void apple2_setup_memory(running_machine &machine, const apple2_memmap_config *config);
void apple2_update_memory(running_machine &machine);



/*----------- defined in video/apple2.c -----------*/

void apple2_video_start(running_machine &machine, const UINT8 *vram, size_t vram_size, UINT32 ignored_softswitches, int hires_modulo);
VIDEO_START( apple2 );
VIDEO_START( apple2p );
VIDEO_START( apple2e );
SCREEN_UPDATE_IND16( apple2 );

#endif /* APPLE2_H_ */
