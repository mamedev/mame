// pthaiku.cpp - portable timer implementation for Haiku

#include "porttime.h"
#include <Looper.h>
#include <MessageRunner.h>
#include <OS.h>

namespace {
    const uint32 timerMessage = 'PTTM';

    struct TimerLooper : BLooper {
        TimerLooper() : BLooper() {
        }


        virtual void MessageReceived(BMessage *message)
        {
            PtCallback *callback;
            void *userData;
            if (message->what == timerMessage && message->FindPointer("callback", (void**)&callback) == B_OK && message->FindPointer("userData", &userData) == B_OK) {
                (*callback)(Pt_Time(), userData);
            }
            BLooper::MessageReceived(message);
        }
    };

    bool time_started_flag = false;
    bigtime_t time_offset;
    TimerLooper *timerLooper;
    BMessageRunner *timerRunner;
}

extern "C" {
    PtError Pt_Start(int resolution, PtCallback *callback, void *userData)
    {
        if (time_started_flag) return ptAlreadyStarted;
        time_offset = system_time();
        if (callback) {
            timerLooper = new TimerLooper;
            timerLooper->Run();
            BMessenger target(timerLooper);
		    BMessage message(timerMessage);
            message.AddPointer("callback", (void*)callback);
            message.AddPointer("userData", userData);
            bigtime_t interval = resolution * 1000;
            timerRunner = new BMessageRunner(target, &message, interval);
		    if(timerRunner->InitCheck() != B_OK) {
                delete timerRunner;
                timerRunner = NULL;
                timerLooper->PostMessage(B_QUIT_REQUESTED);
                timerLooper = NULL;
                return ptHostError;
            }
        }
        time_started_flag = true;
        return ptNoError;
    }


    PtError Pt_Stop()
    {
        if (!time_started_flag) return ptAlreadyStopped;
        time_started_flag = false;
        delete timerRunner;
        timerRunner = NULL;
        timerLooper->PostMessage(B_QUIT_REQUESTED);
        timerLooper = NULL;
        return ptNoError;
    }


    int Pt_Started()
    {
        return time_started_flag;
    }


    PtTimestamp Pt_Time()
    {
        return (system_time() - time_offset) / 1000;
    }


    void Pt_Sleep(int32_t duration)
    {
        snooze(duration * 1000);
    }
}
