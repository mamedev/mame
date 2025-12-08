# PC-9801 SIMM

## References
- https://projectmps.net/simm54.htm
- https://projectmps.net/simm61.htm
- https://web.archive.org/web/20190922055823/https://radioc.web.fc2.com/column/pc98bas/pc98extnecram_en.htm

Proprietary NEC 54SIMM / 61SIMM sockets.

-54 has a 16-bit data width and -61 has 32-bit

For usability reasons we emulate these as a single mappable slot, also not everything is actually NEC but rather third party options exist (I/O Data and Melco / Buffalo)

## TODO
- base/limit I/O registers for 98FELLOW / PC-9821;
- latencies;
