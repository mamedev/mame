#include "test.h"
#include <bx/os.h>

TEST(getProcessMemoryUsed)
{
	CHECK(0 != bx::getProcessMemoryUsed() );
//	DBG("bx::getProcessMemoryUsed %d", bx::getProcessMemoryUsed() );
}
