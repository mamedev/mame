#!/usr/bin/perl
# Flatten Mahalito disk image for modifying contents
#
#   Mahalito compresses disk image data file (*.dat) using
#   a simple algorithm. This utility uncompresses the data file
#   while keeping the output a valid Mahalito disk image.
#
#   This is useful when retreiving or modifying files in a image.
#   The flattened data file (*.dat) can be mounted by:
#    % mount -t msdos -o loop pc100dos.dat /mnt
#   on Linux. See lofiadm manpage if you are using Solaris.
#   After necessary changes the flattened image can be written
#   back to a floppy disk using Mahalito.
#
# Example:
#  % flatmhlt.pl pc100dos.2dd [pc100dos.dat] flat
#    => converts pc100dos.2dd and pc100dos.dat to
#        a flat Mahalito image flat.2dd and flat.dat
#
use strict;
use File::Basename;
my $usage=<<"EOL";
  % flatmhlt.pl pc100dos.2dd [pc100dos.dat] flat
    => converts pc100dos.2dd and pc100dos.dat to 
        a flat Mahalito image flat.2dd and flat.dat
EOL

sub flatten_mahalito {
  my ($metafh,$datafh,$fltmfh,$fltdfh)=@_;
  my $buf;
  read($metafh,$buf,11);
  my ($ver,$ncyl) = unpack("a10C",$buf);
  my %v2f=("ver 1.10  "=>'2HD',"2DD ver1.0"=>'2DD',"2D  ver1.0"=>'2D');
  if(!defined($v2f{$ver})) {
    die "Unknown version string: not a Mahalito image?";
  }
  print $fltmfh $buf;
  for(my $trk=0; $trk<$ncyl; $trk++) {
    for(my $head=0; $head<2; $head++) {
      read($metafh,$buf,2);
      my ($nsec,$mfmp) = unpack("CC",$buf);
      print $fltmfh $buf;
      for(my $sec=0; $sec<$nsec; $sec++) {
	read($metafh,$buf,8);
	my ($repp,$repd,$lcyl,$lhd,$lsec,$slen,$rslen,$ddamp)=unpack("CCCCCCCC",$buf);
	my $sbytes=128*(2**$slen);
	my $rsbytes=128*(2**$rslen);
	print $fltmfh pack("CCCCCCCC",0,0,$lcyl,$lhd,$lsec,$slen,$rslen,$ddamp);
	if($repp) {
	  $buf="";
	  for(my $i=0; $i<$sbytes; $i++) {
	    $buf .= pack("C",$repd);
	  }
	} else {
	  read($datafh,$buf,$sbytes);
	}
	print $fltdfh $buf;
      }
    }
  }
}

if($#ARGV>=1) {
  my $metafile=$ARGV[0];
  my ($metaname,$metapath,$metasuffix)=fileparse($metafile, qr/\.[^.]*/);
  my $datafile;
  if($#ARGV>=2) {
    $datafile=$ARGV[1];
  } else {
    my $datafileS=$metapath.$metaname.".dat";
    my $datafileL=$metapath.$metaname.".DAT";
    if (-e $datafileS) {
      $datafile=$datafileS;
    } elsif (-e $datafileL) {
      $datafile=$datafileL;
    } else {
      die "data file not found (tried ".$datafileS." and ".$datafileL.")";
    }
  }
  my $flatmetaname=$ARGV[$#ARGV].$metasuffix;
  my $flatdataname=$ARGV[$#ARGV].".dat";
  open(my $metafh,'<',$metafile) or die $!;
  open(my $datafh,'<',$datafile) or die $!;
  open(my $fltmfh,'>',$flatmetaname) or die $!;
  open(my $fltdfh,'>',$flatdataname) or die $!;
  binmode($metafh);
  binmode($datafh);
  binmode($fltmfh);
  binmode($fltdfh);
  flatten_mahalito($metafh,$datafh,$fltmfh,$fltdfh);
  close($fltdfh);
  close($fltmfh);
  close($datafh);
  close($metafh);
} else {
  print STDERR $usage;
}
