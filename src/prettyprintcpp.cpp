#include <cstring>

#include "prettyprint.h"

namespace pp {

namespace {
#include "prettyprint_base.c"
}

namespace data {

doc_text::doc_text(const char* t, size_t length) {
    _pp_text(static_cast<pp_doc_text*>(this), t, length);
}

doc_text::doc_text(const char* str) {
    _pp_text(static_cast<pp_doc_text*>(this), str, std::strlen(str));
}

doc_string::doc_string(const std::string& s)
    : s(s)
{
    _pp_text(static_cast<pp_doc_text*>(this), this->s.data(), this->s.size());
}

doc_nest::doc_nest(size_t indent, std::shared_ptr<const doc> nested)
    : s_nested(nested)
{
    _pp_nest(static_cast<pp_doc_nest*>(this), indent, s_nested.get());
}

void doc_nest::set_nested(std::shared_ptr<const doc> nested) {
    s_nested = nested;
    this->nested = s_nested.get();
}


doc_append::doc_append(std::shared_ptr<const doc> a, std::shared_ptr<const doc> b)
    : s_a(a)
    , s_b(b)
{
    _pp_append(static_cast<pp_doc_append*>(this), s_a.get(), s_b.get());
}

doc_group::doc_group(std::shared_ptr<const doc> grouped)
    : s_grouped(grouped)
{
    _pp_group(static_cast<pp_doc_group*>(this), s_grouped.get());
}

static std::shared_ptr<doc> get_words(const char* t) {
    const char* start = t;
    while (*t != '\0' && *t != ' ' && *t != '\n') t++;
    if (*t == '\0') {
        if (start == t) return nil();
        return text(start, t - start);
    }
    else {
        auto rest = get_words(t+1);

        std::shared_ptr<doc> s;
        if (*t == '\n') s = line();
        else s = sep();

        return text(start, t - start) + s + rest;
    }
}

doc_words::doc_words(const std::string& s)
    : doc_nest(0, nullptr)
    , s(std::move(s))
{
    set_nested(get_words(this->s.data()));
}

}

template <typename T, typename... Args>
static std::shared_ptr<doc> make_shared_d(Args&&... args) {
    auto t = std::shared_ptr<T>(new T(std::forward<Args>(args)...), std::default_delete<T>());
    return std::shared_ptr<doc>(t, t->as_doc());
}

static void no_delete(doc*) {}

template <typename T>
static std::shared_ptr<T> make_shared_static(T* v) {
    return std::shared_ptr<T>(v, no_delete);
}

std::shared_ptr<doc> nil() {
    return make_shared_static((doc*)_pp_nil);
}

std::shared_ptr<doc> sep() {
    return make_shared_static((doc*)_pp_sep);
}

std::shared_ptr<doc> text(const char* t, size_t length) {
    return make_shared_d<data::doc_text>(t, length);
}

std::shared_ptr<doc> text(const char* str) {
    return make_shared_d<data::doc_text>(str);
}

std::shared_ptr<doc> text(const std::string& s) {
    return make_shared_d<data::doc_string>(s);
}

std::shared_ptr<doc> line() {
    return make_shared_static((doc*)_pp_line);
}

std::shared_ptr<doc> nest(size_t indent, std::shared_ptr<const doc> nested) {
    return make_shared_d<data::doc_nest>(indent, nested);
}

std::shared_ptr<doc> append(std::shared_ptr<const doc> a, std::shared_ptr<const doc> b) {
    return make_shared_d<data::doc_append>(a, b);
}

std::shared_ptr<doc> group(std::shared_ptr<const doc> grouped) {
    return make_shared_d<data::doc_group>(grouped);
}

std::shared_ptr<doc> operator+(std::shared_ptr<const doc> a, std::shared_ptr<const doc> b) {
    return append(a, b);
}

std::shared_ptr<doc> operator+(std::shared_ptr<const doc> a, const std::string& b) {
    return a + words(b);
}

/** Aliases of append that add a separator. */
std::shared_ptr<doc> operator<<(std::shared_ptr<const doc> a, std::shared_ptr<const doc> b) {
    return a + sep() + b;
}

std::shared_ptr<doc> words(const std::string& words) {
    return make_shared_d<data::doc_words>(words);
}

std::shared_ptr<doc> operator<<(std::shared_ptr<const doc> a, const std::string& w) {
    return a << words(w);
}

settings::settings()
{
    width = 80;
    max_indent = 40;
    evaluate_extension = NULL;
}

change_settings::change_settings() {}

change_settings change_settings::set_width(size_t width) {
    change_settings s;
    s.field = F_WIDTH;
    s.width = width;
    return s;
}

change_settings change_settings::set_max_indent(size_t indent) {
    change_settings s;
    s.field = F_MAX_INDENT;
    s.max_indent = indent;
    return s;
}

change_settings set_width(size_t width) { return change_settings::set_width(width); }
change_settings set_max_indent(size_t indent) { return change_settings::set_max_indent(indent); }

settings& operator<<(settings& a, change_settings const& b) {
    switch (b.field) {
        case change_settings::F_WIDTH:
            a.width = b.width;
            break;
        case change_settings::F_MAX_INDENT:
            a.max_indent = b.max_indent;
            break;
        case change_settings::F_EXT_EVAL:
            a.evaluate_extension = (pp_doc_type_t (*)(const pp_settings*, pp_doc_type_t,pp_doc**))b.ext_eval;
            break;
    }
    return a;
}

namespace impl {
    static void stream_writer(void* data, const char* text, size_t length) {
        auto os = (std::ostream*)data;
        os->write(text, length);
    }

    void write_out(std::ostream* os, pp_settings* s, std::shared_ptr<const doc> d) {
        pp_writer wr;
        wr.write = stream_writer;
        wr.data = (void*)os;

        _pp_pretty(&wr, s, static_cast<const pp_doc*>(d.get()));
    }
}

writer<settings> operator<<(std::ostream& os, change_settings s) {
    auto w = writer<settings>(os);
    w << s;
    return w;
}

std::ostream& operator<<(std::ostream& os, std::shared_ptr<doc> d) {
    auto w = writer<settings>(os);
    return w << d;
}

}

