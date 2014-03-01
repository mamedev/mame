/*****************************************************************************
 *
 * includes/channelf.h
 *
 ****************************************************************************/

#ifndef CHANNELF_H_
#define CHANNELF_H_

#include "emu.h"
#include "cpu/f8/f8.h"
#include "imagedev/cartslot.h"
#include "audio/channelf.h"


/* SKR - 2102 RAM chip on carts 10 and 18 I/O ports */
struct r2102_t
{
	UINT8 d;            /* data bit:inverted logic, but reading/writing cancel out */
	UINT8 r_w;          /* inverted logic: 0 means read, 1 means write */
	UINT8 a[10];        /* addr bits: inverted logic, but reading/writing cancel out */
	UINT16 addr;        /* calculated addr from addr bits */
	UINT8 ram[1024];    /* RAM array */
};


class channelf_state : public driver_device
{
public:
	channelf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_custom(*this,"custom")    ,
		m_maincpu(*this, "maincpu") { }

	DECLARE_READ8_MEMBER(channelf_port_0_r);
	DECLARE_READ8_MEMBER(channelf_port_1_r);
	DECLARE_READ8_MEMBER(channelf_port_4_r);
	DECLARE_READ8_MEMBER(channelf_port_5_r);
	DECLARE_READ8_MEMBER(channelf_2102A_r);
	DECLARE_READ8_MEMBER(channelf_2102B_r);
	DECLARE_WRITE8_MEMBER(channelf_port_0_w);
	DECLARE_WRITE8_MEMBER(channelf_port_1_w);
	DECLARE_WRITE8_MEMBER(channelf_port_4_w);
	DECLARE_WRITE8_MEMBER(channelf_port_5_w);
	DECLARE_WRITE8_MEMBER(channelf_2102A_w);
	DECLARE_WRITE8_MEMBER(channelf_2102B_w);
	UINT8 *m_p_videoram;
	UINT8 m_latch[6];
	r2102_t m_r2102;
	UINT8 m_val_reg;
	UINT8 m_row_reg;
	UINT8 m_col_reg;
	UINT8 port_read_with_latch(UINT8 ext, UINT8 latch_state);
	virtual void video_start();
	DECLARE_PALETTE_INIT(channelf);
	UINT32 screen_update_channelf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( channelf_cart );
	required_device<channelf_sound_device> m_custom;
	required_device<cpu_device> m_maincpu;
	int recalc_palette_offset(int reg1, int reg2);
};

#endif /* CHANNELF_H_ */
