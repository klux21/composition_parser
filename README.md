<p align="center">
  <img src="composition.svg" width="300">
</p>

<h2>A C parser for the tiny, mighty, user friendly, hierarchical data composition format</h2>


This C project is the first implementation of a data composition format parser for parsing
standard conform configurations and communication data.
The format itself isn't limited to C or this implementation of course and the standard
of that format can be found at

https://github.com/klux21/composition

However, this implementation provides a parser for composition format data and files in C
and as a trivial configuration file parser a very different thing than that format itself.

Structured compositions have a very minimal syntax; a document consists of three
types of elements:

  - Entries which consists of a name string that can be followed by an equality sign
    and an argument value.
  - Blocks of subdocuments, as a special type of entry, where the argument is an
    independent composition document enclosed in curly braces.
  - Sections, which divide a data composition into parts and consist of a section name
    enclosed in square brackets, followed by the entries which are belonging to that section.

Blocks can contain sections, and sections can contain blocks as well.
Besides that, there are two types of comments:

  - Line comments that begin with a hash `#` or a semicolon `;` and end at the end
    of the line.
  - Block comments start with a `#*` or `;*` sequence and end with the start sequence in
    reversed order which is `*#` or `*;`.

For a more flexible usage of structured compositions, line feeds are used for terminating
line comments but otherwise they are entry-separating whitespace only.
An example of that format is

```
[server]

host = "localhost"
port = 8080

tls = {
         enabled
         certificate = "/etc/certs/server.pem"

         [ciphers]

         #*
            comment block
         *#

         accept = {
                     TLS_AES_128_CCM_8_SHA256
                     TLS_CHACHA20_POLY1305_SHA256
                     TLS_AES_128_GCM_SHA256
                  }
      }

# line comment

```
That looks a bit like INI except that there is that tls block that contains
a composition of entries which look more or less like an INI file as well.
However, there is a little bit more. 

Equals signs `=` before curly braces aren't really required for the syntax and optional.
The support of sections within compositions ensures compatibility with most existing INI
files but it's also fine to omit sections completely.

That why the configuration file could be more trivial:

```
server {
   host = localhost
   port = 8080

   tls {
      enabled
      certificate = "/etc/certs/server.pem"

      ciphers {
         #* comment block *#
         accept { TLS_AES_128_CCM_8_SHA256  TLS_CHACHA20_POLY1305_SHA256  TLS_AES_128_GCM_SHA256 }
      }
   }
}
# line comment
```

That doesn't match XML, JSON, INI or TOML but a very lightweight, powerful and well structured
configuration format. However, in case of more complex configurations sections may help to
improve the readability.

For dealing with floation points and the binary and octal prefixes 0b and 0o of integers
the tests are using the free open source

 project https://github.com/klux21/str2num

because it' a bad idea to treat numbers with a leading `0` in configuration files as octal values
or don't support hexadecimal, octal or binary number at all because of this.

One of the greatest advantages of composition documents is their very lenient syntax.
The format does not have many special elements. A simple list of whitespace or
line feed separated numbers or words and many existing INI files are already valid composition
documents. The most common file extensions for composition documents are
.ini or .cfg and rarely .cof that indicates a composition file.

The little test project here contains samples.
The C parser itself consists of the C header iniparse.h and the C file inparse.c which have no
dependencies to other libraries and are easy to integrate in all kind of C or C++ projects.

