#!/usr/bin/perl
# Converts an Anex86 FDI disk image to a Mahalito disk image
# 2014 H.Tomari. Public Domain.
#
# reverse-engineered FDI header (nonauthorative)
# struct fdi_header {
#	uint32_t zero;
#	uint32_t density; // 0x30 for 1.44MB, 0x90 for 1.2MB, 0x10 for 2DD?
#	uint32_t header_size; // 0x1000 (4096 bytes)
#	uint32_t image_body_size; // 0x134000 (1261568 bytes)
#	uint32_t sector_size; // 0x0400 (1024 bytes)
#	uint32_t sector_per_track; // 0x0008 (8 sec/track)
#	uint32_t heads; // 0x0002 (2 heads per cylinder)
#	uint32_t cylinders; // 0x004d (77 cyls)
#	unsigned char fill[0x1000-0x20];
# };
#
use strict;
use warnings;

my $usage=<< "EOL";
  % fdi2mhlt.pl input.fdi output
    => converts input.fdi to a Mahalito image output.2hd and output.dat
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

sub calc_N {
  my $sec_sz=shift;
  my $N=0;
  $sec_sz/=256;
  while($sec_sz>0) { 
    $sec_sz=$sec_sz>>1;
    $N++;
  }
  return $N;
}

sub fdi_density_interp {
  my $den_val=shift;
  my $den;
  if(($den_val&0x20)>0) {
    $den="2hd";
    print STDERR "WARNING      : 1.44 MB image detected.\n".
		"WARNING(cont): This image probably cannot be used with Mahalito.\n".
		"WARNING(cont): Use flatmhlt.pl to flatten the output,\n".
		"WARNING(cont): then dd the dat file to 1.44 MB formatted diskette.\n";
  } elsif(($den_val&0xe0)==0) {
    $den="2dd";
  } else {
    $den="2hd";
  }
  return $den;
}

sub fdi_to_mahalito {
  my ($fdifh,$outbase)=@_;
  my $buf;
  # Read FDI Header
  seek $fdifh,0,2;
  my $fdisz=tell $fdifh;
  seek $fdifh,0x4,0;
  read($fdifh, $buf, 0x1C);
  my ($fdi_den,$fdiheader_sz, $fdibody_sz, $sec_sz, $sec_per_trk, $nhead, $ncyl)=
    unpack("VVVVVVV",$buf);
  my $ntrk=$ncyl*$nhead;
  printf STDERR "FDI: %d bytes (%d header + %d body)\n", $fdisz, $fdiheader_sz, $fdibody_sz;
  print STDERR "WARNING: FDI file size does not match header!\n" unless ($fdisz == ($fdiheader_sz+$fdibody_sz));
  my $calc_bodysz=$ncyl*$nhead*$sec_per_trk*$sec_sz;
  printf STDERR "%d cyls, %d heads, %d sectors/track, %d bytes/sector => body %d bytes\n", $ncyl, $nhead, $sec_per_trk, $sec_sz, $calc_bodysz;
  print STDERR "WARNING: Body size does not match geometry!\n" unless ($calc_bodysz == $fdibody_sz);
  seek $fdifh,$fdiheader_sz,0;
  my $N=calc_N($sec_sz);
  my $format_density=fdi_density_interp($fdi_den);

  # write Mahalito header
  my $metaname=$outbase.".".$format_density;
  my $dataname=$outbase.".dat";
  print STDERR "writing to ".$metaname." and ".$dataname."\n";
  open(my $meta,'>',$metaname) or die "$!";
  binmode($meta);
  my %mhltver=("2hd"=>"ver 1.10  ","2dd"=>"2DD ver1.0","2d"=>"2D  ver1.0");
  my $mahalito_header=pack("a10C",$mhltver{$format_density},$ncyl);
  print $meta $mahalito_header;
  open(my $data,'>',$dataname) or die "$!";
  binmode($data);
  # read tracks
  my $trk;
  for($trk=0; $trk<$ntrk; $trk++) {
    my $track_header=pack("CC",$sec_per_trk,1); # MFM
    print $meta $track_header;
    for(my $sec=0; $sec<$sec_per_trk; $sec++) {
      my $sector_data;
      my $bytes_read=read($fdifh,$sector_data,$sec_sz);
      if($bytes_read<$sec_sz) {
	next; # ignore short sector
      }
      my $repeat_chr=detect_repeat($sector_data);
      my $repeatp=(defined($repeat_chr))?1:0;
      $repeat_chr=($repeatp>0)?$repeat_chr:0;
      my $lcyl=int($trk/2);
      my $lhead=int($trk&1);
      my $lrecord=$sec+1;
      my $seclen=$N;
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
  open(my $fdi,'<',$ARGV[0]) or die "$!";
  binmode($fdi);
  fdi_to_mahalito($fdi,$ARGV[1]);
  close($fdi);
} else {
  print STDERR $usage;
}
