#!/usr/bin/perl

use lib 'lib';

use config;
use layout;

print layout::mime_header;
print layout::header('title' => "Welcome");

print layout::footer;
