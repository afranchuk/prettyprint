#include <stdio.h>
#include "prettyprint.h"

int main() {
    doc* basic = pp_appends(
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

    doc* grouped = pp_group(
            pp_appends(pp_string("one"), pp_line(),
                pp_string("two"), pp_line(),
                pp_string("three"), pp_line(),
                pp_string("four")));

    pp_settings settings;
    settings.width = 40;
    settings.max_indent = 20;
    pp_pretty(stdout, &settings, basic);
    fprintf(stdout, "\n\n");

    pp_free(basic);

    pp_pretty(stdout, &settings, grouped);
    fprintf(stdout, "\n\n");

    settings.width = 15;
    settings.max_indent = 4;
    pp_pretty(stdout, &settings, grouped);
    fprintf(stdout, "\n\n");

    pp_free(grouped);

    return 0;
}
