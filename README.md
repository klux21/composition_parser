<p align="center">
  <img src="composition.svg" width="300">
</p>

<h2>The first C parser for the tiny but mighty and user-friendly data composition format</h2>


This C project is the first implementation of a data composition format parser for parsing
standard-conforming configurations and communication data. A data composition parser can
read most INI files as well but the format is hierarchical and a much more powerful thing.
The format itself isn't limited to C or this implementation of course. The standard draft
of the format specification can be found at

https://github.com/klux21/composition

However, this implementation provides a first parser for the composition format data and the
configuration files that are relying on that format. Of course this implementation is a very
different thing than the format itself.
Structured data compositions have a very minimal syntax; a document consists of three
types of elements:

  - Entries which consist of a name string that can be followed by an equality sign
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
That looks a little bit like INI except that there is that tls block that contains
a composition of entries which look more or less like an INI file as well.
However, there is a little bit more. 

The support of sections within compositions ensures compatibility with most existing INI
files but it's also fine to omit sections completely. Also strings don't need any quotes
if they don't contain any whitespace or special characters. The most common escape
sequences are supported inside and outside of quotes as well.

That's why the configuration file could be more trivial:

```
server = {
   host = localhost
   port = 8080

   tls = {
      enabled
      certificate = /etc/certs/server.pem

      ciphers = {
         #* comment block *#
         accept = { TLS_AES_128_CCM_8_SHA256  TLS_CHACHA20_POLY1305_SHA256  TLS_AES_128_GCM_SHA256 }
      }
   }
}
# line comment
```

That doesn't match XML, JSON, INI or TOML but is a very lightweight, powerful and well structured
configuration format. However, in case of more complex configurations sections may help to improve
the readability.

For a platform independent parsing of numbers and ensuring the support the binary and octal
prefixes 0b and 0o for integers and doubles the tests are using the free open source project

 project https://github.com/klux21/str2num

It's a bad idea to treat numbers with a leading `0` in configuration files as octal values
or don't support hexadecimal, octal or binary numbers at all just to prevent that.
The platform independent klux21/str2num handles the prefixes better for all kind of integers,
double and long double.

For platform independ fprintf format string the following project is used

 project https://github.com/klux21/callback_printf

You can execute run_test.sh for running the tests in Unix compatible operating systems.
For Microsoft Visual C++ there exist a Visual Studio project in the VS2010 directory.
klux21/str2num and klux21/callback_printf are expected in parallel directories.

The little test project `composition_test.c` contains several usage samples.
It uses "Civil Usage Public License" as klux21/callback_printf does.
Of course parser uses the very lenient zlib license as klux21/str2num does.

The code of the parser is quite new but already works like a charm in all of my tests.
However, it may change quite a bit in future if the requirements and the number of features grow.

The file reading test in composition_test.c iterates the content of the file `composition_test.ini` only
and prints the found elements and their types to stdout. It's easy to adjust the content of that
configuration file to check what's possible or causes errors.

The parser itself consists of the C header `iniparse.h` and the C file `iniparse.c` only.
Both have no dependencies to other libraries and are easy to integrate in all kind of C or C++
projects for platform-independent reading of configuration files.
'iniparse' because the data composition format was initially intended as a small enhancement of an
existing INI file parser but is a little bit more than that now.
