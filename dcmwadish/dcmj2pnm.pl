#!/usr/bin/perl

use CGI;
#use File::Basename;
#, -attachment=>basename($input));

$query = CGI->new;


my $input = $query->param('file');

if(length($input) <= 0){
  print "Content-type: text/html \n";
  print "\n";

  ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
  printf "%4d-%02d-%02d %02d:%02d:%02d\n",$year+1900,$mon+1,$mday,$hour,$min,$sec;
}else{
  my $width = $query->param('width');
  my $quality = $query->param('quality');
  my $output;

  my $command = "E:\\GitViews\\dcmtk\\dcmwadish\\dcmj2pnm.exe +oj +Wh 5 ";

  if($width){
    $command = $command . " +Sxv " . $width;
  }

  if($quality){
    $command = $command . " +Jq " . $quality;
  }

  open($output, $command . " " . $input . " | ");

  use constant BUFFER_SIZE => 4096;

  my $buffer = "";
  
  #print "Content-type: image/jpeg \n";
  #print "Cache-Control: public, max-age=3600\n";
  print $query->header(-type=>'image/jpeg', -expires=>'+3d');
  #print "\n";
  binmode STDOUT;

  while ($output, read( $output, $buffer, 4096) ) {
      print $buffer;
  }
}

<>; #pause