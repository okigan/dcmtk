#!/usr/bin/perl

use CGI;
#use Digest::MD4 qw(md4 md4_hex md4_base64);
use Digest::MD5 qw(md5 md5_hex md5_base64);

$query = CGI->new;

my $file = $query->param('file');
my $cachepath = $ENV{"TMP"} . "\\";

($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);

if(length($file) <= 0){
  print "Content-type: text/html \n";
  print "\n";


  printf "%4d-%02d-%02d %02d:%02d:%02d\n",$year+1900,$mon+1,$mday,$hour,$min,$sec;
}else{
  my $width = $query->param('width');
  my $quality = $query->param('quality');
  
  my $command = "E:\\GitViews\\dcmtk\\dcmwadish\\dcmj2pnm.exe +oj +Wh 5 ";

  if($width){
    $command = $command . " +Sxv " . $width;
  }

  if($quality){
    $command = $command . " +Jq " . $quality;
  }
  
  $command = $command . " " . $file;
  
  $digest = md5_hex($command);
  my $cachefile = $cachepath . $digest . ".jpg";
  
  $command = $command . " " . $cachefile;
  my $gen = false;
  
  unless(-e $cachefile) {
    system($command);
    $gen = true;
  }
  
  use constant BUFFER_SIZE => 4096;

  my $buffer = "";
  
  
  
  #print $query->header(-type=>'image/jpeg', -expires=>'+3d');
  print "Content-type: image/jpeg \n";
  print "xxxx: " . $cachefile . $gen . $sec . "\n";
  print "\n";
  #print "Cache-Control: public, max-age=3600\n";
  #print "File: " . $cachefile;


  binmode STDOUT;

  open FILE, $cachefile or die $!;
  binmode FILE;
  while ( read( FILE, $buffer, BUFFER_SIZE ) ) {
    print $buffer;
  }
  close FILE;
}

<>; #pause