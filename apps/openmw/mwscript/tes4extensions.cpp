#include "tes4extensions.hpp"

#include <stdexcept>
#include <cmath> // std::round

#include <boost/format.hpp>

#include <MyGUI_LanguageManager.h>

#include <extern/esm4/qust.hpp>

#include <components/compiler/extensions.hpp>
#include <components/compiler/opcodes.hpp>
//#include <components/tes4compiler/extensions.hpp>
#include <components/tes4compiler/opcodes.hpp>

#include <components/interpreter/interpreter.hpp>
#include <components/interpreter/runtime.hpp>
#include <components/interpreter/opcodes.hpp> // classes Opcode0, Opcode1 and Opcode2

#include <components/misc/stringops.hpp>

#include <components/esm/loadskil.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/windowmanager.hpp"
#include "../mwbase/world.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/containerstore.hpp"
#include "../mwworld/actionequip.hpp"
#include "../mwworld/inventorystore.hpp"
#include "../mwworld/esmstore.hpp"

#include "../mwrender/animation.hpp"

#include "interpretercontext.hpp"
#include "ref.hpp"

namespace MWScript
{
    namespace Tes4
    {
        template<class R>
        class OpIsActionRef : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    // ptr is IDGate01 in CGAmbushAGateScript but what we really want is
                    // the object/npc/player trying to open the door.
                    //
                    // Maybe we need to extend MWWorld::Ptr or MWScript::InterpreterContext so
                    // that it can return the reference of the object taking the action.
                    //
                    // NOTE: based on OMW::Engine::activate() it appears that in TES3 NPC's
                    //       can't activate (probably because input manager initiates it)
                    // NOTE: MWMechanics::AiActivate::execute() seems to allow calling
                    //       MWWorld::activate()
                    MWWorld::Ptr ptr = R()(runtime); // ImplicitRef

                    // get the actor stored by MWWorld::activate()
                    // NOTE: assumes actor is always valid
                    MWWorld::Ptr actor = R()(runtime, true/*required*/, false/*activeOnly*/, true/*actor*/);

                    //std::cout << "IsActionRef: actor " << actor.getCellRef().getEditorId() << std::endl;

                    MWWorld::Ptr object;
                    int index = runtime[0].mInteger;
                    if ((index & 0x0800) == 0 && (index & 0x0400) != 0)
                    {
                        int index2 = runtime.getIntegerLiteral(index & 0x03ff);
                        int ref = runtime.getContext().getLocalRef(index2);
                        object
                            = MWBase::Environment::get().getWorld()->searchPtrViaFormId(ref, true/*activeOnly*/);
                    }
                    else
                    {
                        // here the literal string is "player" in CGAmbushAGateScript
                        // FIXME: "playerref" needs support as well?
                        std::string editorId = runtime.getStringLiteral(index);
                        object
                            = MWBase::Environment::get().getWorld()->searchPtrViaEditorId(editorId, true/*activeOnly*/);
                    }
                    runtime.pop();


                    if (actor == object)
                    {
                        std::cout << "ActionRef is "
                            << (object.getBase()->isTypeESM4() ?
                               boost::variant2::get<MWWorld::CellReferenceWrap>(object.getCellRef()).get().getEditorId() :
                               boost::variant2::get<MWWorld::CellRefWrap>(object.getCellRef()).get().getRefId())
                            << std::endl; // FIXME: temp testing

                        runtime.push (true);
                    }
                    else
                        runtime.push (false);
                }
        };

        template<class R>
        class OpGetStage : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    // assumed that there is a mandatory string argument
                    std::string editorId = runtime.getStringLiteral (runtime[0].mInteger);
                    runtime.pop();

                    const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();
                    const ESM4::Quest *quest = store.getForeign<ESM4::Quest>().search(editorId);

                    // FIXME


                    //std::cout << "GetStage: " << editorId << std::endl; // FIXME: temp testing

                    runtime.push (0); // FIXME: just a dummy for testing
                }
        };

        template<class R>
        class OpSetStage : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    // there is a mandatory string argument
                    std::string questName = runtime.getStringLiteral (runtime[0].mInteger);
                    runtime.pop();

                    // also an integer
                    Interpreter::Type_Integer stage = runtime[0].mInteger;
                    runtime.pop();

                    const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();
                    const ESM4::Quest *quest = store.getForeign<ESM4::Quest>().search(questName);

                    // FIXME


                    std::cout << "SetStage: " << questName << ", " << stage << std::endl; // FIXME: temp testing
                }
        };

        template<class R>
        class OpGetLocked : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    //Interpreter::Type_Integer lockLevel = ptr.getCellRef<MWWorld::CellReference>().getLockLevel();
                    bool isLocked = boost::variant2::get<MWWorld::CellReferenceWrap>(ptr.getCellRef()).get().isLocked();

                    std::cout << "GetLocked: " << (isLocked ? "true" : "false") << std::endl; // FIXME: temp testing

                    runtime.push (isLocked);
                }
        };

        template<class R>
        class OpGetOpenState : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    // FIXME: need to be able to retrieve the animation states of doors

                    int openState = 0; // 0 = not a door, 1 = open, 2 = opening, 3 = closed, 4 = closing

                    // FIXME: temp testing
                    //std::cout << "GetOpenState: " << ptr.getForeign<ESM4::Door>()->mBase->mEditorId << std::endl;

                    runtime.push (openState);
                }
        };

        template<class R>
        class OpActivate : public Interpreter::Opcode1
        {
            std::string mCurrentActor;
            std::string mCurrentObject;

            public:
                virtual void execute (Interpreter::Runtime& runtime, unsigned int arg0)
                {

                    MWWorld::Ptr ptr = R()(runtime);

                    if (ptr.isEmpty())
                        return; // e.g. "myParent.activate mySelf 1" when myParent is 0

                    // handle the arguments
                    MWWorld::Ptr actor;
                    Interpreter::Type_Integer flag = 0;
                    if (arg0 >= 1) // explicit
                    {
                        int index = runtime[0].mInteger;
                        if ((index & 0x0800) == 0 && (index & 0x0400) != 0)
                        {
                            int index2 = runtime.getIntegerLiteral(index & 0x03ff);
                            int ref = runtime.getContext().getLocalRef(index2);
                            actor
                                = MWBase::Environment::get().getWorld()->searchPtrViaFormId(ref, true/*activeOnly*/);

                            std::cout << "Activate: first arg " << ESM4::formIdToString(ref) << std::endl;
                        }
                        else
                        {
                            std::string editorId = runtime.getStringLiteral(index);
                            actor
                                = MWBase::Environment::get().getWorld()->searchPtrViaEditorId(editorId, true/*activeOnly*/);
                            std::cout << "Activate: first arg " << editorId << std::endl;
                        }
                        runtime.pop();
                    }
                    else // implicit
                    {
                        // get the actor stored by MWWorld::activate()
                        actor = R()(runtime, true/*required*/, false/*activeOnly*/, true/*actor*/);
                    }

                    // FIXME: currently below flag is not used (prob. need a new flag in executeActivation)
                    // see https://cs.elderscrolls.com/index.php?title=Activate
                    // If the RunOnActivateFlag is set to 1, then the OnActivate block of the object (if any) will be
                    // run instead of the default activation. (In other words, act just as if ActivatorID activated it
                    // directly -- NPC used it, Player moused over and clicked on it, etc.)
                    if (arg0 >= 2)
                    {
                        flag = runtime[0].mInteger;
                        std::cout << "Activate: 2nd arg " << flag << std::endl;
                        runtime.pop();
                    }

                    using namespace boost::variant2;
                    bool tes4Actor = actor.getBase()->isTypeESM4();
#if 1  // FIXME: temp testing
                    if (ptr && actor)
                        std::cout << get<MWWorld::CellReferenceWrap>(ptr.getCellRef()).get().getEditorId() << "Activate: "
                                  << (tes4Actor ?
                                      get<MWWorld::CellReferenceWrap>(actor.getCellRef()).get().getEditorId() :
                                      get<MWWorld::CellRefWrap>(actor.getCellRef()).get().getRefId())
                                      << std::endl;
                    else
                        std::cout << "Activate" << std::endl;
#endif
                    // FIXME: hack to avoid an infinite loop
                    // Apparently some level of looping is expected!? See the "Nesting" section
                    // of https://cs.elderscrolls.com/index.php?title=Activate for more details
                    if (mCurrentActor == (tes4Actor ?
                           get<MWWorld::CellReferenceWrap>(actor.getCellRef()).get().getEditorId() :
                           get<MWWorld::CellRefWrap>(actor.getCellRef()).get().getRefId())
                        &&
                        mCurrentObject == get<MWWorld::CellReferenceWrap>(ptr.getCellRef()).get().getEditorId())
                    {
                        return;
                    }
                    else
                    {
                        mCurrentActor = (tes4Actor ?
                            get<MWWorld::CellReferenceWrap>(actor.getCellRef()).get().getEditorId() :
                            get<MWWorld::CellRefWrap>(actor.getCellRef()).get().getRefId());
                        mCurrentObject = get<MWWorld::CellReferenceWrap>(ptr.getCellRef()).get().getEditorId();
                    }

                    InterpreterContext& context = static_cast<InterpreterContext&> (runtime.getContext());
                    if (flag)
                        context.executeActivationScript(ptr, actor);
                    else
                        context.executeActivation(ptr, actor); // WARNING: currently produces a NullAction

                    mCurrentActor = "";
                    mCurrentObject = "";
                }
        };

        template<class R>
        class OpPlayGroup : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    // first mandatory string arg
                    std::string playgroupId = runtime.getStringLiteral(runtime[0].mInteger);
                    runtime.pop();

                    // second mandatory integer arg
                    Interpreter::Type_Integer flag = runtime[0].mInteger;
                    runtime.pop();

                    //std::cout << "PlayGroup: " << playgroupId << " " << flag << std::endl; // FIXME: temp testing

                    MWRender::Animation *anim = MWBase::Environment::get().getWorld()->getAnimation(ptr);
#if 0
                    if (anim->hasAnimation("forward") || anim->hasAnimation("backward"))
                    {
                        std::cout << " (has animation Forward or Backword)" << std::endl;
                    }
                    else
                        std::cout << std::endl;
#endif

                    // FIXME: should use runtime.getContext() here
                    const_cast<MWWorld::Class&>(ptr.getClass()).playgroup(ptr, playgroupId, flag);
                }
        };

        template<class R>
        class OpGetSelf : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {

                    MWWorld::Ptr ptr = R()(runtime);

                    ESM4::FormId formId = boost::variant2::get<MWWorld::CellReferenceWrap>(ptr.getCellRef()).get().getFormId();

                    std::cout << "GetSelf: " << ESM4::formIdToString(formId) << std::endl; // FIXME: temp testing

                    InterpreterContext& context = static_cast<InterpreterContext&> (runtime.getContext());
                    std::cout << "GetSelf: mTargetFormId: " << ESM4::formIdToString(context.getTargetFormId()) << std::endl;

                    runtime.push (formId);
                }
        };

        template<class R>
        class OpGetParentRef : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {

                    MWWorld::Ptr ptr = R()(runtime);

                    ESM4::FormId formId = boost::variant2::get<MWWorld::CellReferenceWrap>(ptr.getCellRef()).get().getParentFormId();

                    std::cout << "GetParerentRef: " << ESM4::formIdToString(formId) << std::endl; // FIXME: temp testing

                    runtime.push (formId);
                }
        };

        template<class R>
        class OpIsAnimPlaying : public Interpreter::Opcode1
        {
            public:

                //virtual void execute (Interpreter::Runtime& runtime)
                virtual void execute (Interpreter::Runtime& runtime, unsigned int arg0)
                {
                    // Lucky38LightScript (0011B039)
                    //
                    // ptr is a reference (0x0016B5E8) to an activator (0016B5E7) NVProspectorSaloonLights
                    //   the script belongs to the activator baseobj
                    //
                    // arg0 is 1, should be "Left" or "Right"

                    MWWorld::Ptr ptr = R()(runtime);


                    std::string animGroup = "";
                    if (arg0 >= 1) // optional string arg
                    {
                        int index = runtime[0].mInteger;
                        runtime.pop(); // index
                        animGroup = runtime.getStringLiteral(index);
                    }

                    bool isPlaying = false;

                    MWRender::Animation *anim = MWBase::Environment::get().getWorld()->getAnimation(ptr);
                    if (arg0 >= 1 && anim && anim->hasAnimation(animGroup))
                    {
                        // FIXME: temp testing
                        //std::cout << "IsAnimPlaying: anim group " << animGroup <<
                            //(isPlaying ? " is playing" : " is not playing") << std::endl;

                        // FIXME: always returns false
                        isPlaying = anim->isPlaying(animGroup);
                    }

                    runtime.push (isPlaying);
                }
        };

        template<class R>
        class OpGetItemCount : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    // 1 mandatory argument
                    std::string objectEditorId = runtime.getStringLiteral(runtime[0].mInteger);
                    runtime.pop();

                    MWWorld::ContainerStore& containerStore = ptr.getClass().getContainerStore(ptr);
                    int count = containerStore.countForeign(objectEditorId);

                    //std::cout << "GetItemCount: " << objectEditorId << ", " << count << std::endl; // FIXME: temp testing

                    runtime.push (count);  // should this be runtime[0].mInteger = count ?
                }
        };

        template<class R>
        class OpGetCurrentAIProcedure : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    //std::cout << "GetCurrentAIProcedure: " << std::endl; // FIXME: temp testing

                    runtime.push (2); // FIXME: just a dummy for testing
                }
        };

        template<class R>
        class OpEvaluatePackage : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    std::cout << "EvaluatePackage: "
                        << boost::variant2::get<MWWorld::CellReferenceWrap>(ptr.getCellRef()).get().getEditorId() << std::endl; // FIXME: temp testing
                }
        };

        template<class R>
        class OpGetLineOfSight : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    //MWWorld::Ptr ptr = R()(runtime);
                    int index = runtime[0].mInteger;
                    runtime.pop();
                    std::string id = runtime.getStringLiteral (index); // explicit ref editor id

                    // 1 mandatory argument
                    std::string objectEditorId = runtime.getStringLiteral(runtime[0].mInteger);
                    runtime.pop();

                    //std::cout << "GetLineOfSight: " << id << " - " << objectEditorId << std::endl; // FIXME: temp testing

                    runtime.push(1);  // should this be runtime[0].mInteger = 1 ?
                }
        };

        template<class R>
        class OpUnLock : public Interpreter::Opcode1
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime, unsigned int arg0)
                {

                    //MWWorld::Ptr ptr = R()(runtime);

                    // handle the arguments
                    Interpreter::Type_Integer flag = 0;
                    if (arg0 >= 1)
                    {
                        // FIXME: untested, prob wrong
                        Interpreter::Type_Integer flag = runtime[0].mInteger;
                        std::cout << "arg " << flag << std::endl;

                        runtime.pop();
                    }
                    else // implicit
                    {
                        MWWorld::Ptr ptr = R()(runtime);
                    }

                    //std::cout << "UnLock" << std::endl;
                }
        };

        template<class R>
        class OpGetInFaction : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWWorld::Ptr ptr = R()(runtime); // in PrimmResidentScript (000E4490) returns implicit

                    int index = runtime[0].mInteger;
                    runtime.pop();
                    std::string id = runtime.getStringLiteral (index); // faction EditorId in lowercase

                    //if (id != "Player") // FIXME: this is called too often
                    //std::cout << "GetInFaction: " << id << ", " << id2 << std::endl; // FIXME: temp testing

                    // FIXME: how to get the faction value?

                    runtime.push(-1);
                }
        };

        template<class R>
        class OpGetFactionRank : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    //MWWorld::Ptr ptr = R()(runtime);
                    int index = runtime[0].mInteger;
                    runtime.pop();
                    std::string id = runtime.getStringLiteral (index); // explicit ref editor id

                    // mandatory argument (faction id?)
                    std::string id2 = runtime.getStringLiteral(runtime[0].mInteger);
                    runtime.pop();

                    //if (id != "Player") // FIXME: this is called too often
                    //std::cout << "GetFactionRank: " << id << ", " << id2 << std::endl; // FIXME: temp testing

                    // FIXME: how to get the faction value?

                    runtime.push(-1);
                }
        };

        template<class R>
        class OpSetFactionReaction : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    //MWWorld::Ptr ptr = R()(runtime);
                    int index = runtime[0].mInteger;
                    runtime.pop();
                    std::string id = runtime.getStringLiteral (index); // 1st mandatory argument

                    // 2nd mandatory argument
                    std::string id2 = runtime.getStringLiteral(runtime[0].mInteger);
                    runtime.pop();

                    // 3rd mandatory argument
                    Interpreter::Type_Integer value = runtime[0].mInteger;
                    runtime.pop();

                    std::cout << "SetFactionReaction: " << id << ", " << id2 << " " << value << std::endl; // FIXME: temp testing
                }
        };

        template<class R>
        class OpSetActorValue : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    //MWWorld::Ptr ptr = R()(runtime);
                    int index = runtime[0].mInteger;
                    runtime.pop();
                    std::string id = runtime.getStringLiteral (index); // 1st mandatory argument

                    // 2nd mandatory argument
                    Interpreter::Type_Integer value = runtime[0].mInteger;
                    runtime.pop();

                    std::cout << "SetActorValue: " << id << " " << value << std::endl; // FIXME: temp testing
                }
        };

        template<class R>
        class OpSetDestroyed : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    // has 1 mandatory argument
                    Interpreter::Type_Integer value = runtime[0].mInteger;
                    runtime.pop();

                    std::cout << "SetDestroyed: " << value << std::endl; // FIXME: temp testing
                }
        };

        template<class R>
        class OpGetDead : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {

                    MWWorld::Ptr ptr = R()(runtime);

                    //std::cout << "GetDead: " << std::endl; // FIXME: temp testing

                    runtime.push (1); // FIXME:
                }
        };

        template<class R>
        class OpDisableLinkedPathPoints : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {

                    MWWorld::Ptr ptr = R()(runtime);

                    std::cout << "DisableLinkedPathPoints: " << std::endl; // FIXME: temp testing
                }
        };

        template<class R>
        class OpEnableLinkedPathPoints : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {

                    MWWorld::Ptr ptr = R()(runtime);

                    std::cout << "EnableLinkedPathPoints: " << std::endl; // FIXME: temp testing
                }
        };

        template<class R>
        class OpKillActor : public Interpreter::Opcode1
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime, unsigned int arg0)
                {

                    MWWorld::Ptr ptr = R()(runtime);

                    // handle the arguments
                    std::string actor = "";
                    if (arg0 >= 1)
                    {
                        // FIXME: untested, prob wrong
                        int index = runtime[0].mInteger;
                        runtime.pop();
                        actor = runtime.getStringLiteral(index);

                        runtime.pop();
                    }

                    std::cout << "KillActor " << actor << std::endl;

                    // FIXME: should use runtime.getContext() here
                    const_cast<MWWorld::Class&>(ptr.getClass()).killActor(ptr, actor);
                }
        };

        template<class R>
        class OpGetInCell : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {

                    MWWorld::Ptr ptr = R()(runtime);

                    //std::cout << "GetInCell: " << std::endl; // FIXME: temp testing

                    runtime.push (0); // FIXME:
                }
        };

        template<class R>
        class OpLock : public Interpreter::Opcode1
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime, unsigned int arg0)
                {

                    //MWWorld::Ptr ptr = R()(runtime);

                    // handle the arguments
                    Interpreter::Type_Integer flag = 0;
                    if (arg0 >= 1)
                    {
                        // FIXME: untested, prob wrong
                        Interpreter::Type_Integer flag = runtime[0].mInteger;
                        std::cout << "arg " << flag << std::endl;

                        runtime.pop();
                    }
                    else // implicit
                    {
                        MWWorld::Ptr ptr = R()(runtime);
                    }

                    //std::cout << "Lock" << std::endl;
                }
        };

        class OpGetButtonPressed : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    runtime.push (MWBase::Environment::get().getWindowManager()->readPressedButton());
                }
        };

        class OpGetCurrentTime : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    float gamehour
                        = std::round(MWBase::Environment::get().getWorld()->getGlobalFloat("gamehour") * 100);

                    //std::cout << "Current Time " << gamehour / 100 << std::endl;

                    runtime.push (gamehour / 100);
                }
        };

        class OpGetDayofWeek : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    //std::cout << "Day of Week " <<
                        //MWBase::Environment::get().getWorld()->getGlobalInt("gamedayspassed")%7 << std::endl;

                    runtime.push (MWBase::Environment::get().getWorld()->getGlobalInt("gamedayspassed")%7);
                }
        };

        void installOpcodes (Interpreter::Interpreter& interpreter)
        {
            // Actor
            interpreter.installSegment5
                (Tes4Compiler::Tes4Actor::opcodeGetDead, new OpGetDead<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Actor::opcodeSetActorValue, new OpSetActorValue<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Actor::opcodeSetActorValueExplicit, new OpSetActorValue<ExplicitTes4Ref>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Actor::opcodeGetInCell, new OpGetInCell<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Actor::opcodeGetInCellExplicit, new OpGetInCell<ExplicitTes4Ref>);

            // AI
            interpreter.installSegment5
                (Tes4Compiler::Tes4AI::opcodeEvaluatePackage, new OpEvaluatePackage<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4AI::opcodeEvaluatePackageExplicit, new OpEvaluatePackage<ExplicitTes4Ref>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4AI::opcodeGetCurrentAIProcedure, new OpGetCurrentAIProcedure<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4AI::opcodeGetCurrentAIProcedureExplicit, new OpGetCurrentAIProcedure<ExplicitTes4Ref>);

            // Animation
            interpreter.installSegment3
                (Tes4Compiler::Tes4Animation::opcodeIsAnimPlaying, new OpIsAnimPlaying<ImplicitRef>);
            interpreter.installSegment3
                (Tes4Compiler::Tes4Animation::opcodeIsAnimPlayingExplicit, new OpIsAnimPlaying<ExplicitTes4Ref>);

            interpreter.installSegment5
                (Tes4Compiler::Tes4Animation::opcodePlayGroup, new OpPlayGroup<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Animation::opcodePlayGroupExplicit, new OpPlayGroup<ExplicitTes4Ref>);

            // Crime
            interpreter.installSegment3
                (Tes4Compiler::Tes4Crime::opcodeKillActor, new OpKillActor<ImplicitRef>);
            interpreter.installSegment3
                (Tes4Compiler::Tes4Crime::opcodeKillActorExplicit, new OpKillActor<ExplicitTes4Ref>);

            // Faction
            interpreter.installSegment5
                (Tes4Compiler::Tes4Faction::opcodeGetInFaction, new OpGetInFaction<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Faction::opcodeGetInFactionExplicit, new OpGetInFaction<ExplicitTes4Ref>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Faction::opcodeGetFactionRank, new OpGetFactionRank<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Faction::opcodeGetFactionRankExplicit, new OpGetFactionRank<ExplicitTes4Ref>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Faction::opcodeSetFactionReaction, new OpSetFactionReaction<ImplicitRef>);

            // Inventory
            interpreter.installSegment3
                (Tes4Compiler::Tes4Inventory::opcodeActivate, new OpActivate<ImplicitRef>);
            interpreter.installSegment3
                (Tes4Compiler::Tes4Inventory::opcodeActivateExplicit, new OpActivate<ExplicitTes4Ref>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Inventory::opcodeGetItemCount, new OpGetItemCount<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Inventory::opcodeGetItemCountExplicit, new OpGetItemCount<ExplicitTes4Ref>);

            // Movement
            interpreter.installSegment5
                (Tes4Compiler::Tes4Movement::opcodeGetLineOfSight, new OpGetLineOfSight<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Movement::opcodeGetLineOfSightExplicit, new OpGetLineOfSight<ExplicitTes4Ref>);

            // Quest
            interpreter.installSegment5
                (Tes4Compiler::Tes4Quest::opcodeGetStage, new OpGetStage<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Quest::opcodeSetStage, new OpSetStage<ImplicitRef>);

            // Misc
            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeDisableLinkedPathPoints, new OpDisableLinkedPathPoints<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeDisableLinkedPathPointsExplicit, new OpDisableLinkedPathPoints<ExplicitTes4Ref>);

            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeEnableLinkedPathPoints, new OpEnableLinkedPathPoints<ImplicitRef>);

            interpreter.installSegment3
                (Tes4Compiler::Tes4Misc::opcodeLock, new OpLock<ImplicitRef>);
            interpreter.installSegment3
                (Tes4Compiler::Tes4Misc::opcodeLockExplicit, new OpLock<ExplicitTes4Ref>);

            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeSetDestroyed, new OpSetDestroyed<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeSetDestroyedExplicit, new OpSetDestroyed<ExplicitTes4Ref>);

            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeGetLocked, new OpGetLocked<ImplicitRef>);

            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeGetOpenState, new OpGetOpenState<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeGetOpenStateExplicit, new OpGetOpenState<ExplicitTes4Ref>);

            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeGetParentRef, new OpGetParentRef<ImplicitRef>);

            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeGetSelf, new OpGetSelf<ImplicitRef>);

            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeIsActionRef, new OpIsActionRef<ImplicitRef>);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeIsActionRefExplicit, new OpIsActionRef<ExplicitTes4Ref>);
            interpreter.installSegment3
                (Tes4Compiler::Tes4Misc::opcodeUnLock, new OpUnLock<ImplicitRef>);
            interpreter.installSegment3
                (Tes4Compiler::Tes4Misc::opcodeUnLockExplicit, new OpUnLock<ExplicitTes4Ref>);

            // RunestoneReman COW "Tamriel" -6 13
            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeGetButtonPressed, new OpGetButtonPressed);

            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeGetCurrentTime, new OpGetCurrentTime);
            interpreter.installSegment5
                (Tes4Compiler::Tes4Misc::opcodeGetDayofWeek, new OpGetDayofWeek);
        }
    }
}
