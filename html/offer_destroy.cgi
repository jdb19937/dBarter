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

my $offerid = $cgi->param('offer');
if ($offerid) {
  my $offer = new bom::offer $offerid;
  try {
    $client->offer_destroy($offer);
  } catch {
    session::error;
  };
}

print layout::mime_header;
print layout::header('title' => "Cancel Offer");

if ($offerid) {
  print "Offer Cancelled: <code>" . $offerid . "</code>\n";
}

print layout::footer;
