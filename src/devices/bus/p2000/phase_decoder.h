// license:BSD-3-Clause
// copyright-holders:Erwin Jansen
#include <stdint.h>  // for uint8_t
#include <mutex>     // for mutex
#include <queue>     // for queue
#include <string>    // for string
#include <utility>   // for pair
#include <vector>    // for vector

// A phase decoder is capable of converting a signal stream into a
// a series of bits that go together with a clock signal. This phase
// decoder is conform to what you would find in an Philips MDCR220
//
// Signals are converted into bits whenever the line signal
// changes from low to high and vice versa on a clock signal.
//
// A transition on a clock boundary from low to high is a 1.
// A transition on a clock boundary from high to low is a 0
// An intermediate transition halfway between the clock boundary
// can occur when there are consecutive 0s or 1s. See the example
// below where the clock is marked by a |
//
//
//          1    0    1    1    0    0
//   RDA:  _|----|____|--__|----|__--|__--
//   RDC:  _|-___|-___|-___|-___|-___|-___
//          ^                      ^
//          |-- clock signal       |-- intermediate transition.
//
// This particular phase decoder expects a signal of
// 1010 1010 which is used to derive the clock T.
class PhaseDecoder
{
    using TimeSecond = double;
    using TimedBit   = std::pair<bool, TimeSecond>;
    using TimedQueue = std::queue<TimedBit>;

public:
    PhaseDecoder(double tolerance = 0.15);

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
    bool readClock();

    // The current bit, only valid when readClock is true.
    bool readData();

	// Pulls the bit out of the queue.
    bool pullBit();

    // The current derived clock period.
    TimeSecond clockPeriod() { return mClockPeriod; }

    // Returns true if a new bit can be read (i.e. readClock() == true)
    bool signal(bool state, double delay);

    // Reset the clock state, the system will now
    // need to resynchronize.
    void reset();

    std::vector<uint8_t> decode(const std::string& signal,
                                char               high = kDefaultHigh,
                                char               low  = kDefaultLow);
    static std::string   encode(const std::vector<uint8_t>& bytes,
                                uint8_t                     period,
                                char                        high = kDefaultHigh,
                                char                        low  = kDefaultLow);

    static constexpr char kDefaultHigh = '-';
    static constexpr char kDefaultLow  = '_';

private:
    // add a bit and reset the current clock.
    void addBit(bool bit);

    // tries to sync up the signal and calculate the clockperiod.
    bool syncSignal(bool state);

    // y * (1 - tolerance) < x < y * (1 + tolerance)
    bool withinTolerance(double x, double y);

    bool       mLastSignal;
    double     mTolerance;
    int        mNeedSync;
    TimedQueue mBitQueue;
    TimeSecond mLastTime;
    TimeSecond mCurrentClock;
    TimeSecond mClockPeriod;
    std::mutex mBitMutex;

    static constexpr int kSyncBits   = 7;
    static constexpr int kQueueDelay = 2;
};
