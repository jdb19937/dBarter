#!/usr/bin/perl

use lib 'lib';

use CGI;
use bom;
use bom::txn;
use bif;
use session;
use config;
use layout;

my $client;
try {
  $client = session::restore;
} catch {
  $_ eq 'nosession' and session::error("Not connected.  Please login.");
  session::error;
};

my $cgi = new CGI;
my $denomid = $cgi->param("denom");
my $denom = new bom::denom $denomid;

try {
  $client->denom_read($denom);
} catch {
  session::error;
};

print layout::mime_header;
print layout::header('title' => "Claim Info: $denomid");

print "<pre>" . $denom->promise . "</pre>\n";

print layout::footer;
