#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prettyprint.h"

#define DOCAS(d,n) ((const pp_doc_##n*)(d))

static pp_doc _nil = { PP_DOC_NIL };
pp_doc* _pp_nil = &_nil;

static pp_doc _sep = { PP_DOC_SEP };
pp_doc* _pp_sep = &_sep;

void _pp_text(pp_doc_text* restrict result, const char* restrict text, size_t length) {
    result->type = PP_DOC_TEXT;
    result->text = text;
    result->length = length;
}

static pp_doc _line = { PP_DOC_LINE };
pp_doc* _pp_line = &_line;

void _pp_nest(pp_doc_nest* restrict result, size_t indent, const pp_doc* restrict nested) {
    result->type = PP_DOC_NEST;
    result->indent = indent;
    result->nested = nested;
}

void _pp_append(pp_doc_append* restrict result, const pp_doc* restrict a, const pp_doc* restrict b) {
    result->type = PP_DOC_APPEND;
    result->a = a;
    result->b = b;
}

void _pp_group(pp_doc_group* restrict result, const pp_doc* restrict d) {
    result->type = PP_DOC_GROUP;
    result->grouped = d;
}

static int can_flatten(const pp_settings* restrict settings, const pp_doc* restrict d, size_t* restrict remaining) {
    // Evaluate extensions
    pp_doc_type_t tp = d->type;
    while (tp >= PP_DOC_EXTENSION_START) {
        if (settings->evaluate_extension == NULL) return 0;
        tp = settings->evaluate_extension(settings, tp, (pp_doc**)&d);
    }

    switch (d->type) {
        case PP_DOC_NIL:
            return 1;
        case PP_DOC_SEP:
            if (*remaining > 0) *remaining -= 1;
            return 1;
        case PP_DOC_TEXT:
            if (*remaining < DOCAS(d,text)->length) return 0;
            *remaining -= DOCAS(d,text)->length;
            return 1;
        case PP_DOC_LINE:
            if (*remaining < 1) return 0;
            *remaining -= 1;
            return 1;
        case PP_DOC_NEST:
            return can_flatten(settings, DOCAS(d,nest)->nested, remaining);
        case PP_DOC_APPEND:
            if (!can_flatten(settings, DOCAS(d,append)->a, remaining)) return 0;
            return can_flatten(settings, DOCAS(d,append)->b, remaining);
        case PP_DOC_GROUP:
            return can_flatten(settings, DOCAS(d,group)->grouped, remaining);
        default:
            return 0;
    }
}

static void pretty(const pp_writer* restrict writer, const pp_settings* restrict settings, const pp_doc* restrict d, 
        size_t* restrict remaining, size_t indent, int group) {
    // Evaluate extensions
    pp_doc_type_t tp = d->type;
    while (tp >= PP_DOC_EXTENSION_START) {
        if (settings->evaluate_extension == NULL) return;
        tp = settings->evaluate_extension(settings, tp, (pp_doc**)&d);
    }

#define do_write(c,l) writer->write(writer->data,c,l)
    switch (tp) {
        case PP_DOC_NIL:
            break;
        case PP_DOC_SEP:
            if (settings->width - indent != *remaining && *remaining != 0) {
                do_write(" ", 1);
                *remaining -= 1;
            }
            break;
        case PP_DOC_TEXT:
            if (DOCAS(d,text)->length > *remaining) {
                pretty(writer, settings, _pp_line, remaining, indent, group);
            }
            const pp_doc_text* t = DOCAS(d,text);
            size_t len = t->length;
            while (len > *remaining) {
                do_write(t->text + (t->length - len), *remaining);
                len -= *remaining;
                *remaining = 0;
                pretty(writer, settings, _pp_line, remaining, indent, group);
            }
            do_write(t->text + (t->length - len), len);
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
            const pp_doc_nest* n = DOCAS(d,nest);
            size_t newindent = indent + n->indent;
            if (newindent > settings->max_indent) newindent = settings->max_indent;
            pretty(writer, settings, n->nested, remaining, newindent, group);
            break;
        case PP_DOC_APPEND:
            pretty(writer, settings, DOCAS(d,append)->a, remaining, indent, group);
            pretty(writer, settings, DOCAS(d,append)->b, remaining, indent, group);
            break;
        case PP_DOC_GROUP:
            if (0) {}
            size_t r = *remaining;
            const pp_doc* grouped = DOCAS(d,group)->grouped;
            pretty(writer, settings, grouped, remaining, indent, can_flatten(settings, grouped, &r));
            break;
    }
#undef do_write
}

void _pp_pretty(const pp_writer* restrict writer, const pp_settings* restrict settings, const pp_doc* restrict document) {
    size_t remaining = settings->width;
    pretty(writer, settings, document, &remaining, 0, 0);
}

pp_doc* pp_nil(void) {
    return _pp_nil;
}

pp_doc* pp_sep(void) {
    return _pp_sep;
}

pp_doc* pp_text(const char* text, size_t length) {
    pp_doc_text* t = (pp_doc_text*)malloc(sizeof(pp_doc_text));
    if (t == NULL) return NULL;
    _pp_text(t, text, length);
    return (pp_doc*)t;
}

pp_doc* pp_line(void) {
    return _pp_line;
}

pp_doc* pp_nest(size_t indent, const pp_doc* nested) {
    pp_doc_nest* n = (pp_doc_nest*)malloc(sizeof(pp_doc_nest));
    if (n == NULL) return NULL;
    _pp_nest(n, indent, nested);
    return (pp_doc*)n;
}

pp_doc* pp_append(const pp_doc* restrict a, const pp_doc* restrict b) {
    pp_doc_append* d = (pp_doc_append*)malloc(sizeof(pp_doc_append));
    if (d == NULL) return NULL;
    _pp_append(d, a, b);
    return (pp_doc*)d;

}

pp_doc* pp_group(const pp_doc* i) {
    pp_doc_group* d = (pp_doc_group*)malloc(sizeof(pp_doc_group));
    if (d == NULL) return NULL;
    _pp_group(d, i);
    return (pp_doc*)d;
}

void pp_free(pp_doc* d) {
    pp_free_ext(NULL, d);
}

void pp_free_ext(void (*free_ext)(pp_doc* d), pp_doc* d) {
    if (d->type >= PP_DOC_EXTENSION_START) {
        if (free_ext != NULL)
            free_ext(d);
        return;
    }
    else {
        switch (d->type) {
            case PP_DOC_TEXT:
                break;
            case PP_DOC_NEST:
                pp_free_ext(free_ext, (pp_doc*)DOCAS(d,nest)->nested);
                break;
            case PP_DOC_APPEND:
                pp_free_ext(free_ext, (pp_doc*)DOCAS(d,append)->a);
                pp_free_ext(free_ext, (pp_doc*)DOCAS(d,append)->b);
                break;
            case PP_DOC_GROUP:
                pp_free_ext(free_ext, (pp_doc*)DOCAS(d,group)->grouped);
                break;
            case PP_DOC_NIL:
            case PP_DOC_SEP:
            case PP_DOC_LINE:
            default:
                return;
        }
        free(d);
    }
}

pp_doc* pp_string(const char* str) {
    return pp_text(str, strlen(str));
}

pp_doc* pp_words(const char* text) {
    const char* start = text;
    while (*text != '\0' && *text != ' ') text++;
    if (*text == '\0') {
        if (start == text) return pp_nil();
        return pp_text(start, text - start);
    }
    else {
        pp_doc* rest = pp_words(text+1);
        if (rest == NULL) return NULL;

        pp_doc* s = pp_sep();
        
        pp_doc* seprest = pp_append(s, rest);
        if (seprest == NULL) {
            pp_free(rest);
            pp_free(s);
            return NULL;
        }

        pp_doc* t = pp_text(start, text - start);
        if (t == NULL) {
            pp_free(seprest);
            return NULL;
        }

        pp_doc* ret = pp_append(t, seprest);
        if (ret == NULL) {
            pp_free(t);
            pp_free(seprest);
            return NULL;
        }
        return ret;
    }
}

static pp_doc* appends_impl(va_list args) {
    pp_doc* d = va_arg(args, pp_doc*);
    if (d == NULL) return pp_nil();

    pp_doc* rest = appends_impl(args);
    if (rest == NULL) return NULL;
    return pp_append(d, rest);
}

pp_doc* pp_appends_impl(size_t list_end, ...) {
    va_list args;
    va_start(args, list_end);
    pp_doc* res = appends_impl(args);
    va_end(args);
    return res;
}

static void write_file(void* f, const char* text, size_t length) {
    fprintf((FILE*)f, "%.*s", length, text);
}

void pp_pretty(FILE* restrict f, const pp_settings* restrict settings, const pp_doc* restrict document) {
    pp_writer w;
    w.data = f;
    w.write = write_file;
    _pp_pretty(&w, settings, document);
}

