/*----------- defined in video/stadhero.c -----------*/

extern UINT16 *stadhero_pf1_data;
extern UINT16 *stadhero_pf2_control_0;
extern UINT16 *stadhero_pf2_control_1;

VIDEO_START( stadhero );
VIDEO_UPDATE( stadhero );

WRITE16_HANDLER( stadhero_pf1_data_w );
READ16_HANDLER( stadhero_pf2_data_r );
WRITE16_HANDLER( stadhero_pf2_data_w );
