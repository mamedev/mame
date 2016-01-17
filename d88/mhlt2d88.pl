#!/usr/bin/perl
# Mahalito to D88 disk image converter
# 2013 H.Tomari. Public Domain.
#
use strict;
use File::Basename;
my $usage= << "EOL" ;
 Example:
  mhlt2d88 pc100dos.2dd [pc100dos.dat] > pc100dos.d88
   # pc100dos.2dd, pc100dos.dat => pc100dos.d88
EOL

sub mahalito_d88 {
  my ($meta,$data,$d88,$imgtitle) = @_;
  my $quiet=1;
  my $buf;
  my $d88hdr="";
  my $d88offsets="";
  my $d88tracks="";
  my $d88trkptr=0x2b0;
  # Read Mahalito Header
  read($meta,$buf,11);
  my ($ver,$ncyl) = unpack("a10C",$buf);
  my %v2f=("ver 1.10  "=>"2HD","2DD ver1.0"=>"2DD","2D  ver1.0"=>"2D");
  my $format=$v2f{$ver};
  die "Not a Mahalito disk image?" if(!defined($format));
  $quiet or print STDERR 'Version: "'.$ver.'" ('.$format.")\n#Cylinders=".$ncyl."\n";
  # Generate D88 Header
  my %f2t=("2HD"=>0x20,"2DD"=>0x10,"2D"=>0x00);
  my $d88disktype=$f2t{$format};
  $d88hdr=pack("a17x9CC",$imgtitle,0,$d88disktype);
  # Read Mahalito tracks
  for(my $trk=0; $trk<$ncyl; $trk++) {
    for(my $head=0; $head<2; $head++) {
      read($meta,$buf,2);
      my ($nsec,$mfmp) = unpack("CC",$buf);
      $quiet or print STDERR " Track=".$trk." Head=".$head.": #Sectors=".$nsec." ".($mfmp?"MFM":"FM")."\n";
      $d88offsets .= pack("V",$d88trkptr);
      if($nsec==0) {
	# UNFORMATTED TRACK
	$quiet or print STDERR "   UNFORMATTED\n";
	my $d88sect=pack("CCCCvCCCx5v",
			 0,0,0,0,0,0,
			 0,0,0).pack("x256");
	$d88tracks .= $d88sect;
	$d88trkptr += length($d88sect);
      } else {
	for(my $sec=0; $sec<$nsec; $sec++) {
	  # READ MAHALITO SECTOR
	  read($meta,$buf,8);
	  my ($repp,$repd,$lcyl,$lhd,$lsec,$slen,$rslen,$ddamp)=unpack("CCCCCCCC",$buf);
	  my $sbytes=128*(2**$slen);
	  my $rsbytes=128*(2**$rslen);
	  $quiet or print STDERR "    Sector ".$sec." Logical CHS=".$lcyl."/".$lhd."/".$lsec."\n";
	  $quiet or print STDERR "      Length=".$sbytes." (real ".$rsbytes.")   ".($ddamp?"DDAM":"DAM")."\n";
	  if($repp) {
	    $quiet or print STDERR "      Repeat=yes, data=0x".sprintf("%02X",$repd)."\n";
	    $buf="";
	    for(my $i=0; $i<$sbytes; $i++) {
	      $buf.=$repd;
	    }
	  } else { # read data
	    read($data,$buf,$sbytes);
	  }
	  # D88 SECTOR HEADER
	  my $d88sect=pack("CCCCvCCCx5v",
			   $lcyl,$lhd,$lsec,$slen,$nsec,($mfmp?0x0:0x40),
			   ($ddamp?1:0),($ddamp?0x10:0x00),$sbytes).$buf;
	  $d88tracks .= $d88sect;
	  $d88trkptr += length($d88sect);
	}
      }
    }
  }
  # fill unused D88 track pointers
  for(my $trk=2*$ncyl; $trk<164; $trk++) {
    $d88offsets .= pack("V",0);
  }
  # calc. size of image
  my $d88size=0x2b0+length($d88tracks);
  $d88hdr .= pack("V",$d88size);
  binmode($d88);
  print $d88 $d88hdr;
  print $d88 $d88offsets;
  print $d88 $d88tracks;
}

if($#ARGV>=0) {
  my $mhltmeta=$ARGV[0]; # *.{2hd,2dd,2d}
  my $mhltdata;
  my $d88file;
  my ($mhltname,$mhltpath,$mhltsuffix)=fileparse($ARGV[0],qr/\.[^.]*/);
  if($#ARGV==0) {
    my $datfile=$mhltpath.$mhltname.".dat";  # *.dat
    my $datfileL=$mhltpath.$mhltname.".DAT"; # *.DAT
    if(-e $datfile) {
      $mhltdata=$datfile;
    } elsif (-e $datfileL) {
      $mhltdata=$datfileL;
    } else {
      die "$datfile or $datfileL not found.";
    }
  } else {
    $mhltdata=$ARGV[1];
  }
  open(my $metah,'<',$mhltmeta) or die 'cannot open metadata: $!';
  open(my $datah,'<',$mhltdata) or die 'cannot open data: $!';
  binmode($metah);
  binmode($datah);
  mahalito_d88($metah,$datah,\*STDOUT,$mhltname);
  close($datah);
  close($metah);
} else {
  print STDERR $usage;
}

