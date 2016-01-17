# d88split

Splits concatenated D88/D77 disk images.
* **D88** is an image format for NEC PC-88/PC-98 floppy disks.
* **D77** is the same format for Fujitsu FM-77 series computers.

Written in Perl, released to the public domain.

## Examples

List images packed in a file:
```
% ./d88split.pl l kohaku.d77
 Image        Protect    Type     Bytes 
----------------------------------------
Kohaku_1            0    2D       473776
Kohaku_2            0    2D       473776
Kohaku_3            0    2D       473776
Kohaku_4            0    2D       473776
```

Extract images:
```
% ./d88split.pl e kohaku.d77
Kohaku_1 => kohaku#1.d77 ( 473776 bytes)
Kohaku_2 => kohaku#2.d77 ( 473776 bytes)
Kohaku_3 => kohaku#3.d77 ( 473776 bytes)
Kohaku_4 => kohaku#4.d77 ( 473776 bytes)
```

# mhlt2d88

Converts Mahalito disk image to D88 format.
* **Mahalito** is a disk image tool for PC-98.
* **D88** is another popular format for transferring PC-98 disk images.

## Examples

```
./mhlt2d88.pl pc100dos.2dd > pc100dos.d88
```
pc100dos.2dd and pc100dos.dat are converted to pc100dos.d88.

# flatmhlt

Flatten Mahalito disk image for modifying contents

Mahalito compresses disk image data file (*.dat) using
a simple algorithm. This utility uncompresses the data file
while keeping the output a valid Mahalito disk image.

This is useful when retreiving or modifying files in a image.
The flattened data file (*.dat) can be mounted by:
```
% mount -t msdos -o loop pc100dos.dat /mnt
```
on Linux. See lofiadm manpage if you are using Solaris.
After necessary changes the flattened image can be written
back to a floppy disk using Mahalito.

##Example

```
% flatmhlt.pl pc100dos.2dd [pc100dos.dat] flat
    => converts pc100dos.2dd and pc100dos.dat to
        a flat Mahalito image flat.2dd and flat.dat
```

# xdf2mhlt

Converts X68k XDF disk image to Mahalito format.

## Example

```
./xdf2mhlt.pl sharrier.xdf sharrier
```
convert sharrier.xdf to sharrier.2hd and sharrier.dat.

# fdi2mhlt

Converts Anex86 FDI disk image to Mahalito format

2HD, 2DD images are supported (based on reverse-engineering).
1.44 MB format is also supported on this tool, but Mahalito doesn't.
Use flatmhlt.pl then dd the dat file to standard-formatted 1.44 MB diskette.

## Example

```
$ ./fdi2mhlt.pl blkwarsv.fdi blkwarsv
FDI: 1265664 bytes (4096 header + 1261568 body)
77 cyls, 2 heads, 8 sectors/track, 1024 bytes/sector => body 1261568 bytes
writing to blkwarsv.2hd and blkwarsv.dat
```

# d882mhlt

Converts D88/D77 disk image to Mahalito format.
Concatenated D88 disk images are supported.

## Example

```
% ./d882mhlt.pl kohaku.d77 ko
Kohaku_1 => ko#1.2d and ko#1.dat
Kohaku_2 => ko#2.2d and ko#2.dat
Kohaku_3 => ko#3.2d and ko#3.dat
Kohaku_4 => ko#4.2d and ko#4.dat
```

# nfd2mhlt

Converts T98-Next NFD floppy disk image to Mahalito format.
Tested with NFD R0 images only.
NFD R1 support is experimental, and it hasn't been tested.
This is because no R1 image is currently available to me.

```
% ./nfd2mhlt.pl No1.NFD dos62no1
NFD R0 #cyl 77 #hd 2 #sec 8 (1024 byte/sector)
writing to dos62no1.2hd and dos62no1.dat
```

# References

1. mahalito.doc in mhlt214.lzh -- Describes Mahalito format
2. quasi88-0.6.4/document/FORMAT.TXT in quasi88-0.6.4.tgz -- describes D88 format
3. http://www.geocities.jp/t98next/nfdr0.txt -- NFD R0 format specification
4. http://www.geocities.jp/t98next/nfdr1.txt -- NFD R1 format specification
