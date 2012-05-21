ADDRESS_MAP_EXTERN( buggychl_mcu_map, 8 );

WRITE8_DEVICE_HANDLER( buggychl_mcu_w );
READ8_DEVICE_HANDLER( buggychl_mcu_r );
READ8_DEVICE_HANDLER( buggychl_mcu_status_r );

DECLARE_LEGACY_DEVICE(BUGGYCHL_MCU, buggychl_mcu);
