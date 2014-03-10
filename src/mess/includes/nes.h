/*****************************************************************************

    nes.h

    Nintendo Entertainment System / Famicom

 ****************************************************************************/

#ifndef NES_H_
#define NES_H_


#include "video/ppu2c0x.h"
#include "bus/nes/nes_slot.h"
#include "bus/nes/nes_carts.h"
#include "sound/nes_apu.h"
#include "imagedev/cassette.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define NTSC_CLOCK           N2A03_DEFAULTCLOCK     /* 1.789772 MHz */
#define PAL_CLOCK              (26601712.0/16)        /* 1.662607 MHz */

#define NES_BATTERY_SIZE 0x2000


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct nes_input
{
	UINT32 shift;
	UINT32 i0, i1, i2;
};

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
	enum
	{
		TIMER_ZAPPER_TICK,
		TIMER_LIGHTGUN_TICK
	};

	nes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ppu(*this, "ppu"),
			m_sound(*this, "nessound"),
			m_cartslot(*this, "nes_slot"),
			m_cassette(*this, "tape")
		{ }

	/* video-related */
	int m_last_frame_flip;

	/* misc */

	ioport_port       *m_io_ctrlsel;
	ioport_port       *m_io_fckey[9];
	ioport_port       *m_io_subkey[13];
	ioport_port       *m_io_pad[4];
	ioport_port       *m_io_powerpad[2];
	ioport_port       *m_io_mahjong[4];
	ioport_port       *m_io_ftrainer[4];
	ioport_port       *m_io_cc_left;
	ioport_port       *m_io_cc_right;
	ioport_port       *m_io_zapper1_t;
	ioport_port       *m_io_zapper1_x;
	ioport_port       *m_io_zapper1_y;
	ioport_port       *m_io_zapper2_t;
	ioport_port       *m_io_zapper2_x;
	ioport_port       *m_io_zapper2_y;
	ioport_port       *m_io_paddle;
	ioport_port       *m_io_paddle_btn;
	ioport_port       *m_io_exp;

	UINT8      *m_vram;
	UINT8      *m_ciram; //PPU nametable RAM - external to PPU!

	required_device<cpu_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;
	required_device<nesapu_device> m_sound;
	optional_device<nes_cart_slot_device> m_cartslot;
	optional_device<cassette_image_device> m_cassette;

	int nes_ppu_vidaccess(int address, int data);
	void ppu_nmi(int *ppu_regs);

	DECLARE_READ8_MEMBER(nes_in0_r);
	DECLARE_READ8_MEMBER(nes_in1_r);
	DECLARE_WRITE8_MEMBER(nes_in0_w);
	DECLARE_WRITE8_MEMBER(nes_in1_w);
	DECLARE_READ8_MEMBER(fc_in0_r);
	DECLARE_READ8_MEMBER(fc_in1_r);
	DECLARE_WRITE8_MEMBER(fc_in0_w);
	DECLARE_WRITE8_MEMBER(nes_vh_sprite_dma_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void video_reset();
	DECLARE_PALETTE_INIT(nes);
	UINT32 screen_update_nes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER(psg_4015_r);
	DECLARE_WRITE8_MEMBER(psg_4015_w);
	DECLARE_WRITE8_MEMBER(psg_4017_w);
	void state_register();

	/***** FDS-floppy related *****/

	DECLARE_WRITE8_MEMBER(fds_chr_w);
	DECLARE_READ8_MEMBER(fds_chr_r);
	DECLARE_WRITE8_MEMBER(fds_nt_w);
	DECLARE_READ8_MEMBER(fds_nt_r);

	int m_disk_expansion;

	UINT8 m_fds_sides;
	UINT8 *m_fds_data;    // here, we store a copy of the disk
	UINT8 *m_fds_ram; // here, we emulate the RAM adapter

	/* Variables which can change */
	UINT8 m_fds_motor_on;
	UINT8 m_fds_door_closed;
	UINT8 m_fds_current_side;
	UINT32 m_fds_head_position;
	UINT8 m_fds_status0;
	UINT8 m_fds_read_mode;
	UINT8 m_fds_write_reg;
	int m_fds_mirroring;

	int m_IRQ_enable, m_IRQ_enable_latch;
	UINT16 m_IRQ_count, m_IRQ_count_latch;

	/* these are used in the mapper 20 handlers */
	int m_fds_last_side;
	int m_fds_count;
	DECLARE_READ8_MEMBER(nes_fds_r);
	DECLARE_WRITE8_MEMBER(nes_fds_w);
	DECLARE_DRIVER_INIT(famicom);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(nes_disk);
	DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER(nes_disk);

	void fds_irq(int scanline, int vblank, int blanked);

	// input related
	UINT32 m_pad_latch[4];
	UINT8 m_zapper_latch[2][3];
	UINT8 m_paddle_latch, m_paddle_btn_latch;
	UINT8 m_mjpanel_latch;
	UINT8 m_fck_scan, m_fck_mode;
	UINT8 m_mic_obstruct;
	UINT8 m_powerpad_latch[2];
	UINT8 m_ftrainer_scan;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	memory_bank       *m_prg_bank_mem[5];
};

#endif /* NES_H_ */
