#ifndef PRETTYPRINT_H
#define PRETTYPRINT_H

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
 * Uses union rather than separate structs to simplify the no-memory-alloc API
 * and to possibly facilitate use with a pool allocator.
 */
typedef struct _doc {
    /**
     * @brief The type of the document.
     */
    pp_doc_type_t type;
    union {
        struct {
            /**
             * @brief The first document to be appended.
             *
             * Valid when type == @p PP_DOC_APPEND.
             */
            const struct _doc* doc_a;
            /**
             * @brief The second document to be appended.
             *
             * Valid when type == @p PP_DOC_APPEND.
             */
            const struct _doc* doc_b;
        };
        struct {
            /**
             * @brief The text to display.
             *
             * Valid when type == @p PP_DOC_TEXT.
             */
            const char* text;
            /**
             * @brief The length of the text to display.
             *
             * Valid when type == @p PP_DOC_TEXT.
             */
            size_t length;
        };
        struct {
            /**
             * @brief The amount by which to increase the indent.
             *
             * Valid when type == @p PP_DOC_NEST.
             */
            size_t indent;
            /**
             * @brief The document to nest with the new indentation.
             *
             * Valid when type == @p PP_DOC_NEST.
             */
            const struct _doc* doc_nested;
        };
        /**
         * @brief The document to group.
         *
         * Valid when type == @p PP_DOC_GROUP.
         */
        const struct _doc* doc_grouped;
    };
} doc;


/**
 * @brief Initialize a nil document.
 *
 * @param result The document to initialize.
 */
void _pp_nil(doc* result);

/**
 * @brief Initialize a separator document.
 *
 * A separator is most commonly represented as a space, but may be omitted in
 * newlines.
 *
 * @param result The document to initialize.
 */
void _pp_sep(doc* result);

/**
 * @brief Initialize a text document.
 *
 * @param result The document to initialize.
 * @param text The text of the document. Ownership of memory is not accounted for.
 * @param length The length of text to use.
 */
void _pp_text(doc* result, const char* text, size_t length);

/**
 * @brief Initialize a line document.
 *
 * @param result The document to initialize.
 */
void _pp_line(doc* result);

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
void _pp_nest(doc* result, size_t indent, const doc* nested);

/**
 * @brief Initialize an appended document.
 *
 * @param result The document to initialize.
 * @param a The first document to append.
 * @param b The second document to append.
 */
void _pp_append(doc* result, const doc* a, const doc* b);

/**
 * @brief Initialize a grouped document.
 *
 * Grouped documents will display on a single line if possible, replacing all
 * @p line instances with spaces.
 *
 * @param result The document to initialize.
 * @param d The document to group.
 */
void _pp_group(doc* result, const doc* d);

/** @} */

/** @defgroup PPAPI Pretty-printing API
 * @{
 */

typedef struct {
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
} pp_settings;

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

/**
 * @brief Pretty print a document.
 *
 * @param writer The writer to use.
 * @param settings The settings to use when printing.
 * @param document The document to print.
 */
void _pp_pretty(const pp_writer* writer, const pp_settings* settings, const doc* document);

/** @} */


/** @defgroup MallocAPI Malloc API
 * 
 * Most users should find this sufficient.
 * @{
 */

/**
 * @brief Create a nil document.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
doc* pp_nil(void);

/**
 * @brief Create a separator document.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
doc* pp_sep(void);

/**
 * @brief Create a text document.
 *
 * @param text The text for the document.
 * @param length The length of the text.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
doc* pp_text(const char* text, size_t length);

/**
 * @brief Create a line document.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
doc* pp_line(void);

/**
 * @brief Create a nested document.
 *
 * @param indent The amount by which to increase the indentation.
 * @param nested The nested document.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
doc* pp_nest(size_t indent, const doc* nested);

/**
 * @brief Create an appended document.
 *
 * @param a The first document to append.
 * @param b The second document to append.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
doc* pp_append(const doc* a, const doc* b);

/**
 * @brief Create a grouped document.
 *
 * @param d The document to group.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
doc* pp_group(const doc* d);

/**
 * @brief Free a document.
 *
 * This frees the memory of all sub-documents as well.
 *
 * @param d The document to free.
 */
void pp_free(doc* d);

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
doc* pp_string(const char* str);

/**
 * @brief Create a document with space-separated words.
 *
 * The words in the null-terminated string @p words (as determined by the space
 * characters in the string) are made into separate text documents and
 * appended.
 *
 * @param words The string to split into words.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
doc* pp_words(const char* words);

/**
 * @brief Append all documents passed as parameters.
 *
 * @param ... A variable list of documents to append.
 *
 * @return The document, or NULL if the document could not be allocated.
 */
#define pp_appends(...) pp_appends_impl(0, __VA_ARGS__, NULL)
doc* pp_appends_impl(size_t count, ...);

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
void pp_pretty(FILE* f, const pp_settings* settings, const doc* document);

/** @} */

#endif
