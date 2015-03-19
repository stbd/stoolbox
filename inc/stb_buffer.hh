#ifndef STB_BUFFER_HH_
#define STB_BUFFER_HH_

#include <string>

namespace stb { namespace buffer
{

class Buffer
{
public:
    virtual ~Buffer(){}
    virtual bool ready() const = 0;
    virtual size_t size() const = 0;
    virtual void read(char * output) const = 0;
};

class InputFile : public Buffer
{
public:
    InputFile(const char * pathToFile);
    InputFile(std::string & pathToFile);
    InputFile(const InputFile & other);
    InputFile & operator = (const InputFile & other);

    bool ready() const;
    size_t size() const { return m_fileSize; }
    void read(char * output) const;

private:
    std::string m_path;
    size_t m_fileSize;
    bool m_ok;
};

class StaticMemory : public Buffer
{
public:
    StaticMemory(const char * data, const size_t size);
    StaticMemory(const StaticMemory & other);
    StaticMemory & operator = (const StaticMemory & other);

    bool ready() const { return true; }
    size_t size() const { return m_size; }
    void read(char * buffer) const ;

private:
    const char * m_data;
    size_t m_size;
};
}

}

#endif