Uniform Resource Identifier (URI): Generic Syntax

   The following example URIs illustrate several URI schemes and
   variations in their common syntax components:

      ftp://ftp.is.co.za/rfc/rfc1808.txt

      http://www.ietf.org/rfc/rfc2396.txt

      ldap://[2001:db8::7]/c=GB?objectClass?one

      mailto:John.Doe@example.com

      news:comp.infosystems.www.servers.unix

      tel:+1-816-555-1212

      telnet://192.0.2.16:80/

      urn:oasis:names:specification:docbook:dtd:xml:4.1.2

Syntax Components

   The generic URI syntax consists of a hierarchical sequence of
   components referred to as the scheme, authority, path, query, and
   fragment.

      URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]

      hier-part   = "//" authority path-abempty
                  / path-absolute
                  / path-rootless
                  / path-empty

   The scheme and path components are required, though the path may be
   empty (no characters).  When authority is present, the path must
   either be empty or begin with a slash ("/") character.  When
   authority is not present, the path cannot begin with two slash
   characters ("//").  

   The following are two example URIs and their component parts:

         foo://example.com:8042/over/there?name=ferret#nose
         \_/   \______________/\_________/ \_________/ \__/
          |           |            |            |        |
       scheme     authority       path        query   fragment
          |   _____________________|__
         / \ /                        \
         urn:example:animal:ferret:nose


    scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
    authority   = [ userinfo "@" ] host [ ":" port ]
    userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
    host        = IP-literal / IPv4address / reg-name
    IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
    IPvFuture  = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
    IPv6address =                            6( h16 ":" ) ls32
                /                       "::" 5( h16 ":" ) ls32
                / [               h16 ] "::" 4( h16 ":" ) ls32
                / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
                / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
                / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
                / [ *4( h16 ":" ) h16 ] "::"              ls32
                / [ *5( h16 ":" ) h16 ] "::"              h16
                / [ *6( h16 ":" ) h16 ] "::"

    ls32        = ( h16 ":" h16 ) / IPv4address
                ; least-significant 32 bits of address

    h16         = 1*4HEXDIG
                ; 16 bits of address represented in hexadecimal
----
    IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet

    dec-octet   = DIGIT                 ; 0-9
                / %x31-39 DIGIT         ; 10-99
                / "1" 2DIGIT            ; 100-199
                / "2" %x30-34 DIGIT     ; 200-249
                / "25" %x30-35          ; 250-255
----
    reg-name    = *( unreserved / pct-encoded / sub-delims )
    port        = *DIGIT
    path        = path-abempty    ; begins with "/" or is empty
                / path-absolute   ; begins with "/" but not "//"
                / path-noscheme   ; begins with a non-colon segment
                / path-rootless   ; begins with a segment
                / path-empty      ; zero characters

    path-abempty  = *( "/" segment )
    path-absolute = "/" [ segment-nz *( "/" segment ) ]
    path-noscheme = segment-nz-nc *( "/" segment )
    path-rootless = segment-nz *( "/" segment )
    path-empty    = 0<pchar>
    segment       = *pchar
    segment-nz    = 1*pchar
    segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
                ; non-zero-length segment without any colon ":"

    pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
----
    query       = *( pchar / "/" / "?" )
    fragment    = *( pchar / "/" / "?" )
----
    URI-reference = URI / relative-ref
    relative-ref  = relative-part [ "?" query ] [ "#" fragment ]

    relative-part   = "//" authority path-abempty
                    / path-absolute
                    / path-noscheme
                    / path-empty
----
    absolute-URI  = scheme ":" hier-part [ "?" query ]
___
    unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
    reserved    = gen-delims / sub-delims
    gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
    sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
                  / "*" / "+" / "," / ";" / "="
    pct-encoded = "%" HEXDIG HEXDIG