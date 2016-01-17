#!/usr/bin/perl
# D88/D77 multi-volume format splitter
#       D88: PC-88/PC-98 floppy disk image
#       D77: FM77 floppy disk image
# 2013 H.Tomari. Public Domain.
#
use strict;
use File::Basename;
my $usage= << "EOL" ;
 d88split l image.d88  # lists images in the file
 d88split e image.d88  # extracts images from the file
                         image#1.d88, image#2.d88, ...
  (To concatenate D88 files use cat a.d88 b.d88 > c.d88 )
EOL

if($#ARGV==1 && $ARGV[0] =~ /^[le]$/) {
  open(my $src,'<',$ARGV[1]) or die "image open failed: $!";
  binmode($src);
  my $buf;
  my ($srcname,$srcpath,$srcsuffix) = fileparse($ARGV[1],qr/\.[^.]*/);
  my $filecnt=1;
  if($ARGV[0] =~ "l") {
    print STDERR " Image        Protect    Type     Bytes \n".
                 "----------------------------------------\n";
  }
  do {
    read($src,$buf,32);
    my ($title,$wpp,$dsktyp,$len) = unpack("a17x9CCV",$buf);
    my $dsktyps=("2D","2DD","2HD")[$dsktyp>>4];
    my $wps=$wpp?1:0;
    if($ARGV[0] =~ "l") {
      format STDOUT =
@<<<<<<<<<<<<<<<< @##    @<<<< @########
$title, $wps, $dsktyps, $len
.
      write();
      seek($src,$len-32,1);
    } else {
      my $dstname=$srcname."#".$filecnt.$srcsuffix;
      open(my $dst,'>',$dstname) or die "destination open failed: $!";
      binmode($dst);
      print $dst $buf;
      read($src,$buf,$len-32);
      print $dst $buf;
      close $dst;
      print STDOUT $title." => ".$dstname." ( ".$len." bytes)\n";
    }
    $filecnt=$filecnt+1;
  } while(!eof($src));
  close $src;
} else {
  print STDERR $usage;
}
