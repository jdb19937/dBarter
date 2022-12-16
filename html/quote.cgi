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

my $denomid = $cgi->param('denom');
my $au = new bom::denom 'au';
my $denom = new bom::denom $denomid;
my $buy_offer, $sell_offer;
my @buy_offers = ( );
my @sell_offers = ( );
my $buy_offer;
my $sell_offer;

if ($denomid) {
  try {
    @buy_offers = $client->offer_list_by_denom($denom, $au);
    @sell_offers = $client->offer_list_by_denom($au, $denom);
    $buy_offer = $buy_offers[0];
    $sell_offer = $sell_offers[0];
    $client->offer_read($buy_offer) if $buy_offer;
    $client->offer_read($sell_offer) if $sell_offer;
  } catch {
    session::error;
  };
}

print layout::mime_header;
print layout::header('title' => "Quote: $denomid");

print qq{
  <form action="quote.cgi" method="post">
  Claim: <input type=text size=40 name=denom value="$denomid">
  </form>
};

if ($denomid) {
  print "<hr><pre>\n";
  print "Claim: $denomid\n";

  if ($buy_offer) {
    my $ask_quant = $buy_offer->bid_value->quant / $buy_offer->ask_value->quant;
    print "Ask:   " . '*{' . $ask_quant . '}' . "\n";
  } else {
    print "Ask:   n/a\n";
  }
  if ($sell_offer) {
    my $bid_quant = $sell_offer->ask_value->quant /
     $sell_offer->bid_value->quant;
    print "Bid:   " . '*{' . $bid_quant . '}' . "\n";
  } else {
    print "Bid:   n/a\n";
  }
  print "</pre>\n";
}

print layout::footer;
