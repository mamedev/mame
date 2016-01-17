#!/usr/bin/perl
# Converts an T98-Next NFD R0 disk image to a Mahalito disk image
# 2014 H.Tomari. Public Domain.
#
use strict;
use warnings;

my $usage=<< "EOL";
  % nfd2mhlt.pl input.nfd output
    => converts input.nfd to a Mahalito image output.2hd and output.dat
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

sub read_trkinfo_nfdr0 {
  my $nfd=shift;
  my $buf;
  my @secinfo;	# sector info (C/H/R/N/MFM/DDAM/PDA)
  my @seccount; # sector count per track
  print STDERR "NFD R0 ";
  for(my $trk=0; $trk<163; $trk++) {
    my $nsec=0;
    for(my $sec=0; $sec<26; $sec++) {
      next if(read($nfd,$buf,0x10)<0x10);
      my @sec=unpack("C11x5",$buf);
      next if($sec[0]==0xff);
      $nsec++;
      my @sec_reqd=($sec[0], $sec[1], $sec[2], $sec[3], $sec[4], $sec[5], $sec[10]);
      push @secinfo, \@sec_reqd;
    }
    push @seccount, $nsec if($nsec>0);
  }
  return (\@secinfo, \@seccount);
}

# Not tested -- I dont have any NFD R1 image.
sub read_trkinfo_nfdr1 {
  my $nfd=shift;
  print STDERR "NFD R1 ";
  my $buf;
  my @secinfo;	# sector info (C/H/R/N/MFM/DDAM/PDA/Skip)
  my @seccount; # sector count per track
  my @skipafter; # bytes to skip after processing a sector
  read($nfd,$buf,164*4);
  my @trkptr=unpack("V164",$buf);
  foreach (@trkptr) {
    next if($_==0);
    seek $nfd, $_, 0;
    read($nfd,$buf,0x10); # Read NFD_TRACK_ID1
    my ($wSector, $wDiag)=unpack("vv",$buf);
    push @seccount, $wSector;
    for(my $secnum=0; $secnum<$wSector; $secnum++) {
      read($nfd,$buf,0x10);
      my @sec=unpack("C12x4",$buf);
      my @sec_reqd=($sec[0], $sec[1], $sec[2], $sec[3], $sec[4], $sec[5], $sec[11],$sec[10]);
      push @secinfo, \@sec_reqd;
    }
    my $skipbytes=0;
    for(my $diagnum=0; $diagnum<$wDiag; $diagnum++) {
      read($nfd,$buf,0x10);
      my ($retry, $len)=unpack("xxxxxxxxxCVxx",$buf);
      $skipbytes+=(1+$retry)*$len;
    }
    push @skipafter, $skipbytes;
  }
  return (\@secinfo, \@seccount, \@skipafter);
}

sub detect_media {
  my ($pda,$N)=@_;
  my $type;
  if($pda==0x30) {
    print STDERR "WARNING      : 1.44 MB image detected.\n".
		"WARNING(cont): This image probably cannot be used with Mahalito.\n".
		"WARNING(cont): Use flatmhlt.pl to flatten the output,\n".
		"WARNING(cont): then dd the dat file to 1.44 MB formatted diskette.\n";
    $type="2hd";
  } elsif($pda==0x10) {
    $type="2dd";
  } elsif($pda==0x90) {
    $type="2hd";
  } elsif($N==2) { # dubious
    $type="2dd";
  } else {
    $type="2hd";
  }
  return $type;
}

sub N_to_bytes {
  my $N=shift;
  my $bytes=128*(2**$N);
  return $bytes;
}

sub nfd_to_mahalito {
  my ($nfdfh,$outbase)=@_;
  my $buf;
  read($nfdfh,$buf,0x120);
  my ($magic,$comment,$nfdhd_sz,$prot,$nhead)=unpack('a15xa256VCC',$buf);
  my @secinfo;
  my @seccount;
  my @skipafter;
  if($magic eq "T98FDDIMAGE.R0\0") {
    my ($secinfo_, $seccount_)=read_trkinfo_nfdr0($nfdfh);
    @secinfo=@$secinfo_;
    @seccount=@$seccount_;
  } elsif($magic eq "T98FDDIMAGE.R1\0") {
    print STDERR "WARNING: NFD R1 detected. R1 support is experimental.\n";
    my ($secinfo_, $seccount_,$skipafter_)=read_trkinfo_nfdr1($nfdfh);
    @secinfo=@$secinfo_;
    @seccount=@$seccount_;
    @skipafter=@$skipafter_;
  } else {
    print STDERR "Invalid magic. Aborting\n";
    return;
  }

  my $mediatype=detect_media(${$secinfo[0]}[6],${$secinfo[0]}[3]);
  my $ntrk=$#seccount;
  my $ncyl=(1+$#seccount)/$nhead;
  my $firstN=${$secinfo[0]}[3];
  printf STDERR "#cyl %d #hd %d #sec %d (%d byte/sector)\n",
    $ncyl,$nhead,$seccount[0],N_to_bytes($firstN);

  seek $nfdfh, $nfdhd_sz, 0; # move to body
  # write Mahalito header
  my $metaname=$outbase.".".$mediatype;
  my $dataname=$outbase.".dat";
  print STDERR "writing to ".$metaname." and ".$dataname."\n";
  open(my $meta,'>',$metaname) or die "$!";
  open(my $data,'>',$dataname) or die "$!";
  binmode($meta);
  binmode($data);
  my %mhltver=("2hd"=>"ver 1.10  ","2dd"=>"2DD ver1.0","2d"=>"2D  ver1.0");
  my $mahalito_header=pack("a10C",$mhltver{$mediatype},$ncyl);
  print $meta $mahalito_header;

  # read tracks
  my $secptr=0;
  foreach my $s (0..$#seccount) {
    my $nsector=$seccount[$s];
    my $mfm=${$secinfo[$secptr]}[4];
    my $track_header=pack("CC",$nsector,$mfm); # MFM
    print $meta $track_header;
    my $lastsec=$secptr+$nsector;
    for(; $secptr<$lastsec; $secptr++) {
      my $bytes=N_to_bytes(${$secinfo[$secptr]}[3]);
      my $sector_data;
      my $bytes_read=read($nfdfh,$sector_data,$bytes);
      if($bytes_read<$bytes) {
	print STDERR "Failed to read input. Trying to continue...\n";
	next;
      }
      my $repeat_chr=detect_repeat($sector_data);
      my $repeatp=(defined($repeat_chr))?1:0;
      $repeat_chr=($repeatp>0)?$repeat_chr:0;
      my $lcyl=${$secinfo[$secptr]}[0];
      my $lhead=${$secinfo[$secptr]}[1];
      my $lrecord=${$secinfo[$secptr]}[2];
      my $seclen=${$secinfo[$secptr]}[3];
      my $ddam=${$secinfo[$secptr]}[5];
      my $sector_info=pack("CCCCCCCC",$repeatp,$repeat_chr,
			  $lcyl,$lhead,$lrecord,
			  $seclen,$seclen,
			  $ddam);
      print $meta $sector_info;
      if($repeatp<1) {
	print $data $sector_data;
      }
      if(defined(${$secinfo[$secptr]}[7])) {
	my $bytes_to_skip=${$secinfo[$secptr]}[7]*$bytes;
	seek $nfdfh, $bytes_to_skip, 1;
      }
    }
    if(defined($skipafter[$s])) {
      my $bytes_to_skip=$skipafter[$s];
      seek $nfdfh, $bytes_to_skip, 1 if $bytes_to_skip>0;
    }
  }
  close($data);
  close($meta);
}

if($#ARGV==1) {
  open(my $nfd,'<',$ARGV[0]) or die "$!";
  binmode($nfd);
  nfd_to_mahalito($nfd,$ARGV[1]);
  close($nfd);
} else {
  print STDERR $usage;
}
