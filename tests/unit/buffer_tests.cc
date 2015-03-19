#define BOOST_TEST_MODULE unit_test_buffer
#include <boost/test/unit_test.hpp>

#include "stb_buffer.hh"

const std::string expectedData("data123456789");

BOOST_AUTO_TEST_CASE(test_buffer_with_static_memory)
{
    stb::buffer::StaticMemory memory(expectedData.c_str(), expectedData.length());
    stb::buffer::Buffer * buffer = &memory;

    BOOST_CHECK_EQUAL(buffer->ready(), true);
    BOOST_CHECK_EQUAL(buffer->size(), expectedData.size());

    std::string output(buffer->size(), ' ');
    buffer->read(&output[0]);
    BOOST_CHECK(output == expectedData);
}

/* Uncommented as test case requires a file on disk
BOOST_AUTO_TEST_CASE(test_buffer_with_file_access)
{
    stb::buffer::InputFile file("test.txt");
    stb::buffer::Buffer * buffer = &file;

    BOOST_CHECK_EQUAL(buffer->ready(), true);
    BOOST_CHECK_EQUAL(buffer->size(), (size_t)13);

    std::string output(buffer->size(), ' ');
    buffer->read(&output[0]);
    BOOST_CHECK(output == expectedData);
}
*/