// license:BSD-3-Clause
// copyright-holders:Yochizo
class exzisus_state : public driver_device
{
public:
	exzisus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cpuc(*this, "cpuc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_objectram1(*this, "objectram1"),
		m_videoram1(*this, "videoram1"),
		m_sharedram_ac(*this, "sharedram_ac"),
		m_sharedram_ab(*this, "sharedram_ab"),
		m_objectram0(*this, "objectram0"),
		m_videoram0(*this, "videoram0") { }

	required_device<cpu_device> m_cpuc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_objectram1;
	required_shared_ptr<uint8_t> m_videoram1;
	required_shared_ptr<uint8_t> m_sharedram_ac;
	required_shared_ptr<uint8_t> m_sharedram_ab;
	required_shared_ptr<uint8_t> m_objectram0;
	required_shared_ptr<uint8_t> m_videoram0;

	void cpua_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpub_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coincounter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpub_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
