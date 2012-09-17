/*****************************************************************************
 *
 * includes/c65.h
 *
 ****************************************************************************/

#ifndef C65_H_
#define C65_H_

#include "includes/c64_legacy.h"
#include "machine/6526cia.h"

struct dma_t
{
	int version;
	UINT8 data[4];
};

struct fdc_t
{
	int state;

	UINT8 reg[0x0f];

	UINT8 buffer[0x200];
	int cpu_pos;
	int fdc_pos;

	UINT16 status;

	attotime time;
	int head,track,sector;
};

struct expansion_ram_t
{
	UINT8 reg;
};

class c65_state : public legacy_c64_state
{
public:
	c65_state(const machine_config &mconfig, device_type type, const char *tag)
		: legacy_c64_state(mconfig, type, tag) { }

	UINT8 *m_chargen;
	UINT8 *m_interface;
	int m_charset_select;
	int m_c64mode;
	UINT8 m_6511_port;
	UINT8 m_keyline;
	int m_old_value;
	int m_nmilevel;
	dma_t m_dma;
	int m_dump_dma;
	fdc_t m_fdc;
	expansion_ram_t m_expansion_ram;
	int m_io_on;
	int m_io_dc00_on;
	DECLARE_DRIVER_INIT(c65);
	DECLARE_DRIVER_INIT(c65pal);

	DECLARE_READ8_MEMBER( sid_potx_r );
	DECLARE_READ8_MEMBER( sid_poty_r );
	DECLARE_MACHINE_START(c65);
	DECLARE_PALETTE_INIT(c65);
	UINT32 screen_update_c65(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in machine/c65.c -----------*/

/*extern UINT8 *c65_memory; */
/*extern UINT8 *c65_basic; */
/*extern UINT8 *c65_kernal; */
/*extern UINT8 *c65_dos; */
/*extern UINT8 *c65_monitor; */
/*extern UINT8 *c65_graphics; */

void c65_bankswitch (running_machine &machine);
//void c65_colorram_write (running_machine &machine, int offset, int value);

int c65_dma_read(running_machine &machine, int offset);
int c65_dma_read_color(running_machine &machine, int offset);
void c65_vic_interrupt(running_machine &machine, int level);
void c65_bankswitch_interface(running_machine &machine, int value);


INTERRUPT_GEN( c65_frame_interrupt );

extern const mos6526_interface c65_cia0;
extern const mos6526_interface c65_cia1;

#endif /* C65_H_ */
