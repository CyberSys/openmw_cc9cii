#ifndef GAME_MWCLASS_FOREIGNARMOR_H
#define GAME_MWCLASS_FOREIGNARMOR_H

#include "../mwworld/class.hpp"

namespace MWClass
{
    class ForeignArmor : public MWWorld::Class
    {
            virtual MWWorld::Ptr copyToCellImpl(const MWWorld::Ptr &ptr, MWWorld::CellStore &cell) const;

        public:

            virtual std::string getId (const MWWorld::Ptr& ptr) const;

            virtual void insertObjectRendering (const MWWorld::Ptr& ptr, const std::string& model, MWRender::RenderingInterface& renderingInterface) const;

            virtual void insertObject(const MWWorld::Ptr& ptr, const std::string& model, MWWorld::PhysicsSystem& physics) const;

            virtual std::string getName (const MWWorld::Ptr& ptr) const;

            virtual bool hasToolTip (const MWWorld::Ptr& ptr) const;

            virtual MWGui::ToolTipInfo getToolTipInfo (const MWWorld::Ptr& ptr) const;

            virtual boost::shared_ptr<MWWorld::Action> activate (const MWWorld::Ptr& ptr, const MWWorld::Ptr& actor) const;

            virtual std::pair<std::vector<int>, bool> getEquipmentSlots (const MWWorld::Ptr& ptr) const;

            virtual int getValue (const MWWorld::Ptr& ptr) const;

            virtual std::string getUpSoundId (const MWWorld::Ptr& ptr) const;

            virtual std::string getDownSoundId (const MWWorld::Ptr& ptr) const;

            virtual float getArmorRating (const MWWorld::Ptr& ptr) const;

            virtual std::string getInventoryIcon (const MWWorld::Ptr& ptr) const;

            virtual std::string getModel(const MWWorld::Ptr &ptr) const;

            virtual std::pair<int, std::string> canBeEquipped(const MWWorld::Ptr &ptr, const MWWorld::Ptr &npc) const;

            static void registerSelf();
    };
}

#endif