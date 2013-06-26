/*************************************************************************

    video/crt.h

    CRT video emulation for TX-0 and PDP-1

*************************************************************************/

#ifndef CRT_H_
#define CRT_H_


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CRT_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, CRT, 0) \
	MCFG_DEVICE_CONFIG(_interface)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct crt_interface
{
	int num_levels;
	int offset_x, offset_y;
	int width, height;
};


struct crt_point
{
	crt_point() :
		intensity(0),
		next(0) {}

	int intensity;      /* current intensity of the pixel */
						/* a node is not in the list when (intensity == -1) */
	int next;           /* index of next pixel in list */
};

// ======================> crt_device

class crt_device : public device_t
{
public:
	crt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~crt_device() { }

protected:
	// device-level overrides
	virtual void device_start();

public:
	void plot(int x, int y);
	void eof();
	void update(bitmap_ind16 &bitmap);

private:
	crt_point *m_list; /* array of (crt_window_width*crt_window_height) point */
	int *m_list_head;  /* head of the list of lit pixels (index in the array) */
						/* keep a separate list for each display line (makes the video code slightly faster) */

	int m_decay_counter;  /* incremented each frame (tells for how many frames the CRT has decayed between two screen refresh) */

	/* CRT window */
	int m_num_intensity_levels;
	int m_window_offset_x;
	int m_window_offset_y;
	int m_window_width;
	int m_window_height;
};

extern const device_type CRT;



#endif /* CRT_H_ */
