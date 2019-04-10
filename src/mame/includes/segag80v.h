// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega vector hardware

*************************************************************************/

#include "audio/segasnd.h"
#include "machine/segag80.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "video/vector.h"

#include "screen.h"

class segag80v_state : public segag80snd_common
{
public:
	segag80v_state(const machine_config &mconfig, device_type type, const char *tag)
		: segag80snd_common(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_vectorram(*this, "vectorram"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_speech(*this, "segaspeech"),
		m_usb(*this, "usbsnd"),
		m_aysnd(*this, "aysnd"),
		m_vector(*this, "vector"),
		m_screen(*this, "screen")
	{ }

	void g80v_base(machine_config &config);
	void tacscan(machine_config &config);
	void elim2(machine_config &config);
	void startrek(machine_config &config);
	void zektor(machine_config &config);
	void spacfury(machine_config &config);

	void init_zektor();
	void init_startrek();
	void init_elim4();
	void init_elim2();
	void init_tacscan();
	void init_spacfury();

	DECLARE_INPUT_CHANGED_MEMBER(service_switch);
	DECLARE_CUSTOM_INPUT_MEMBER(elim4_joint_coin_r);

private:
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_vectorram;

	required_device<cpu_device> m_maincpu;
	optional_device<samples_device> m_samples;
	optional_device<speech_sound_device> m_speech;
	optional_device<usb_sound_device> m_usb;
	optional_device<ay8912_device> m_aysnd;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;


	uint8_t m_mult_data[2];
	uint16_t m_mult_result;
	uint8_t m_spinner_select;
	uint8_t m_spinner_sign;
	uint8_t m_spinner_count;
	offs_t m_scrambled_write_pc;
	segag80_decrypt_func m_decrypt;
	int m_min_x;
	int m_min_y;
	DECLARE_READ8_MEMBER(g80v_opcode_r);
	DECLARE_WRITE8_MEMBER(mainram_w);
	DECLARE_WRITE8_MEMBER(vectorram_w);
	DECLARE_READ8_MEMBER(mangled_ports_r);
	DECLARE_WRITE8_MEMBER(spinner_select_w);
	DECLARE_READ8_MEMBER(spinner_input_r);
	DECLARE_READ8_MEMBER(elim4_input_r);
	DECLARE_WRITE8_MEMBER(multiply_w);
	DECLARE_READ8_MEMBER(multiply_r);
	DECLARE_WRITE8_MEMBER(coin_count_w);
	DECLARE_WRITE8_MEMBER(unknown_w);

	DECLARE_WRITE8_MEMBER(elim1_sh_w);
	DECLARE_WRITE8_MEMBER(elim2_sh_w);
	DECLARE_WRITE8_MEMBER(zektor1_sh_w);
	DECLARE_WRITE8_MEMBER(zektor2_sh_w);
	DECLARE_WRITE8_MEMBER(spacfury1_sh_w);
	DECLARE_WRITE8_MEMBER(spacfury2_sh_w);

	DECLARE_WRITE8_MEMBER(usb_ram_w);

	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_segag80v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline bool adjust_xy(int rawx, int rawy, int *outx, int *outy);
	void sega_generate_vector_list();
	offs_t decrypt_offset(address_space &space, offs_t offset);
	inline uint8_t demangle(uint8_t d7d6, uint8_t d5d4, uint8_t d3d2, uint8_t d1d0);

	void main_map(address_map &map);
	void opcodes_map(address_map &map);
	void main_portmap(address_map &map);
};
