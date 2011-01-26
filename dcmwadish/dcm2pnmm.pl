#!/usr/bin/perl

use CGI;

$query = CGI->new;


my $input = $query->param('file');
my $output;

open($output, "E:\\GitViews\\dcmwado\\dcm2pnm.exe +on +Wh 5 " . $input . " | ");

use constant BUFFER_SIZE => 4096;

my $buffer = "";

print "Content-type: image/png \n\n";
binmode STDOUT;

while ($output, read( $output, $buffer, 4096) ) {
    print $buffer;
}

<>; #pause