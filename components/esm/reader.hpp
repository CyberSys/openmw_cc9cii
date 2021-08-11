#ifndef COMPONENT_ESM_READER_H
#define COMPONENT_ESM_READER_H

#include <vector>

#include "common.hpp" // MasterData

namespace ToUTF8
{
    class Utf8Encoder;
}

namespace ESM
{
    class Reader
    {
    public:
        virtual ~Reader() {}

        static Reader* getReader(const std::string& filename);

        virtual inline bool isEsm4() const = 0;

        virtual inline bool hasMoreRecs() const = 0;

        virtual inline void setEncoder(ToUTF8::Utf8Encoder* encoder) = 0;

        // used to check for dependencies e.g. CS::Editor::run()
        virtual inline const std::vector<MasterData>& getGameFiles() const = 0;

        // used by ContentSelector::ContentModel::addFiles()
        virtual inline const std::string getAuthor() const = 0;
        virtual inline const std::string getDesc() const = 0;
        virtual inline int getFormat() const = 0;

        // used by CSMWorld::Data::startLoading() and getTotalRecords() for loading progress bar
        virtual inline int getRecordCount() const = 0;

        virtual void setModIndex(std::uint32_t index) = 0;

        // used by CSMWorld::Data::getTotalRecords()
        virtual void close() = 0;

    protected:
        bool getStringImpl(std::string& str, std::size_t size,
                Files::IStreamPtr filestream, ToUTF8::Utf8Encoder* encoder, bool hasNull = false);
    };
}

#endif // COMPONENT_ESM_READER_H
