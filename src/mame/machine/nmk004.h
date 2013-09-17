#include "sound/2203intf.h"
#include "sound/okim6295.h"

#define FM_CHANNELS          6
#define PSG_CHANNELS         3
#define EFFECTS_CHANNELS     8

struct psg_control
{
/* C220      */ UINT8  flags;
/* C221-C222 */ UINT16 note_timer;
/* C223-C224 */ UINT16 note_length;
/* C225      */ UINT8  volume_timer;
/* C227-C228 */ UINT16 current;     // current position in control table
/* C229-C22A */ UINT16 return_address[16];  // return address when control table calls a subtable
				int return_address_depth;
/* C22B-C22C */ UINT16 loop_start;  // first instruction of loop
/* C22D      */ UINT8  loop_times;  // number of times to loop
/* C22E      */ UINT8  volume_shape;
/* C22F      */ UINT8  volume_position;
/* C230      */ UINT8  octave;  // base octave
/* C231      */ UINT8  note;    // note to play
/* C233      */ UINT8  note_period_hi_bits;
};

struct fm_control
{
UINT8 note;
/* C020      */ UINT8  flags;
/* C021      */ UINT8  slot;    // for ym2203 keyon command
/* C022-C039 */ UINT8  voice_params[0x18];  // parameters for the YM2203 to configure sound shape
/* C03A-C03B */ UINT16 f_number;
/* C03C      */ UINT8  self_feedback;
/* C03D      */ UINT8  note_duration_table_select;
/* C03E-C03F */ UINT16 current; // current position in control table
/* C040-C041 */ UINT16 loop_start;  // first instruction of loop
/* C042      */ UINT8  loop_times;  // number of times to loop
/* C043-C044 */ UINT16 return_address[16];  // return address when control table calls a subtable
				int    return_address_depth;
/* C045      */ UINT8  octave;
/* C046-C047 */ UINT16 timer1;
/* C048-C049 */ UINT16 timer2;
/* C04A-C04B */ UINT16 timer1_duration;
/* C04C-C04D */ UINT16 timer2_duration;
/* C04E      */ UINT8  modulation_table_number;
/* C04F-C050 */ UINT16 modulation_timer;
/* C051-C052 */ UINT16 modulation_table;
/* C053-C054 */ UINT16 modulation_table_position;
/* C055-C056 */ UINT16 note_period;
/* C057-C05A */ UINT8  voice_volume[4]; // parameters for the YM2203 to configure sound shape
/* C05C      */ UINT8  must_update_voice_params;
};

struct effects_control
{
/* C1A0      */ UINT8  flags;
/* C1BE-C1BF */ UINT16 current; // current position in control table
/* C1C0-C1C1 */ UINT16 loop_start;  // first instruction of loop
/* C1C2      */ UINT8  loop_times;  // number of times to loop
/* C1C3-C1C4 */ UINT16 return_address[16];  // return address when control table calls a subtable
				int    return_address_depth;
/* C1C6-C1C7 */ UINT16 timer;
/* C1CA-C1CB */ UINT16 timer_duration;
};

class nmk004_device : public device_t
{
public:
	nmk004_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~nmk004_device() {}

	void ym2203_irq_handler(int irq);
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	const UINT8 *m_rom;   // NMK004 data ROM
	UINT8 m_from_main;    // command from main CPU
	UINT8 m_to_main;      // answer to main CPU
	int m_protection_check;

	ym2203_device *m_ymdevice;
	okim6295_device *m_oki1device;
	okim6295_device *m_oki2device;

	/* C001      */ UINT8 m_last_command;     // last command received
	/* C016      */ UINT8 m_oki_playing;      // bitmap of active Oki channels
	/* C020-C19F */ struct fm_control m_fm_control[FM_CHANNELS];
	/* C220-C2DF */ struct psg_control m_psg_control[PSG_CHANNELS];
	/* C1A0-C21F */ struct effects_control m_effects_control[EFFECTS_CHANNELS];

	TIMER_CALLBACK_MEMBER( real_init );
	UINT8 read8(int address);
	UINT16 read16(int address);
	void oki_play_sample(int sample_no);
	void oki_update_state(void);
	void effects_update(int channel);
	void fm_update(int channel);
	void fm_voices_update(void);
	void psg_update(int channel);
	void get_command(void);
	void update_music(void);
};

extern const device_type NMK004;

#define MCFG_NMK004_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NMK004, 0)
