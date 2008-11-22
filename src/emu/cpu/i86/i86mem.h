typedef struct _memory_interface memory_interface;
struct _memory_interface
{
	offs_t	fetch_xor;

	UINT8	(*rbyte)(const address_space *, offs_t);
	UINT16	(*rword)(const address_space *, offs_t);
	void	(*wbyte)(const address_space *, offs_t, UINT8);
	void	(*wword)(const address_space *, offs_t, UINT16);
};
