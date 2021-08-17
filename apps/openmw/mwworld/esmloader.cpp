#include "esmloader.hpp"
#include "esmstore.hpp"

#include <components/esm3/reader.hpp>

namespace MWWorld
{

EsmLoader::EsmLoader(MWWorld::ESMStore& store, std::vector<ESM::Reader*>& readers,
  ToUTF8::Utf8Encoder* encoder, Loading::Listener& listener)
  : ContentLoader(listener)
  , mEsm(readers)
  , mStore(store)
  , mEncoder(encoder)
{
}

void EsmLoader::load(const boost::filesystem::path& filepath, int& index)
{
  ContentLoader::load(filepath.filename(), index);

  ESM::Reader* reader = ESM::Reader::getReader(filepath.string());
  reader->setEncoder(mEncoder);

  reader->setModIndex(index);
  reader->setGlobalReaderList(&mEsm);
  //lEsm.open(filepath.string());
  mEsm[index] = reader;
  mStore.load(*mEsm[index], &mListener);
}

} /* namespace MWWorld */
