/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/thread.h>

bx::DefaultAllocator s_allocator;
bx::MpScUnboundedBlockingQueue<void> s_mpsc(&s_allocator);

int32_t threadExit0(bx::Thread* _thread, void*)
{
	_thread->pop();

	s_mpsc.push(reinterpret_cast<void*>(uintptr_t(0x1300) ) );

	return bx::kExitSuccess;
}

int32_t threadExit1(bx::Thread* _thread, void*)
{
	BX_UNUSED(_thread);

	s_mpsc.push(reinterpret_cast<void*>(uintptr_t(0x89) ) );

	return bx::kExitFailure;
}

TEST_CASE("Semaphore", "")
{
	bx::Semaphore sem;
	REQUIRE(!sem.wait(10) );

	sem.post(1);
	REQUIRE(sem.wait() );
}

TEST_CASE("Thread", "")
{
	bx::Thread th;

	REQUIRE(!th.isRunning() );

	th.init(threadExit0);
	REQUIRE(th.isRunning() );
	th.push(NULL);
	th.shutdown();

	REQUIRE(!th.isRunning() );
	REQUIRE(th.getExitCode() == 0);

	th.init(threadExit1);
	REQUIRE(th.isRunning() );
	th.shutdown();

	REQUIRE(!th.isRunning() );
	REQUIRE(th.getExitCode() == 1);
}

TEST_CASE("MpScUnboundedBlockingQueue", "")
{
	void* p0 = s_mpsc.pop();
	void* p1 = s_mpsc.pop();

	uintptr_t result = uintptr_t(p0) | uintptr_t(p1);

	REQUIRE(result == 0x1389);
}
