/* pmhaiku.cpp -- PortMidi os-dependent code */

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <MidiConsumer.h>
#include <MidiEndpoint.h>
#include <MidiProducer.h>
#include <MidiRoster.h>
#include <MidiSynth.h>
#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"

namespace {
    struct PmInputConsumer : BMidiLocalConsumer {
        PmInputConsumer(PmInternal *midi) :
            BMidiLocalConsumer("PortMidi input consumer"),
            midi(midi)
        {
        }


        void Data(uchar *data, size_t length, bool atomic, bigtime_t time)
        {
            if (!atomic)
                return; // should these be also supported?

            if (data[0] == B_SYS_EX_START) {
                pm_read_bytes(midi, data, length, time / 1000);
            } else {
                PmEvent event;
                switch (length) {
                case 1:
                    event.message = Pm_Message(data[0], 0, 0);
                    break;
                case 2:
                    event.message = Pm_Message(data[0], data[1], 0);
                    break;
                case 3:
                    event.message = Pm_Message(data[0], data[1], data[2]);
                    break;
                default:
                    printf("Unexpected message length for short message, got %" B_PRIuSIZE "\n", length);
                    break;
                }
                event.timestamp = time / 1000;
                pm_read_short(midi, &event);
            }
        }

    private:
        PmInternal *midi;
    };

    struct PmOutputInfo {
        BMidiLocalProducer *producer;
        std::vector<unsigned char> sysexBuffer;
    };

    struct PmSynthOutputInfo {
        BMidiSynth midiSynth;
        std::vector<unsigned char> sysexBuffer;
    };


    PmTimestamp synchronize(PmInternal *midi)
    {
        return 0;
    }


    PmError in_open(PmInternal *midi, void *driverInfo)
    {
        int32 endpointID = (int32)(intptr_t)pm_descriptors[midi->device_id].descriptor;
        BMidiProducer *producer = BMidiRoster::FindProducer(endpointID);
        if (!producer)
            return pmInvalidDeviceId;
        PmInputConsumer *consumer = new PmInputConsumer(midi);
        status_t status = producer->Connect(consumer);
        if (status != B_OK) {
            consumer->Release();
            producer->Release();
            strcpy(pm_hosterror_text, strerror(status));
            pm_hosterror = TRUE;
            return pmHostError;
        }
        midi->api_info = consumer;
        producer->Release();
        return pmNoError;
    }


    PmError in_abort(PmInternal *midi)
    {
        return pmNoError;
    }


    PmError in_close(PmInternal *midi)
    {
        int32 endpointID = (int32)(intptr_t)pm_descriptors[midi->device_id].descriptor;
        BMidiProducer *producer = BMidiRoster::FindProducer(endpointID);
        if (!producer)
            return pmInvalidDeviceId;
        PmInputConsumer *consumer = (PmInputConsumer*)midi->api_info;
        status_t status = producer->Disconnect(consumer);
        if (status != B_OK) {
            consumer->Release();
            producer->Release();
            strcpy(pm_hosterror_text, strerror(status));
            pm_hosterror = TRUE;
            return pmHostError;
        }
        consumer->Release();
        midi->api_info = NULL;
        producer->Release();
        return pmNoError;
    }


    PmError out_open(PmInternal *midi, void *driverInfo)
    {
        int32 endpointID = (int32)(intptr_t)pm_descriptors[midi->device_id].descriptor;
        BMidiConsumer *consumer = BMidiRoster::FindConsumer(endpointID);
        if (!consumer)
            return pmInvalidDeviceId;
        BMidiLocalProducer *producer = new BMidiLocalProducer("PortMidi output producer");
        status_t status = producer->Connect(consumer);
        if (status != B_OK) {
            consumer->Release();
            producer->Release();
            strcpy(pm_hosterror_text, strerror(status));
            pm_hosterror = TRUE;
            return pmHostError;
        }
        PmOutputInfo *info = new PmOutputInfo;
        info->producer = producer;
        midi->api_info = info;
        consumer->Release();
        return pmNoError;
    }


    PmError out_abort(PmInternal *midi)
    {
        return pmNoError;
    }


    PmError out_close(PmInternal *midi)
    {
        int32 endpointID = (int32)(intptr_t)pm_descriptors[midi->device_id].descriptor;
        BMidiConsumer *consumer = BMidiRoster::FindConsumer(endpointID);
        if (!consumer)
            return pmInvalidDeviceId;
        PmOutputInfo *info = (PmOutputInfo*)midi->api_info;
        status_t status = info->producer->Disconnect(consumer);
        if (status != B_OK) {
            consumer->Release();
            midi->api_info = NULL;
            info->producer->Release();
            delete info;
            strcpy(pm_hosterror_text, strerror(status));
            pm_hosterror = TRUE;
            return pmHostError;
        }
        consumer->Release();
        midi->api_info = NULL;
        info->producer->Release();
        delete info;
        return pmNoError;
    }


    PmError write_short(PmInternal *midi, PmEvent *buffer)
    {
        PmOutputInfo *info = (PmOutputInfo*)midi->api_info;
        uchar data[3];
        data[0] = Pm_MessageStatus(buffer->message);
        data[1] = Pm_MessageData1(buffer->message);
        data[2] = Pm_MessageData2(buffer->message);
        size_t length = pm_midi_length(data[0]);

        info->producer->SprayData(data, length, true, buffer->timestamp * 1000);

        // TODO: handle latency != 0
        return pmNoError;
    }


    PmError begin_sysex(PmInternal *midi, PmTimestamp timestamp)
    {
        PmOutputInfo *info = (PmOutputInfo*)midi->api_info;
        info->sysexBuffer.clear();
        return pmNoError;
    }


    PmError end_sysex(PmInternal *midi, PmTimestamp timestamp)
    {
        PmOutputInfo *info = (PmOutputInfo*)midi->api_info;
        info->producer->SpraySystemExclusive(&info->sysexBuffer[0], info->sysexBuffer.size(), timestamp * 1000);
        info->sysexBuffer.clear();
        return pmNoError;
    }


    PmError write_byte(PmInternal *midi, unsigned char byte, PmTimestamp timestamp)
    {
        PmOutputInfo *info = (PmOutputInfo*)midi->api_info;
        info->sysexBuffer.push_back(byte);
        return pmNoError;
    }


    PmError write_realtime(PmInternal *midi, PmEvent *buffer)
    {
        PmOutputInfo *info = (PmOutputInfo*)midi->api_info;
        info->producer->SpraySystemRealTime(Pm_MessageStatus(buffer->message), buffer->timestamp * 1000);
        return pmNoError;
    }


    PmError synth_open(PmInternal *midi, void *driverInfo)
    {
        PmSynthOutputInfo *info = new PmSynthOutputInfo;
        info->midiSynth.EnableInput(true, true);
        midi->api_info = info;
        return pmNoError;
    }


    PmError synth_abort(PmInternal *midi)
    {
        return pmNoError;
    }


    PmError synth_close(PmInternal *midi)
    {
        PmSynthOutputInfo *info = (PmSynthOutputInfo*)midi->api_info;
        delete info;
        midi->api_info = NULL;
        return pmNoError;
    }


    PmError write_short_synth(PmInternal *midi, PmEvent *buffer)
    {
        PmSynthOutputInfo *info = (PmSynthOutputInfo*)midi->api_info;
        uchar data[3];
        data[0] = Pm_MessageStatus(buffer->message);
        data[1] = Pm_MessageData1(buffer->message);
        data[2] = Pm_MessageData2(buffer->message);

        switch(data[0] & 0xf0) {
        case B_NOTE_OFF:
            info->midiSynth.NoteOff((data[0] & 0x0f) + 1, data[1], data[2], buffer->timestamp);
            break;
        case B_NOTE_ON:
            info->midiSynth.NoteOn((data[0] & 0x0f) + 1, data[1], data[2], buffer->timestamp);
            break;
        case B_KEY_PRESSURE:
            info->midiSynth.KeyPressure((data[0] & 0x0f + 1), data[1], data[2], buffer->timestamp);
            break;
        case B_CONTROL_CHANGE:
            info->midiSynth.ControlChange((data[0] & 0x0f) + 1, data[1], data[2], buffer->timestamp);
            break;
        case B_PROGRAM_CHANGE:
            info->midiSynth.ProgramChange((data[0] & 0x0f) + 1, data[1], buffer->timestamp);
            break;
        case B_CHANNEL_PRESSURE:
            info->midiSynth.ChannelPressure((data[0] & 0x0f) + 1, data[1], buffer->timestamp);
            break;
        case B_PITCH_BEND:
            info->midiSynth.PitchBend((data[0] & 0x0f) + 1, data[1], data[2], buffer->timestamp);
            break;
        }

        // TODO: handle latency != 0
        return pmNoError;
    }


    PmError begin_sysex_synth(PmInternal *midi, PmTimestamp timestamp)
    {
        PmSynthOutputInfo *info = (PmSynthOutputInfo*)midi->api_info;
        info->sysexBuffer.clear();
        return pmNoError;
    }


    PmError end_sysex_synth(PmInternal *midi, PmTimestamp timestamp)
    {
        PmSynthOutputInfo *info = (PmSynthOutputInfo*)midi->api_info;
        info->midiSynth.SystemExclusive(&info->sysexBuffer[0], info->sysexBuffer.size(), timestamp);
        info->sysexBuffer.clear();
        return pmNoError;
    }


    PmError write_byte_synth(PmInternal *midi, unsigned char byte, PmTimestamp timestamp)
    {
        PmSynthOutputInfo *info = (PmSynthOutputInfo*)midi->api_info;
        info->sysexBuffer.push_back(byte);
        return pmNoError;
    }


    PmError write_realtime_synth(PmInternal *midi, PmEvent *buffer)
    {
        PmSynthOutputInfo *info = (PmSynthOutputInfo*)midi->api_info;
        info->midiSynth.SystemRealTime(Pm_MessageStatus(buffer->message), buffer->timestamp);
        return pmNoError;
    }


    PmError write_flush(PmInternal *midi, PmTimestamp timestamp)
    {
        return pmNoError;
    }


    unsigned int check_host_error(PmInternal *midi)
    {
        return 0;
    }


    pm_fns_node pm_in_dictionary = {
        none_write_short,
        none_sysex,
        none_sysex,
        none_write_byte,
        none_write_short,
        none_write_flush,
        synchronize,
        in_open,
        in_abort,
        in_close,
        success_poll,
        check_host_error
    };

    pm_fns_node pm_out_dictionary = {
        write_short,
        begin_sysex,
        end_sysex,
        write_byte,
        write_realtime,
        write_flush,
        synchronize,
        out_open,
        out_abort,
        out_close,
        none_poll,
        check_host_error
    };


    pm_fns_node pm_synth_dictionary = {
        write_short_synth,
        begin_sysex_synth,
        end_sysex_synth,
        write_byte_synth,
        write_realtime_synth,
        write_flush,
        synchronize,
        synth_open,
        synth_abort,
        synth_close,
        none_poll,
        check_host_error
    };


    PmError create_virtual(int is_input, const char *name, void *driverInfo)
    {
        BMidiEndpoint *endpoint = is_input ? (BMidiEndpoint*)new BMidiLocalProducer(name) : new BMidiLocalConsumer(name);
        if (!endpoint->IsValid()) {
            endpoint->Release();
            strcpy(pm_hosterror_text, "Endpoint could not be created");
            pm_hosterror = TRUE;
            return pmHostError;
        }
        status_t status = endpoint->Register();
        if (status != B_OK) {
            endpoint->Release();
            strcpy(pm_hosterror_text, strerror(status));
            pm_hosterror = TRUE;
            return pmHostError;
        }
        return pm_add_device(const_cast<char*>("Haiku MIDI kit"), name, is_input, TRUE, (void*)(intptr_t)endpoint->ID(), is_input ? &pm_in_dictionary : &pm_out_dictionary);
    }

    PmError delete_virtual(PmDeviceID id)
    {
        int32 endpointID = (int32)(intptr_t)pm_descriptors[id].descriptor;
        BMidiEndpoint *endpoint = BMidiRoster::FindEndpoint(endpointID);
        //TODO: handle connected producers and consumers
        status_t status = endpoint->Unregister();
        // release twice to actually free the endpoint (FindEndpoint increases the ref-count)
        endpoint->Release();
        endpoint->Release();
        if (status != B_OK) {
            strcpy(pm_hosterror_text, strerror(status));
            pm_hosterror = TRUE;
            return pmHostError;
        }
        return pmNoError;
    }
}

extern "C" {
    void pm_init()
    {
        pm_add_interf(const_cast<char*>("Haiku MIDI kit"), create_virtual, delete_virtual);

        pm_add_device(const_cast<char*>("Haiku MIDI kit"), "Soft Synth", FALSE, FALSE, NULL, &pm_synth_dictionary);

        int32 id = 0;
        BMidiEndpoint *endpoint;

        while ((endpoint = BMidiRoster::NextEndpoint(&id)) != NULL) {
            bool isInput = endpoint->IsProducer();
            pm_add_device(const_cast<char*>("Haiku MIDI kit"), endpoint->Name(), isInput, FALSE, (void*)(intptr_t)id, isInput ? &pm_in_dictionary : &pm_out_dictionary);
            endpoint->Release();
        }
    }


    void pm_term()
    {
        int i;
        for (i = 0; i < pm_descriptor_len; i++) {
            PmInternal *midi = pm_descriptors[i].pm_internal;
            if (midi && midi->api_info) {
                // device is still open, close it
                (*midi->dictionary->close)(midi);
            }
            if (pm_descriptors[i].pub.is_virtual && !pm_descriptors[i].deleted) {
                delete_virtual(i);
            }
        }
    }


    PmDeviceID Pm_GetDefaultInputDeviceID()
    {
        Pm_Initialize();
        return pm_default_input_device_id;
    }


    PmDeviceID Pm_GetDefaultOutputDeviceID()
    {
        Pm_Initialize();
        return pm_default_output_device_id;
    }


    void *pm_alloc(size_t s)
    {
        return malloc(s); 
    }


    void pm_free(void *ptr)
    {
         free(ptr); 
    }
}
