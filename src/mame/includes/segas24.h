// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
 * Sega System 24
 *
 */

#include "video/segaic24.h"
#include "sound/dac.h"

class segas24_state : public driver_device
{
public:
	segas24_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_dac(*this, "dac"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_generic_paletteram_16(*this, "paletteram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<dac_device> m_dac;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT16> m_generic_paletteram_16;

	static const UINT8  mahmajn_mlt[8];
	static const UINT8 mahmajn2_mlt[8];
	static const UINT8      qgh_mlt[8];
	static const UINT8 bnzabros_mlt[8];
	static const UINT8   qrouka_mlt[8];
	static const UINT8 quizmeku_mlt[8];
	static const UINT8   dcclub_mlt[8];

	int fdc_status;
	int fdc_track;
	int fdc_sector;
	int fdc_data;
	int fdc_phys_track;
	int fdc_irq;
	int fdc_drq;
	int fdc_span;
	int fdc_index_count;
	UINT8 *fdc_pt;
	int track_size;
	int cur_input_line;
	UINT8 hotrod_ctrl_cur;
	UINT8 resetcontrol;
	UINT8 prev_resetcontrol;
	UINT8 curbank;
	UINT8 mlatch;
	const UINT8 *mlatch_table;

	UINT16 irq_tdata, irq_tval;
	UINT8 irq_tmode, irq_allow0, irq_allow1;
	int irq_timer_pend0;
	int irq_timer_pend1;
	int irq_yms;
	int irq_vblank;
	int irq_sprite;
	attotime irq_synctime, irq_vsynctime;
	timer_device *irq_timer;
	timer_device *irq_timer_clear;
	//timer_device *irq_frc;
	timer_device *frc_cnt_timer;
	UINT8 frc_mode;

	UINT16 *shared_ram;
	UINT8 (segas24_state::*io_r)(UINT8 port);
	void (segas24_state::*io_w)(UINT8 port, UINT8 data);
	UINT8 io_cnt, io_dir;

	segas24_tile *vtile;
	segas24_sprite *vsprite;
	segas24_mixer *vmixer;

	DECLARE_WRITE_LINE_MEMBER(irq_ym);
	DECLARE_READ16_MEMBER(  sys16_paletteram_r );
	DECLARE_WRITE16_MEMBER( sys16_paletteram_w );
	DECLARE_READ16_MEMBER(  irq_r );
	DECLARE_WRITE16_MEMBER( irq_w );
	DECLARE_READ16_MEMBER(  fdc_r );
	DECLARE_WRITE16_MEMBER( fdc_w );
	DECLARE_READ16_MEMBER(  fdc_status_r );
	DECLARE_WRITE16_MEMBER( fdc_ctrl_w );
	DECLARE_READ16_MEMBER(  curbank_r );
	DECLARE_WRITE16_MEMBER( curbank_w );
	DECLARE_READ8_MEMBER(  frc_mode_r );
	DECLARE_WRITE8_MEMBER( frc_mode_w );
	DECLARE_READ8_MEMBER(  frc_r );
	DECLARE_WRITE8_MEMBER( frc_w );
	DECLARE_READ16_MEMBER(  mlatch_r );
	DECLARE_WRITE16_MEMBER( mlatch_w );
	DECLARE_READ16_MEMBER(  hotrod3_ctrl_r );
	DECLARE_WRITE16_MEMBER( hotrod3_ctrl_w );
	DECLARE_READ16_MEMBER(  iod_r );
	DECLARE_WRITE16_MEMBER( iod_w );
	DECLARE_READ16_MEMBER ( sys16_io_r );
	DECLARE_WRITE16_MEMBER( sys16_io_w );

	UINT8 hotrod_io_r(UINT8 port);
	UINT8 dcclub_io_r(UINT8 port);
	UINT8 mahmajn_io_r(UINT8 port);

	void hotrod_io_w(UINT8 port, UINT8 data);
	void mahmajn_io_w(UINT8 port, UINT8 data);

	void fdc_init();
	void reset_reset();
	void reset_bank();
	void irq_init();
	void irq_timer_sync();
	void irq_timer_start(int old_tmode);
	void reset_control_w(UINT8 data);
	DECLARE_DRIVER_INIT(crkdown);
	DECLARE_DRIVER_INIT(quizmeku);
	DECLARE_DRIVER_INIT(qrouka);
	DECLARE_DRIVER_INIT(roughrac);
	DECLARE_DRIVER_INIT(qgh);
	DECLARE_DRIVER_INIT(gground);
	DECLARE_DRIVER_INIT(mahmajn2);
	DECLARE_DRIVER_INIT(sspiritj);
	DECLARE_DRIVER_INIT(mahmajn);
	DECLARE_DRIVER_INIT(hotrod);
	DECLARE_DRIVER_INIT(sspirits);
	DECLARE_DRIVER_INIT(dcclub);
	DECLARE_DRIVER_INIT(bnzabros);
	DECLARE_DRIVER_INIT(dcclubfd);
	DECLARE_DRIVER_INIT(qsww);
	DECLARE_DRIVER_INIT(sgmast);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_system24(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer_clear_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_frc_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_vbl);
};
