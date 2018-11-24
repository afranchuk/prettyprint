#include "prettyprint.h"

#define DOCAS(d,n) ((const pp_doc_##n*)(d))

#if PRETTYPRINT_USE_CPP == 0
#define RESTRICT restrict
#else
#define RESTRICT
#endif

static pp_doc _nil = { PP_DOC_NIL };
pp_doc* _pp_nil = &_nil;

static pp_doc _sep = { PP_DOC_SEP };
pp_doc* _pp_sep = &_sep;

void _pp_text(pp_doc_text* RESTRICT result, const char* RESTRICT text, size_t length) {
    result->type = PP_DOC_TEXT;
    result->text = text;
    result->length = length;
}

static pp_doc _line = { PP_DOC_LINE };
pp_doc* _pp_line = &_line;

void _pp_nest(pp_doc_nest* RESTRICT result, size_t indent, const pp_doc* RESTRICT nested) {
    result->type = PP_DOC_NEST;
    result->indent = indent;
    result->nested = nested;
}

void _pp_append(pp_doc_append* RESTRICT result, const pp_doc* RESTRICT a, const pp_doc* RESTRICT b) {
    result->type = PP_DOC_APPEND;
    result->a = a;
    result->b = b;
}

void _pp_group(pp_doc_group* RESTRICT result, const pp_doc* RESTRICT d) {
    result->type = PP_DOC_GROUP;
    result->grouped = d;
}

static int can_flatten(const pp_settings* RESTRICT settings, const pp_doc* RESTRICT d, size_t* RESTRICT remaining) {
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

static void pretty(const pp_writer* RESTRICT writer, const pp_settings* RESTRICT settings, const pp_doc* RESTRICT d, 
        size_t* RESTRICT remaining, size_t indent, int group) {
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
            {
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
            }
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
            {
                if (0) {}
                const pp_doc_nest* n = DOCAS(d,nest);
                size_t newindent = indent + n->indent;
                if (newindent > settings->max_indent) newindent = settings->max_indent;
                pretty(writer, settings, n->nested, remaining, newindent, group);
            }
            break;
        case PP_DOC_APPEND:
            pretty(writer, settings, DOCAS(d,append)->a, remaining, indent, group);
            pretty(writer, settings, DOCAS(d,append)->b, remaining, indent, group);
            break;
        case PP_DOC_GROUP:
            {
                if (0) {}
                size_t r = *remaining;
                const pp_doc* grouped = DOCAS(d,group)->grouped;
                pretty(writer, settings, grouped, remaining, indent, can_flatten(settings, grouped, &r));
            }
            break;
    }
#undef do_write
}

void _pp_pretty(const pp_writer* RESTRICT writer, const pp_settings* RESTRICT settings, const pp_doc* RESTRICT document) {
    size_t remaining = settings->width;
    pretty(writer, settings, document, &remaining, 0, 0);
}

