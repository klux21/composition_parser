# Changelog of the Data Composition parser

## composition_parser_1.1.2 / 2026-07-08
 - get_C_char is nonstatic now and part of the API
 - the tests are using the %v option of callback_print with pvc_unescape for a sample of an
   unquoted and unescaped output while leaving the file buffer unchanged for an subsequent second iteration.

## composition_parser_1.1.1 / 2026-07-03
 - Additional argument bFindBlockEnd for bIniEntryFind added that specifies whether the end of a block
   should be searched or whether the start of the block should be returned only.
 - The zero-copy parsing of the test data in composition_test.c simplified.

## composition_parser_1.1.0 / 2026-07-02
 - pIniFindNextSection takes the address of the iterating character pointer as it's first argument now.
   This makes the code for iterating the file sections a lot easier now. 
 - Windows console output of the test project supports UTF-8 now.  

## composition_parser_1.0.1 / 2026-06-29

 - Earlier versions of the standard allowed entry names followed by a blocks without a
   mandatory `=` that makes the entry name the name of the following block.
   This caused ambiguity between intentionally unnamed blocks and blocks who were intended
   to represent an entries value.

   To resolve this, the opening curly brace of a block `{` must be preceded by a `=` now for
   becoming the value of an entry. If not preceded by a `=` blocks are still valid entries
   but the name of such a block is empty now.

## composition_parser_1.0.0 / 2026-06-28

 - Initial version that supports the data composition format standard.
