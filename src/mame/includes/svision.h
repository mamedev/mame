// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/svision.h
 *
 ****************************************************************************/

#ifndef SVISION_H_
#define SVISION_H_

#include "cpu/m6502/m65c02.h"
#include "audio/svis_snd.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

struct svision_t
{
	emu_timer *timer1;
	int timer_shot;
};

struct svision_pet_t
{
	int state;
	int on, clock, data;
	uint8_t input;
	emu_timer *timer;
};

struct tvlink_t
{
	uint32_t palette[4/*0x40?*/]; /* rgb8 */
	int palette_on;
};

class svision_state : public driver_device
{
public:
	svision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "custom"),
		m_cart(*this, "cartslot"),
		m_reg(*this, "reg"),
		m_videoram(*this, "videoram"),
		m_joy(*this, "JOY"),
		m_joy2(*this, "JOY2"),
		m_palette(*this, "palette")  { }

	int *m_dma_finished;
	svision_t m_svision;
	svision_pet_t m_pet;
	tvlink_t m_tvlink;

	uint8_t svision_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void svision_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tvlink_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tvlink_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_svisions();
	void init_svision();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void palette_init_svision(palette_device &palette);
	void palette_init_svisionp(palette_device &palette);
	void palette_init_svisionn(palette_device &palette);
	void machine_reset_tvlink();
	uint32_t screen_update_svision(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tvlink(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void svision_frame_int(device_t &device);
	void svision_pet_timer(void *ptr, int32_t param);
	void svision_timer(void *ptr, int32_t param);
	void svision_pet_timer_dev(timer_device &timer, void *ptr, int32_t param);
	void svision_irq();
	image_init_result device_image_load_svision_cart(device_image_interface &image);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<svision_sound_device> m_sound;
	required_device<generic_slot_device> m_cart;
	required_shared_ptr<uint8_t> m_reg;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport m_joy;
	optional_ioport m_joy2;
	required_device<palette_device> m_palette;

	memory_region *m_cart_rom;
	memory_bank *m_bank1;
	memory_bank *m_bank2;
};

#endif /* SVISION_H_ */
