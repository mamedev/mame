// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont

#include "includes/saturn.h"
#include "machine/gen_latch.h"

class stv_state : public saturn_state
{
public:
	stv_state(const machine_config &mconfig, device_type type, const char *tag)
		: saturn_state(mconfig, type, tag),
		m_adsp(*this, "adsp"),
		m_adsp_pram(*this, "adsp_pram"),
		m_cryptdevice(*this, "315_5881"),
		m_5838crypt(*this, "315_5838"),
		m_soundlatch(*this, "soundlatch")
	{
	}

	void init_astrass();
	void init_batmanfr();
	void init_finlarch();
	void init_decathlt();
	void init_sanjeon();
	void init_puyosun();
	void init_winterht();
	void init_gaxeduel();
	void init_rsgun();
	void init_groovef();
	void init_sandor();
	void init_cottonbm();
	void init_smleague();
	void init_nameclv3();
	void init_danchiq();
	void init_hanagumi();
	void init_cotton2();
	void init_seabass();
	void init_stv();
	void init_thunt();
	void init_critcrsh();
	void init_stvmp();
	void init_sasissu();
	void init_dnmtdeka();
	void init_ffreveng();
	void init_fhboxers();
	void init_pblbeach();
	void init_sss();
	void init_diehard();
	void init_danchih();
	void init_shienryu();
	void init_elandore();
	void init_prikura();
	void init_maruchan();
	void init_colmns97();
	void init_grdforce();
	void init_suikoenb();
	void init_magzun();
	void init_shanhigw();
	void init_sokyugrt();
	void init_vfremix();
	void init_twcup98();
	void init_znpwfv();
	void init_othellos();
	void init_mausuke();

	uint8_t stv_ioga_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void stv_ioga_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t critcrsh_ioga_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t magzun_ioga_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void magzun_ioga_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t stvmp_ioga_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void stvmp_ioga_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t stv_ioga_r32(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void stv_ioga_w32(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t critcrsh_ioga_r32(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t stvmp_ioga_r32(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void stvmp_ioga_w32(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t magzun_ioga_r32(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void magzun_ioga_w32(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t magzun_hef_hack_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t magzun_rx_hack_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

	image_init_result load_cart(device_image_interface &image, generic_slot_device *slot);
	image_init_result device_image_load_stv_cart1(device_image_interface &image) { return load_cart(image, m_cart1); }
	image_init_result device_image_load_stv_cart2(device_image_interface &image) { return load_cart(image, m_cart2); }
	image_init_result device_image_load_stv_cart3(device_image_interface &image) { return load_cart(image, m_cart3); }
	image_init_result device_image_load_stv_cart4(device_image_interface &image) { return load_cart(image, m_cart4); }

	void install_stvbios_speedups( void );

	void machine_start_stv();
	void machine_reset_stv();

	/* Batman Forever specifics */
	optional_device<adsp2181_device>    m_adsp;
	optional_shared_ptr<uint32_t> m_adsp_pram;

	struct
	{
		uint16_t bdma_internal_addr;
		uint16_t bdma_external_addr;
		uint16_t bdma_control;
		uint16_t bdma_word_count;
	} m_adsp_regs;

	void machine_reset_batmanfr();
	uint16_t adsp_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void adsp_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void batmanfr_sound_comms_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// protection specific variables and functions (see machine/stvprot.c)
	uint32_t m_abus_protenable;
	uint32_t m_abus_protkey;

	uint32_t m_a_bus[4];

	uint32_t common_prot_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void common_prot_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	void install_common_protection();
	void stv_register_protection_savestates();



	optional_device<sega_315_5881_crypt_device> m_cryptdevice;
	optional_device<sega_315_5838_comp_device> m_5838crypt;
	optional_device<generic_latch_16_device> m_soundlatch; // batmanfr
	uint16_t crypt_read_callback(uint32_t addr);
	uint16_t crypt_read_callback_ch1(uint32_t addr);
	uint16_t crypt_read_callback_ch2(uint32_t addr);
};


#define MASTER_CLOCK_352 57272720
#define MASTER_CLOCK_320 53693174
#define CEF_1   m_vdp1_regs[0x010/2]|=0x0002
#define CEF_0   m_vdp1_regs[0x010/2]&=~0x0002
#define BEF_1   m_vdp1_regs[0x010/2]|=0x0001
#define BEF_0   m_vdp1_regs[0x010/2]&=~0x0001
#define STV_VDP1_TVMR ((m_vdp1_regs[0x000/2])&0xffff)
#define STV_VDP1_VBE  ((STV_VDP1_TVMR & 0x0008) >> 3)
#define STV_VDP1_TVM  ((STV_VDP1_TVMR & 0x0007) >> 0)

#define IRQ_VBLANK_IN  1 << 0
#define IRQ_VBLANK_OUT 1 << 1
#define IRQ_HBLANK_IN  1 << 2
#define IRQ_TIMER_0    1 << 3
#define IRQ_TIMER_1    1 << 4
#define IRQ_DSP_END    1 << 5
#define IRQ_SOUND_REQ  1 << 6
#define IRQ_SMPC       1 << 7
#define IRQ_PAD        1 << 8
#define IRQ_DMALV2     1 << 9
#define IRQ_DMALV1     1 << 10
#define IRQ_DMALV0     1 << 11
#define IRQ_DMAILL     1 << 12
#define IRQ_VDP1_END   1 << 13
#define IRQ_ABUS       1 << 15

GFXDECODE_EXTERN( stv );
