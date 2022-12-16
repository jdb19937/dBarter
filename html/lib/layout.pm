package layout;
use CGI;
use CGI::Cookie;

sub mime_header {
  my %param = @_;
  my $header = CGI::header(map { '-' . $_ => $param{$_} } keys(%param));
  $header =~ s/\r//g;
  $header
}

sub header {
  my %param = (
    bgcolor => '#ffffff',
    @_
  );

  qq{
    <html>
    <head>
    <title>$param{'title'}</title>
    </head>
    
    <body bgcolor="$param{'bgcolor'}">
    <img src="images/dbarter.gif">
    <hr noshade>
    [
      <a href="login.cgi">Login</a> |
      <a href="register.cgi">Create New Account</a> |
      <a href="balance.cgi">View Portfolio</a> |
      <a href="denom_search.cgi">Search Claims</a> |
      <a href="offer_create.cgi">Place Offer</a> |
      <a href="offer_list.cgi">View Open Offers</a> |
      <a href="quote.cgi">Get Quotes</a>
    ]
    <h1 align=center><code>$param{'title'}</code></h1>
  }
}

sub footer {
  my %param = (
    @_
  );
  
  qq{
    <hr noshade>
    Created by:
    <i>Dan Brumleve</i> <code>&lt;<a href="mailto:nothing\@shout.net">nothing\@shout.net</a>&gt;</code>
    </body>
    </html>
  }
}

sub error {
  my ($text, %param) = @_;
  
  qq{
    <font color="#ff0000"><code>$text</code></font>
  }
}

1;
