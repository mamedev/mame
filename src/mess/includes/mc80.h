/*****************************************************************************
 *
 * includes/mc80.h
 *
 ****************************************************************************/

#ifndef MC80_H_
#define MC80_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"

class mc80_state : public driver_device
{
public:
	mc80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_p_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu") { }

	DECLARE_WRITE8_MEMBER(mc8030_zve_write_protect_w);
	DECLARE_WRITE8_MEMBER(mc8030_vis_w);
	DECLARE_WRITE8_MEMBER(mc8030_eprom_prog_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_READ8_MEMBER(mc80_port_b_r);
	DECLARE_READ8_MEMBER(mc80_port_a_r);
	DECLARE_WRITE8_MEMBER(mc80_port_a_w);
	DECLARE_WRITE8_MEMBER(mc80_port_b_w);
	DECLARE_READ8_MEMBER(zve_port_a_r);
	DECLARE_READ8_MEMBER(zve_port_b_r);
	DECLARE_WRITE8_MEMBER(zve_port_a_w);
	DECLARE_WRITE8_MEMBER(zve_port_b_w);
	DECLARE_READ8_MEMBER(asp_port_a_r);
	DECLARE_READ8_MEMBER(asp_port_b_r);
	DECLARE_WRITE8_MEMBER(asp_port_a_w);
	DECLARE_WRITE8_MEMBER(asp_port_b_w);
	optional_shared_ptr<UINT8> m_p_videoram;
	DECLARE_MACHINE_RESET(mc8020);
	DECLARE_VIDEO_START(mc8020);
	DECLARE_MACHINE_RESET(mc8030);
	DECLARE_VIDEO_START(mc8030);
	UINT32 screen_update_mc8020(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mc8030(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(mc8020_kbd);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
	IRQ_CALLBACK_MEMBER(mc8020_irq_callback);
	IRQ_CALLBACK_MEMBER(mc8030_irq_callback);
	required_device<cpu_device> m_maincpu;
};


/*----------- defined in machine/mc80.c -----------*/

/*****************************************************************************/
/*                            Implementation for MC80.2x                     */
/*****************************************************************************/

extern const z80ctc_interface mc8020_ctc_intf;
extern const z80pio_interface mc8020_z80pio_intf;

/*****************************************************************************/
/*                            Implementation for MC80.3x                     */
/*****************************************************************************/


extern const z80pio_interface mc8030_zve_z80pio_intf;
extern const z80pio_interface mc8030_asp_z80pio_intf;
extern const z80ctc_interface mc8030_zve_z80ctc_intf;
extern const z80ctc_interface mc8030_asp_z80ctc_intf;
extern const z80dart_interface mc8030_asp_z80sio_intf;

#endif /* MC80_H_ */
