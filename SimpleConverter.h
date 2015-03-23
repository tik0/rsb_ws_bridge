#pragma once

#include <rsb/converter/Converter.h>

namespace converter {

/**
 * A simple converter
 */
class SimpleConverter: public rsb::converter::Converter<std::string> {
public:
    SimpleConverter();

    std::string serialize(const rsb::AnnotatedData& data,
            std::string& wire);

    rsb::AnnotatedData deserialize(const std::string& wireSchema,
            const std::string& wire);
};

}