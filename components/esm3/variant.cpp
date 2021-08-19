#include "variant.hpp"

#include <cassert>
#include <stdexcept>

#include "common.hpp"
#include "reader.hpp"
#include "variantimp.hpp"

namespace
{
    template <typename T, bool orDefault = false>
    struct GetValue
    {
        constexpr T operator()(int value) const { return static_cast<T>(value); }

        constexpr T operator()(float value) const { return static_cast<T>(value); }

        template <typename V>
        constexpr T operator()(const V&) const
        {
            if constexpr (orDefault)
                return T {};
            else
                throw std::runtime_error("cannot convert variant");
        }
    };

    template <typename T>
    struct SetValue
    {
        T mValue;

        explicit SetValue(T value) : mValue(value) {}

        void operator()(int& value) const { value = static_cast<int>(mValue); }

        void operator()(float& value) const { value = static_cast<float>(mValue); }

        template <typename V>
        void operator()(V&) const { throw std::runtime_error("cannot convert variant"); }
    };
}

std::string ESM3::Variant::getString() const
{
    return std::get<std::string>(mData);
}

int ESM3::Variant::getInteger() const
{
    return std::visit(GetValue<int>{}, mData);
}

float ESM3::Variant::getFloat() const
{
    return std::visit(GetValue<float>{}, mData);
}

// NOTE: assumes sub-record header was just read before calling this method
void ESM3::Variant::read (Reader& reader, Format format)
{
    const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();

    // type
    ESM::VarType type = ESM::VT_Unknown;

    if (format == Format_Global)
    {
        if (subHdr.typeId == ESM3::SUB_FNAM)
        {
            char typeId = '\0';
            assert(subHdr.dataSize == 1);
            reader.get(typeId);

            if (typeId == 's')
                type = ESM::VT_Short;
            else if (typeId == 'l')
                type = ESM::VT_Long;
            else if (typeId == 'f')
                type = ESM::VT_Float;
            else
                reader.fail ("illegal global variable type " + std::string(&typeId, 1));
        }
        else
            reader.fail ("global variable type not found");
    }
    else if (format == Format_Gmst)
    {
        if (!reader.hasMoreSubs())
        {
            type = ESM::VT_None;
        }
        else
        {
            reader.getSubRecordHeader();

            if (subHdr.typeId == ESM3::SUB_STRV)
            {
                type = ESM::VT_String;
            }
            else if (subHdr.typeId == ESM3::SUB_INTV)
            {
                type = ESM::VT_Int;
            }
            else if (subHdr.typeId == ESM3::SUB_FLTV)
            {
                type = ESM::VT_Float;
            }
            else
                reader.fail ("invalid subrecord: " + ESM::printName(subHdr.typeId));
        }
    }
    else if (format == Format_Info)
    {
        reader.getSubRecordHeader();

        if (subHdr.typeId == ESM3::SUB_INTV)
        {
            type = ESM::VT_Int;
        }
        else if (subHdr.typeId == ESM3::SUB_FLTV)
        {
            type = ESM::VT_Float;
        }
        else
            reader.fail ("invalid subrecord: " + ESM::printName(subHdr.typeId));
    }
    else if (format == Format_Local)
    {
        reader.getSubRecordHeader();

        if (subHdr.typeId == ESM3::SUB_INTV)
        {
            type = ESM::VT_Int;
        }
        else if (subHdr.typeId == ESM3::SUB_FLTV)
        {
            type = ESM::VT_Float;
        }
        else if (subHdr.typeId == ESM3::SUB_STTV)
        {
            type = ESM::VT_Short;
        }
        else
            reader.fail ("invalid subrecord: " + ESM::printName(subHdr.typeId));
    }

    setType (type);

    std::visit(ReadESMVariantValue {reader, format, mType}, mData);
}

void ESM3::Variant::write (ESM::ESMWriter& esm, Format format) const
{
    if (mType==ESM::VT_Unknown)
    {
        throw std::runtime_error ("can not serialise variant of unknown type");
    }
    else if (mType==ESM::VT_None)
    {
        if (format==Format_Global)
            throw std::runtime_error ("can not serialise variant of type none to global format");

        if (format==Format_Info)
            throw std::runtime_error ("can not serialise variant of type none to info format");

        if (format==Format_Local)
            throw std::runtime_error ("can not serialise variant of type none to local format");

        // nothing to do here for GMST format
    }
    else
        std::visit(WriteESMVariantValue {esm, format, mType}, mData);
}

void ESM3::Variant::write (std::ostream& stream) const
{
    switch (mType)
    {
        case ESM::VT_Unknown:

            stream << "variant unknown";
            break;

        case ESM::VT_None:

            stream << "variant none";
            break;

        case ESM::VT_Short:

            stream << "variant short: " << std::get<int>(mData);
            break;

        case ESM::VT_Int:

            stream << "variant int: " << std::get<int>(mData);
            break;

        case ESM::VT_Long:

            stream << "variant long: " << std::get<int>(mData);
            break;

        case ESM::VT_Float:

            stream << "variant float: " << std::get<float>(mData);
            break;

        case ESM::VT_String:

            stream << "variant string: \"" << std::get<std::string>(mData) << "\"";
            break;
    }
}

void ESM3::Variant::setType (ESM::VarType type)
{
    if (type!=mType)
    {
        switch (type)
        {
            case ESM::VT_Unknown:
            case ESM::VT_None:
                mData = std::monostate {};
                break;

            case ESM::VT_Short:
            case ESM::VT_Int:
            case ESM::VT_Long:
                mData = std::visit(GetValue<int, true>{}, mData);
                break;

            case ESM::VT_Float:
                mData = std::visit(GetValue<float, true>{}, mData);
                break;

            case ESM::VT_String:
                mData = std::string {};
                break;
        }

        mType = type;
    }
}

void ESM3::Variant::setString (const std::string& value)
{
    std::get<std::string>(mData) = value;
}

void ESM3::Variant::setString (std::string&& value)
{
    std::get<std::string>(mData) = std::move(value);
}

void ESM3::Variant::setInteger (int value)
{
    std::visit(SetValue(value), mData);
}

void ESM3::Variant::setFloat (float value)
{
    std::visit(SetValue(value), mData);
}

std::ostream& ESM3::operator<< (std::ostream& stream, const Variant& value)
{
    value.write (stream);
    return stream;
}
