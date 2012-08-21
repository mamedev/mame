/****************************************************************************

    TI-99 Speech Synthesizer
    See spchsyn.c for documentation

    Michael Zapf, October 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TISPEECH__
#define __TISPEECH__

#include "emu.h"
#include "peribox.h"
#include "sound/tms5220.h"

extern const device_type TI99_SPEECH;

class ti_speech_synthesizer_device : public ti_expansion_card_device
{
public:
	ti_speech_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	void crureadz(offs_t offset, UINT8 *value) { };
	void cruwrite(offs_t offset, UINT8 value) { };

	DECLARE_WRITE_LINE_MEMBER( speech_ready );

	DECLARE_READ8_MEMBER( spchrom_read );
	DECLARE_WRITE8_MEMBER( spchrom_load_address );
	DECLARE_WRITE8_MEMBER( spchrom_read_and_branch );

protected:
	void			device_start();
	void			device_reset(void);
	const rom_entry *device_rom_region() const;
	machine_config_constructor device_mconfig_additions() const;
	void			device_config_complete();

private:
	tmc0285n_device	*m_vsp;

	UINT8			*m_speechrom;			// pointer to speech ROM data
	int 			m_load_pointer;			// which 4-bit nibble will be affected by load address
	int 			m_rombits_count;		// current bit position in ROM
	UINT32			m_sprom_address;		// 18 bit pointer in ROM
	UINT32			m_sprom_length;			// length of data pointed by speechrom_data, from 0 to 2^18
};

#endif
