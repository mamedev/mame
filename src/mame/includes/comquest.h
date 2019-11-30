// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/comquest.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_COMQUEST_H
#define MAME_INCLUDES_COMQUEST_H


class comquest_state : public driver_device
{
public:
	comquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	uint8_t m_data[128][8];
	void *m_timer;
	int m_line;
	int m_dma_activ;
	int m_state;
	int m_count;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_comquest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	void comquest(machine_config &config);
	void comquest_mem(address_map &map);
};

#endif // MAME_INCLUDES_COMQUEST_H
