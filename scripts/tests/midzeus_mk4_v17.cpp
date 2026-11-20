// license:BSD-3-Clause
// copyright-holders:blahm1d
#include "../../src/mame/williams/midzeus_mk4_v17.h"
#include <array>
#include <cassert>
#include <cstdio>

int main()
{
    using namespace mk4_v17;
    int16_t const matrix[3][3]={{16384,0,0},{0,16384,0},{0,0,16384}};
    int16_t const light[3]={64,0,0};
    std::array<uint8_t,128> table; table.fill(160);
    auto read=[&](unsigned i){return table.at(i);};
    auto evaluate=[&](uint32_t ptr,uint32_t n=256,uint8_t alpha=255,bool valid=true,bool lng=true,uint32_t tex=0){
        return intensity(n,matrix,light,valid,ptr,tex,0x4b23cb00,alpha,lng,int64_t(8192)*32768,read);
    };
    assert(evaluate(0x0f023000)==49152);
    assert(evaluate(0x0f023000,256,128)==26624);
    assert(evaluate(0x0f023000,0)==65535);
    assert(evaluate(0x0f023000,0,128)==34815);
    assert(evaluate(0x0e023000)==65535);
    assert(evaluate(0x0f023000,256,255,false)==65535);
    assert(evaluate(0x0f023000,256,255,true,false)==65535);
    assert(evaluate(0x0f023000,256,255,true,true,0x009d138a)==65535);
    assert(evaluate(0x0f058080,256,255,true,true,0x006d7800)==59392);
    assert(evaluate(0x0f02f170,256,255,true,true,0x058252c0)==59392);
    assert(depth_intensity(0)==65535);
    assert(depth_intensity(int64_t(500000)*32768)==12288);
    for(auto const &m:materials) {
        for(unsigned b=0;b<4;b++) table[b]=uint8_t(m.signature>>(8*b));
        auto const expected=uint32_t(std::clamp(160*m.gain+256*m.bias,0,131071));
        assert(evaluate(0x0f000000|m.pointer)==expected);
    }
    table.fill(160); table[63]=200;
    assert(evaluate(0x0f0230f4)==98302);
    assert(rectangle_color(0xffff,0xffffff)==0xffff);
    assert(rectangle_color(0x7fff,0x808080)==0x3def);
    assert(rectangle_color(0xffff,0)==0x8000);
    assert(channel(255,0,255,255,0x4b23cb00,false)==31);
    assert(channel(255,0,40,255,0x4b23cb00,false)==5);
    assert(channel(255,0,240,255,0x4b23cb00,false)==30);
    assert(channel(200,20,128,128,0x40b68800,false)==8);
    assert(channel(255,0,128,255,0xdd23dd00,true)==16);
    assert(sign10(512)==-512 && sign10(1023)==-1 && sign10(511)==511);
    assert(floor_shift(-67108865,26)==-2 && floor_shift(67108865,26)==1);
    puts("PASS: MK4 v17 guards, all 23 material entries, rim, shaft, alpha, rectangle and blend cases");
}
