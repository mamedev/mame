// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Aaron Giles, R. Belmont, hap, Philip Bennett
/***************************************************************************

    Taito Ensoniq ES5505-based sound hardware

****************************************************************************/

#include "cpu/m68000/m68000.h"
#include "sound/es5506.h"
#include "machine/mc68681.h"
#include "machine/mb87078.h"

class taito_en_device : public device_t

{
public:
	taito_en_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~taito_en_device() {}

	DECLARE_READ8_MEMBER( en_68000_share_r );
	DECLARE_WRITE8_MEMBER( en_68000_share_w );
	DECLARE_WRITE16_MEMBER( en_es5505_bank_w );
	DECLARE_WRITE8_MEMBER( en_volume_w );

	//todo: hook up cpu/es5510
	DECLARE_READ16_MEMBER( es5510_dsp_r );
	DECLARE_WRITE16_MEMBER( es5510_dsp_w );

	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);

	DECLARE_WRITE8_MEMBER(mb87078_gain_changed);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// inherited devices/pointers
	required_device<cpu_device> m_audiocpu;
	required_device<es5505_device> m_ensoniq;
	required_device<mc68681_device> m_duart68681;
	required_device<mb87078_device> m_mb87078;
	required_shared_ptr<UINT32> m_snd_shared_ram;

	//todo: hook up cpu/es5510
	UINT16   m_es5510_dsp_ram[0x200];
	UINT32   m_es5510_gpr[0xc0];
	UINT32   m_es5510_dram[1<<24];
	UINT32   m_es5510_dol_latch;
	UINT32   m_es5510_dil_latch;
	UINT32   m_es5510_dadr_latch;
	UINT32   m_es5510_gpr_latch;
	UINT8    m_es5510_ram_sel;
};

extern const device_type TAITO_EN;

MACHINE_CONFIG_EXTERN( taito_en_sound );

#define MCFG_TAITO_EN_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TAITO_EN, 0)
