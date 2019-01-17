#ifndef GAME_MWCLASS_FOREIGNDOOR_H
#define GAME_MWCLASS_FOREIGNDOOR_H

#include <extern/esm4/door.hpp>

#include "../mwworld/class.hpp"

namespace MWClass
{
    class ForeignDoor : public MWWorld::Class
    {
            void ensureCustomData (const MWWorld::Ptr& ptr) const;
    
            virtual MWWorld::Ptr copyToCellImpl(const MWWorld::Ptr &ptr, MWWorld::CellStore &cell) const;

        public:

            /// Return ID of \a ptr
            virtual std::string getId (const MWWorld::Ptr& ptr) const;

            virtual void insertObjectRendering (const MWWorld::Ptr& ptr, const std::string& model, MWRender::RenderingInterface& renderingInterface) const;
            ///< Add reference into a cell for rendering

            virtual void insertObject(const MWWorld::Ptr& ptr, const std::string& model, MWWorld::PhysicsSystem& physics) const;

            virtual std::string getName (const MWWorld::Ptr& ptr) const;
            ///< \return name (the one that is to be presented to the user; not the internal one);
            /// can return an empty string.

            static void registerSelf();

            virtual std::string getModel(const MWWorld::Ptr &ptr) const;

            virtual boost::shared_ptr<MWWorld::Action> activate (const MWWorld::Ptr& ptr,
                const MWWorld::Ptr& actor) const;
            ///< Generate action for activation

            virtual bool hasToolTip (const MWWorld::Ptr& ptr) const;
            ///< @return true if this object has a tooltip when focused (default implementation: false)

            virtual MWGui::ToolTipInfo getToolTipInfo (const MWWorld::Ptr& ptr) const;
            ///< @return the content of the tool tip to be displayed. raises exception if the object has no tooltip.

            static std::string getDestination (const MWWorld::LiveCellRef<ESM4::Door>& door);
            ///< @return destination cell name or token

            /// 0 = nothing, 1 = opening, 2 = closing
            virtual int getDoorState (const MWWorld::Ptr &ptr) const;

            /// This does not actually cause the door to move. Use World::activateDoor instead.
            virtual void setDoorState (const MWWorld::Ptr &ptr, int state) const;

            virtual void readAdditionalState (const MWWorld::Ptr& ptr, const ESM::ObjectState& state)
                const;
            ///< Read additional state from \a state into \a ptr.

            virtual void writeAdditionalState (const MWWorld::Ptr& ptr, ESM::ObjectState& state)
                const;
            ///< Write additional state from \a ptr into \a state.
    };
}

#endif