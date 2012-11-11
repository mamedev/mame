#include "machine/i8255.h"
#include "includes/galaxold.h"

class scramble_state : public galaxold_state
{
public:
	scramble_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxold_state(mconfig, type, tag),
		  m_ppi8255_0(*this, "ppi8255_0"),
		  m_ppi8255_1(*this, "ppi8255_1"),
		  m_soundram(*this, "soundram")
	{ }

	optional_device<i8255_device>  m_ppi8255_0;
	optional_device<i8255_device>  m_ppi8255_1;
	optional_shared_ptr<UINT8> m_soundram;

	UINT8 m_cavelon_bank;
	UINT8 m_security_2B_counter;
	UINT8 m_xb;

	// harem
	UINT8 m_harem_decrypt_mode;
	UINT8 m_harem_decrypt_bit;
	UINT8 m_harem_decrypt_clk;
	UINT8 m_harem_decrypt_count;
	UINT8 *m_harem_decrypted_data;
	UINT8 *m_harem_decrypted_opcodes;

	DECLARE_CUSTOM_INPUT_MEMBER(darkplnt_custom_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ckongs_coinage_r);
	DECLARE_READ8_MEMBER(hncholms_prot_r);
	DECLARE_READ8_MEMBER(scramble_soundram_r);
	DECLARE_READ8_MEMBER(mars_ppi8255_0_r);
	DECLARE_READ8_MEMBER(mars_ppi8255_1_r);
	DECLARE_WRITE8_MEMBER(scramble_soundram_w);
	DECLARE_WRITE8_MEMBER(hotshock_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(scramble_filter_w);
	DECLARE_WRITE8_MEMBER(frogger_filter_w);
	DECLARE_WRITE8_MEMBER(mars_ppi8255_0_w);
	DECLARE_WRITE8_MEMBER(mars_ppi8255_1_w);

	DECLARE_WRITE8_MEMBER(harem_decrypt_bit_w);
	DECLARE_WRITE8_MEMBER(harem_decrypt_clk_w);
	DECLARE_WRITE8_MEMBER(harem_decrypt_rst_w);

	DECLARE_DRIVER_INIT(cavelon);
	DECLARE_DRIVER_INIT(mariner);
	DECLARE_DRIVER_INIT(mrkougb);
	DECLARE_DRIVER_INIT(scramble_ppi);
	DECLARE_DRIVER_INIT(mars);
	DECLARE_DRIVER_INIT(ckongs);
	DECLARE_DRIVER_INIT(mimonscr);
	DECLARE_DRIVER_INIT(hotshock);
	DECLARE_DRIVER_INIT(ad2083);
	DECLARE_DRIVER_INIT(devilfsh);
	DECLARE_DRIVER_INIT(mrkougar);
	DECLARE_DRIVER_INIT(harem);

	DECLARE_DRIVER_INIT(scobra);
	DECLARE_DRIVER_INIT(stratgyx);
	DECLARE_DRIVER_INIT(tazmani2);
	DECLARE_DRIVER_INIT(darkplnt);
	DECLARE_DRIVER_INIT(mimonkey);
	DECLARE_DRIVER_INIT(mimonsco);
	DECLARE_DRIVER_INIT(rescue);
	DECLARE_DRIVER_INIT(minefld);
	DECLARE_DRIVER_INIT(hustler);
	DECLARE_DRIVER_INIT(hustlerd);
	DECLARE_DRIVER_INIT(billiard);
	DECLARE_MACHINE_RESET(scramble);
	DECLARE_MACHINE_RESET(explorer);
	DECLARE_WRITE8_MEMBER(scramble_protection_w);
	DECLARE_READ8_MEMBER(scramble_protection_r);
	DECLARE_WRITE_LINE_MEMBER(scramble_sh_7474_q_callback);
};


/*----------- defined in machine/scramble.c -----------*/

extern const i8255_interface(scramble_ppi_0_intf);
extern const i8255_interface(scramble_ppi_1_intf);
extern const i8255_interface(stratgyx_ppi_1_intf);

DECLARE_READ8_HANDLER( triplep_pip_r );
DECLARE_READ8_HANDLER( triplep_pap_r );

DECLARE_READ8_HANDLER( hunchbks_mirror_r );
DECLARE_WRITE8_HANDLER( hunchbks_mirror_w );



/*----------- defined in audio/scramble.c -----------*/

void scramble_sh_init(running_machine &machine);


DECLARE_READ8_DEVICE_HANDLER( scramble_portB_r );
DECLARE_READ8_DEVICE_HANDLER( frogger_portB_r );

DECLARE_READ8_DEVICE_HANDLER( hotshock_soundlatch_r );

DECLARE_WRITE8_DEVICE_HANDLER( scramble_sh_irqtrigger_w );
DECLARE_WRITE8_DEVICE_HANDLER( mrkougar_sh_irqtrigger_w );

DECLARE_WRITE8_DEVICE_HANDLER( harem_portA_w );
DECLARE_WRITE8_DEVICE_HANDLER( harem_portB_w );

MACHINE_CONFIG_EXTERN( ad2083_audio );

