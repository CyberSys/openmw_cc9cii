#include "foreignobject.hpp"

#include <iostream> // FIXME

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <OgreSkeletonInstance.h>
#include <OgreResourceGroupManager.h> // FIXME

#include <QRegExp>

#include "../../model/world/data.hpp"
#include "../../model/world/ref.hpp"
#include "../../model/world/refidcollection.hpp"

#include "../world/physicssystem.hpp"

#include "elements.hpp"

void CSVRender::ForeignObject::clearSceneNode (Ogre::SceneNode *node)
{
    for (Ogre::SceneNode::ObjectIterator iter = node->getAttachedObjectIterator();
        iter.hasMoreElements(); )
    {
        Ogre::MovableObject* object = dynamic_cast<Ogre::MovableObject*> (iter.getNext());
        node->getCreator()->destroyMovableObject (object);
    }

    for (Ogre::SceneNode::ChildNodeIterator iter = node->getChildIterator();
        iter.hasMoreElements(); )
    {
        Ogre::SceneNode* childNode = dynamic_cast<Ogre::SceneNode*> (iter.getNext());
        clearSceneNode (childNode);
        node->getCreator()->destroySceneNode (childNode);
   }
}

void CSVRender::ForeignObject::clear()
{
    mObject.setNull();

    if (mBase)
        clearSceneNode (mBase);
}

void CSVRender::ForeignObject::update()
{
    if(!mObject.isNull())
        mPhysics->removePhysicsObject(mBase->getName());

    clear();

    std::string model;
    int error = 0; // 1 referenceable does not exist, 2 referenceable does not specify a mesh

    //const CSMForeign::RefIdCollection& referenceables = mData.getReferenceables();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Container> >& container = mData.getForeignContainers();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::AnimObject> >& anio = mData.getForeignAnimObjs();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::MiscItem> >& misc = mData.getForeignMiscItems();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Activator> >& acti = mData.getForeignActivators();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Npc> >& npc = mData.getForeignNpcs();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Armor> >& armor = mData.getForeignArmors();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Flora> >& flora = mData.getForeignFloras();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Grass> >& grass = mData.getForeignGrasses();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Tree> >& trees = mData.getForeignTrees();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Light> >& lights = mData.getForeignLights();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Book> >& book = mData.getForeignBooks();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Furniture> >& furn = mData.getForeignFurnitures();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Sound> >& sound = mData.getForeignSounds();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Weapon> >& weap = mData.getForeignWeapons();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Door> >& door = mData.getForeignDoors();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Ammo> >& ammo = mData.getForeignAmmos();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Clothing> >& cloth = mData.getForeignClothings();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Potion> >& potion = mData.getForeignPotions();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Apparatus> >& appa = mData.getForeignApparatuses();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Ingredient> >& ingr = mData.getForeignIngredients();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::SigilStone> >& sigil = mData.getForeignSigilStones();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::SoulGem> >& soul = mData.getForeignSoulGems();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Key> >& keys = mData.getForeignKeys();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Hair> >& hair = mData.getForeignHairs();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Eyes> >& eyes = mData.getForeignEyesSet();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Creature> >& crea = mData.getForeignCreatures();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::LeveledCreature> >& lvlc = mData.getForeignLvlCreatures();
    const CSMForeign::IdCollection<CSMForeign::IdRecord<ESM4::Static> >& referenceables = mData.getForeignStatics(); // FIXME: use statics only for now

    //int index = referenceables.searcFormhId (mReferenceableId);
    // get the formId of the base object
    const CSMForeign::CellRefCollection<CSMForeign::CellRef>& refs = mData.getForeignReferences();
    ESM4::FormId baseObj = refs.getRecord(refs.searchFormId(mReferenceId)).get().mBaseObj;
    int index = referenceables.searchFormId(baseObj);

    ESM4::FormId parent = refs.getRecord(refs.searchFormId(mReferenceId)).get().mEsp.parent;
    if (parent == 0x000e4ee3 || // FIXME testing WhiterunPlayerHouseKitchenStart
        parent == 0x000e4ef9 ||               // WhiterunPlayerHouseAlchemyLaboratoryStart
        parent == 0x000e4f0a ||               // WhiterunPlayerHousePackageHouseCarlStart
        parent == 0x000e4efb ||               // WhiterunPlayerHouseLoftStart
        parent == 0x000e4ef1 ||               // WhiterunPlayerHouseDiningRoomStart
        parent == 0x000e4ecb /*||               // WhiterunPlayerHouseBedroomStart
        parent == 0x000e4ec8 ||               // WhiterunPlayerHouseAlchemyLaboratory
        parent == 0x000e4eca ||               // WhiterunPlayerHousePackageHouseCarl
        parent == 0x000e4ec9 */                 // WhiterunPlayerHouseDecoratoinDiningRoom
        )
    {
        index = 0;
        error = 3; // Do not draw
    }

    // FIXME: this is a massive hack to get around the lack of a referenceable table
    if (index==-1)
    {
        error = 1;

        int contIndex = container.searchFormId(baseObj);
        if (contIndex != -1)
        {
            error = 0;

            model = container.getData (contIndex,
                   container.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();

            if (model.empty())
                error = 2;
        }
        else if (npc.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = npc.searchFormId(baseObj);
            model = npc.getData (extraIndex,
                   npc.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            std::cout << "obj is an npc " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (crea.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = crea.searchFormId(baseObj);
            model = crea.getData (extraIndex,
                   crea.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            std::cout << "obj is an creature " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (lvlc.searchFormId(baseObj) != -1)
        {
            const CSMForeign::IdRecord<ESM4::LeveledCreature>& lcreature = lvlc.getRecord(ESM4::formIdToString(baseObj)).get();
            ESM4::FormId templ = 0;
            for (unsigned int i = lcreature.mLvlObject.size(); i-- > 0;)
            {
                templ = lcreature.mLvlObject[i].item;
                //ESM4::FormId templ = lcreature.mTemplate;
                int extraIndex = -1;
                extraIndex = crea.searchFormId(templ);
                if (extraIndex != -1)
                {
                    CSMForeign::IdRecord<ESM4::Creature> creaRec = crea.getRecord(extraIndex).get();

                    model = crea.getData (extraIndex,
                       crea.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();

                    // Need to do something similar to ctor of MWRender::CreatureAnimation, i.e.
                    // create a creature object which is called from MWRender::Actors::insertCreature()
                    // or do something more complicated like MWRender::NpcAnimation::updateNpcBase()
                    // which is called when constructed by MWRender::Actors::insertNpc()

                    std::cout << "obj is an leveled creature " << i /*ESM4::formIdToString(templ)*/ << ", " << model << std::endl;
                    //std::cout << "creature inventory " << creaRec.mInventory.size() << std::endl;
                    for (unsigned int j = 0; j < creaRec.mInventory.size(); ++j)
                    {
                        int invIndex = cloth.searchFormId(creaRec.mInventory[j].item);
                        if (invIndex != -1)
                        {
                            const CSMForeign::IdRecord<ESM4::Clothing>& clothRec = cloth.getRecord(invIndex).get();
                            std::cout << clothRec.mEditorId << std::endl;
#if 0
                            NifOgre::ObjectScenePtr object
                                = NifOgre::Loader::createObjects(mObjects[id]->getObject()->mSkelBase,
                                                                 "Bip01", // not used for skinned
                                                                 /*"Bip01"*/"", // ??
                                                                 mObjects[id]->getSceneNode(),
                                                                 "meshes\\"+clothRec.mModel);
                            object->setVisibilityFlags (Element_Reference);
#endif
                        }

                        invIndex = armor.searchFormId(creaRec.mInventory[j].item);
                        if (invIndex != -1)
                        {
                            const CSMForeign::IdRecord<ESM4::Armor>& armorRec = armor.getRecord(invIndex).get();
                            std::cout << armorRec.mEditorId << std::endl;
                        }

                        invIndex = weap.searchFormId(creaRec.mInventory[j].item);
                        if (invIndex != -1)
                        {
                            const CSMForeign::IdRecord<ESM4::Weapon>& weapRec = weap.getRecord(invIndex).get();
                            std::cout << weapRec.mEditorId << std::endl;
                        }
                    }
                    for (unsigned int j = 0; j < creaRec.mNif.size(); ++j)
                    {
                        std::cout << "Nif " << creaRec.mNif[j] << std::endl;
                    }

                    //break;
                }
                else if (lvlc.searchFormId(templ) != -1)
                    std::cout << "obj is a lvl creature " << i << ", " << ESM4::formIdToString(templ) << ", " << model << std::endl;
                else
                    std::cout << "obj is not a creature " << i << ", " << ESM4::formIdToString(templ) << ", " << model << std::endl;

            }

            if (model.empty()) // not a creature
            {
                error = 2;

                for (unsigned int i = 0; i < lcreature.mLvlObject.size(); ++i)
                {
                    templ = lcreature.mLvlObject[i].item;
                    //ESM4::FormId templ = lcreature.mTemplate;
                    int extraIndex = -1;
                    extraIndex = npc.searchFormId(templ);
                    if (extraIndex != -1)
                    {
                        model = npc.getData (extraIndex,
                           npc.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
#if 0
                        // FIXME temp testing
                        CSMForeign::IdRecord<ESM4::Npc npcRec> = npc.getRecord(extraIndex).get();
                        for (unsigned int j = 0; j < npcRec.mInventory.size(); ++j)
                        {
                            int invIndex = cloth.searchFormId(npcRec.mInventory[j].item);
                            if (invIndex != -1)
                            {
                                const CSMForeign::IdRecord<ESM4::Clothing>& clothRec = cloth.getRecord(invIndex).get();
                                model = clothRec.mModel;
                                //std::cout << clothRec.mEditorId << std::endl;
                                break;
                            }
                        }
#endif
                        break;
                    }
                }
                std::cout << "obj is an leveled npc " << ESM4::formIdToString(templ) << ", " << model << std::endl;

                if (!model.empty())
                    error = 0;
            }
            else
            {
                //std::cout << "obj is an leveled creature " << ESM4::formIdToString(templ) << ", " << model << std::endl;
                error = 0;
            }
        }
        else if (eyes.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = eyes.searchFormId(baseObj);
            model = eyes.getData (extraIndex,
                   eyes.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            std::cout << "obj is an eye " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 3; // FIXME
            else
                error = 0;
        }
        else if (hair.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = hair.searchFormId(baseObj);
            model = hair.getData (extraIndex,
                   hair.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            std::cout << "obj is hair " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (armor.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = armor.searchFormId(baseObj);
            model = armor.getData (extraIndex,
                   armor.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a armor obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (anio.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = anio.searchFormId(baseObj);
            model = anio.getData (extraIndex,
                   anio.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            std::cout << "obj is a anim obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (misc.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = misc.searchFormId(baseObj);
            model = misc.getData (extraIndex,
                   misc.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a misc obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (sound.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = sound.searchFormId(baseObj);
            //std::cout << "obj is a sound obj " << ESM4::formIdToString(baseObj) << std::endl;
            model = "marker_sound.nif";

            if (model.empty())
                error = 3; // FIXME for testing only
            else
                error = 0;
        }
        else if (weap.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = weap.searchFormId(baseObj);
            model = weap.getData (extraIndex,
                   weap.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a weapon obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (door.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = door.searchFormId(baseObj);
            model = door.getData (extraIndex,
                   door.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a door obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (ammo.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = ammo.searchFormId(baseObj);
            model = ammo.getData (extraIndex,
                   ammo.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a ammo obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (cloth.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = cloth.searchFormId(baseObj);
            model = cloth.getData (extraIndex,
                   cloth.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a cloth obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (potion.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = potion.searchFormId(baseObj);
            model = potion.getData (extraIndex,
                   potion.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a potion obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (appa.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = appa.searchFormId(baseObj);
            model = appa.getData (extraIndex,
                   appa.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a appa obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (ingr.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = ingr.searchFormId(baseObj);
            model = ingr.getData (extraIndex,
                   ingr.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a ingr obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (sigil.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = sigil.searchFormId(baseObj);
            model = sigil.getData (extraIndex,
                   sigil.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            std::cout << "obj is a sigil stone " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (soul.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = soul.searchFormId(baseObj);
            model = soul.getData (extraIndex,
                   soul.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a soul gem " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (keys.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = keys.searchFormId(baseObj);
            model = keys.getData (extraIndex,
                   keys.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            std::cout << "obj is a key " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (book.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = book.searchFormId(baseObj);
            model = book.getData (extraIndex,
                   book.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a book obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (furn.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = furn.searchFormId(baseObj);
            model = furn.getData (extraIndex,
                   furn.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            //std::cout << "obj is a furniture obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else if (grass.searchFormId(baseObj) != -1)
        {
            int extraIndex = -1;
            extraIndex = grass.searchFormId(baseObj);
            model = grass.getData (extraIndex,
                   grass.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
            std::cout << "obj is a grass " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

            if (model.empty())
                error = 2;
            else
                error = 0;
        }
        else
        {
            int florIndex = flora.searchFormId(baseObj);
            if (florIndex != -1)
            {
                model = flora.getData (florIndex,
                       flora.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();

                if (model.empty())
                    error = 2;
                else
                    error = 0;
            }
            else
            {
                int actiIndex = acti.searchFormId(baseObj);
                if (actiIndex != -1)
                {
                    model = acti.getData (actiIndex,
                           acti.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
                    //std::cout << "obj is an acti obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

                    if (model.empty())
                        error = 2;
                    else
                        error = 0;
                }
                else
                {
                    int treeIndex = trees.searchFormId(baseObj);
                    if (treeIndex != -1)
                    {
                        std::string realModel = trees.getData (treeIndex,
                               trees.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
                        //model = "_DoD_flora_tree_b_1.NIF";
                        //model = "J_W\\JW_TREE_A2.NIF";
                        //if (QString(realModel.c_str()).contains(QRegExp("nif$", Qt::CaseInsensitive)))
                            //model = realModel;
                        if (QString(realModel.c_str()).contains(QRegExp("Tree", Qt::CaseInsensitive)))
                            model = "f\\flora_tree_01.nif";
                        else if (QString(realModel.c_str()).contains(QRegExp("Shrub", Qt::CaseInsensitive)))
                            model = "f\\flora_bush_01.nif";
                        else
                        {
                            model = "plants\\floraflaxyellow.nif";
                            //std::cout << "obj is an tree obj " << ESM4::formIdToString(baseObj)
                                      //<< ", " << realModel << std::endl;
                        }

                        if (model.empty())
                            error = 2;
                        else
                            error = 0;
                    }
                    else
                    {
                        int lightIndex = lights.searchFormId(baseObj);
                        if (lightIndex != -1)
                        {
                            model = lights.getData (lightIndex,
                                   lights.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();
                            //std::cout << "obj is an light obj " << ESM4::formIdToString(baseObj) << ", " << model << std::endl;

                            if (model.empty())
                                error = 3;
                            else
                                error = 0;
                        }
                        else
                        {
                            std::cout << "obj not static/anio/misc/acti/container/whatever "
                                      << ESM4::formIdToString(baseObj) << std::endl;
    //const CSMForeign::RefCollection& refs = mData.getForeignReferences();
    //ESM4::FormId baseObj = refs.getRecord(refs.searchFormId(mReferenceId)).get().mBaseObj;
    //int index = referenceables.searchFormId(baseObj);
                        }
                    }
                }
            }
        }
        //else
            //std::cout << "obj not static/anio/misc/acti/container " << ESM4::formIdToString(baseObj) << std::endl;
    }
    else
    {
        /// \todo check for Deleted state (error 1)

        model = referenceables.getData (index,
                referenceables.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).toString().toUtf8().constData();

        if (model.empty())
            error = 2;

#if 0
        // sanity check position
        const CSMForeign::CellRef& reference = getReference();
        std::cout << "object cell " << floor(reference.mPos.pos[0]/4096) << ", "
                                    << floor(reference.mPos.pos[1]/4096) << ", "
                                    <<       reference.mPos.pos[2]       << std::endl;
#endif
    }

    //meshes\furniture\blacksmithforgemarker.nif
    //meshes\furniture\smeltermarker.nif has more than 256 bones
    if (error || QString(model.c_str()).contains(QRegExp("blacksmithforgemarker", Qt::CaseInsensitive))
              || QString(model.c_str()).contains(QRegExp("smeltermarker", Qt::CaseInsensitive)))
    {
        Ogre::Entity* entity = mBase->getCreator()->createEntity (Ogre::SceneManager::PT_CUBE);
        entity->setMaterialName("BaseWhite"); /// \todo adjust material according to error
        entity->setVisibilityFlags (Element_Reference);

        if (error != 3) // FIXME for testing Skyrim
            mBase->attachObject (entity); // ignore script lights or sounds
    }
    else
    {
        // Oblivion.eam contains some paths with leading space but can't use below since
        // some file names have spaces
        //   model.erase(std::remove_if(model.begin(), model.end(), ::isspace), model.end());

        // first find the beginning of the filename
        std::string::size_type separator = model.find_last_of('\\');
        std::string filename;
        if (separator != std::string::npos)
            filename = model.substr(separator+1);
        else
            filename = model;

        // then trim the front
        std::string::size_type start = filename.find_first_not_of(' ');

        if (start == std::string::npos)
            throw std::runtime_error("empty model " + model); // huh? just checked above

        // now put it back together
        std::string trimmedModel = model.substr(0, separator+1) + filename.substr(start);

        // FIXME: quick hack to continue testing
        if (!Ogre::ResourceGroupManager::getSingleton().resourceExistsInAnyGroup("Meshes\\" + trimmedModel))
        {
            std::cout << "Giving up rendering obj, missing Meshes\\" << trimmedModel << std::endl;
            return;
        }

        //std::cout << "Using model: " << model << std::endl;
        mObject = NifOgre::Loader::createObjects (mBase, "Meshes\\" + trimmedModel);

        if (!mObject.isNull())
            mObject->setVisibilityFlags (Element_Reference); // FIXME: find root cause of null pointer

        if (mObject->mSkelBase && mObject->mSkelBase->getSkeleton()->hasBone("AttachLight"))
        {
            std::cout << "Light, model: " << model << std::endl;
            mObject->mLights.push_back(mBase->getCreator()->createLight());
            Ogre::Light *light = mObject->mLights.back();
            light->setType(Ogre::Light::LT_POINT);
            light->setDiffuseColour(0.9, 0.8, 0.0);
            //http://www.ogre3d.org/tikiwiki/tiki-index.php?page=-Point%20Light%20Attenuation
            light->setSpecularColour(1.0, 1.0, 0.0);
            //light->setCastShadows(true);

            float radius = 512;

            // copied from MWRender::Animation
            float threshold = 0.03f;
            float linearAttenuation = /*linearValue*/3.0 / radius;
            float quadraticAttenuation = /*quadraticValue*/16.0 / std::pow(radius, 2);
            float activationRange = std::max(activationRange, 1.0f / (threshold * linearAttenuation));
            //float activationRange = std::sqrt(1.0f / (threshold * quadraticAttenuation));
            light->setAttenuation(activationRange, 0.5, linearAttenuation, quadraticAttenuation);

            mObject->mSkelBase->attachObjectToBone("AttachLight", light);
        }

        if (mPhysics && !(mReferenceId == 0))
        {
            const CSMForeign::CellRef& reference = getReference();

            // position
            Ogre::Vector3 position;
            if (!mForceBaseToZero)
                position = Ogre::Vector3(reference.mPos.pos[0], reference.mPos.pos[1], reference.mPos.pos[2]);

            // orientation
            Ogre::Quaternion xr (Ogre::Radian (-reference.mPos.rot[0]), Ogre::Vector3::UNIT_X);
            Ogre::Quaternion yr (Ogre::Radian (-reference.mPos.rot[1]), Ogre::Vector3::UNIT_Y);
            Ogre::Quaternion zr (Ogre::Radian (-reference.mPos.rot[2]), Ogre::Vector3::UNIT_Z);

            // NOTE: physics system and mouse picking assume reference naming convension (i.e."^ref#")
            // FIXME: sometimes wrong model name is passed to Ogre
            mPhysics->addObject("meshes\\" + trimmedModel, mBase->getName(), "ref#"+ESM4::formIdToString(mReferenceId), reference.mScale, position, zr*yr*xr); // FIXME: EXperiment
        }
    }
}

void CSVRender::ForeignObject::adjust()
{
    if (mReferenceId == 0)
        return;

    const CSMForeign::CellRef& reference = getReference();

    // position
    if (!mForceBaseToZero)
        mBase->setPosition (Ogre::Vector3 (
            reference.mPos.pos[0], reference.mPos.pos[1], reference.mPos.pos[2]));

    // orientation
    Ogre::Quaternion xr (Ogre::Radian (-reference.mPos.rot[0]), Ogre::Vector3::UNIT_X);
    Ogre::Quaternion yr (Ogre::Radian (-reference.mPos.rot[1]), Ogre::Vector3::UNIT_Y);
    Ogre::Quaternion zr (Ogre::Radian (-reference.mPos.rot[2]), Ogre::Vector3::UNIT_Z);

    mBase->setOrientation (zr*yr*xr); // FIXME: EXperiment

    // scale
    mBase->setScale (reference.mScale, reference.mScale, reference.mScale);
}

const CSMForeign::CellRef& CSVRender::ForeignObject::getReference() const
{
    if (mReferenceId == 0)
        throw std::logic_error ("object does not represent a reference");

    const CSMForeign::CellRefCollection<CSMForeign::CellRef>& refs = mData.getForeignReferences();
    return refs.getRecord (refs.searchFormId(mReferenceId)).get();
}

CSVRender::ForeignObject::ForeignObject (const CSMWorld::Data& data, Ogre::SceneNode *cellNode,
    ESM4::FormId id, bool referenceable, boost::shared_ptr<CSVWorld::PhysicsSystem> physics,
    bool forceBaseToZero)
: mData (data), mBase (0), mForceBaseToZero (forceBaseToZero), mPhysics(physics)
{
    mBase = cellNode->createChildSceneNode();

    mReferenceId = id;
#if 0
    if (referenceable)
    {
        mReferenceableId = id;
    }
    else
    {
        mReferenceId = id;
        mReferenceableId = getReference().mRefID;
    }
#endif

    update();
    adjust();
}

CSVRender::ForeignObject::~ForeignObject()
{
    clear();

    if (mBase)
    {
        if(mPhysics) // preview may not have physics enabled
            mPhysics->removeObject(mBase->getName());

        mBase->getCreator()->destroySceneNode (mBase);
    }
}

bool CSVRender::ForeignObject::referenceableDataChanged (const QModelIndex& topLeft,
    const QModelIndex& bottomRight)
{
    const CSMWorld::RefIdCollection& referenceables = mData.getReferenceables();

    int index = referenceables.searchId (mReferenceableId);

    if (index!=-1 && index>=topLeft.row() && index<=bottomRight.row())
    {
        update();
        adjust();
        return true;
    }

    return false;
}

bool CSVRender::ForeignObject::referenceableAboutToBeRemoved (const QModelIndex& parent, int start,
    int end)
{
    const CSMWorld::RefIdCollection& referenceables = mData.getReferenceables();

    int index = referenceables.searchId (mReferenceableId);

    if (index!=-1 && index>=start && index<=end)
    {
        // Deletion of referenceable-type objects is handled outside of Object.
        if (!(mReferenceId == 0))
        {
            update();
            adjust();
            return true;
        }
    }

    return false;
}

bool CSVRender::ForeignObject::referenceDataChanged (const QModelIndex& topLeft,
    const QModelIndex& bottomRight)
{
    if (mReferenceId == 0)
        return false;

    const CSMForeign::CellRefCollection<CSMForeign::CellRef>& references = mData.getForeignReferences();

    int index = references.searchFormId(mReferenceId);

    if (index!=-1 && index>=topLeft.row() && index<=bottomRight.row())
    {
        int columnIndex =
            references.findColumnIndex (CSMWorld::Columns::ColumnId_ReferenceableId);

        if (columnIndex>=topLeft.column() && columnIndex<=bottomRight.row())
        {
            mReferenceableId =
                references.getData (index, columnIndex).toString().toUtf8().constData();
        }

        update();
        adjust();
        return true;
    }

    return false;
}

std::string CSVRender::ForeignObject::getReferenceId() const
{
    return "";// mReferenceId;
}

std::string CSVRender::ForeignObject::getReferenceableId() const
{
    return mReferenceableId;
}

NifOgre::ObjectScenePtr CSVRender::ForeignObject::getObject() const
{
    return mObject;
}
Ogre::SceneNode *CSVRender::ForeignObject::getSceneNode() const
{
    return mBase;
}