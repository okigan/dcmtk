#!/usr/bin/perl

binmode(STDOUT);

print "Content-type: image/png\n\n"; #<-- common http header

my $output;

open($output, "E:\\GitViews\\dcmwado\\dcm2pnm.exe +on +Wh 5 CT.dcm | ");

my $buffer = "";

local *IMAGE;

while ($output, read( $output, $buffer, 4096) ) {
    print $buffer;
}
close IMAGE;

<>; #pause