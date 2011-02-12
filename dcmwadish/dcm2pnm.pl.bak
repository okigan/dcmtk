#!/usr/bin/perl

use CGI;

$query = CGI->new;

print "Content-type: image/png \n\n"; #<-- common http header
binmode STDOUT;

my $input = $query->param('file');
my $output= "E:\\GitViews\\dcmwado\\tmp.png";

#output to a file for all the wrong reasons
my $rc = system("E:\\GitViews\\dcmwado\\dcm2pnm.exe", "+on", "+Wh", "5", $input, $output);

use constant BUFFER_SIZE     => 4096;
use constant IMAGE_DIRECTORY => "E:\\GitViews\\dcmwado\\";

my $buffer = "";

local *IMAGE;
open IMAGE, $output or die "Cannot open file $image: $!";
binmode(IMAGE);
while ( read( IMAGE, $buffer, BUFFER_SIZE ) ) {
    print $buffer;
}
close IMAGE;

<>; #pause