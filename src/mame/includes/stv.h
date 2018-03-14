// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont

#include "includes/saturn.h"
#include "audio/rax.h"
#include "machine/eepromser.h"
#include "machine/ticket.h"

class stv_state : public saturn_state
{
public:
	stv_state(const machine_config &mconfig, device_type type, const char *tag)
		: saturn_state(mconfig, type, tag),
		m_cart1(*this, "stv_slot1"),
		m_cart2(*this, "stv_slot2"),
		m_cart3(*this, "stv_slot3"),
		m_cart4(*this, "stv_slot4"),
		m_rax(*this, "rax"),
		m_eeprom(*this, "eeprom"),
		m_cryptdevice(*this, "315_5881"),
		m_5838crypt(*this, "315_5838"),
		m_hopper(*this, "hopper")
	{
	}

	DECLARE_DRIVER_INIT(astrass);
	DECLARE_DRIVER_INIT(batmanfr);
	DECLARE_DRIVER_INIT(finlarch);
	DECLARE_DRIVER_INIT(decathlt);
	DECLARE_DRIVER_INIT(sanjeon);
	DECLARE_DRIVER_INIT(puyosun);
	DECLARE_DRIVER_INIT(winterht);
	DECLARE_DRIVER_INIT(gaxeduel);
	DECLARE_DRIVER_INIT(rsgun);
	DECLARE_DRIVER_INIT(groovef);
	DECLARE_DRIVER_INIT(sandor);
	DECLARE_DRIVER_INIT(cottonbm);
	DECLARE_DRIVER_INIT(smleague);
	DECLARE_DRIVER_INIT(nameclv3);
	DECLARE_DRIVER_INIT(danchiq);
	DECLARE_DRIVER_INIT(hanagumi);
	DECLARE_DRIVER_INIT(cotton2);
	DECLARE_DRIVER_INIT(seabass);
	DECLARE_DRIVER_INIT(stv);
	DECLARE_DRIVER_INIT(thunt);
	DECLARE_DRIVER_INIT(critcrsh);
	DECLARE_DRIVER_INIT(stvmp);
	DECLARE_DRIVER_INIT(sasissu);
	DECLARE_DRIVER_INIT(dnmtdeka);
	DECLARE_DRIVER_INIT(ffreveng);
	DECLARE_DRIVER_INIT(fhboxers);
	DECLARE_DRIVER_INIT(pblbeach);
	DECLARE_DRIVER_INIT(sss);
	DECLARE_DRIVER_INIT(diehard);
	DECLARE_DRIVER_INIT(danchih);
	DECLARE_DRIVER_INIT(shienryu);
	DECLARE_DRIVER_INIT(elandore);
	DECLARE_DRIVER_INIT(prikura);
	DECLARE_DRIVER_INIT(maruchan);
	DECLARE_DRIVER_INIT(colmns97);
	DECLARE_DRIVER_INIT(grdforce);
	DECLARE_DRIVER_INIT(suikoenb);
	DECLARE_DRIVER_INIT(magzun);
	DECLARE_DRIVER_INIT(shanhigw);
	DECLARE_DRIVER_INIT(sokyugrt);
	DECLARE_DRIVER_INIT(vfremix);
	DECLARE_DRIVER_INIT(twcup98);
	DECLARE_DRIVER_INIT(znpwfv);
	DECLARE_DRIVER_INIT(othellos);
	DECLARE_DRIVER_INIT(mausuke);
	DECLARE_DRIVER_INIT(hopper);

	DECLARE_READ8_MEMBER(stv_ioga_r);
	DECLARE_WRITE8_MEMBER(stv_ioga_w);
	DECLARE_READ8_MEMBER(critcrsh_ioga_r);
	DECLARE_READ8_MEMBER(magzun_ioga_r);
	DECLARE_WRITE8_MEMBER(magzun_ioga_w);
	DECLARE_READ8_MEMBER(stvmp_ioga_r);
	DECLARE_WRITE8_MEMBER(stvmp_ioga_w);
	DECLARE_READ32_MEMBER(stv_ioga_r32);
	DECLARE_WRITE32_MEMBER(stv_ioga_w32);
	DECLARE_READ32_MEMBER(critcrsh_ioga_r32);
	DECLARE_READ32_MEMBER(stvmp_ioga_r32);
	DECLARE_WRITE32_MEMBER(stvmp_ioga_w32);
	DECLARE_READ32_MEMBER(magzun_ioga_r32);
	DECLARE_WRITE32_MEMBER(magzun_ioga_w32);
	DECLARE_READ32_MEMBER(magzun_hef_hack_r);
	DECLARE_READ32_MEMBER(magzun_rx_hack_r);
	DECLARE_WRITE8_MEMBER(hop_ioga_w);
	DECLARE_WRITE32_MEMBER(hop_ioga_w32);

	image_init_result load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart1 ) { return load_cart(image, m_cart1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart2 ) { return load_cart(image, m_cart2); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart3 ) { return load_cart(image, m_cart3); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart4 ) { return load_cart(image, m_cart4); }
	optional_device<generic_slot_device> m_cart1;
	optional_device<generic_slot_device> m_cart2;
	optional_device<generic_slot_device> m_cart3;
	optional_device<generic_slot_device> m_cart4;

	void install_stvbios_speedups( void );

	DECLARE_MACHINE_START(stv);
	DECLARE_MACHINE_RESET(stv);

	DECLARE_MACHINE_RESET(batmanfr);
	DECLARE_WRITE32_MEMBER(batmanfr_sound_comms_w);
	optional_device<acclaim_rax_device> m_rax;

	uint8_t     m_port_sel,m_mux_data;
	uint8_t     m_system_output;
	uint8_t     m_ioga_mode;
	uint8_t     m_ioga_portg;
	uint16_t    m_serial_tx;

	// protection specific variables and functions (see machine/stvprot.c)
	uint32_t m_abus_protenable;
	uint32_t m_abus_protkey;

	uint32_t m_a_bus[4];

	DECLARE_READ32_MEMBER( common_prot_r );
	DECLARE_WRITE32_MEMBER( common_prot_w );

	void install_common_protection();
	void stv_register_protection_savestates();

	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<sega_315_5881_crypt_device> m_cryptdevice;
	optional_device<sega_315_5838_comp_device> m_5838crypt;
	optional_device<ticket_dispenser_device> m_hopper;
	uint16_t crypt_read_callback(uint32_t addr);
	uint16_t crypt_read_callback_ch1(uint32_t addr);
	uint16_t crypt_read_callback_ch2(uint32_t addr);

	DECLARE_READ8_MEMBER(pdr1_input_r);
	DECLARE_READ8_MEMBER(pdr2_input_r);
	DECLARE_WRITE8_MEMBER(pdr1_output_w);
	DECLARE_WRITE8_MEMBER(pdr2_output_w);
	void stv_select_game(int gameno);
	uint8_t     m_prev_gamebank_select;
	void stv_slot(machine_config &config);
	void stv_cartslot(machine_config &config);
	void stv(machine_config &config);
	void hopper(machine_config &config);
	void batmanfr(machine_config &config);
	void stv_5838(machine_config &config);
	void stv_5881(machine_config &config);
	void stvcd(machine_config &config);
	void sound_mem(address_map &map);
	void stv_mem(address_map &map);
	void stvcd_mem(address_map &map);
};

class stvpc_state : public stv_state
{
public:
	using stv_state::stv_state;
	static constexpr feature_type unemulated_features() { return feature::CAMERA | feature::PRINTER; }
};

//#define MASTER_CLOCK_352 57272720
//#define MASTER_CLOCK_320 53693174
#define CEF_1   m_vdp1_regs[0x010/2]|=0x0002
#define CEF_0   m_vdp1_regs[0x010/2]&=~0x0002
#define BEF_1   m_vdp1_regs[0x010/2]|=0x0001
#define BEF_0   m_vdp1_regs[0x010/2]&=~0x0001
#define STV_VDP1_TVMR ((m_vdp1_regs[0x000/2])&0xffff)
#define STV_VDP1_VBE  ((STV_VDP1_TVMR & 0x0008) >> 3)
#define STV_VDP1_TVM  ((STV_VDP1_TVMR & 0x0007) >> 0)

GFXDECODE_EXTERN( stv );
