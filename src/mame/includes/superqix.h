// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni, Nicola Salmoria, Tomasz Slanina
#include "sound/samples.h"

class superqix_state : public driver_device
{
public:
	enum
	{
		MCU_ACKNOWLEDGE, /// TODO: get rid of this hack!
		HLE_68705_WRITE
	};

	superqix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_mcu(*this,"mcu"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_bitmapram(*this, "bitmapram"),
		m_bitmapram2(*this, "bitmapram2"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_mcu;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_bitmapram;
	optional_shared_ptr<uint8_t> m_bitmapram2;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	std::unique_ptr<int16_t[]> m_samplebuf;
	// MCU HLE and/or 8751 related
	uint8_t m_port1;          // HLE-related for superqix
	uint8_t m_port2;          // HLE-related for superqix
	uint8_t m_port3;          // HLE-related for superqix
	uint8_t m_port3_latch;    // HLE-related for superqix
	uint8_t m_fromZ80pending; // HLE-related for superqix, to add a delay to z80->mcu comms
	int m_curr_player;      // HLE-related for prebillian

	// commmon 68705/8751/HLE
	uint8_t m_fromMCU;        // byte latch for 68705/8751->z80 comms
	uint8_t m_fromZ80;        // byte latch for z80->68705/8751 comms
	bool m_Z80HasWritten;   // z80 has written to latch flag
	bool m_MCUHasWritten;   // 68705/8751 has written to latch flag

	// 68705 related
	uint8_t m_portA_in;      // actual input to pins
	//uint8_t m_portB_in;      //
	//uint8_t m_portC_in;      //
	//uint8_t m_portA_out;     // actual output on pins (post ddr)
	uint8_t m_portB_out;     //
	uint8_t m_portC_out;     //

	//uint8_t m_portA_internal;// output before applying ddr
	uint8_t m_portB_internal;//
	uint8_t m_portC_internal;//
	uint8_t m_ddrA;
	uint8_t m_ddrB;
	uint8_t m_ddrC;

	//general machine stuff
	bool m_invert_coin_lockout;
	int m_gfxbank;
	bool m_show_bitmap;
	bool m_nmi_mask;

	// spinner quadrature stuff
	int m_oldpos[2];
	int m_sign[2];

	std::unique_ptr<bitmap_ind16> m_fg_bitmap[2];
	tilemap_t *m_bg_tilemap;


	DECLARE_WRITE8_MEMBER(pbillian_sample_trigger_w);
	DECLARE_READ8_MEMBER(mcu_acknowledge_r);
	DECLARE_WRITE8_MEMBER(bootleg_mcu_p1_w);
	DECLARE_WRITE8_MEMBER(mcu_p3_w);
	DECLARE_READ8_MEMBER(bootleg_mcu_p3_r);
	DECLARE_READ8_MEMBER(sqix_system_status_r);
	DECLARE_WRITE8_MEMBER(sqixu_mcu_p2_w);
	DECLARE_READ8_MEMBER(sqixu_mcu_p3_r);
	DECLARE_READ8_MEMBER(nmi_ack_r);
	DECLARE_WRITE8_MEMBER(bootleg_flipscreen_w);
	DECLARE_READ8_MEMBER(hotsmash_68705_portA_r);
	DECLARE_WRITE8_MEMBER(hotsmash_68705_portB_w);
	DECLARE_READ8_MEMBER(hotsmash_68705_portC_r);
	DECLARE_WRITE8_MEMBER(hotsmash_68705_portC_w);
	DECLARE_WRITE8_MEMBER(hotsmash_68705_ddr_a_w);
	DECLARE_WRITE8_MEMBER(hotsmash_68705_ddr_b_w);
	DECLARE_WRITE8_MEMBER(hotsmash_68705_ddr_c_w);
	DECLARE_WRITE8_MEMBER(hotsmash_Z80_mcu_w);
	DECLARE_READ8_MEMBER(hotsmash_Z80_mcu_r);
	DECLARE_CUSTOM_INPUT_MEMBER(pbillian_semaphore_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(superqix_semaphore_input_r);
	DECLARE_WRITE8_MEMBER(pbillian_Z80_mcu_w);
	DECLARE_WRITE8_MEMBER(superqix_videoram_w);
	DECLARE_WRITE8_MEMBER(superqix_bitmapram_w);
	DECLARE_WRITE8_MEMBER(superqix_bitmapram2_w);
	DECLARE_WRITE8_MEMBER(pbillian_0410_w);
	DECLARE_WRITE8_MEMBER(superqix_0410_w);
	DECLARE_READ8_MEMBER(sqix_from_mcu_r);
	//DECLARE_READ8_MEMBER(superqix_ay1_a_r);
	DECLARE_READ8_MEMBER(in4_mcu_r); //DECLARE_READ8_MEMBER(superqix_ay1_b_r);
	DECLARE_WRITE8_MEMBER(sqix_z80_mcu_w);
	DECLARE_READ8_MEMBER(bootleg_in0_r);
	DECLARE_READ8_MEMBER(hotsmash_ay_port_a_r);
	DECLARE_READ8_MEMBER(pbillian_ay_port_a_r);
	DECLARE_READ8_MEMBER(pbillian_ay_port_b_r);
	SAMPLES_START_CB_MEMBER(pbillian_sh_start);
	DECLARE_DRIVER_INIT(perestro);
	DECLARE_DRIVER_INIT(sqix);
	DECLARE_DRIVER_INIT(sqixr0);
	TILE_GET_INFO_MEMBER(pb_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(sqix_get_bg_tile_info);
	DECLARE_MACHINE_START(pbillian);
	DECLARE_VIDEO_START(pbillian);
	DECLARE_MACHINE_START(superqix);
	DECLARE_VIDEO_START(superqix);
	DECLARE_PALETTE_DECODER(BBGGRRII);
	uint32_t screen_update_pbillian(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_superqix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sqix_timer_irq);
	TIMER_CALLBACK_MEMBER(mcu_acknowledge_callback);
	TIMER_CALLBACK_MEMBER(hle_68705_w_cb);
	void pbillian_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void superqix_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	int read_dial(int player);
	void machine_init_common();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
