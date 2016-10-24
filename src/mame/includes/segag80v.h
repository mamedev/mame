// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega vector hardware

*************************************************************************/
#include "sound/samples.h"
#include "machine/segag80.h"
#include "audio/segasnd.h"
#include "video/vector.h"

class segag80v_state : public driver_device
{
public:
	segag80v_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_vectorram(*this, "vectorram"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_speech(*this, "segaspeech"),
		m_usb(*this, "usbsnd"),
		m_vector(*this, "vector"),
		m_screen(*this, "screen"){ }

	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_vectorram;

	required_device<cpu_device> m_maincpu;
	optional_device<samples_device> m_samples;
	optional_device<speech_sound_device> m_speech;
	optional_device<usb_sound_device> m_usb;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;


	uint8_t m_mult_data[2];
	uint16_t m_mult_result;
	uint8_t m_spinner_select;
	uint8_t m_spinner_sign;
	uint8_t m_spinner_count;
	segag80_decrypt_func m_decrypt;
	int m_min_x;
	int m_min_y;
	void mainram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vectorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mangled_ports_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void spinner_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t spinner_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t elim4_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void multiply_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t multiply_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void coin_count_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void unknown_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value elim4_joint_coin_r(ioport_field &field, void *param);
	void elim1_sh_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void elim2_sh_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zektor1_sh_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zektor2_sh_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spacfury1_sh_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spacfury2_sh_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void service_switch(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void usb_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_zektor();
	void init_startrek();
	void init_elim4();
	void init_elim2();
	void init_tacscan();
	void init_spacfury();
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_segag80v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline bool adjust_xy(int rawx, int rawy, int *outx, int *outy);
	void sega_generate_vector_list();
	offs_t decrypt_offset(address_space &space, offs_t offset);
	inline uint8_t demangle(uint8_t d7d6, uint8_t d5d4, uint8_t d3d2, uint8_t d1d0);
};
