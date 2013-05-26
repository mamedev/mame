//============================================================
//  TYPE DEFINITIONS
//============================================================

class monitor_info
{
public:
	monitor_info() { }

	virtual char *			device_name() = 0;

private:
	static monitor_info *	m_next;					// pointer to next monitor in list

	float					m_aspect;				// computed/configured aspect ratio of the physical device
	int                 	m_width;				// requested width for this monitor
	int                 	m_height;				// requested height for this monitor
};
