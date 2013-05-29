#ifndef __ENTERP_H__
#define __ENTERP_H__


#define NICK_PALETTE_SIZE   256

#include "machine/ram.h"
#include "audio/dave.h"
#include "video/epnick.h"

class ep_state : public driver_device
{
public:
	ep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dave(*this, "custom"),
		m_nick(*this, "nick"),
		m_ram(*this, RAM_TAG),
		m_joy(*this, "JOY1") { }

	required_device<cpu_device> m_maincpu;
	required_device<dave_sound_device> m_dave;
	required_device<nick_device> m_nick;
	required_device<ram_device> m_ram;
	required_ioport m_joy;

	UINT8 exdos_card_value;  /* state of the wd1770 irq/drq lines */
	UINT8 keyboard_line;     /* index of keyboard line to read */

	ioport_port *m_key[10];

	DECLARE_READ8_MEMBER(exdos_card_r);
	DECLARE_WRITE8_MEMBER(exdos_card_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
	UINT32 screen_update_enterp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(enterprise_dave_reg_write);
	DECLARE_READ8_MEMBER(enterprise_dave_reg_read);
	DECLARE_WRITE_LINE_MEMBER(enterp_wd1770_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(enterp_wd1770_drq_w);
	void enterprise_update_memory_page(address_space &space, offs_t page, int index);
};


#endif /* __ENTERP_H__ */
