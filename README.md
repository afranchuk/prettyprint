# C/C++ pretty printing library

This library (or libraries, there being one library for C and one with C++
bindings) implements the interface described in Wadler's paper "[A
Prettier Printer][pretty]".

## Differences from the paper

The pretty-printing combinators do not reduce or simplify when called so that
the base C API may exist without making any memory allocations. All reduction is
done when pretty-printing, which is likely an acceptable tradeoff since one is
likely to only pretty-print the structure once, or in some cases maybe a few
times with varying settings and/or different outputs.

Not all higher-level functions shown in the paper are implemented, but the basic
`nil`, `text`, `sep` (not explicitly mentioned in the paper, but often found in
implementations as a soft line break), `line`, `nest`, `append`, and `group` are
implemented, as well as some functions to make the C api more friendly.

## C API

The C api is documented in [prettyprint.h][c-api]. It has a basic set of
functions that make no memory allocations, and then equivalent functions that
use malloc. The intent is that most users use the malloc-ing functions.

### Memory Ownership

The provided `pp_free` function assumes nested documents are owned by the parent
document, but other than that makes no assumptions about ownership. This is
particularly important with regard to the `text` document: it does *not* assume
the string is owned (and thus you may leak memory if they are not cleaned up
properly). Use extensions to introduce memory ownership semantics.

### Extensions

You may specify custom document types. These types must be at or above
`PP_DOC_EXTENSION_START`. These documents may be allocated however you choose,
and may contain any extra data. Read some more detail in the documentation for
the `evaluate_extension` member of the `pp_settings` struct.

The gist of it is that you may specify an extension evaluator in the
pretty-print settings, and this evaluator is used when extensions are
encountered. It also gets a reference to the settings, so you may add extra
settings used by extensions to a custom settings object (that has `pp_settings`
somewhere in it). See the [c example][cex] for inspiration. That example adds
document types that

* take memory ownership of a string (assuming `pp_free_ext` is used),
* print the time when pretty-printed, and
* can be filtered based on an added setting.

## C++ API

Coming soon!


[pretty]: https://homepages.inf.ed.ac.uk/wadler/papers/prettier/prettier.pdf
[c-api]: src/prettyprint.h
[cex]: example/c-api.c
