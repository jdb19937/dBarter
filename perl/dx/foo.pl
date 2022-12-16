#!/usr/bin/perl

use Data::Dumper;

use dx;

my $log = new dx::log 'path' => '/tmp/danslog';
$log->open;

my $x = new dx::x 'log' => $log; 
my $y = new dx::x 'log' => $log;

print "x->open: " . $x->open . "\n";
#print "y->open: " . $y->open . "\n";

print "x->lock_rw: " . $x->lock_rw('dan') . "\n";
print "y->lock_rw: " . $y->lock_rw('dan') . "\n";

print "x->unlock: " . $x->unlock('dan') . "\n";
#print "y->lock_rw: " . $y->lock_rw('dan') . "\n";
#print "y->unlock: " . $y->unlock('dan') . "\n";
print "x->commit: " . $x->commit . "\n";
#print "y->abort: " . $y->abort . "\n";

$log->close;
exit 0;
