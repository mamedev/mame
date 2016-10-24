// license:BSD-3-Clause
// copyright-holders:Mike Appolo
/*************************************************************************

    Atari Major Havoc hardware

*************************************************************************/

#define MHAVOC_CLOCK        10000000
#define MHAVOC_CLOCK_5M     (MHAVOC_CLOCK/2)
#define MHAVOC_CLOCK_2_5M   (MHAVOC_CLOCK/4)
#define MHAVOC_CLOCK_1_25M  (MHAVOC_CLOCK/8)
#define MHAVOC_CLOCK_625K   (MHAVOC_CLOCK/16)

#define MHAVOC_CLOCK_156K   (MHAVOC_CLOCK_625K/4)
#define MHAVOC_CLOCK_5K     (MHAVOC_CLOCK_625K/16/8)
#define MHAVOC_CLOCK_2_4K   (MHAVOC_CLOCK_625K/16/16)


class mhavoc_state : public driver_device
{
public:
	mhavoc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_zram0(*this, "zram0"),
		m_zram1(*this, "zram1"),
		m_alpha(*this, "alpha"),
		m_gamma(*this, "gamma"){ }

	required_shared_ptr<uint8_t> m_zram0;
	required_shared_ptr<uint8_t> m_zram1;
	required_device<cpu_device> m_alpha;
	optional_device<cpu_device> m_gamma;
	uint8_t m_alpha_data;
	uint8_t m_alpha_rcvd;
	uint8_t m_alpha_xmtd;
	uint8_t m_gamma_data;
	uint8_t m_gamma_rcvd;
	uint8_t m_gamma_xmtd;
	uint8_t m_player_1;
	uint8_t m_alpha_irq_clock;
	uint8_t m_alpha_irq_clock_enable;
	uint8_t m_gamma_irq_clock;
	uint8_t m_has_gamma_cpu;
	uint8_t m_speech_write_buffer;
	uint8_t dual_pokey_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dual_pokey_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mhavoc_alpha_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mhavoc_gamma_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mhavoc_gamma_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mhavoc_alpha_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mhavoc_alpha_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mhavoc_gamma_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mhavoc_ram_banksel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mhavoc_rom_banksel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mhavoc_out_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void alphaone_out_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mhavoc_out_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mhavocrv_speech_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mhavocrv_speech_strobe_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t quad_pokeyn_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void quad_pokeyn_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value tms5220_r(ioport_field &field, void *param);
	ioport_value mhavoc_bit67_r(ioport_field &field, void *param);
	ioport_value gamma_rcvd_r(ioport_field &field, void *param);
	ioport_value gamma_xmtd_r(ioport_field &field, void *param);
	ioport_value alpha_rcvd_r(ioport_field &field, void *param);
	ioport_value alpha_xmtd_r(ioport_field &field, void *param);
	ioport_value clock_r(ioport_field &field, void *param);
	void init_mhavocrv();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void delayed_gamma_w(void *ptr, int32_t param);
	void mhavoc_cpu_irq_clock(timer_device &timer, void *ptr, int32_t param);
};
