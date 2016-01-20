///This file holds the main classes of NIF Records used by everything else.
#ifndef OPENMW_COMPONENTS_NIF_BASE_HPP
#define OPENMW_COMPONENTS_NIF_BASE_HPP

#include "record.hpp"
#include "niffile.hpp"
#include "recordptr.hpp"
#include "nifstream.hpp"
#include "nifkey.hpp"

namespace Nif
{
/** A record that can have extra data. The extra data objects
    themselves descend from the Extra class, and all the extra data
    connected to an object form a linked list
*/
class Extra : public Record
{
public:
    NiExtraDataPtr extra; // FIXME: how to make this part of extras rather than keep separate members?
    NiExtraDataList extras;
    bool hasExtras;

    void read(NIFStream *nif);
    void post(NIFFile *nif);
};

// NiTimeController
class Controller : public Record
{
public:
    ControllerPtr next;
    int flags;
    float frequency, phase;
    float timeStart, timeStop;
    ControlledPtr target;

    void read(NIFStream *nif);
    void post(NIFFile *nif);
};

/// Anything that has a controller
class Controlled : public Extra // FIXME: should be changed from "is an Extra" to "has an Extra"
{
public:
    ControllerPtr controller;

    void read(NIFStream *nif);
    void post(NIFFile *nif);
};

class NiParticleModifier : public Record
{
public:
    NiParticleModifierPtr extra;
    ControllerPtr controller;

    void read(NIFStream *nif);
    void post(NIFFile *nif);
};

/// Has name, extra-data and controller
class Named : public Controlled
{
public:
    std::string name;

    void read(NIFStream *nif);
};
typedef Named NiSequenceStreamHelper;

} // Namespace
#endif
