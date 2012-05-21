WRITE8_HANDLER( konami_sh_irqtrigger_w );
READ8_HANDLER( trackfld_sh_timer_r );
READ8_DEVICE_HANDLER( trackfld_speech_r );
WRITE8_DEVICE_HANDLER( trackfld_sound_w );
READ8_HANDLER( hyperspt_sh_timer_r );
WRITE8_DEVICE_HANDLER( hyperspt_sound_w );
WRITE8_HANDLER( konami_SN76496_latch_w );
WRITE8_DEVICE_HANDLER( konami_SN76496_w );

DECLARE_LEGACY_SOUND_DEVICE(TRACKFLD_AUDIO, trackfld_audio);
