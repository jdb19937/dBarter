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

my $bs = $cgi->param("bs");
my $offer;
my ($ask, $bid);

if (defined($bs)) {
  my $denomid = $cgi->param("denom");
  my $quant0txt = $cgi->param("quant0");
  my $quant1txt = $cgi->param("quant1");
  my $quant0 = new dmath::bq $quant0txt;
  my $quant1 = new dmath::bq $quant1txt;

  my $denom = new bom::denom $denomid;
  my $au = new bom::denom 'au';

  if ($bs) {
    $ask = new bom::value $denom, $quant0;
    $bid = new bom::value $au, $quant1 * $quant0;
  } else {
    $bid = new bom::value $denom, $quant0;
    $ask = new bom::value $au, $quant1 * $quant0;
  }

  try {
    $offer = $client->offer_create($ask, $bid);
  } catch {
    session::error("Cannot create offer.");
  };
}

print layout::mime_header;
print layout::header('title' => "Place New Offer");

print qq{
  <form action="offer_create.cgi" method="post">
  <select name="bs"><option value=1>Buy<option value=0>Sell</select>
  <input type=text name=quant0 size=10> units of
  <input type=text name=denom size=20><br> at
  <input type=text name=quant1> <code>*</code> per unit.<br>
  <input type=submit value="Place Offer">
  </form>
};

if ($offer) {
  print "Offer Created: <code>" . $offer->id . "</code>\n";
}

print layout::footer;
