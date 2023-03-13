#ifndef EXAMPLE_PATTERN_DOSNT_MATCH_H
#define EXAMPLE_PATTERN_DOSNT_MATCH_H

#include <string>
#include "Generic.h"

namespace Exception {

class ExampleMessageDoesntMatch : public Exception::Generic {
public:
    ExampleMessageDoesntMatch(std::string id)
        : Generic("Not a valid capture group [" + id + "]") {}
    const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//EXAMPLE_PATTERN_DOSNT_MATCH_H