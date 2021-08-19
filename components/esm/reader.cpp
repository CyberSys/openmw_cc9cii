//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>
#include <stdexcept>

#include <components/files/constrainedfilestream.hpp>

#include "../esm3/reader.hpp"
#include "../esm4/reader.hpp"

namespace ESM
{
    Reader* Reader::getReader(const std::string &filename)
    {
        Files::IStreamPtr esmStream(Files::openConstrainedFileStream (filename.c_str ()));

        std::uint32_t modVer = 0; // get the first 4 bytes of the record header only
        esmStream->read((char*)&modVer, sizeof(modVer));
        if (esmStream->gcount() == sizeof(modVer))
        {
            esmStream->seekg(0);

            if (modVer == ESM4::REC_TES4)
            {
                return new ESM4::Reader(esmStream, filename);
            }
            else
            {
                return new ESM3::Reader(esmStream, filename);
            }
        }

        throw std::runtime_error("Unknown file format");
    }

    bool Reader::getStringImpl(std::string& str, std::size_t size,
            Files::IStreamPtr filestream, ToUTF8::Utf8Encoder* encoder, bool hasNull)
    {
        std::size_t newSize = size;

        if (encoder)
        {
            if (!hasNull)
                newSize += 1; // Utf8Encoder::getLength() expects a null terminator

            std::string tmp;
            tmp.resize(newSize);
            filestream->read(&tmp[0], size);
            if ((std::size_t)filestream->gcount() == size)
            {
                if (hasNull)
                {
                    assert (tmp[newSize - 1] == '\0'
                            && "ESM4::Reader::getString string is not terminated with a null");
                }
                else
                {
                    assert (tmp[newSize - 2] != '\0'
                            && "ESM4::Reader::getString string is unexpectedly terminated with a null");
                    tmp[newSize-1] = '\0'; // for Utf8Encoder::getLength()
                }

                encoder->toUtf8(tmp, str, newSize - 1);
                // FIXME: the encoder converts �keep� as “keep� which increases the length,
                //        which results in below resize() truncating the string
                if (str.size() == newSize)   // ascii
                    str.resize(newSize - 1); // don't want the null terminator
                else
                {
                    // WARN: this is a horrible hack but fortunately there's only a few like this
                    std::string tmp2(str.data());
                    str.swap(tmp2);
                }

                return true;
            }
        }
        else
        {
            if (hasNull)
                newSize -= 1; // don't read the null terminator yet

            str.resize(newSize); // assumed C++11
            filestream->read(&str[0], newSize);
            if ((std::size_t)filestream->gcount() == newSize)
            {
                if (hasNull)
                {
                    char ch;
                    filestream->read(&ch, 1); // read the null terminator
                    assert (ch == '\0'
                            && "ESM4::Reader::getString string is not terminated with a null");
                }
                else
                {
                    assert (str[newSize - 1] != '\0'
                            && "ESM4::Reader::getString string is unexpectedly terminated with a null");
                }

                return true;
            }
        }

        str.clear();
        return false; // FIXME: throw instead?
    }
}
