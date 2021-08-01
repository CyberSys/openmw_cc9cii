#include "variantimp.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>
#include <stdexcept>
#include <cmath>

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

// NOTE: this method assumes the sub-record header was just read
//       (see ESM3::Variant::read() code block "else if (format == Format_Gmst)")
void ESM3::readESMVariantValue(Reader& reader, Variant::Format format, ESM::VarType type, std::string& out)
{
    if (type!=ESM::VT_String)
        throw std::logic_error ("not a string type");

    if (format==Variant::Format_Global)
        reader.fail ("global variables of type string not supported");

    if (format==Variant::Format_Info)
        reader.fail ("info variables of type string not supported");

    if (format==Variant::Format_Local)
        reader.fail ("local variables of type string not supported");

    // GMST
    //out = esm.getHString();
    assert(reader.subRecordHeader().typeId == ESM3::SUB_STRV);
    reader.getString(out); // NOTE: string not null terminated
}

void ESM3::writeESMVariantValue(ESM::ESMWriter& esm, Variant::Format format, ESM::VarType type, const std::string& in)
{
    if (type!=ESM::VT_String)
        throw std::logic_error ("not a string type");

    if (format==Variant::Format_Global)
        throw std::runtime_error ("global variables of type string not supported");

    if (format==Variant::Format_Info)
        throw std::runtime_error ("info variables of type string not supported");

    if (format==Variant::Format_Local)
        throw std::runtime_error ("local variables of type string not supported");

    // GMST
    esm.writeHNString("STRV", in);
}

void ESM3::readESMVariantValue(Reader& reader, Variant::Format format, ESM::VarType type, int& out)
{
    if (type != ESM::VT_Short && type != ESM::VT_Long && type != ESM::VT_Int)
        throw std::logic_error ("not an integer type");

    if (format == Variant::Format_Global)
    {
        float value;
        //esm.getHNT (value, "FLTV");
        reader.getSubRecordHeader();
        assert(reader.subRecordHeader().typeId == ESM3::SUB_INTV);
        reader.get(value);

        if (type == ESM::VT_Short)
            if (std::isnan(value))
                out = 0;
            else
                out = static_cast<short> (value);
        else if (type == ESM::VT_Long)
            out = static_cast<int> (value);
        else
            reader.fail ("unsupported global variable integer type");
    }
    else if (format == Variant::Format_Gmst || format == Variant::Format_Info)
    {
        if (type != ESM::VT_Int)
        {
            std::ostringstream stream;
            stream
                << "unsupported " <<(format==Variant::Format_Gmst ? "gmst" : "info")
                << " variable integer type";
            reader.fail (stream.str());
        }

        //esm.getHT(out);
        reader.get(out);
    }
    else if (format == Variant::Format_Local)
    {
        if (type == ESM::VT_Short)
        {
            short value;
            //esm.getHT(value);
            reader.get(value);
            out = value;
        }
        else if (type == ESM::VT_Int)
        {
            //esm.getHT(out);
            reader.get(out);
        }
        else
            reader.fail("unsupported local variable integer type");
    }
}

void ESM3::writeESMVariantValue(ESM::ESMWriter& esm, Variant::Format format, ESM::VarType type, int in)
{
    if (type!=ESM::VT_Short && type!=ESM::VT_Long && type!=ESM::VT_Int)
        throw std::logic_error ("not an integer type");

    if (format==Variant::Format_Global)
    {
        if (type==ESM::VT_Short || type==ESM::VT_Long)
        {
            float value = static_cast<float>(in);
            esm.writeHNString ("FNAM", type==ESM::VT_Short ? "s" : "l");
            esm.writeHNT ("FLTV", value);
        }
        else
            throw std::runtime_error ("unsupported global variable integer type");
    }
    else if (format==Variant::Format_Gmst || format==Variant::Format_Info)
    {
        if (type!=ESM::VT_Int)
        {
            std::ostringstream stream;
            stream
                << "unsupported " <<(format==Variant::Format_Gmst ? "gmst" : "info")
                << " variable integer type";
            throw std::runtime_error (stream.str());
        }

        esm.writeHNT("INTV", in);
    }
    else if (format==Variant::Format_Local)
    {
        if (type==ESM::VT_Short)
            esm.writeHNT("STTV", static_cast<short>(in));
        else if (type == ESM::VT_Int)
            esm.writeHNT("INTV", in);
        else
            throw std::runtime_error("unsupported local variable integer type");
    }
}

void ESM3::readESMVariantValue(Reader& reader, Variant::Format format, ESM::VarType type, float& out)
{
    if (type != ESM::VT_Float)
        throw std::logic_error ("not a float type");

    if (format == Variant::Format_Global)
    {
        //esm.getHNT(out, "FLTV");
        reader.getSubRecordHeader();
        assert(reader.subRecordHeader().typeId == ESM3::SUB_FLTV);
        reader.get(out);
    }
    else if (format == Variant::Format_Gmst || format == Variant::Format_Info || format == Variant::Format_Local)
    {
        //esm.getHT(out);
        reader.get(out);
    }
}

void ESM3::writeESMVariantValue(ESM::ESMWriter& esm, Variant::Format format, ESM::VarType type, float in)
{
    if (type!=ESM::VT_Float)
        throw std::logic_error ("not a float type");

    if (format==Variant::Format_Global)
    {
        esm.writeHNString ("FNAM", "f");
        esm.writeHNT("FLTV", in);
    }
    else if (format==Variant::Format_Gmst || format==Variant::Format_Info || format==Variant::Format_Local)
    {
        esm.writeHNT("FLTV", in);
    }
}
