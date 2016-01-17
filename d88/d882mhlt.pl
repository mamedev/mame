#!/usr/bin/perl
# Converts a D88 image to Mahalito image(s)
# 2013 H.Tomari. Public Domain.
#
use strict;
use warnings;
my $usage=<< "EOL";
  % d882mhlt.pl input.d88 output
    => converts input.d88 to a Mahalito image output.{2d,2dd,2hd} and output.dat
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

sub d88_to_mahalito {
  my ($d88fh,$outbase)=@_;
  my $buf;
  my $outsuffixnum=0;
  my $filecnt=1;
  my $quiet=1;
  do {
    my $top_of_image=tell $d88fh;
    read($d88fh,$buf,32);
    my ($title,$wpp,$dsktyp,$len) = unpack("a17x9CCV",$buf);
    my $dsktyps=("2d","2dd","2hd")[$dsktyp>>4];
    my $mhltbase;
    if((-s $d88fh)>$len) {
      $mhltbase=$outbase."#".$filecnt;
    } else {
      $mhltbase=$outbase;
    }
    my $metaname=$mhltbase.".".$dsktyps;
    my $dataname=$mhltbase.".dat";
    print STDERR $title." => ".$metaname." and ".$dataname."\n";
    open(my $meta,'>',$metaname) or die "$!";
    open(my $data,'>',$dataname) or die "$!";
    binmode($meta);
    binmode($data);
    # read track offsets
    read($d88fh,$buf,4);
    my $fsttrk=unpack("V",$buf);
    my $ntrkoffsbytes=$fsttrk-0x20;
    read($d88fh,$buf,$ntrkoffsbytes-4);
    my $ntrkremain=$ntrkoffsbytes/4-1;
    my @trk2plus=unpack("V".$ntrkremain,$buf);
    my @trkoffs=($fsttrk,@trk2plus);
    my $maxtrk=0;
    # count number of tracks
    for(my $i=0; $i<=$#trkoffs; $i++) {
      if($trkoffs[$i]>0) {
	$quiet or printf("Track %03d at %08X\n",$i,$trkoffs[$i]);
	$maxtrk=$i;
      }
    }
    # Generate Mahalito metadata header
    my %mhltver=("2hd"=>"ver 1.10  ","2dd"=>"2DD ver1.0","2d"=>"2D  ver1.0");
    my $ncyl=int(($maxtrk+1)/2);
    my $wbuf=pack("a10C",$mhltver{$dsktyps},$ncyl);
    print $meta $wbuf;		# META
    # Read sectors
    my $trk;
    for($trk=0; $trk<=$maxtrk; $trk++) {
      if($trkoffs[$trk]) {
	my $offs_of_this_track=$top_of_image+$trkoffs[$trk];
	seek $d88fh,$offs_of_this_track,0;
	my $sec=0;
	my ($lcyl,$lhd,$lrec,$lsize,$nsec,$den,$del,$stat,$dsz);
	do {
	  read($d88fh,$buf,16);
	  ($lcyl,$lhd,$lrec,$lsize,$nsec,$den,$del,$stat,$dsz)=
	    unpack("CCCCvCCCx5v",$buf);
	  if($sec==0) {
	    # track header
	    my $mfmp=($den==0x40)?0:1;
	    my $track_header=pack("CC",$nsec,$mfmp);
	    print $meta $track_header;
	  }
	  read($d88fh,$buf,$dsz);
	  $sec++;
	  $quiet or print STDERR "C/H/R/N = ".$lcyl."/".$lhd."/".$lrec."/".$lsize."\n";
	  my $rep_chr=detect_repeat($buf);
	  my $isRepeat=(defined($rep_chr))?1:0;
	  $rep_chr=($isRepeat)?$rep_chr:0;
	  my $mhlt_ddam=($del==0x10)?1:0;
	  my $mhlt_sec_header=pack("CCCCCCCC",$isRepeat,$rep_chr,
				   $lcyl,$lhd,$lrec,$lsize,$lsize,$mhlt_ddam);
	  print $meta $mhlt_sec_header;
	  if($isRepeat<1) {
	    print $data $buf;		# MAHALITO DATA
	  }
	} while($sec<$nsec);
      } else { # hole in a image? possibly an unformatted track
	my $track_header=pack("CC",0,1);
	print $meta $track_header;
	print STDERR "WARN: empty track at track ".$trk."\n";
      }
    }
    close($data);
    close($meta);
    my $offs_of_next_image=$top_of_image+$len;
    seek $d88fh,$offs_of_next_image,0;
    $filecnt++;
  } while(!eof($d88fh));
}

if($#ARGV==1) {
  open(my $d88,'<',$ARGV[0]) or die "$!";
  binmode($d88);
  d88_to_mahalito($d88,$ARGV[1]);
  close($d88);
} else {
  print STDERR $usage;
}
