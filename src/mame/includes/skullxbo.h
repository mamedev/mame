/*************************************************************************

    Atari Skull & Crossbones hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"

class skullxbo_state : public atarigen_state
{
public:
	skullxbo_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_scanline_timer(*this, "scan_timer") { }

	required_device<atari_jsa_ii_device> m_jsa;
	required_device<timer_device> m_scanline_timer;

	virtual void update_interrupts();
	virtual void scanline_update(screen_device &screen, int scanline);
	DECLARE_WRITE16_MEMBER(skullxbo_halt_until_hblank_0_w);
	DECLARE_READ16_MEMBER(special_port1_r);
	DECLARE_WRITE16_MEMBER(skullxbo_mobwr_w);
	DECLARE_DRIVER_INIT(skullxbo);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(skullxbo);
	DECLARE_MACHINE_RESET(skullxbo);
	DECLARE_VIDEO_START(skullxbo);
	UINT32 screen_update_skullxbo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);
	void skullxbo_scanline_update(int scanline);
	DECLARE_WRITE16_MEMBER( skullxbo_playfieldlatch_w );
	DECLARE_WRITE16_MEMBER( skullxbo_xscroll_w );
	DECLARE_WRITE16_MEMBER( skullxbo_yscroll_w );
	DECLARE_WRITE16_MEMBER( skullxbo_mobmsb_w );
};
