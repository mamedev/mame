#!/usr/bin/perl
# Converts an X68k XDF disk image to a Mahalito disk image
# 2013 H.Tomari. Public Domain.
#
use strict;
use warnings;

my $usage=<< "EOL";
  % xdf2mhlt.pl input.xdf output
    => converts input.xdf to a Mahalito image output.2hd and output.dat
EOL

sub detect_repeat {
  my $data=shift;
  my @dataarray=unpack('C*',$data);
  my $chr=$dataarray[0];
  foreach(@dataarray) {
    return undef if($_ ne $chr);
  }
  return $chr;
}

sub xdf_to_mahalito {
  my ($xdffh,$outbase)=@_;
  my $metaname=$outbase.".2hd";
  my $dataname=$outbase.".dat";
  print STDERR "writing to ".$metaname." and ".$dataname."\n";
  open(my $meta,'>',$metaname) or die "$!";
  open(my $data,'>',$dataname) or die "$!";
  binmode($meta);
  binmode($data);
  # detect cylinder count (usually 77 tracks)
  seek $xdffh,0,2;
  my $xdfsz=tell $xdffh;
  seek $xdffh,0,0;
  my $ntrk=int(($xdfsz+8191)/8192);
  my $ncyl=int(($ntrk+1)/2);
  print STDERR "XDF is ".$xdfsz." bytes long. #track= ".$ntrk.", #cyl= ".$ncyl."\n";
  # write Mahalito header
  my $mahalito_header=pack("a10C","ver 1.10  ",$ncyl);
  print $meta $mahalito_header;
  # read tracks
  my $trk;
  for($trk=0; $trk<$ntrk; $trk++) {
    my $original_pos=tell $xdffh;
    seek $xdffh,8192,1;
    my $after_track_pos=tell $xdffh;
    my $track_len=$after_track_pos-$original_pos;
    seek $xdffh,-8192,1;
    my $nsector=int(($track_len+1023)/1024);
    my $track_header=pack("CC",$nsector,1); # MFM
    print $meta $track_header;
    for(my $sec=0; $sec<$nsector; $sec++) {
      my $sector_data;
      my $bytes_read=read($xdffh,$sector_data,1024);
      if($bytes_read<1024) {
	next; # ignore short sector
      }
      my $repeat_chr=detect_repeat($sector_data);
      my $repeatp=(defined($repeat_chr))?1:0;
      $repeat_chr=($repeatp>0)?$repeat_chr:0;
      my $lcyl=int($trk/2);
      my $lhead=int($trk&1);
      my $lrecord=$sec+1;
      my $seclen=3;
      my $ddam=0;
      my $sector_info=pack("CCCCCCCC",$repeatp,$repeat_chr,
			  $lcyl,$lhead,$lrecord,
			  $seclen,$seclen,
			  $ddam);
      print $meta $sector_info;
      if($repeatp<1) {
	print $data $sector_data;
      }
    }
  }
  close($data);
  close($meta);
}

if($#ARGV==1) {
  open(my $xdf,'<',$ARGV[0]) or die "$!";
  binmode($xdf);
  xdf_to_mahalito($xdf,$ARGV[1]);
  close($xdf);
} else {
  print STDERR $usage;
}
