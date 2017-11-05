#include <glm/glm.hpp>

int test_int_size()
{
	return
		sizeof(glm::int_t) != sizeof(glm::lowp_int) &&
		sizeof(glm::int_t) != sizeof(glm::mediump_int) && 
		sizeof(glm::int_t) != sizeof(glm::highp_int);
}

int test_uint_size()
{
	return
		sizeof(glm::uint_t) != sizeof(glm::lowp_uint) &&
		sizeof(glm::uint_t) != sizeof(glm::mediump_uint) && 
		sizeof(glm::uint_t) != sizeof(glm::highp_uint);
}

int test_int_precision()
{
	return (
		sizeof(glm::lowp_int) <= sizeof(glm::mediump_int) && 
		sizeof(glm::mediump_int) <= sizeof(glm::highp_int)) ? 0 : 1;
}

int test_uint_precision()
{
	return (
		sizeof(glm::lowp_uint) <= sizeof(glm::mediump_uint) && 
		sizeof(glm::mediump_uint) <= sizeof(glm::highp_uint)) ? 0 : 1;
}

int main()
{
	int Error = 0;

	Error += test_int_size();
	Error += test_int_precision();
	Error += test_uint_size();
	Error += test_uint_precision();

	return Error;
}
