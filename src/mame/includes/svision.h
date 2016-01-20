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
	UINT8 input;
	emu_timer *timer;
};

struct tvlink_t
{
	UINT32 palette[4/*0x40?*/]; /* rgb8 */
	int palette_on;
};

class svision_state : public driver_device
{
public:
	svision_state(const machine_config &mconfig, device_type type, std::string tag)
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

	DECLARE_READ8_MEMBER(svision_r);
	DECLARE_WRITE8_MEMBER(svision_w);
	DECLARE_READ8_MEMBER(tvlink_r);
	DECLARE_WRITE8_MEMBER(tvlink_w);
	DECLARE_DRIVER_INIT(svisions);
	DECLARE_DRIVER_INIT(svision);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(svision);
	DECLARE_PALETTE_INIT(svisionp);
	DECLARE_PALETTE_INIT(svisionn);
	DECLARE_MACHINE_RESET(tvlink);
	UINT32 screen_update_svision(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tvlink(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(svision_frame_int);
	TIMER_CALLBACK_MEMBER(svision_pet_timer);
	TIMER_CALLBACK_MEMBER(svision_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(svision_pet_timer_dev);
	void svision_irq();
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(svision_cart);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<svision_sound_device> m_sound;
	required_device<generic_slot_device> m_cart;
	required_shared_ptr<UINT8> m_reg;
	required_shared_ptr<UINT8> m_videoram;
	required_ioport m_joy;
	optional_ioport m_joy2;
	required_device<palette_device> m_palette;

	memory_region *m_cart_rom;
	memory_bank *m_bank1;
	memory_bank *m_bank2;
};

#endif /* SVISION_H_ */
