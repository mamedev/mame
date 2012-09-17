#include "sound/samples.h"
#include "sound/ay8910.h"

extern const ay8910_interface cclimber_ay8910_interface;
extern const samples_interface cclimber_samples_interface;
DECLARE_WRITE8_HANDLER( cclimber_sample_trigger_w );
DECLARE_WRITE8_HANDLER( cclimber_sample_rate_w );
DECLARE_WRITE8_HANDLER( cclimber_sample_volume_w );
