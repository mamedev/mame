/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"

class offtwall_state : public atarigen_state
{
public:
	offtwall_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
		  m_bankrom_base(*this, "bankrom_base") { }

	UINT16 *m_bankswitch_base;
	required_shared_ptr<UINT16> m_bankrom_base;
	UINT32 m_bank_offset;

	UINT16 *m_spritecache_count;
	UINT16 *m_unknown_verify_base;
	DECLARE_READ16_MEMBER(offtwall_atarivc_r);
	DECLARE_WRITE16_MEMBER(offtwall_atarivc_w);
	DECLARE_READ16_MEMBER(special_port3_r);
	DECLARE_WRITE16_MEMBER(io_latch_w);
	DECLARE_READ16_MEMBER(bankswitch_r);
	DECLARE_READ16_MEMBER(bankrom_r);
	DECLARE_READ16_MEMBER(spritecache_count_r);
	DECLARE_READ16_MEMBER(unknown_verify_r);
};


/*----------- defined in video/offtwall.c -----------*/

VIDEO_START( offtwall );
SCREEN_UPDATE_IND16( offtwall );
