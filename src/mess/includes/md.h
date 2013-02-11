#ifndef __MD__
#define __MD__

// Temporary include file to handle SVP add-on, until converted to a proper slot device
// Afterwards, these classes can be moved back to mess/drivers/megadriv.c!

#include "includes/megadriv.h"
#include "machine/md_slot.h"

class md_cons_state : public md_base_state
{
public:
	md_cons_state(const machine_config &mconfig, device_type type, const char *tag)
		: md_base_state(mconfig, type, tag),
		m_slotcart(*this, "mdslot")
	{ }

	emu_timer *m_mess_io_timeout[3];
	int m_mess_io_stage[3];

	optional_device<md_cart_slot_device> m_slotcart;

	DECLARE_DRIVER_INIT(mess_md_common);
	DECLARE_DRIVER_INIT(genesis);
	DECLARE_DRIVER_INIT(md_eur);
	DECLARE_DRIVER_INIT(md_jpn);
};

class mdsvp_state : public md_cons_state
{
public:
	mdsvp_state(const machine_config &mconfig, device_type type, const char *tag)
		: md_cons_state(mconfig, type, tag) { }

	UINT8 *m_iram; // IRAM (0-0x7ff)
	UINT8 *m_dram; // [0x20000];
	UINT32 m_pmac_read[6];  // read modes/addrs for PM0-PM5
	UINT32 m_pmac_write[6]; // write ...
	PAIR m_pmc;
	UINT32 m_emu_status;
	UINT16 m_XST;       // external status, mapped at a15000 and a15002 on 68k side.
	UINT16 m_XST2;      // status of XST (bit1 set when 68k writes to XST)
};

class pico_state : public md_cons_state
{
public:
	pico_state(const machine_config &mconfig, device_type type, const char *tag)
		: md_cons_state(mconfig, type, tag),
		m_picocart(*this, "picoslot") { }

	optional_device<pico_cart_slot_device> m_picocart;
	UINT8 m_page_register;
};



ADDRESS_MAP_EXTERN( svp_ssp_map, driver_device );
ADDRESS_MAP_EXTERN( svp_ext_map, driver_device );
extern void svp_init(running_machine &machine);
extern cpu_device *_svp_cpu;


#endif
