#!/usr/bin/perl
use strict;
use warnings;

use XML::LibXML;

my $filename = $ARGV[0];

my $parser = XML::LibXML->new();
my $xmldoc = $parser->parse_file($filename);


print "PROBLEM\tINSTANCE\tEXECUTABLE\tSTATUS\tTIME\tMEMORY\tEXIT_CODE\tCHECK\n";

for my $sample ($xmldoc->findnodes('//benchmark'))
{
	for my $problem ( $sample->findnodes('./@id') )
	{
		for my $instance ($sample->findnodes('./testcase'))
		{			
			my @a = $instance->attributes();
			my $inst = $a[ 0 ]->textContent();
			
			for my $executable ($instance->findnodes('./command'))
			{	
				my @attributelist = $executable->attributes();
				my $exec = $attributelist[ 0 ]->textContent();

				my @stats = $executable->findnodes( './pyrunlim/stats' );
				my $i = $stats[ 0 ];
				my $status = $i->findnodes('@status');
				#my $time = $i->findnodes('@time');
				my $time = $i->findnodes('@real');
				my $memory = $i->findnodes('@memory');
				my $result = $i->findnodes('@result');
				$status =~ s/\s/-/;
				$status =~ s/\s/-/;
				$time =~ s/\./,/;
 				$memory =~ s/\./,/;
				my @checker = $executable->findnodes( './validator' );
				my $j = $checker[ 0 ];
				my $check = $j->findnodes('@response');

				print $problem->textContent(), "\t$inst\t$exec\t$status\t$time\t$memory\t$result\t$check\n";
			}
		}		
	}
}
