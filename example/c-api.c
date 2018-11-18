#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "prettyprint.h"

void print_basic() {
    pp_doc* basic = pp_appends(
            pp_words("hello world"),
            pp_nest(4, pp_appends(
                    pp_line(), pp_words("indented"),
                    pp_nest(4, pp_appends(
                            pp_line(), pp_words("more indented"),
                            pp_line(), pp_words("across lines"))),
                    pp_line(), pp_words("and with really long lines with many words including "
                                        "veryveryveryverylongandcontiguouswordswhichneedwrapping")
                    ))
            );

    pp_settings settings = {0};
    settings.width = 40;
    settings.max_indent = 20;
    pp_pretty(stdout, &settings, basic);
    fprintf(stdout, "\n\n");

    pp_free(basic);
}

void print_grouped() {
    pp_doc* grouped = pp_group(
            pp_appends(pp_string("one"), pp_line(),
                pp_string("two"), pp_line(),
                pp_string("three"), pp_line(),
                pp_string("four")));

    pp_settings settings = {0};
    settings.width = 40;
    settings.max_indent = 20;
    pp_pretty(stdout, &settings, grouped);
    fprintf(stdout, "\n\n");

    settings.width = 15;
    settings.max_indent = 4;
    pp_pretty(stdout, &settings, grouped);
    fprintf(stdout, "\n\n");

    pp_free(grouped);
}

enum {
    PP_DOC_OWNED_TEXT = PP_DOC_EXTENSION_START,
    PP_DOC_TIME,
    PP_DOC_FILTERED
} doc_type_extensions_t;

// Create an owned string which behaves like a text document but will be free'd
// by the free_ext function.
pp_doc* pp_owned_string(const char* text) {
    pp_doc* d = pp_string(text);
    d->type = PP_DOC_OWNED_TEXT;
    return d;
}

// We can reuse a single pp_doc_text for times since the text values are only
// being used to return text at pretty-print time.
pp_doc_text _pp_time = { PP_DOC_TIME };

// Create a document which displays the time when pretty-printed.
pp_doc* pp_time() {
    return (pp_doc*)&_pp_time;
}

typedef struct {
    pp_doc_type_t type;
    int v;
    const pp_doc* inner;
} pp_doc_filtered;

// Create a document which can be filtered out by a value in the pretty-print
// settings.
pp_doc* pp_filtered(int v, const pp_doc* inner) {
    pp_doc_filtered* d = (pp_doc_filtered*)malloc(sizeof(pp_doc_filtered));
    if (d == NULL) return NULL;
    d->type = PP_DOC_FILTERED;
    d->v = v;
    d->inner = inner;
    return (pp_doc*)d;
}

typedef struct {
    pp_settings s;
    int filter_value;
} pp_settings_ext;

pp_doc_type_t eval_ext(const pp_settings* settings, pp_doc_type_t tp, pp_doc** d) {
    pp_settings_ext* ss = (pp_settings_ext*)settings;
    switch (tp) {
        case PP_DOC_OWNED_TEXT:
            return PP_DOC_TEXT;
        case PP_DOC_TIME:
            if (0) {}
            time_t tm = time(NULL);
            char* timestr = ctime(&tm);
            timestr[strlen(timestr)-1] = '\0'; // Get rid of ending newline
            pp_doc_text* t = (pp_doc_text*)*d;
            t->text = timestr;
            t->length = strlen(timestr);
            return PP_DOC_TEXT;
        case PP_DOC_FILTERED:
            if (0) {}
            pp_doc_filtered* f = (pp_doc_filtered*)*d;
            if (ss->filter_value >= f->v) {
                *d = (pp_doc*)f->inner;
                return (*d)->type;
            }
            else return PP_DOC_NIL;
        default:
            return PP_DOC_NIL;
    }
}

void free_ext(pp_doc* d) {
    switch (d->type) {
        case PP_DOC_OWNED_TEXT:
            free((char*)((pp_doc_text*)d)->text);
            break;
        case PP_DOC_FILTERED:
            pp_free_ext(free_ext, (pp_doc*)((pp_doc_filtered*)d)->inner);
            break;
        case PP_DOC_TIME:
        default:
            return;
    }
    free(d);
}

void print_ext() {
    char* c = (char*)malloc(sizeof(char)*50);
    sprintf(c, "%d %f", 42, 3.14);

    pp_settings_ext esettings = {0};
    esettings.s.width = 80;
    esettings.s.max_indent = 40;
    esettings.s.evaluate_extension = eval_ext;
    esettings.filter_value = 2;

    pp_doc* extensions = pp_appends(
            pp_owned_string(c),
            pp_line(), pp_string("time:"), pp_sep(), pp_time(),
            pp_filtered(1, pp_appends(pp_line(), pp_string("one"))),
            pp_filtered(2, pp_appends(pp_line(), pp_string("two"))),
            pp_filtered(3, pp_appends(pp_line(), pp_string("three"))),
            pp_filtered(4, pp_appends(pp_line(), pp_string("four")))
            );

    pp_pretty(stdout, (const pp_settings*)&esettings, extensions);
    fprintf(stdout, "\n\n");

    pp_free_ext(free_ext, extensions);
}

int main() {
    print_basic();
    print_grouped();
    print_ext();

    return 0;
}

