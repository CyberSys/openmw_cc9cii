#include "metadata.hpp"

#include <components/esm3/tes3.hpp>
#include <components/esm3/reader.hpp>
#include <components/esm/esmwriter.hpp>

void CSMWorld::MetaData::blank()
{
    mFormat = ESM3::Header::CurrentFormat;
    mAuthor.clear();
    mDescription.clear();
}

void CSMWorld::MetaData::load (ESM3::Reader& reader)
{
    mFormat = reader.getFormat();
    mAuthor = reader.getAuthor();
    mDescription = reader.getDesc();
}

void CSMWorld::MetaData::save (ESM::ESMWriter& esm) const
{
    esm.setFormat (mFormat);
    esm.setAuthor (mAuthor);
    esm.setDescription (mDescription);
}
