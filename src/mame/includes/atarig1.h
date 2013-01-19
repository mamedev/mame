/*************************************************************************

    Atari G1 hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "cpu/m68000/m68000.h"

class atarig1_state : public atarigen_state
{
public:
	atarig1_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_mo_command(*this, "mo_command") { }

	required_device<m68000_device> m_maincpu;

	bool            m_is_pitfight;

	UINT8           m_which_input;
	required_shared_ptr<UINT16> m_mo_command;

	UINT16 *        m_bslapstic_base;
	void *          m_bslapstic_bank0;
	UINT8           m_bslapstic_bank;
	bool            m_bslapstic_primed;

	int             m_pfscroll_xoffset;
	UINT16          m_current_control;
	UINT8           m_playfield_tile_bank;
	UINT16          m_playfield_xscroll;
	UINT16          m_playfield_yscroll;

	device_t *      m_rle;
	virtual void device_post_load();
	virtual void update_interrupts();
	virtual void scanline_update(screen_device &screen, int scanline);
	DECLARE_WRITE16_MEMBER(mo_control_w);
	DECLARE_WRITE16_MEMBER(mo_command_w);
	DECLARE_READ16_MEMBER(special_port0_r);
	DECLARE_WRITE16_MEMBER(a2d_select_w);
	DECLARE_READ16_MEMBER(a2d_data_r);
	DECLARE_READ16_MEMBER(pitfightb_cheap_slapstic_r);
	void update_bank(int bank);
	DECLARE_DRIVER_INIT(hydrap);
	DECLARE_DRIVER_INIT(hydra);
	DECLARE_DRIVER_INIT(pitfight9);
	DECLARE_DRIVER_INIT(pitfight7);
	DECLARE_DRIVER_INIT(pitfightj);
	DECLARE_DRIVER_INIT(pitfight);
	DECLARE_DRIVER_INIT(pitfightb);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(atarig1);
	DECLARE_MACHINE_RESET(atarig1);
	DECLARE_VIDEO_START(atarig1);
	UINT32 screen_update_atarig1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_atarig1(screen_device &screen, bool state);
private:
	void init_common(offs_t slapstic_base, int slapstic, bool is_pitfight);
	void pitfightb_cheap_slapstic_init();
};
