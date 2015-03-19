#include "stb_buffer.hh"

#include <fstream>
#include <cstring>

using namespace stb;

namespace stb { namespace buffer
{

/******************* InputFile *******************/
static bool initFile(std::string & path, size_t & sizeOfFile)
{
    std::ifstream file(path, std::ios::binary | std::ios_base::in);
    if (file.good() && file.is_open()) {
        file.seekg(0, std::ios_base::end);
        const std::streampos endpos = file.tellg();
        file.seekg(0, std::ios_base::beg);
        sizeOfFile = static_cast<size_t>(endpos - file.tellg());
        return true;
    }
    return false;
}

InputFile::InputFile(const char * pathToFile)
: m_path(pathToFile),
m_ok(false)
{
    m_ok = initFile(m_path, m_fileSize);
}
InputFile::InputFile(std::string & pathToFile)
: m_path(pathToFile),
m_ok(false)
{
    m_ok = initFile(m_path, m_fileSize);
}

InputFile::InputFile(const InputFile & other)
: m_path(other.m_path),
m_fileSize(other.m_fileSize),
m_ok(other.m_ok)
{}

InputFile & InputFile::operator = (const InputFile & other)
{
    m_path = other.m_path;
    m_fileSize = other.m_fileSize;
    m_ok = other.m_ok;
    return *this;
}

bool InputFile::ready() const
{
    return m_ok;
}

void InputFile::read(char * output) const
{
    std::ifstream file(m_path, std::ios::binary | std::ios_base::in);
    if (file.good() && file.is_open()) {
        file.read(output, m_fileSize);
    }
}

/******************* StaticMemory *******************/

StaticMemory::StaticMemory(const char * data, const size_t size)
: m_data(data),
m_size(size)
{}

StaticMemory::StaticMemory(const StaticMemory & other)
: m_data(other.m_data),
m_size(other.m_size)
{}

StaticMemory & StaticMemory::operator = (const StaticMemory & other)
{
    m_data = other.m_data;
    m_size = other.m_size;
    return *this;
}

void StaticMemory::read(char * output) const
{
    memcpy(output, m_data, m_size);
}

}
}
