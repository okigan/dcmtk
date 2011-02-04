#!/usr/bin/perl

use CGI;

$query = CGI->new;


my $input = $query->param('file');
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

print "Content-type: image/png \n";
print "Cache-Control: private, max-age=3600\n";
print "\n";
binmode STDOUT;

while ($output, read( $output, $buffer, 4096) ) {
    print $buffer;
}

<>; #pause