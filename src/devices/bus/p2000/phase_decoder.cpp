// license:BSD-3-Clause
// copyright-holders:Erwin Jansen
#include "phase_decoder.h"

#include <assert.h>  // for assert
#include <bitset>    // for bitset
#include <cstdint>   // for uint8_t
#include <iterator>  // for end, begin

PhaseDecoder::PhaseDecoder(double tolerance)
: mTolerance(tolerance)
{
    reset();
}

// A clock that will be true when there is a read is possible.
// The read clock will return true if a bit can be read, and is
// a timed signal.
//
// Keep in mind that readClock can be high for quite a while!
// so you should check that it goes low before doing another read.
//
// If the clock runs on time T then the readclock should be high for
// - a minimum of 0,55T
// - ideal        0,75T
// - a maximum of 0,95T
bool PhaseDecoder::readClock()
{
    std::lock_guard<std::mutex> guard(mBitMutex);
    if (mBitQueue.empty())
        return false;

    // Note that this can be 0 when we read the first ever bit.
    TimedBit bit = mBitQueue.front();
    return mCurrentClock < (bit.second * 0.75);
}

// The current bit, only valid when readClock is true.
bool PhaseDecoder::readData()
{
    std::lock_guard<std::mutex> guard(mBitMutex);
    if (mBitQueue.empty())
        return false;

    return mBitQueue.front().first;
};

bool PhaseDecoder::pullBit()
{
    std::lock_guard<std::mutex> guard(mBitMutex);
    if (mBitQueue.empty())
    {
        return false;
    }
    auto bit = mBitQueue.front().first;
    mBitQueue.pop();
    return bit;
}

bool PhaseDecoder::signal(bool state, double delay)
{
    mCurrentClock += delay;
    if (state == mLastSignal)
    {
        if (mNeedSync == 0 && mCurrentClock > mClockPeriod
            && !withinTolerance(mCurrentClock, mClockPeriod))
        {
            // We might be at the last bit in a sequence, meaning we
			// are only getting the reference signale for a while.
            reset();
            return true;
        }
        return false;
    }

	// A transition happened!
    mLastSignal = state;
    if (mNeedSync > 0)
    {
        // We have not yet determined our clock period.
        return syncSignal(state);
    }

    // We are within bounds of the current clock
    if (withinTolerance(mCurrentClock, mClockPeriod))
    {
        addBit(state);
        return true;
    };

    // We went out of sync, our clock is wayyy out of bounds.
    if (mCurrentClock > mClockPeriod)
    {
        reset();
    }

    // We are likely halfway in our clock signal..
    return false;
};

// Reset the clock state, the system will now
// need to resynchronize.
void PhaseDecoder::reset()
{
    mLastSignal   = false;
    mLastTime     = {};
    mCurrentClock = {};
    mClockPeriod  = {};
    mNeedSync     = kSyncBits;
}

void PhaseDecoder::addBit(bool bit)
{
    std::lock_guard<std::mutex> guard(mBitMutex);
    mBitQueue.push(std::make_pair(bit, mClockPeriod));
    if (mBitQueue.size() > kQueueDelay)
    {
        mBitQueue.pop();
    }
    mCurrentClock = {};
}

bool PhaseDecoder::syncSignal(bool state)
{
    mNeedSync--;
    if (mNeedSync == kSyncBits - 1)
    {
        // First bit!
        if (state)
        {
            // We can only synchronize when we go up
            // on the first bit.
            addBit(true);
        }
        return false;
    }
    if (mNeedSync == kSyncBits - 2)
    {
        static_assert(kSyncBits >= 2, "Need at least 2 bits to synchronize!");
        // Update the clock period of the bit we just added to the queue.
        mClockPeriod            = mCurrentClock;
        mBitQueue.back().second = mClockPeriod;
    }
    if (!withinTolerance(mCurrentClock, mClockPeriod))
    {
        // Clock is way off!
        reset();
        return false;
    }

    // We've derived a clock period.
    auto div     = kSyncBits - mNeedSync;
    mClockPeriod = ((div - 1) * mClockPeriod + mCurrentClock) / div;
    addBit(state);
    return true;
}

// y * (1 - tolerance) < x < y * (1 + tolerance)
bool PhaseDecoder::withinTolerance(double x, double y)
{
    return (y * (1 - mTolerance)) < x && x < (y * (1 + mTolerance));
}

std::vector<uint8_t> PhaseDecoder::decode(const std::string& str, char high, char low)
{
    std::vector<uint8_t> readBytes;
    std::bitset<8>       currentByte;
    int                  bits = 0;
    for (auto ch : str)
    {
        if (ch != high && ch != low)  // Skip all non transitions.
            continue;

        bool transition = (ch == high);
        if (signal(transition, 1))
        {
            currentByte <<= 1;
            currentByte.set(0, pullBit());
            bits++;
            if (bits == 8)
            {
                readBytes.push_back(currentByte.to_ulong());
                bits = 0;
                currentByte.reset();
            }
        }
    }
    return readBytes;
}

static std::string encodeBit(bool current, bool next, int period, char high, char low)
{
    if (next != current)
    {
        return current ? std::string(period, high) : std::string(period, low);
    }
    int inPeriod = period / 2;
    return (current ? std::string(inPeriod, high) : std::string(inPeriod, low))
           + (current ? std::string(inPeriod, low) : std::string(inPeriod, high));
}

static std::string encodeInternal(const std::vector<uint8_t>& bytes,
                                  uint8_t                     period,
                                  char                        high,
                                  char                        low)
{
    assert((period & 0x01) == 0);  // Divisible by 2.
    std::string encoded   = "";
    auto        biterator = bytes.begin();
    for (int i = 0; !bytes.empty() && i < bytes.size() - 1; i++)
    {
        std::bitset<9> currentByte = *biterator;
        ++biterator;
        currentByte <<= 1;
        currentByte.set(0, (*biterator & 0x80));
        for (int i = 8; i > 0; i--)
        {
            encoded += encodeBit(currentByte.test(i), currentByte.test(i - 1), period, high, low);
        }
    }
    if (biterator != bytes.end())
    {
        std::bitset<9> currentByte = *biterator;
        currentByte <<= 1;
        currentByte.set(0, 0);
        for (int i = 8; i > 0; i--)
        {
            encoded += encodeBit(currentByte.test(i), currentByte.test(i - 1), period, high, low);
        }
    }
    return encoded;
}

std::string PhaseDecoder::encode(const std::vector<uint8_t>& bytes,
                                 uint8_t                     period,
                                 char                        high,
                                 char                        low)
{
    std::vector<uint8_t> header{ 0xAA };
    header.insert(std::end(header), std::begin(bytes), std::end(bytes));
    return std::string(period, low) + encodeInternal(header, period, high, low)
           + std::string(period, low);
}
