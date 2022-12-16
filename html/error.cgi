#!/usr/bin/perl

use lib 'lib';

use CGI;
use config;
use layout;

my $cgi = new CGI;
my $message = $cgi->param("message") || "An unrecoverable error has occurred.";

print layout::mime_header;
print layout::header('title' => "Welcome");

print qq{
  <center>
  <font color="#ff0000">$message</font>
  </center>
};

print layout::footer;
