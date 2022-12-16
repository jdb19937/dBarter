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
my $pattern = $cgi->param("search");
my @matches;

if ($pattern) {
  try {
    @matches = $client->denom_search($pattern);
  } catch {
    session::error;
  };
} elsif ($cgi->param("submit") eq "List All Claims") {
  try {
    @matches = $client->denom_list_all;
  } catch {
    session::error;
  };
}

print layout::mime_header;
print layout::header('title' => "Claim Search");

print qq{
  <form action="denom_search.cgi" method="post">
  Search String: <input type=text size=40 name=search value="$pattern">
  <input name=submit type=submit value="Search">
  <input name=submit type=submit value="List All Claims">
  </form>
};

if ($pattern || @matches) {
  print "<hr>\n";
  if (@matches) {
    print "<ol>\n";
    for my $match (@matches) {
      print "<li><code><a href=\"denom_read.cgi?denom=" .
       $match->id . "\">". $match->id . "</a></code>\n";
    }
    print "</ol>\n";
  } else {
    print "No matches found.\n";
  }
}

print layout::footer;
