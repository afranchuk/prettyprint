#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prettyprint.h"

void _pp_nil(doc* result) {
    result->type = PP_DOC_NIL;
}

void _pp_sep(doc* result) {
    result->type = PP_DOC_SEP;
}

void _pp_text(doc* restrict result, const char* restrict text, size_t length) {
    result->type = PP_DOC_TEXT;
    result->text = text;
    result->length = length;
}

void _pp_line(doc* result) {
    result->type = PP_DOC_LINE;
}

void _pp_nest(doc* restrict result, size_t indent, const doc* restrict nested) {
    result->type = PP_DOC_NEST;
    result->indent = indent;
    result->doc_nested = nested;
}

void _pp_append(doc* restrict result, const doc* restrict a, const doc* restrict b) {
    result->type = PP_DOC_APPEND;
    result->doc_a = a;
    result->doc_b = b;
}

void _pp_group(doc* restrict result, const doc* restrict d) {
    result->type = PP_DOC_GROUP;
    result->doc_grouped = d;
}

static int can_flatten(const doc* restrict d, size_t* restrict remaining) {
    switch (d->type) {
        case PP_DOC_NIL:
            return 1;
        case PP_DOC_SEP:
            if (*remaining > 0) *remaining -= 1;
            return 1;
        case PP_DOC_TEXT:
            if (*remaining < d->length) return 0;
            *remaining -= d->length;
            return 1;
        case PP_DOC_LINE:
            if (*remaining < 1) return 0;
            *remaining -= 1;
            return 1;
        case PP_DOC_NEST:
            return can_flatten(d->doc_nested, remaining);
        case PP_DOC_APPEND:
            if (!can_flatten(d->doc_a, remaining)) return 0;
            return can_flatten(d->doc_b, remaining);
        case PP_DOC_GROUP:
            return can_flatten(d->doc_grouped, remaining);
        default:
            return 0;
    }
}

static void pretty(const pp_writer* restrict writer, const pp_settings* restrict settings, const doc* restrict d, 
        size_t* restrict remaining, size_t indent, int group) {
#define do_write(c,l) writer->write(writer->data,c,l)
    switch (d->type) {
        case PP_DOC_NIL:
            break;
        case PP_DOC_SEP:
            if (settings->width - indent != *remaining && *remaining != 0) {
                do_write(" ", 1);
                *remaining -= 1;
            }
            break;
        case PP_DOC_TEXT:
            if (d->length > *remaining) {
                doc l;
                _pp_line(&l);
                pretty(writer, settings, &l, remaining, indent, group);
            }
            size_t len = d->length;
            while (len > *remaining) {
                do_write(d->text + (d->length - len), *remaining);
                len -= *remaining;
                *remaining = 0;
                doc l;
                _pp_line(&l);
                pretty(writer, settings, &l, remaining, indent, group);
            }
            do_write(d->text + (d->length - len), len);
            *remaining -= len;
            break;
        case PP_DOC_LINE:
            if (group) {
                do_write(" ", 1);
                *remaining -= 1;
            }
            else {
                do_write("\n", 1);
                for (size_t i = 0; i < indent; i++) do_write(" ", 1);
                *remaining = settings->width - indent;
            }
            break;
        case PP_DOC_NEST:
            if (0) {}
            size_t newindent = indent + d->indent;
            if (newindent > settings->max_indent) newindent = settings->max_indent;
            pretty(writer, settings, d->doc_nested, remaining, newindent, group);
            break;
        case PP_DOC_APPEND:
            pretty(writer, settings, d->doc_a, remaining, indent, group);
            pretty(writer, settings, d->doc_b, remaining, indent, group);
            break;
        case PP_DOC_GROUP:
            if (0) {}
            size_t r = *remaining;
            pretty(writer, settings, d->doc_grouped, remaining, indent, can_flatten(d->doc_grouped, &r));
            break;
    }
#undef do_write
}

void _pp_pretty(const pp_writer* restrict writer, const pp_settings* restrict settings, const doc* restrict document) {
    size_t remaining = settings->width;
    pretty(writer, settings, document, &remaining, 0, 0);
}

static doc* alloc_doc(void) {
    return (doc*)malloc(sizeof(doc));
}

doc* pp_nil(void) {
    doc* d = alloc_doc();
    if (d == NULL) return NULL;
    _pp_nil(d);
    return d;
}

doc* pp_sep(void) {
    doc* d = alloc_doc();
    if (d == NULL) return NULL;
    _pp_sep(d);
    return d;
}

doc* pp_text(const char* text, size_t length) {
    doc* d = alloc_doc();
    if (d == NULL) return NULL;
    _pp_text(d, text, length);
    return d;
}

doc* pp_line(void) {
    doc* d = alloc_doc();
    if (d == NULL) return NULL;
    _pp_line(d);
    return d;
}

doc* pp_nest(size_t indent, const doc* nested) {
    doc* d = alloc_doc();
    if (d == NULL) return NULL;
    _pp_nest(d, indent, nested);
    return d;
}

doc* pp_append(const doc* restrict a, const doc* restrict b) {
    doc* d = alloc_doc();
    if (d == NULL) return NULL;
    _pp_append(d, a, b);
    return d;

}

doc* pp_group(const doc* i) {
    doc* d = alloc_doc();
    if (d == NULL) return NULL;
    _pp_group(d, i);
    return d;
}

void pp_free(doc* d) {
    switch (d->type) {
        case PP_DOC_NIL:
        case PP_DOC_SEP:
        case PP_DOC_TEXT:
        case PP_DOC_LINE:
            break;
        case PP_DOC_NEST:
            pp_free((doc*)d->doc_nested);
            break;
        case PP_DOC_APPEND:
            pp_free((doc*)d->doc_a);
            pp_free((doc*)d->doc_b);
            break;
        case PP_DOC_GROUP:
            pp_free((doc*)d->doc_grouped);
            break;
        default:
            break;
    }

    free(d);
}

doc* pp_string(const char* str) {
    return pp_text(str, strlen(str));
}

doc* pp_words(const char* text) {
    const char* start = text;
    while (*text != '\0' && *text != ' ') text++;
    if (*text == '\0') {
        if (start == text) return pp_nil();
        return pp_text(start, text - start);
    }
    else {
        doc* rest = pp_words(text+1);
        if (rest == NULL) return NULL;

        doc* s = pp_sep();
        if (s == NULL) {
            pp_free(rest);
            return NULL;
        }
        
        doc* seprest = pp_append(s, rest);
        if (seprest == NULL) {
            pp_free(rest);
            pp_free(s);
            return NULL;
        }

        doc* t = pp_text(start, text - start);
        if (t == NULL) {
            pp_free(seprest);
            return NULL;
        }

        doc* ret = pp_append(t, seprest);
        if (ret == NULL) {
            pp_free(t);
            pp_free(seprest);
            return NULL;
        }
        return ret;
    }
}

static doc* appends_impl(va_list args) {
    doc* d = va_arg(args, doc*);
    if (d == NULL) return pp_nil();

    doc* rest = appends_impl(args);
    if (rest == NULL) return NULL;
    return pp_append(d, rest);
}

doc* pp_appends_impl(size_t list_end, ...) {
    va_list args;
    va_start(args, list_end);
    doc* res = appends_impl(args);
    va_end(args);
    return res;
}

static void write_file(void* f, const char* text, size_t length) {
    fprintf((FILE*)f, "%.*s", length, text);
}

void pp_pretty(FILE* restrict f, const pp_settings* restrict settings, const doc* restrict document) {
    pp_writer w;
    w.data = f;
    w.write = write_file;
    _pp_pretty(&w, settings, document);
}

