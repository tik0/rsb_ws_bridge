#include "SimpleConverter.h"
#include <string.h>

using namespace std;

using namespace boost;

using namespace rsb;
using namespace rsb::converter;

namespace converter {

SimpleConverter::SimpleConverter() :
    Converter<string> ("converter::any", "any", true) {
}

string SimpleConverter::serialize(const AnnotatedData& data, string& wire) {
    // We will never use this function.
    return getWireSchema();
}

AnnotatedData SimpleConverter::deserialize(const string& wireSchema,
        const string& wire) {

    std::string *data = new std::string(wire);

    // Return (a shared_ptr to) the constructed object along with its
    // data-type.
    return make_pair(getDataType(), boost::shared_ptr<std::string> (data));
}

}