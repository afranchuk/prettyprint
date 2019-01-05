#include <iostream>

#include "prettyprint.h"

void print_basic() {
    auto doc = pp::nil();
    doc = doc + "hello world";
    doc = doc + pp::nest(4, pp::line()
            + "indented"
            + pp::nest(4, pp::line()
                + "more indented" + pp::line()
                + "across lines")
            + pp::line() + "and with really long lines with many words including "
                           "veryveryveryverylongandcontiguouswordswhichneedwrapping");

    auto settings = std::cout << pp::set_width(40);
    settings << pp::set_max_indent(20) << doc
             << std::endl << std::endl;
}

void print_grouped() {
    auto doc = pp::group( pp::nil() +
            "one" + pp::line() +
            "two" + pp::line() +
            "three" + pp::line() +
            "four");

    auto settings = std::cout << pp::set_width(40);
    settings << pp::set_max_indent(20) << doc
             << std::endl << std::endl;

    settings = std::cout << pp::set_width(15);
    settings << pp::set_max_indent(4) << doc
             << std::endl << std::endl;
}

int main() {
    print_basic();
    print_grouped();

    return 0;
}
