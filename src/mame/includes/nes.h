// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Fabio Priuli
/*****************************************************************************

    nes.h

    Nintendo Entertainment System / Famicom

 ****************************************************************************/

#ifndef NES_H_
#define NES_H_


#include "video/ppu2c0x.h"
#include "bus/nes/nes_slot.h"
#include "bus/nes/nes_carts.h"
#include "bus/nes_ctrl/ctrl.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define NTSC_CLOCK           N2A03_DEFAULTCLOCK     /* 1.789772 MHz */
#define PAL_CLOCK              (26601712.0/16)        /* 1.662607 MHz */

#define NES_BATTERY_SIZE 0x2000


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/*PPU fast banking constants and structures */

#define CHRROM 0
#define CHRRAM 1


/*PPU nametable fast banking constants and structures */

#define CIRAM 0
#define ROM 1
#define EXRAM 2
#define MMC5FILL 3
#define CART_NTRAM 4

#define NES_BATTERY 0
#define NES_WRAM 1


class nes_state : public driver_device
{
public:
	nes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ppu(*this, "ppu"),
			m_ctrl1(*this, "ctrl1"),
			m_ctrl2(*this, "ctrl2"),
			m_exp(*this, "exp"),
			m_cartslot(*this, "nes_slot"),
			m_disk(*this, "disk")
		{ }

	/* video-related */
	int m_last_frame_flip;

	/* misc */
	ioport_port       *m_io_disksel;

	UINT8      *m_vram;
	UINT8      *m_ciram; //PPU nametable RAM - external to PPU!

	required_device<cpu_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;
	required_device<nes_control_port_device> m_ctrl1;
	required_device<nes_control_port_device> m_ctrl2;
	optional_device<nes_control_port_device> m_exp;
	optional_device<nes_cart_slot_device> m_cartslot;
	optional_device<nes_disksys_device> m_disk;

	int nes_ppu_vidaccess(int address, int data);
	void ppu_nmi(int *ppu_regs);

	DECLARE_READ8_MEMBER(nes_in0_r);
	DECLARE_READ8_MEMBER(nes_in1_r);
	DECLARE_WRITE8_MEMBER(nes_in0_w);
	DECLARE_READ8_MEMBER(fc_in0_r);
	DECLARE_READ8_MEMBER(fc_in1_r);
	DECLARE_WRITE8_MEMBER(fc_in0_w);
	DECLARE_WRITE8_MEMBER(nes_vh_sprite_dma_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void video_reset() override;
	DECLARE_PALETTE_INIT(nes);
	UINT32 screen_update_nes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	NESCTRL_BRIGHTPIXEL_CB(bright_pixel);

	DECLARE_DRIVER_INIT(famicom);

	// these are needed until we modernize the FDS controller
	DECLARE_MACHINE_START(fds);
	DECLARE_MACHINE_START(famitwin);
	DECLARE_MACHINE_RESET(fds);
	DECLARE_MACHINE_RESET(famitwin);
	void setup_disk(nes_disksys_device *slot);

private:
	memory_bank       *m_prg_bank_mem[5];
};

#endif /* NES_H_ */
