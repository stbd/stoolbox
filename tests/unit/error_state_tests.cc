#define BOOST_TEST_MODULE unit_test_error
#include <boost/test/unit_test.hpp>

#include "stb_error.hh"
#include <string>

using namespace stb;

namespace stb {
    extern void setError(const char * format, ...);
}

BOOST_AUTO_TEST_CASE(test_error_state)
{
    BOOST_CHECK_EQUAL(stb::isError(), false);
    
    stb::setError("%s - %u", "String", 5);
    BOOST_CHECK_EQUAL(stb::isError(), true);

    std::string description(stb::getErrorDescription());
    BOOST_CHECK(description == "String - 5");

    stb::clearError();
    BOOST_CHECK_EQUAL(stb::isError(), false);
}