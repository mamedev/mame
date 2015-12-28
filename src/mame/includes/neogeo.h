// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Ernesto Corvi,Andrew Prime,Zsolt Vasvari
// thanks-to:Fuzz
/*************************************************************************

    Neo-Geo hardware

*************************************************************************/

#include "machine/upd1990a.h"
#include "machine/ng_memcard.h"
#include "video/neogeo_spr.h"
#include "bus/neogeo/neogeo_slot.h"
#include "bus/neogeo/neogeo_carts.h"
#include "bus/neogeo/cmc_prot.h"
#include "bus/neogeo/pcm2_prot.h"
#include "bus/neogeo/pvc_prot.h"
#include "bus/neogeo/bootleg_prot.h"
#include "bus/neogeo/kof2002_prot.h"
#include "bus/neogeo/fatfury2_prot.h"
#include "bus/neogeo/kof98_prot.h"
#include "bus/neogeo/sbp_prot.h"
#include "bus/neogeo/kog_prot.h"

// On scanline 224, /VBLANK goes low 56 mclks (14 pixels) from the rising edge of /HSYNC.
// Two mclks after /VBLANK goes low, the hardware sets a pending IRQ1 flip-flop.
#define NEOGEO_VBLANK_IRQ_HTIM (attotime::from_ticks(56+2, NEOGEO_MASTER_CLOCK))


class neogeo_state : public driver_device
{
public:
	neogeo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_banked_cart(*this, "banked_cart"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_region_maincpu(*this, "maincpu"),
		m_region_sprites(*this, "sprites"),
		m_region_fixed(*this, "fixed"),
		m_region_fixedbios(*this, "fixedbios"),
		//m_bank_vectors(*this, "vectors"),
		//m_bank_cartridge(*this, "cartridge"),
		m_bank_audio_main(*this, "audio_main"),
		m_upd4990a(*this, "upd4990a"),
		m_save_ram(*this, "saveram"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_memcard(*this, "memcard"),
		m_sprgen(*this, "spritegen"),
		m_use_cart_vectors(0),
		m_use_cart_audio(0),
		m_cartslot1(*this, "cartslot1"),
		m_cartslot2(*this, "cartslot2"),
		m_cartslot3(*this, "cartslot3"),
		m_cartslot4(*this, "cartslot4"),
		m_cartslot5(*this, "cartslot5"),
		m_cartslot6(*this, "cartslot6"),
		m_currentslot(-1)
	{ }

	DECLARE_WRITE8_MEMBER(io_control_w);
	DECLARE_READ16_MEMBER(memcard_r);
	DECLARE_WRITE16_MEMBER(memcard_w);
	DECLARE_WRITE8_MEMBER(audio_command_w);
	DECLARE_READ8_MEMBER(audio_command_r);
	DECLARE_READ8_MEMBER(audio_cpu_bank_select_r);
	DECLARE_WRITE8_MEMBER(audio_cpu_enable_nmi_w);
	DECLARE_WRITE8_MEMBER(system_control_w);
	DECLARE_READ16_MEMBER(neogeo_unmapped_r);
	DECLARE_READ16_MEMBER(neogeo_paletteram_r);
	DECLARE_WRITE16_MEMBER(neogeo_paletteram_w);
	DECLARE_READ16_MEMBER(neogeo_video_register_r);
	DECLARE_WRITE16_MEMBER(neogeo_video_register_w);
	READ16_MEMBER(banked_vectors_r);
	void set_slot_number(int slot);

	DECLARE_CUSTOM_INPUT_MEMBER(get_memcard_status);
	DECLARE_CUSTOM_INPUT_MEMBER(get_audio_result);

	TIMER_CALLBACK_MEMBER(display_position_interrupt_callback);
	TIMER_CALLBACK_MEMBER(display_position_vblank_callback);
	TIMER_CALLBACK_MEMBER(vblank_interrupt_callback);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(neo_cartridge);

	// MVS-specific
	DECLARE_WRITE16_MEMBER(save_ram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(mahjong_controller_r);
	DECLARE_CUSTOM_INPUT_MEMBER(multiplexed_controller_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kizuna4p_controller_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kizuna4p_start_r);
	DECLARE_INPUT_CHANGED_MEMBER(select_bios);

	UINT32 screen_update_neogeo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_DRIVER_INIT(neogeo);
	DECLARE_DRIVER_INIT(mvs);

	optional_device<neogeo_banked_cart_device> m_banked_cart;

protected:
	void neogeo_postload();
	void update_interrupts();
	void create_interrupt_timers();
	void start_interrupt_timers();
	void neogeo_acknowledge_interrupt(UINT16 data);

	void neogeo_main_cpu_banking_init();
	void neogeo_audio_cpu_banking_init(int set_entry);
	void adjust_display_position_interrupt_timer();
	void neogeo_set_display_position_interrupt_control(UINT16 data);
	void neogeo_set_display_counter_msb(UINT16 data);
	void neogeo_set_display_counter_lsb(UINT16 data);
	void set_video_control( UINT16 data );

	void create_rgb_lookups();
	void set_pens();
	void neogeo_set_screen_shadow( int data );
	void neogeo_set_palette_bank( int data );

	void audio_cpu_check_nmi();
	void select_controller( UINT8 data );
	void set_save_ram_unlock( UINT8 data );
	void set_outputs(  );
	void set_output_latch( UINT8 data );
	void set_output_data( UINT8 data );








	// device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;

	// memory
	required_memory_region m_region_maincpu;
	required_memory_region m_region_sprites;
	required_memory_region m_region_fixed;
	optional_memory_region m_region_fixedbios;
	//required_memory_bank   m_bank_vectors;
	//optional_memory_bank   m_bank_cartridge;  // optional because of neocd
	optional_memory_bank   m_bank_audio_main; // optional because of neocd
	memory_bank           *m_bank_audio_cart[4];

	// MVS-specific devices
	optional_device<upd4990a_device> m_upd4990a;
	optional_shared_ptr<UINT16> m_save_ram;

	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_device<ng_memcard_device> m_memcard;

	// configuration
	enum {NEOGEO_MVS, NEOGEO_AES, NEOGEO_CD} m_type;

	// internal state
	UINT8      m_controller_select;
	bool       m_recurse;
	bool       m_audio_cpu_nmi_enabled;
	bool       m_audio_cpu_nmi_pending;

	// MVS-specific state
	UINT8      m_save_ram_unlocked;
	UINT8      m_output_data;
	UINT8      m_output_latch;
	UINT8      m_el_value;
	UINT8      m_led1_value;
	UINT8      m_led2_value;

	// video hardware, including maincpu interrupts
	// TODO: make into a device
	virtual void video_start() override;
	virtual void video_reset() override;

	emu_timer  *m_display_position_interrupt_timer;
	emu_timer  *m_display_position_vblank_timer;
	emu_timer  *m_vblank_interrupt_timer;
	UINT32     m_display_counter;
	UINT8      m_vblank_interrupt_pending;
	UINT8      m_display_position_interrupt_pending;
	UINT8      m_irq3_pending;
	UINT8      m_display_position_interrupt_control;
	UINT8      m_vblank_level;
	UINT8      m_raster_level;

	required_device<neosprite_optimized_device> m_sprgen;
	UINT16 get_video_control(  );

	// color/palette related
	std::vector<UINT16> m_paletteram;
	UINT8        m_palette_lookup[32][4];
	const pen_t *m_bg_pen;
	int          m_screen_shadow;
	int          m_palette_bank;



	int m_use_cart_vectors;
	int m_use_cart_audio;

	// cart slots
	optional_device<neogeo_cart_slot_device> m_cartslot1;
	optional_device<neogeo_cart_slot_device> m_cartslot2;
	optional_device<neogeo_cart_slot_device> m_cartslot3;
	optional_device<neogeo_cart_slot_device> m_cartslot4;
	optional_device<neogeo_cart_slot_device> m_cartslot5;
	optional_device<neogeo_cart_slot_device> m_cartslot6;
	neogeo_cart_slot_device* m_cartslots[6];
	int m_currentslot;


public:
	DECLARE_READ16_MEMBER(neogeo_slot_rom_low_r);
	DECLARE_READ16_MEMBER(neogeo_slot_rom_low_bectors_r);

};


class neogeo_noslot_state : public neogeo_state
{
	public:
		neogeo_noslot_state(const machine_config &mconfig, device_type type, const char *tag)
			: neogeo_state(mconfig, type, tag),
			/* legacy cartridge specifics */
			m_mslugx_prot(*this, "mslugx_prot"),
			m_sma_prot(*this, "sma_prot"),
			m_cmc_prot(*this, "cmc_prot"),
			m_pcm2_prot(*this, "pcm2_prot"),
			m_pvc_prot(*this, "pvc_prot"),
			m_bootleg_prot(*this, "bootleg_prot"),
			m_kof2002_prot(*this, "kof2002_prot"),
			m_fatfury2_prot(*this, "fatfury2_prot"),
			m_kof98_prot(*this, "kof98_prot"),
			m_sbp_prot(*this, "sbp_prot") {}


	DECLARE_DRIVER_INIT(fatfury2);
	DECLARE_DRIVER_INIT(zupapa);
	DECLARE_DRIVER_INIT(kof98);
	DECLARE_DRIVER_INIT(mslugx);
	DECLARE_DRIVER_INIT(kof99);
	DECLARE_DRIVER_INIT(kof99k);
	DECLARE_DRIVER_INIT(garou);
	DECLARE_DRIVER_INIT(garouh);
	DECLARE_DRIVER_INIT(garoubl);
	DECLARE_DRIVER_INIT(mslug3);
	DECLARE_DRIVER_INIT(mslug3h);
	DECLARE_DRIVER_INIT(mslug3b6);
	DECLARE_DRIVER_INIT(kof2000);
	DECLARE_DRIVER_INIT(kof2000n);
	DECLARE_DRIVER_INIT(kof2001);
	DECLARE_DRIVER_INIT(cthd2003);
	DECLARE_DRIVER_INIT(ct2k3sp);
	DECLARE_DRIVER_INIT(ct2k3sa);
	DECLARE_DRIVER_INIT(mslug4);
	DECLARE_DRIVER_INIT(ms4plus);
	DECLARE_DRIVER_INIT(ganryu);
	DECLARE_DRIVER_INIT(s1945p);
	DECLARE_DRIVER_INIT(preisle2);
	DECLARE_DRIVER_INIT(bangbead);
	DECLARE_DRIVER_INIT(nitd);
	DECLARE_DRIVER_INIT(sengoku3);
	DECLARE_DRIVER_INIT(rotd);
	DECLARE_DRIVER_INIT(kof2002);
	DECLARE_DRIVER_INIT(kof2002b);
	DECLARE_DRIVER_INIT(kf2k2pls);
	DECLARE_DRIVER_INIT(kf2k2mp);
	DECLARE_DRIVER_INIT(kf2k2mp2);
	DECLARE_DRIVER_INIT(kof10th);
	DECLARE_DRIVER_INIT(kf10thep);
	DECLARE_DRIVER_INIT(kf2k5uni);
	DECLARE_DRIVER_INIT(kof2k4se);
	DECLARE_DRIVER_INIT(matrim);
	DECLARE_DRIVER_INIT(matrimbl);
	DECLARE_DRIVER_INIT(pnyaa);
	DECLARE_DRIVER_INIT(mslug5);
	DECLARE_DRIVER_INIT(ms5pcb);
	DECLARE_DRIVER_INIT(ms5plus);
	DECLARE_DRIVER_INIT(svcpcb);
	DECLARE_DRIVER_INIT(svc);
	DECLARE_DRIVER_INIT(svcboot);
	DECLARE_DRIVER_INIT(svcplus);
	DECLARE_DRIVER_INIT(svcplusa);
	DECLARE_DRIVER_INIT(svcsplus);
	DECLARE_DRIVER_INIT(samsho5);
	DECLARE_DRIVER_INIT(samsho5b);
	DECLARE_DRIVER_INIT(kf2k3pcb);
	DECLARE_DRIVER_INIT(kof2003);
	DECLARE_DRIVER_INIT(kof2003h);
	DECLARE_DRIVER_INIT(kf2k3bl);
	DECLARE_DRIVER_INIT(kf2k3pl);
	DECLARE_DRIVER_INIT(kf2k3upl);
	DECLARE_DRIVER_INIT(samsh5sp);
	DECLARE_DRIVER_INIT(jockeygp);
	DECLARE_DRIVER_INIT(vliner);
	DECLARE_DRIVER_INIT(kof97oro);
	DECLARE_DRIVER_INIT(lans2004);
	DECLARE_DRIVER_INIT(sbp);

	void install_banked_bios();
	// non-carts
	void svcpcb_gfx_decrypt();
	void svcpcb_s1data_decrypt();
	void kf2k3pcb_gfx_decrypt();
	void kf2k3pcb_decrypt_s1data();
	void kf2k3pcb_sp1_decrypt();


	// legacy
	optional_device<mslugx_prot_device> m_mslugx_prot;
	optional_device<sma_prot_device> m_sma_prot;
	optional_device<cmc_prot_device> m_cmc_prot;
	optional_device<pcm2_prot_device> m_pcm2_prot;
	optional_device<pvc_prot_device> m_pvc_prot;
	optional_device<ngbootleg_prot_device> m_bootleg_prot;
	optional_device<kof2002_prot_device> m_kof2002_prot;
	optional_device<fatfury2_prot_device> m_fatfury2_prot;
	optional_device<kof98_prot_device> m_kof98_prot;
	optional_device<sbp_prot_device> m_sbp_prot;
};

class neogeo_noslot_kog_state : public neogeo_state
{
public:
	neogeo_noslot_kog_state(const machine_config &mconfig, device_type type, const char *tag)
		: neogeo_state(mconfig, type, tag),
		/* legacy cartridge specifics */
		m_bootleg_prot(*this, "bootleg_prot"),
		m_kog_prot(*this, "kog_prot") {}


	DECLARE_DRIVER_INIT(kog);

	// legacy
	optional_device<ngbootleg_prot_device> m_bootleg_prot;
	optional_device<kog_prot_device> m_kog_prot;
};

/*----------- defined in drivers/neogeo.c -----------*/

MACHINE_CONFIG_EXTERN( neogeo_base );
MACHINE_CONFIG_EXTERN( neogeo_arcade );
INPUT_PORTS_EXTERN(neogeo);
ADDRESS_MAP_EXTERN(neogeo_main_map,16);

/*************************************
 *
 *  Neo-Geo bios
 *
 *************************************

    These are the known Bios Roms, Set options.bios to the one you want.

    The Universe bios roms are supported because they're now used on enough PCBs
    to be considered 'in active arcade use' rather than just homebrew hacks.
    Some may be missing, there have been multiple CRCs reported for the same
    revision in some cases (the Universe bios has an option for entering / displaying
    a serial number; these should be noted as such if they're added).

    The 'japan-hotel' BIOS is a dump of an MVS which could be found in some japanese
    hotels. it is a custom MVS mobo which uses MVS carts but it hasn't jamma
    connector and it's similar to a console with a coin mechanism, so it's a sort
    of little coin op console installed in hotels.

    The sp-45.sp1 bios is the latest 'ASIA' revision. Japan-j3.bin is the latest 'JAPAN'
    revision. Both of them are also used in the sp-4x.sp1 bios of the Jamma PCB boards.

    The current Neo-Geo MVS system set (SFIX/SM1/000-LO) used is from a NEO-MVH MV1FS board.
    Other boards (MV1xx / MV2x / MV4x /MV6x) other system sets?

    Zoom ROM (LO)    128K   TC531000CP      1x 128Kx8   Zoom look-up table ROM
    Fix ROM (SFIX)   128K   27C1000         1x 128Kx8   Text layer graphics ROM
    Sound ROM (SM1)  128K   27C1000/23C1000 1x 128Kx8   Z80 program ROM

*/

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios+1)) /* Note '+1' */

#define NEOGEO_BIOS \
	ROM_REGION16_BE( 0x80000, "mainbios", 0 ) \
	ROM_SYSTEM_BIOS( 0, "euro", "Europe MVS (Ver. 2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "sp-s2.sp1",         0x00000, 0x020000, CRC(9036d879) SHA1(4f5ed7105b7128794654ce82b51723e16e389543) ) /* Europe, 1 Slot, has also been found on 2 Slot and 4 Slot (the old hacks were designed for this one) */ \
	ROM_SYSTEM_BIOS( 1, "euro-s1", "Europe MVS (Ver. 1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "sp-s.sp1",          0x00000, 0x020000, CRC(c7f2fa45) SHA1(09576ff20b4d6b365e78e6a5698ea450262697cd) ) /* Europe, 4 Slot */ \
	\
	ROM_SYSTEM_BIOS( 2, "us", "US MVS (Ver. 2?)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "sp-u2.sp1",         0x00000, 0x020000, CRC(e72943de) SHA1(5c6bba07d2ec8ac95776aa3511109f5e1e2e92eb) ) /* US, 2 Slot */ \
	ROM_SYSTEM_BIOS( 3, "us-e", "US MVS (Ver. 1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 3, "sp-e.sp1",          0x00000, 0x020000, CRC(2723a5b5) SHA1(5dbff7531cf04886cde3ef022fb5ca687573dcb8) ) /* US, 6 Slot (V5?) */ \
	ROM_SYSTEM_BIOS( 4, "us-v2", "US MVS (4 slot, Ver 2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(4, "v2.bin",            0x00000, 0x020000, CRC(62f021f4) SHA1(62d372269e1b3161c64ae21123655a0a22ffd1bb) ) /* US, 4 slot */ \
	\
	ROM_SYSTEM_BIOS( 5, "asia", "Asia MVS (Ver. 3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 5, "asia-s3.rom",       0x00000, 0x020000, CRC(91b64be3) SHA1(720a3e20d26818632aedf2c2fd16c54f213543e1) ) /* Asia */ \
	\
	ROM_SYSTEM_BIOS( 6, "japan", "Japan MVS (Ver. 3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 6, "vs-bios.rom",       0x00000, 0x020000, CRC(f0e8f27d) SHA1(ecf01eda815909f1facec62abf3594eaa8d11075) ) /* Japan, Ver 6 VS Bios */ \
	ROM_SYSTEM_BIOS( 7, "japan-s2", "Japan MVS (Ver. 2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 7, "sp-j2.sp1",         0x00000, 0x020000, CRC(acede59c) SHA1(b6f97acd282fd7e94d9426078a90f059b5e9dd91) ) /* Japan, Older */ \
	ROM_SYSTEM_BIOS( 8, "japan-s1", "Japan MVS (Ver. 1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 8, "sp1.jipan.1024",    0x00000, 0x020000, CRC(9fb0abe4) SHA1(18a987ce2229df79a8cf6a84f968f0e42ce4e59d) ) /* Japan, Older */ \
	ROM_SYSTEM_BIOS( 9, "mv1c", "NEO-MVH MV1C" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 9, "sp-45.sp1",         0x00000, 0x080000, CRC(03cc9f6a) SHA1(cdf1f49e3ff2bac528c21ed28449cf35b7957dc1) ) /* Latest Asia bios */ \
	ROM_SYSTEM_BIOS( 10, "japan-j3", "Japan MVS (J3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 10, "japan-j3.bin",      0x00000, 0x020000, CRC(dff6d41f) SHA1(e92910e20092577a4523a6b39d578a71d4de7085) ) /* Latest Japan bios; correct chip label unknown */ \
	ROM_SYSTEM_BIOS(11, "japan-hotel", "Custom Japanese Hotel" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(11, "sp-1v1_3db8c.bin",  0x00000, 0x020000, CRC(162f0ebe) SHA1(fe1c6dd3dfcf97d960065b1bb46c1e11cb7bf271) ) /* 'rare MVS found in japanese hotels' shows v1.3 in test mode */ \
	\
	ROM_SYSTEM_BIOS(12, "unibios31", "Universe Bios (Hack, Ver. 3.1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(12, "uni-bios_3_1.rom",  0x00000, 0x020000, CRC(0c58093f) SHA1(29329a3448c2505e1ff45ffa75e61e9693165153) ) /* Universe Bios v3.1 (hack) */ \
	ROM_SYSTEM_BIOS(13, "unibios30", "Universe Bios (Hack, Ver. 3.0)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(13, "uni-bios_3_0.rom",  0x00000, 0x020000, CRC(a97c89a9) SHA1(97a5eff3b119062f10e31ad6f04fe4b90d366e7f) ) /* Universe Bios v3.0 (hack) */ \
	ROM_SYSTEM_BIOS(14, "unibios23", "Universe Bios (Hack, Ver. 2.3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(14, "uni-bios_2_3.rom",  0x00000, 0x020000, CRC(27664eb5) SHA1(5b02900a3ccf3df168bdcfc98458136fd2b92ac0) ) /* Universe Bios v2.3 (hack) */ \
	ROM_SYSTEM_BIOS(15, "unibios23o", "Universe Bios (Hack, Ver. 2.3, older?)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(15, "uni-bios_2_3o.rom", 0x00000, 0x020000, CRC(601720ae) SHA1(1b8a72c720cdb5ee3f1d735bbcf447b09204b8d9) ) /* Universe Bios v2.3 (hack) alt version, withdrawn? */ \
	ROM_SYSTEM_BIOS(16, "unibios22", "Universe Bios (Hack, Ver. 2.2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(16, "uni-bios_2_2.rom",  0x00000, 0x020000, CRC(2d50996a) SHA1(5241a4fb0c63b1a23fd1da8efa9c9a9bd3b4279c) ) /* Universe Bios v2.2 (hack) */ \
	ROM_SYSTEM_BIOS(17, "unibios21", "Universe Bios (Hack, Ver. 2.1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(17, "uni-bios_2_1.rom",  0x00000, 0x020000, CRC(8dabf76b) SHA1(c23732c4491d966cf0373c65c83c7a4e88f0082c) ) /* Universe Bios v2.1 (hack) */ \
	ROM_SYSTEM_BIOS(18, "unibios20", "Universe Bios (Hack, Ver. 2.0)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(18, "uni-bios_2_0.rom",  0x00000, 0x020000, CRC(0c12c2ad) SHA1(37bcd4d30f3892078b46841d895a6eff16dc921e) ) /* Universe Bios v2.0 (hack) */ \
	ROM_SYSTEM_BIOS(19, "unibios13", "Universe Bios (Hack, Ver. 1.3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(19, "uni-bios_1_3.rom",  0x00000, 0x020000, CRC(b24b44a0) SHA1(eca8851d30557b97c309a0d9f4a9d20e5b14af4e) ) /* Universe Bios v1.3 (hack) */ \
	ROM_SYSTEM_BIOS(20, "unibios12", "Universe Bios (Hack, Ver. 1.2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(20, "uni-bios_1_2.rom",  0x00000, 0x020000, CRC(4fa698e9) SHA1(682e13ec1c42beaa2d04473967840c88fd52c75a) ) /* Universe Bios v1.2 (hack) */ \
	ROM_SYSTEM_BIOS(21, "unibios12o", "Universe Bios (Hack, Ver. 1.2, older)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(21, "uni-bios_1_2o.rom", 0x00000, 0x020000, CRC(e19d3ce9) SHA1(af88ef837f44a3af2d7144bb46a37c8512b67770) ) /* Universe Bios v1.2 (hack) alt version */ \
	ROM_SYSTEM_BIOS(22, "unibios11", "Universe Bios (Hack, Ver. 1.1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(22, "uni-bios_1_1.rom",  0x00000, 0x020000, CRC(5dda0d84) SHA1(4153d533c02926a2577e49c32657214781ff29b7) ) /* Universe Bios v1.1 (hack) */ \
	ROM_SYSTEM_BIOS(23, "unibios10", "Universe Bios (Hack, Ver. 1.0)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS(23, "uni-bios_1_0.rom",  0x00000, 0x020000, CRC(0ce453a0) SHA1(3b4c0cd26c176fc6b26c3a2f95143dd478f6abf9) ) /* Universe Bios v1.0 (hack) */




#define NEO_BIOS_AUDIO_64K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x20000, "audiocpu", 0 ) \
	ROM_LOAD( name, 0x00000, 0x10000, hash ) \
	ROM_RELOAD(     0x10000, 0x10000 )

#define NEO_BIOS_AUDIO_128K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x30000, "audiocpu", 0 ) \
	ROM_LOAD( name, 0x00000, 0x20000, hash ) \
	ROM_RELOAD(     0x10000, 0x20000 )

#define NEO_BIOS_AUDIO_256K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x50000, "audiocpu", 0 ) \
	ROM_LOAD( name, 0x00000, 0x40000, hash ) \
	ROM_RELOAD(     0x10000, 0x40000 )

#define NEO_BIOS_AUDIO_512K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x90000, "audiocpu", 0 ) \
	ROM_LOAD( name, 0x00000, 0x80000, hash ) \
	ROM_RELOAD(     0x10000, 0x80000 )


#define NEO_BIOS_AUDIO_ENCRYPTED_128K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "audiocrypt", 0 ) \
	ROM_LOAD( name, 0x00000, 0x20000, hash )
#define NEO_BIOS_AUDIO_ENCRYPTED_256K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "audiocrypt", 0 ) \
	ROM_LOAD( name, 0x00000, 0x40000, hash )
#define NEO_BIOS_AUDIO_ENCRYPTED_512K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "audiocrypt", 0 ) \
	ROM_LOAD( name,      0x00000, 0x80000, hash )

#define NO_DELTAT_REGION


#define NEO_SFIX_64K(name, hash) \
	ROM_REGION( 0x20000, "fixed", 0 ) \
	ROM_LOAD( name, 0x000000, 0x10000, hash ) \
	ROM_REGION( 0x20000, "fixedbios", 0 ) \
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) ) \
	ROM_Y_ZOOM

#define NEO_SFIX_128K(name, hash) \
	ROM_REGION( 0x20000, "fixed", 0 ) \
	ROM_LOAD( name, 0x000000, 0x20000, hash ) \
	ROM_REGION( 0x20000, "fixedbios", 0 ) \
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) ) \
	ROM_Y_ZOOM

#define ROM_Y_ZOOM \
	ROM_REGION( 0x20000, "zoomy", 0 ) \
	ROM_LOAD( "000-lo.lo", 0x00000, 0x20000, CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a) )
