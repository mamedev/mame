README.txt - for pm_test directory

These are all test programs for PortMidi

Because device numbers depend on the system, there is no automated
script to run all tests on PortMidi.

To run the full set of tests manually:

Note: everything is run from the ../Debug or ../Release directory.
Actual or example input is marked with >>, e.g., >>0 means type 0<ENTER>
Comments are shown in square brackets [like this]

1. ./qtest -- output should show a bunch of tests and no error message.

2. ./testio [test input]
Latency in ms: >>0
enter your choice... >>1
Type input number: >>6  [pick a working input device]
[play some notes, look for note-on (0x90) with pitch and velocity data]

3. ./testio [test input (fail w/assert)]
Latency in ms: >>0
enter your choice... >>2
Type input number: >>6  [pick a working input device]
[play some notes, program will abort after 5 messages
(this test only applies to a Debug build, otherwise
the assert() macro is disabled.)]

4. ./testio [test input (fail w/NULL assign)]
Latency in ms: >>0
enter your choice... >>3
Type input number: >>6  [pick a working input device]
[play some notes, program will Segmentation fault after 5 messages
(this test may not Segfault in the Release build; if not
try testing with a Debug build.)]

5. ./testio [test output, no latency]
Latency in ms: >>0
enter your choice... >>4
Type output number: >>2  [pick a working output device]
>> [type ENTER when prompted (7 times)]
[hear note on, note off, note on, note off, chord]

6. ./testio [test output, latency > 0]
Latency in ms: >>300
enter your choice... >>4
Type output number: >>2  [pick a working output device]
>> [type ENTER when prompted (7 times)]
[hear note on, note off, note on, note off, arpeggiated chord
 (delay of 300ms should be apparent)]

7. ./testio [for both, no latency]
Latency in ms: >>0
enter your choice... >>5
Type input number: >>6  [pick a working input device]
Type output number: >>2  [pick a working output device]
[play notes on input, hear them on output]

8. ./testio [for both, latency > 0]
Latency in ms: >>300
enter your choice... >>5
Type input number: >>6  [pick a working input device]
Type output number: >>2  [pick a working output device]
[play notes on input, hear them on output (delay of 300ms is apparent)]

9. ./testio [stream test]
Latency in ms: >>0 [does not matter]
enter your choice... >>6
Type output number: >>2  [pick a working output device]
>> [type ENTER to start]
[hear 4 notes: C D E F# at one note per second, then all turn off]
ready to close and terminate... (type ENTER) :>> [type ENTER (twice)]

10. ./testio [isochronous out]
Latency in ms: >>300
enter your choice... >>7
Type output number: >>2  [pick a working output device]
ready to send program 1 change... (type ENTER): >> [type ENTER]
[hear 80 notes, exactly 4 notes per second, no jitter]

11. ./latency [no MIDI, histogram]
Choose timer period (in ms, >= 1): >>1
? >>1 [No MIDI traffic option]
[wait about 10 seconds]
>> [type ENTER]
[output should be something like ... Maximum latency: 1 milliseconds]

12. ./latency [MIDI input, histogram]
Choose timer period (in ms, >= 1): >>1
? >>2 [MIDI input option]
Midi input device number: >>6  [pick a working input device]
[wait about 5 seconds, play input for 10 seconds ]
>> [type ENTER]
[output should be something like ... Maximum latency: 3 milliseconds]

13. ./latency [MIDI output, histogram]
Choose timer period (in ms, >= 1): >>1
? >>3 [MIDI output option]
Midi output device number: >>2  [pick a working output device]
Midi output should be sent every __ callback iterations: >>50
[wait until you hear notes for 5 or 10 seconds]
>> [type ENTER to stop]
[output should be something like ... Maximum latency: 2 milliseconds]

14. ./latency [MIDI input and output, histogram]
Choose timer period (in ms, >= 1): >>1
? >>4 [MIDI input and output option]
Midi input device number: >>6  [pick a working input device]
Midi output device number: >>2  [pick a working output device]
Midi output should be sent every __ callback iterations: >>50
[wait until you hear notes, simultaneously play notes for 5 or 10 seconds]
>> [type ENTER to stop]
[output should be something like ... Maximum latency: 1 milliseconds]

15. ./mm [test with device input]
Type input device number: >>6  [pick a working input device]
[play some notes, see notes printed]
>>q [Type q ENTER when finished to exit]

16. ./midithread -i 6 -o 2 [use working input/output device numbers]
>>5  [enter a transposition number]
[play some notes, hear parallel 4ths]
>>q [quit after ENTER a couple of times]

17. ./midiclock         [in one shell]
    ./mm                [in another shell]
[Goal is send clock messages to MIDI monitor program. This requires
 either a hardware loopback (MIDI cable from OUT to IN on interface)
 or a software loopback (macOS IAC bus or ALSA MIDI Through Port)]
[For midiclock application:]
    Type output device number: >>0  [pick a device with loopback]
    Type ENTER to start MIDI CLOCK: >> [type ENTER]
[For mm application:]
    Type input device number: >>1 [pick device with loopback]
    [Wait a few seconds]
    >>s  [to get Clock Count]
    >>s  [expect to get a higher Clock Count]
[For midiclock application:]
    >>c  [turn off clocks]
[For mm application:]
    >>s  [to get Clock Count]
    >>s  [expect to Clock Count stays the same]
[For midiclock application:]
    >>t  [turn on time code, see Time Code Quarter Frame messages from mm]
    >>q  [to quit]
[For mm application:]
    >>q  [to quit]

18. ./midithru -i 6 -o 2 [use working input/output device numbers]
[Play notes on input evice; notes are sent immediately and also with a
 2 sec delay to the output device; program terminates in 60 seconds or
 when you play B3 (B below Middle C)]
>> [ENTER to exit]

19. ./recvvirtual -h [in one shell, macOS and Linux only]
    ./recvvirtual -m vvv [for mac, or -c vvv -p vvvport for linux]
    ./testio [in another shell]
[For testio application:]
    Latency in ms: >>0 
    enter your choice... >>4 [test output]
    Type output number: >>9 [select the "portmidi (output)" device]
    [type ENTER to each prompt, see that recvvirtual "Got message 0"
     through "Got message 9"]
    >> [ENTER to quit]
[For recvvirtual application:]
    >> [ENTER to quit]

20. ./sendvirtual -h [in one shell, macOS and Linux only]
    ./sendvirtual -m vvv [for mac, or -c vvv -p vvvport for linux]
    ./mm [in another shell]
[For mm application:]
    Type input device number: >>10 [select the "portmidi" device]
[For sendvirtual application:]
    Type ENTER to send messages: >> [type ENTER]
    [see NoteOn and off messages received by mm for Key 60-64]
    >> [ENTER to quit]
[For mm application:]
    >>q [and ENTER twice to quit]

21. ./sysex [no latency]
[This requires either a hardware loopback (MIDI cable from OUT to IN
 on interface) or a software loopback (macOS IAC bus or ALSA MIDI
 Through Port)]
>>l [for loopback test]
Type output device number: >>0 [pick output device to loopback]
Latency in milliseconds: >>0
Type input  device number: >>0 [pick input device for loopback]
[Program will send 100,000 bytes. After awhile, program will quit.
 You can read the Cummulative bytes/sec value.]

22. ./sysex [latency > 0]
[This requires either a hardware loopback (MIDI cable from OUT to IN
 on interface) or a software loopback (macOS IAC bus or ALSA MIDI
 Through Port)]
>>l [for loopback test]
Type output device number: >>0 [pick output device to loopback]
Latency in milliseconds: >>100
Type input  device number: >>0 [pick input device for loopback]
[Program will send 100,000 bytes. After awhile, program will quit. You
 can read the Cummulative bytes/sec value; it is affected by latency.]

23. ./fast [no latency]
    ./fastrcv [in another shell]
[This is a speed check, especially for macOSX IAC bus connections,
 which are known to drop messages if you send messages too fast.
 fast and fastrcv must use a loopback to function.]
[In fastrcv:]
    Input device number: >>1 [pick a non-hardware device if possible]
[In fast:]
    Latency in ms: >>0
    Rate in messages per second: >>10000
    Duration in seconds: >>10
    Output device number: >>0 [pick a non-hardware device if possible]
    sending output...
[see message counts and times; on Linux you should get about 10000 
 messages/s; on macOS you should get about 1800 messages/s; Windows 
 does not have software ports, so data rate might be limited by the 
 loopback device you use.]

Check output of fastrcv: there should be no errors, just msg/sec.]
 
24. ./fast [latency > 0]
    ./fastrcv [in another shell]
[This is a speed check, especially for macOSX IAC bus connections, 
 which are known to drop messages if you send messages too fast. 
 fast and fastrcv must use a loopback to function.]
[In fastrcv:]
    Input device number: >>1 [pick a non-hardware device if possible]
[In fast:]
    Latency in ms: >>30 [Note for ALSA, use latency * msgs/ms < 400]
    Rate in messages per second: >>10000 
    Duration in seconds: >>10 
    Output device number: >>0 [pick a non-hardware device if possible]
    sending output... 
[see message counts and times; on Linux you should get about 10000 
 messages/s; on macOS you should get about 1800 messages/s; Windows 
 does not have software ports, so data rate might be limited by the 
 loopback device you use.]

Check output of fastrcv: there should be no errors, just msg/sec.]

25. ./fast [virtual output port, latency = 0, macOS and Linux only]
    ./fastrcv [in another shell]
[Start fast first:]
    Latency in ms: >>0
    Rate in messages per second: >>10000 
    Duration in seconds: >>10 
    Output device number: >>9 [enter number listed for "Create virtual
                               port named 'fast' (output)"]
    Pausing so you can connect a receiver to the newly created
        "fast" port. Type ENTER to proceed: 
[In fastrcv:]
    Input device number: >>3 [pick the device named "fast (input)"]
[In fast:]
    >>  [type ENTER to start]
[see message counts and times as above ]

Check output of fastrcv: there should be no errors, just msg/sec.]

26. ./fast [virtual output port, latency > 0, macOS and Linux only]
    ./fastrcv [in another shell]
[Start fast first:]
    Latency in ms: >>30 [Note for ALSA, use latency * msgs/ms < 400]
    Rate in messages per second: >>10000 
    Duration in seconds: >>10 
    Output device number: >>9 [enter number listed for "Create virtual
                               port named 'fast' (output)"]
    Pausing so you can connect a receiver to the newly created
        "fast" port. Type ENTER to proceed: 
[In fastrcv:]
    Input device number: >>3 [pick the device named "fast (input)"]
[In fast:]
    >>  [type ENTER to start]
[see message counts and times as above ]

Check output of fastrcv: there should be no errors, just msg/sec.]

27. ./fast [latency = 0, macOS and Linux only]
    ./fastrcv [virtual input port, in another shell]
[In fastrcv:]
    Input device number: >>8 [enter number listed for "Create virtual 
                              port named 'fastrcv' (input)"]
[In fast:]
    Latency in ms: >>0
    Rate in messages per second: >>10000 
    Duration in seconds: >>10 
    Output device number: >>7 [pick the device named "fastrcv (output)"]
    sending output... 
[see message counts and times as above ]

Check output of fastrcv: there should be no errors, just msg/sec.]

28. ./fast [latency > 0, macOS and Linux only]
    ./fastrcv [virtual input port, in another shell]
[In fastrcv:]
    Input device number: >>8 [enter number listed for "Create virtual 
                              port named 'fastrcv' (input)"]
[In fast:]
    Latency in ms: >>30 [Note for ALSA, use latency * msgs/ms < 400]
    Rate in messages per second: >>10000 
    Duration in seconds: >>10 
    Output device number: >>7 [pick the device named "fastrcv (output)"]
    sending output... 
[see message counts and times as above ]

Check output of fastrcv: there should be no errors, just msg/sec.]

29. ./midithru -v -n [virtual input and output, macOS and Linux only]
    ./fast [latency = 0]
    ./fastrcv [in another shell]
[Start midithru first, it will run for 60 seconds]
[In fastrcv:]
    Input device number: >>3 [pick the device named 
                              port named "midithru (input)"]
[In fast:]
    Latency in ms: >>0
    Rate in messages per second: >>10000 
    Duration in seconds: >>10 
    Output device number: >>8 [pick the device named "midithru (output)"]
    sending output... 
[see message counts and times as above, on Mac, output from fast to
 midithru AND output from midithru to fastrcv are rate limited, so
 as in other tests, it will take more than 10s to receive all the
 messages and the receiving message rate will be about 1800 messages/second]

30. ./multivirtual [macOS and Linux only]
    ./testio
    ./testio
[Start multivirtual first]
[In first testio:]
    Latency in ms: >>0 
    enter your choice... >>5 [test both]
    Type input number: >>1  [pick portmidi1 (input) 
    Type output number: >>4  [pick portmidi1 (output) 
[In second testio:]
    Latency in ms: >>10
    enter your choice... >>5 [test both]
    Type input number: >>2  [pick portmidi2 (input) 
    Type output number: >>5  [pick portmidi2 (output) 
[In multivirtual:]
    Type ENTER to send messages: >>  [type ENTER to start]
[see that each testio gets 11 messages (0 to 10) at reasonable times
 (e.g. 2077 to 7580, and the "@" times (real times) should match the
 timestamps). multivirtual should also report reasonable times and
 line near the end of output should be "Got 11 messages from
 portmidi1 and 11 from portmidi2; expected 11."]

31. ./multivirtual [macOS and Linux only]
    ./multivirtual
[Second instance should report "PortMidi call failed...
   PortMidi: Cannot create virtual device: name is taken"]

32. pmlist
    ./pmlist  [check the output]
              [plug in or remove a device]
    >> [type RETURN]
              [check for changes in device list]
    >>q

    


