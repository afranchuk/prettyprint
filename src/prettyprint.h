#ifndef PRETTYPRINT_H
#define PRETTYPRINT_H

#ifndef PRETTYPRINT_USE_CPP
#define PRETTYPRINT_USE_CPP __cplusplus
#endif

#if PRETTYPRINT_USE_CPP != 0
#include <cstddef>
#endif

typedef enum {
    PP_DOC_NIL,
    PP_DOC_TEXT,
    PP_DOC_SEP,
    PP_DOC_LINE,
    PP_DOC_NEST,
    PP_DOC_APPEND,
    PP_DOC_GROUP,
    PP_DOC_EXTENSION_START = 100
} pp_doc_type_t;

/** @defgroup NoMemAlloc No-memory-allocation API
 * @{
 */

/**
 * @brief A document object.
 *
 * This is a bare document object; other objects extend it.
 */
typedef struct {
    /**
     * @brief The type of the document.
     */
    pp_doc_type_t type;
} pp_doc;

/**
 * @brief A text document object.
 */
typedef struct {
    pp_doc_type_t type;
    /**
     * @brief The text to display.
     */
    const char* text;
    /**
     * @brief The length of the text to display.
     */
    size_t length;
} pp_doc_text;

/**
 * @brief An append document object.
 */
typedef struct {
    pp_doc_type_t type;
    /**
     * @brief The first document to be appended.
     */
    const pp_doc* a;
    /**
     * @brief The second document to be appended.
     */
    const pp_doc* b;
} pp_doc_append;

/**
 * @brief A nest document object.
 */
typedef struct {
    pp_doc_type_t type;
    /**
     * @brief The amount by which to increase the indent.
     */
    size_t indent;
    /**
     * @brief The document to nest with the new indentation.
     */
    const pp_doc* nested;
} pp_doc_nest;

/**
 * @brief A group document object.
 */
typedef struct {
    pp_doc_type_t type;
    /**
     * @brief The document to group.
     */
    const pp_doc* grouped;
} pp_doc_group;

/** @defgroup PPAPI Pretty-printing API
 * @{
 */

typedef struct _pp_settings pp_settings;

struct _pp_settings {
    /**
     * @brief The maximum width of a line.
     */
    size_t width;
    /**
     * @brief The maximum indent allowed.
     *
     * Anything exceeding this indent will be truncated to it.
     */
    size_t max_indent;
    /**
     * @brief Extension evaluator function.
     *
     * Set to NULL to not evaluate any extensions. In this case, if an
     * extension document is encountered, it will be completely ignored.
     *
     * If a document has a type that is greater than or equal to @p
     * PP_DOC_EXTENSION_START, this function is called on the document. It
     * should return the new type that the document should be considered as.
     * Note that the document's fields (besides @p type) should be set
     * accordingly.
     *
     * This is useful for taking advantage of custom settings. The @p settings
     * object that is passed to @p _pp_print could be a struct that merely
     * starts with the @p pp_settings struct, and contains more settings that
     * interacts with extensions in different ways. Likewise the extension
     * documents need to (when applicable, based on the types to which they
     * will evaluate) contain a @p doc struct at the beginning, but may contain
     * extra data. This function will be called repeatedly until the returned type
     * is less than @p PP_DOC_EXTENSION_START.
     *
     * Because the @p type field is not expected to be overwritten, it is
     * possible to change state in the document such that the document would
     * behave the same if another call to @p _pp_print is made, although it is
     * up to extension implementors to fulfill this behavior (if desired).
     *
     * Note that @p d may be set by this method to change the location of the
     * document to be used.
     *
     * @param settings The settings object.
     * @param type The type of the document.
     * @param d A double-pointer to The document to evaluate. May be changed.
     */
    pp_doc_type_t (*evaluate_extension)(const pp_settings* settings, pp_doc_type_t type, pp_doc** d);
};

#if PRETTYPRINT_USE_CPP == 0 || PRETTYPRINT_CPP_INTERNAL == 1

/** @} */

/** @defgroup AdvancedPP Advanced pretty-printing API
 * @{
 */

typedef struct {
    /**
     * @brief The function to use to write out text.
     *
     * @param data The data member.
     * @param text The text to write.
     * @param length The length of the text to write.
     */
    void (*write)(void* data, const char* text, size_t length);
    /**
     * @brief Data to pass to the write function.
     */
    void* data;
} pp_writer;

/** @} */

#endif

#if PRETTYPRINT_USE_CPP == 0

/**
 * @brief A nil document.
 */
extern pp_doc* _pp_nil;

/**
 * @brief A separator document.
 *
 * A separator is most commonly represented as a space, but may be omitted in
 * newlines.
 */
extern pp_doc* _pp_sep;

/**
 * @brief Initialize a text document.
 *
 * @param result The document to initialize.
 * @param text The text of the document. Ownership of memory is not accounted for.
 * @param length The length of text to use.
 */
void _pp_text(pp_doc_text* result, const char* text, size_t length);

/**
 * @brief A line document.
 */
extern pp_doc* _pp_line;

/**
 * @brief Initialize a nested document.
 *
 * Nested documents are an easy way to make indented sections. Indentation
 * takes effect after newlines in nested.
 *
 * @param result The document to initialize.
 * @param indent The amount by which to increase the indent.
 * @param nested The nested document.
 */
void _pp_nest(pp_doc_nest* result, size_t indent, const pp_doc* nested);

/**
 * @brief Initialize an appended document.
 *
 * @param result The document to initialize.
 * @param a The first document to append.
 * @param b The second document to append.
 */
void _pp_append(pp_doc_append* result, const pp_doc* a, const pp_doc* b);

/**
 * @brief Initialize a grouped document.
 *
 * Grouped documents will display on a single line if possible, replacing all
 * @p line instances with spaces.
 *
 * @param result The document to initialize.
 * @param d The document to group.
 */
void _pp_group(pp_doc_group* result, const pp_doc* d);

/** @} */

/** @addtogroup AdvancedPP
 * @{
 */

/**
 * @brief Pretty print a document.
 *
 * @param writer The writer to use.
 * @param settings The settings to use when printing.
 * @param document The document to print.
 */
void _pp_pretty(const pp_writer* writer, const pp_settings* settings, const pp_doc* document);

/** @} */

/** @defgroup MallocAPI Malloc API
 * 
 * Most users should find this API sufficient.
 * @{
 */

/**
 * @brief Create a nil document.
 *
 * @return The document (not malloc'd).
 */
pp_doc* pp_nil(void);

/**
 * @brief Create a separator document.
 *
 * @return The document (not malloc'd).
 */
pp_doc* pp_sep(void);

/**
 * @brief Create a text document.
 *
 * @param text The text for the document.
 * @param length The length of the text.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
pp_doc* pp_text(const char* text, size_t length);

/**
 * @brief Create a line document.
 *
 * @return The document (not malloc'd).
 */
pp_doc* pp_line(void);

/**
 * @brief Create a nested document.
 *
 * @param indent The amount by which to increase the indentation.
 * @param nested The nested document.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
pp_doc* pp_nest(size_t indent, const pp_doc* nested);

/**
 * @brief Create an appended document.
 *
 * @param a The first document to append.
 * @param b The second document to append.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
pp_doc* pp_append(const pp_doc* a, const pp_doc* b);

/**
 * @brief Create a grouped document.
 *
 * @param d The document to group.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
pp_doc* pp_group(const pp_doc* d);

/**
 * @brief Free a document.
 *
 * This frees the memory of all sub-documents as well.
 *
 * @param d The document to free.
 */
void pp_free(pp_doc* d);

/**
 * @brief An extension-aware free function.
 *
 * If @p free_ext is NULL, ignores extension documents.
 *
 * This frees the memory of all sub-documents, and calls @p free_ext when an
 * extension document is encountered. @p free_ext is expected to also free the
 * document pointer (if necessary).
 *
 * @param free_ext The free function used for extension documents.
 * @param d The document to free.
 */
void pp_free_ext(void (*free_ext)(pp_doc* d), pp_doc* d);

/** @} */

/** @defgroup HighFunc Higher-level functions
 * @{
 */

/**
 * @brief Create a text document from a null-terminated string.
 *
 * The length is determined with @p strlen().
 *
 * @param str The string to use for the text document.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
pp_doc* pp_string(const char* str);

/**
 * @brief Create a document with space-separated words.
 *
 * The words in the null-terminated string @p words (as determined by the space
 * characters in the string) are made into separate text documents and
 * appended. Any newlines in the string are made into line documents.
 *
 * @param words The string to split into words.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
pp_doc* pp_words(const char* words);

/**
 * @brief Append all documents passed as parameters.
 *
 * @param ... A variable list of documents to append.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
#define pp_appends(...) pp_appends_impl(0, __VA_ARGS__, NULL)
pp_doc* pp_appends_impl(size_t count, ...);

/** @} */

/** @addtogroup PPAPI
 * @{
 */

/**
 * @brief Pretty print a document.
 *
 * @param f The file pointer to which to print the document.
 * @param settings The settings to use when printing.
 * @param document The document to print.
 */
void pp_pretty(FILE* f, const pp_settings* settings, const pp_doc* document);

/** @} */

#else

#include <memory>
#include <sstream>
#include <string>

/** @defgroup CXXAPI C++ API
 * @{
 */

namespace pp {

struct doc : public pp_doc {};

namespace data {

template <typename T>
struct from_doc : public T {
    doc* as_doc() { return reinterpret_cast<doc*>(static_cast<T*>(this)); }
};

struct doc_text : public from_doc<pp_doc_text> {
    doc_text(const char* t, size_t length);
    doc_text(const char* str);
};

struct doc_string : public from_doc<pp_doc_text> {
    doc_string(const std::string& s);
private:
    const std::string s;
};

struct doc_nest : public from_doc<pp_doc_nest> {
    doc_nest(size_t indent, std::shared_ptr<const doc> nested);
protected:
    void set_nested(std::shared_ptr<const doc> nested);
private:
    std::shared_ptr<const doc> s_nested;
};

struct doc_append : public from_doc<pp_doc_append> {
    doc_append(std::shared_ptr<const doc> a, std::shared_ptr<const doc> b);
private:
    std::shared_ptr<const doc> s_a;
    std::shared_ptr<const doc> s_b;
};

struct doc_group : public from_doc<pp_doc_group> {
    doc_group(std::shared_ptr<const doc> grouped);
private:
    std::shared_ptr<const doc> s_grouped;
};

struct doc_words : public doc_nest {
    doc_words(const std::string& s);
private:
    const std::string s;
};

}

std::shared_ptr<doc> nil();

std::shared_ptr<doc> sep();

std::shared_ptr<doc> text(const char* t, size_t length);
std::shared_ptr<doc> text(const char* str);
std::shared_ptr<doc> text(const std::string& s);

std::shared_ptr<doc> line();

std::shared_ptr<doc> nest(size_t indent, std::shared_ptr<const doc> nested);

std::shared_ptr<doc> append(std::shared_ptr<const doc> a, std::shared_ptr<const doc> b);

std::shared_ptr<doc> group(std::shared_ptr<const doc> grouped);

std::shared_ptr<doc> words(const std::string& words);

/** Alias of append. */
std::shared_ptr<doc> operator+(std::shared_ptr<const doc> a, std::shared_ptr<const doc> b);
std::shared_ptr<doc> operator+(std::shared_ptr<const doc> a, const std::string& words);

/** Aliases of append that add a separator. */
std::shared_ptr<doc> operator<<(std::shared_ptr<const doc> a, std::shared_ptr<const doc> b);
std::shared_ptr<doc> operator<<(std::shared_ptr<const doc> a, const std::string& words);

template <typename T>
std::shared_ptr<doc> operator<<(std::shared_ptr<const doc> a, T& b) {
    std::stringstream str;
    str << b;
    return a << str.str();
}

/** @} */

/** @defgroup CXXPPAPI C++ Pretty-Printing API
 *
 * @{
 */

struct settings : public pp_settings {
    settings();
};

struct change_settings {
    static change_settings set_width(size_t width);
    static change_settings set_max_indent(size_t indent);
    template <typename S>
    static change_settings set_extension_evaluator(
        pp_doc_type_t (*eval)(const S* settings, pp_doc_type_t type, doc** d)) {
        change_settings s;
        s.field = F_EXT_EVAL;
        s.ext_eval = (pp_doc_type_t (*)(const settings*, pp_doc_type_t, doc**))eval;
        return s;
    }

private:
    enum {
        F_WIDTH,
        F_MAX_INDENT,
        F_EXT_EVAL
    } field;
    union {
        size_t width;
        size_t max_indent;
        pp_doc_type_t (*ext_eval)(const settings* s, pp_doc_type_t type, doc** d);
    };
    change_settings();

    friend settings& operator<<(settings& a, change_settings const& b);
};

change_settings set_width(size_t width);
change_settings set_max_indent(size_t indent);

template <typename Settings>
struct writer {
    explicit writer(std::ostream& os)
        : os(&os)
    {}

private:
    Settings s;
    std::ostream* os;

    template <typename S2>
    friend writer<S2>& operator<<(writer<S2>& w, change_settings const& s);
    template <typename S2>
    friend std::ostream& operator<<(writer<S2>& w, std::shared_ptr<const doc> d);
};

template <typename Settings>
writer<Settings>& operator<<(writer<Settings>& w, change_settings const& s) {
    w.s << s;
    return w;
}

namespace impl {

void write_out(std::ostream* os, pp_settings* s, std::shared_ptr<const doc> d);

}

template <typename Settings>
std::ostream& operator<<(writer<Settings>& w, std::shared_ptr<const doc> d) {
    impl::write_out(w.os, static_cast<pp_settings*>(&w.s), d);
    return *w.os;
}

writer<settings> operator<<(std::ostream& os, change_settings s);
std::ostream& operator<<(std::ostream& os, std::shared_ptr<doc> d);

/** @} */

}

#endif

#endif
