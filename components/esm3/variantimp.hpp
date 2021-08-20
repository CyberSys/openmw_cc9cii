#ifndef ESM3_VARIANTIMP_H
#define ESM3_VARIANTIMP_H

#include <string>
#include <functional>

#include "variant.hpp"

namespace ESM3
{
    void readESMVariantValue(Reader& reader, Variant::Format format, ESM::VarType type, std::string& value);

    void readESMVariantValue(Reader& reader, Variant::Format format, ESM::VarType type, float& value);

    void readESMVariantValue(Reader& reader, Variant::Format format, ESM::VarType type, int& value);

    void writeESMVariantValue(ESM::ESMWriter& writer, Variant::Format format, ESM::VarType type, const std::string& value);

    void writeESMVariantValue(ESM::ESMWriter& writer, Variant::Format format, ESM::VarType type, float value);

    void writeESMVariantValue(ESM::ESMWriter& writer, Variant::Format format, ESM::VarType type, int value);

    struct ReadESMVariantValue
    {
        std::reference_wrapper<Reader> mReader;
        Variant::Format mFormat;
        ESM::VarType mType;

        ReadESMVariantValue(Reader& reader, Variant::Format format, ESM::VarType type)
            : mReader(reader), mFormat(format), mType(type) {}

        void operator()(std::monostate) const {}

        template <typename T>
        void operator()(T& value) const
        {
            readESMVariantValue(mReader.get(), mFormat, mType, value);
        }
    };

    struct WriteESMVariantValue
    {
        std::reference_wrapper<ESM::ESMWriter> mWriter;
        Variant::Format mFormat;
        ESM::VarType mType;

        WriteESMVariantValue(ESM::ESMWriter& writer, Variant::Format format, ESM::VarType type)
            : mWriter(writer), mFormat(format), mType(type) {}

        void operator()(std::monostate) const {}

        template <typename T>
        void operator()(const T& value) const
        {
            writeESMVariantValue(mWriter.get(), mFormat, mType, value);
        }
    };
}

#endif
