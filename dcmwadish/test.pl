#!/usr/bin/perl
print "Content-type: image/png \n\n"; #<-- common http header
binmode STDOUT;

my $rc = system("E:\\GitViews\\dcmwado\\dcm2pnm.exe", "+on", "+Wh", "5", "CT.dcm", "E:\\GitViews\\dcmwado\\out.png");

use constant BUFFER_SIZE     => 4096;
use constant IMAGE_DIRECTORY => "E:\\GitViews\\dcmwado\\";

my $buffer = "";

local *IMAGE;
open IMAGE, IMAGE_DIRECTORY . "out.png" or die "Cannot open file $image: $!";
binmode(IMAGE);
while ( read( IMAGE, $buffer, BUFFER_SIZE ) ) {
    print $buffer;
}
close IMAGE;

<>; #pause