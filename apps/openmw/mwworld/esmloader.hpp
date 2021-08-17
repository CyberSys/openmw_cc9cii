#ifndef ESMLOADER_HPP
#define ESMLOADER_HPP

#include <vector>

#include "contentloader.hpp"

namespace ToUTF8
{
  class Utf8Encoder;
}

namespace ESM
{
    class Reader;
}

namespace MWWorld
{

class ESMStore;

struct EsmLoader : public ContentLoader
{
    EsmLoader(MWWorld::ESMStore& store, std::vector<ESM::Reader*>& readers,
      ToUTF8::Utf8Encoder* encoder, Loading::Listener& listener);

    void load(const boost::filesystem::path& filepath, int& index) override;

    private:
      std::vector<ESM::Reader*>& mEsm;
      MWWorld::ESMStore& mStore;
      ToUTF8::Utf8Encoder* mEncoder;
};

} /* namespace MWWorld */

#endif // ESMLOADER_HPP
