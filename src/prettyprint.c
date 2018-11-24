#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prettyprint.h"
#include "prettyprint_base.c"

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
    while (*text != '\0' && *text != ' ' && *text != '\n') text++;
    if (*text == '\0') {
        if (start == text) return pp_nil();
        return pp_text(start, text - start);
    }
    else {
        pp_doc* rest = pp_words(text+1);
        if (rest == NULL) return NULL;

        pp_doc* s;
        if (*text == '\n') s = pp_line();
        else s = pp_sep();
        
        pp_doc* srest = pp_append(s, rest);
        if (srest == NULL) {
            pp_free(rest);
            pp_free(s);
            return NULL;
        }

        pp_doc* t = pp_text(start, text - start);
        if (t == NULL) {
            pp_free(srest);
            return NULL;
        }

        pp_doc* ret = pp_append(t, srest);
        if (ret == NULL) {
            pp_free(t);
            pp_free(srest);
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

