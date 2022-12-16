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

my $portfolio;
try {
  $portfolio = $client->balance;
} catch {
  session::error;
};

print layout::mime_header;
print layout::header('title' => "Balance");

print qq{
  <table border=3 cellpadding=4>
    <tr>
      <th>Denomination</th>
      <th>Quantity</th>
    </tr>
};

for my $denomid (sort keys %$portfolio) {
  my $quant = $$portfolio{$denomid};
  print qq{
    <tr>
      <td><code><a href="denom_read.cgi?denom=$denomid">$denomid</a></code></td>
      <td><code>$quant</code></td>
    </tr>
  };
}

print qq{
  </table>
};

print layout::footer;
