// license:BSD-3-Clause
#include "../../src/mame/skeleton/alphasmart_usb.h"
#include <cassert>
#include <iostream>

struct bus {
    alphasmart_usb_chip chip;
    bool scl=true, sda=true;
    void lines(bool c, bool d) { scl=c;sda=d;chip.lines(c,d); }
    void start() { lines(false,true);lines(true,true);lines(true,false);lines(false,false); }
    void stop() { lines(false,false);lines(true,false);lines(true,true); }
    bool write(uint8_t value) {
        for(int bit=7;bit>=0;--bit) { lines(false,(value>>bit)&1);lines(true,sda);lines(false,sda); }
        lines(false,true);lines(true,true);bool ack=!chip.sda_r();lines(false,true);return ack;
    }
    uint8_t read(bool last) {
        uint8_t value=0;
        for(int bit=7;bit>=0;--bit) { lines(false,true);lines(true,true);value=(value<<1)|chip.sda_r();lines(false,true); }
        lines(false,last);lines(true,last);lines(false,last);lines(false,true);return value;
    }
    void command(uint8_t cmd) { start();assert(write(0x36));assert(write(cmd));stop(); }
    void data(uint8_t value) { start();assert(write(0x34));assert(write(value));stop(); }
    uint8_t data() {start();assert(write(0x35));auto v=read(true);stop();return v;}
};
int main() {
    bus b;
    b.command(0xd1);b.data(0x80);assert(b.chip.address==0x80);
    b.command(0xf3);b.start();assert(b.write(0x34));assert(b.write(0x91));assert(b.write(3));b.stop();
    assert(b.chip.mode==0x91 && b.chip.clock==3);
    b.chip.vbus=true;
    uint8_t setup[8]={0x80,6,0,1,0,0,18,0};unsigned n=8;
    assert(b.chip.packet(0,false,true,setup,n)==0);
    b.command(0xf4);b.start();assert(b.write(0x35));assert(b.read(false)==4);assert(b.read(true)==0);b.stop();
    b.command(0x42);assert(b.data()==0x21);assert(b.chip.irq_r());
    b.command(2);assert(b.data()==1);b.command(0xf0);
    b.start();assert(b.write(0x35));assert(b.read(false)==0);assert(b.read(false)==8);
    for(unsigned i=0;i<8;++i) assert(b.read(i==7)==setup[i]);
    b.stop();
    b.command(0xf2);assert(b.chip.ep[2].full); // must ACK SETUP first
    b.command(0xf1);b.command(0xf2);assert(!b.chip.ep[2].full);
    b.command(3);b.command(0xf1);b.command(0xf0);b.start();assert(b.write(0x34));
    assert(b.write(0));assert(b.write(2));assert(b.write(0x12));assert(b.write(0x34));b.stop();b.command(0xfa);
    n=8;assert(b.chip.packet(0,true,false,setup,n)==0 && n==2 && setup[0]==0x12 && setup[1]==0x34);
    assert(alphasmart_usb_chip::index(1,true)==4 && alphasmart_usb_chip::index(1,false)==5);
    b.command(0xd8);b.data(2);
    uint8_t data[8]={1,2,3};n=3;
    assert(b.chip.packet(1,false,false,data,n)==0);
    assert(b.chip.packet(1,false,false,data,n)==1); // full OUT FIFO NAKs without overwriting
    assert(b.chip.ep[5].data[1]==3 && b.chip.ep[5].data[4]==3);
    b.command(5);b.command(0xf2);
    assert(b.chip.packet(1,false,false,data,n)==0);
    b.command(0x45);b.data(1);
    assert(b.chip.packet(1,false,false,data,n)==2);
    b.command(0x45);b.data(0);
    assert(!b.chip.ep[5].full && !b.chip.ep[5].stalled);
    b.command(0x42);b.data(1);b.command(0x43);b.data(1);
    n=8;assert(b.chip.packet(0,false,true,setup,n)==0);
    assert(!b.chip.ep[2].stalled && !b.chip.ep[3].stalled);
    b.start();assert(!b.write(0x38));b.stop(); // no ACK at unrelated I2C addresses
    b.chip.frame=0x5ab;b.command(0xf5);b.start();assert(b.write(0x35));
    assert(b.read(false)==0xab);assert(b.read(true)==5);b.stop();
    b.chip.reset(true);assert(b.chip.connected() && b.chip.interrupts==0x4000 && b.chip.address==0);
    b.command(0xf4);b.start();assert(b.write(0x35));assert(b.read(false)==0);assert(b.read(true)==0x40);b.stop();assert(b.chip.irq_r());
    std::cout << "PDIUSBD11 I2C, FIFO, SETUP, interrupt and reset tests passed\n";
}
