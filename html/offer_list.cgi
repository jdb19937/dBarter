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


print layout::mime_header;
print layout::header('title' => "Open Offers");
my @matches;
try {
  @matches = $client->offer_list;
  for my $offer (@matches) {
    $client->offer_read($offer);
  }
} catch {
  session::error;
};

print qq{
  <table border=2>
    <tr>
      <th>Offer ID</th>
      <th>Ask</th>
      <th>Bid</th>
      <th>Ask/Bid Ratio</th>
      <th>Bid/Ask Ratio</th>
      <th>Cancel</th>
    </tr>
};

for my $offer (@matches) {
  print "<tr>\n";
  print "<td>" . $offer->id . "</td>\n";
  print "<td>" . $offer->ask_value . "</td>\n";
  print "<td>" . $offer->bid_value . "</td>\n";

  my $ab = dmath::bq->new($offer->ask_value->quant / $offer->bid_value->quant);
  my $ba = dmath::bq->new($offer->bid_value->quant / $offer->ask_value->quant);

  print "<td>" . $ab . "</td>\n";
  print "<td>" . $ba . "</td>\n";
  print "<td><code><a href=\"offer_destroy.cgi?offer=" .
   $offer->id . "\">[cancel]</a></code></td>\n";
  print "</tr>\n";
}

print qq{
  </table>
};

print layout::footer;
