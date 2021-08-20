#include "mappings.hpp"

#include <stdexcept>

namespace ESM3
{
    ESM3::BodyPart::MeshPart getMeshPart(ESM3::PartReferenceType type)
    {
        switch(type)
        {
            case ESM3::PRT_Head:
                return ESM3::BodyPart::MP_Head;
            case ESM3::PRT_Hair:
                return ESM3::BodyPart::MP_Hair;
            case ESM3::PRT_Neck:
                return ESM3::BodyPart::MP_Neck;
            case ESM3::PRT_Cuirass:
                return ESM3::BodyPart::MP_Chest;
            case ESM3::PRT_Groin:
                return ESM3::BodyPart::MP_Groin;
            case ESM3::PRT_RHand:
                return ESM3::BodyPart::MP_Hand;
            case ESM3::PRT_LHand:
                return ESM3::BodyPart::MP_Hand;
            case ESM3::PRT_RWrist:
                return ESM3::BodyPart::MP_Wrist;
            case ESM3::PRT_LWrist:
                return ESM3::BodyPart::MP_Wrist;
            case ESM3::PRT_RForearm:
                return ESM3::BodyPart::MP_Forearm;
            case ESM3::PRT_LForearm:
                return ESM3::BodyPart::MP_Forearm;
            case ESM3::PRT_RUpperarm:
                return ESM3::BodyPart::MP_Upperarm;
            case ESM3::PRT_LUpperarm:
                return ESM3::BodyPart::MP_Upperarm;
            case ESM3::PRT_RFoot:
                return ESM3::BodyPart::MP_Foot;
            case ESM3::PRT_LFoot:
                return ESM3::BodyPart::MP_Foot;
            case ESM3::PRT_RAnkle:
                return ESM3::BodyPart::MP_Ankle;
            case ESM3::PRT_LAnkle:
                return ESM3::BodyPart::MP_Ankle;
            case ESM3::PRT_RKnee:
                return ESM3::BodyPart::MP_Knee;
            case ESM3::PRT_LKnee:
                return ESM3::BodyPart::MP_Knee;
            case ESM3::PRT_RLeg:
                return ESM3::BodyPart::MP_Upperleg;
            case ESM3::PRT_LLeg:
                return ESM3::BodyPart::MP_Upperleg;
            case ESM3::PRT_Tail:
                return ESM3::BodyPart::MP_Tail;
            default:
                throw std::runtime_error("PartReferenceType " +
                    std::to_string(type) + " not associated with a mesh part");
        }
    }

    std::string getBoneName(ESM3::PartReferenceType type)
    {
        switch(type)
        {
            case ESM3::PRT_Head:
                return "head";
            case ESM3::PRT_Hair:
                return "head"; // This is purposeful.
            case ESM3::PRT_Neck:
                return "neck";
            case ESM3::PRT_Cuirass:
                return "chest";
            case ESM3::PRT_Groin:
                return "groin";
            case ESM3::PRT_Skirt:
                return "groin";
            case ESM3::PRT_RHand:
                return "right hand";
            case ESM3::PRT_LHand:
                return "left hand";
            case ESM3::PRT_RWrist:
                return "right wrist";
            case ESM3::PRT_LWrist:
                return "left wrist";
            case ESM3::PRT_Shield:
                return "shield bone";
            case ESM3::PRT_RForearm:
                return "right forearm";
            case ESM3::PRT_LForearm:
                return "left forearm";
            case ESM3::PRT_RUpperarm:
                return "right upper arm";
            case ESM3::PRT_LUpperarm:
                return "left upper arm";
            case ESM3::PRT_RFoot:
                return "right foot";
            case ESM3::PRT_LFoot:
                return "left foot";
            case ESM3::PRT_RAnkle:
                return "right ankle";
            case ESM3::PRT_LAnkle:
                return "left ankle";
            case ESM3::PRT_RKnee:
                return "right knee";
            case ESM3::PRT_LKnee:
                return "left knee";
            case ESM3::PRT_RLeg:
                return "right upper leg";
            case ESM3::PRT_LLeg:
                return "left upper leg";
            case ESM3::PRT_RPauldron:
                return "right clavicle";
            case ESM3::PRT_LPauldron:
                return "left clavicle";
            case ESM3::PRT_Weapon:
                return "weapon bone";
            case ESM3::PRT_Tail:
                return "tail";
            default:
                throw std::runtime_error("unknown PartReferenceType");
        }
    }

    std::string getMeshFilter(ESM3::PartReferenceType type)
    {
        switch(type)
        {
            case ESM3::PRT_Hair:
                return "hair";
            default:
                return getBoneName(type);
        }
    }
}
