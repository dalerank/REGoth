//
// Created by andre on 03.06.16.
//

#include "Externals.h"
#include <daedalus/DaedalusVM.h>
#include <utils/logger.h>
#include <daedalus/DaedalusStdlib.h>
#include <components/VobClasses.h>
#include <engine/GameEngine.h>
#include <logic/PlayerController.h>
#include <ui/PrintScreenMessages.h>


void ::Logic::ScriptExternals::registerStdLib(Daedalus::DaedalusVM& vm, bool verbose)
{
    Daedalus::registerDaedalusStdLib(vm, verbose);
    Daedalus::registerGothicEngineClasses(vm);
}

void ::Logic::ScriptExternals::registerEngineExternals(World::WorldInstance& world, Daedalus::DaedalusVM* vm, bool verbose)
{
    Engine::BaseEngine* engine = world.getEngine();
    World::WorldInstance* pWorld = &world;

    // TODO: Refractor
    auto getNPCByInstance = [vm, engine](size_t instance)
    {
        Daedalus::GameState::NpcHandle hnpc = ZMemory::handleCast<Daedalus::GameState::NpcHandle>
                (vm->getDATFile().getSymbolByIndex(instance).instanceDataHandle);

        // Get data of npc this belongs to
        Daedalus::GEngineClasses::C_Npc& npcData = vm->getGameState().getNpc(hnpc);
        VobTypes::ScriptInstanceUserData* userData = reinterpret_cast<VobTypes::ScriptInstanceUserData*>(npcData.userPtr);

		
		if(userData)
		{
			World::WorldInstance& world = engine->getWorldInstance(userData->world);
			VobTypes::NpcVobInformation vob = VobTypes::asNpcVob(world, userData->vobEntity);

			return vob;
		}
		else{
			LogWarn() << "No userptr on npc: " << npcData.name[0];

			VobTypes::NpcVobInformation vob;
			vob.entity.invalidate();

			return vob;
		}

        
    };

    auto getItemByInstance = [vm, engine](size_t instance)
    {
        Daedalus::GameState::ItemHandle hitem = ZMemory::handleCast<Daedalus::GameState::ItemHandle>
                (vm->getDATFile().getSymbolByIndex(instance).instanceDataHandle);

        // Get data of npc this belongs to
        Daedalus::GEngineClasses::C_Item& itemData = vm->getGameState().getItem(hitem);
        VobTypes::ScriptInstanceUserData* userData = reinterpret_cast<VobTypes::ScriptInstanceUserData*>(itemData.userPtr);

		if(userData)
		{
			World::WorldInstance& world = engine->getWorldInstance(userData->world);
			Vob::VobInformation vob = Vob::asVob(world, userData->vobEntity);

			return vob;
		}
		else{
			LogWarn() << "No userptr on item: " << itemData.name;

			Vob::VobInformation vob;
			vob.entity.invalidate();

			return vob;
		}
    };

    /**
     * Mdl_SetVisual
     */
    vm->registerExternalFunction("Mdl_SetVisual", [=](Daedalus::DaedalusVM& vm) {
        std::string visual = vm.popString();

        uint32_t arr_self;
        int32_t self = vm.popVar(arr_self); if(verbose) LogInfo() << "self: " << self;

        // TODO: Need a better API for this
        Daedalus::GameState::NpcHandle hnpc = ZMemory::handleCast<Daedalus::GameState::NpcHandle>
                (vm.getDATFile().getSymbolByIndex(self).instanceDataHandle);
        Daedalus::GEngineClasses::C_Npc& npcData = vm.getGameState().getNpc(hnpc);

        if(npcData.userPtr && vm.getDATFile().getSymbolByIndex(self).instanceDataHandle.isValid())
        {
            VobTypes::ScriptInstanceUserData* userData = reinterpret_cast<VobTypes::ScriptInstanceUserData*>(npcData.userPtr);

            //LogInfo() << "Setting MDL visual to: " << visual;

            World::WorldInstance& world = engine->getWorldInstance(userData->world);
            VobTypes::NpcVobInformation vob = VobTypes::asNpcVob(world, userData->vobEntity);
            VobTypes::NPC_SetModelVisual(vob, visual);
        }
    });

    /**
     * Mdl_SetVisualBody
     */
    vm->registerExternalFunction("Mdl_SetVisualBody", [=](Daedalus::DaedalusVM& vm){

        int32_t armorInstance = vm.popDataValue();
        size_t teethTexNr = static_cast<size_t>(vm.popDataValue());
        size_t headTexNr = static_cast<size_t>(vm.popDataValue());
        std::string head = vm.popString();
        int32_t bodyTexColor = vm.popDataValue();
        int32_t bodyTexNr = vm.popDataValue();
        std::string body = vm.popString();

        uint32_t arr_self;
        int32_t self = vm.popVar(arr_self); if(verbose) LogInfo() << "self: " << self;




        // TODO: Need a better API for this
        Daedalus::GameState::NpcHandle hnpc = ZMemory::handleCast<Daedalus::GameState::NpcHandle>
                (vm.getDATFile().getSymbolByIndex(self).instanceDataHandle);
        Daedalus::GEngineClasses::C_Npc& npcData = vm.getGameState().getNpc(hnpc);

        //LogInfo() << "Self hdl idx:" << vm.getDATFile().getSymbolByIndex(self).instanceDataHandle.index << ", Valid: " << vm.getDATFile().getSymbolByIndex(self).instanceDataHandle.isValid();
        //LogInfo() << " - Userdata: " << npcData.userPtr;

        // TODO: Implement the other visuals
        if(npcData.userPtr && vm.getDATFile().getSymbolByIndex(self).instanceDataHandle.isValid())
        {
            VobTypes::ScriptInstanceUserData* userData = reinterpret_cast<VobTypes::ScriptInstanceUserData*>(npcData.userPtr);

            if(verbose) LogInfo() << "Setting visual to: " << body;

            World::WorldInstance& world = engine->getWorldInstance(userData->world);
            VobTypes::NpcVobInformation vob = VobTypes::asNpcVob(world, userData->vobEntity);

            // Set main visual
            VobTypes::NPC_SetBodyMesh(vob, body, bodyTexNr, bodyTexColor);

            // Set head-mesh
            VobTypes::NPC_SetHeadMesh(vob, head, headTexNr, teethTexNr);
        }

        // TODO: Wrap this up
        // Set armor visual
        if(armorInstance != -1)
        {
            // TODO: Right now, this equips the item automatically. When this is done properly, call the equip-method here
            vm.getGameState().addInventoryItem(static_cast<size_t>(armorInstance), hnpc);
        }
    });

    /**
     * ta_min
     */
    vm->registerExternalFunction("ta_min", [=](Daedalus::DaedalusVM& vm){
        std::string waypoint = vm.popString(); if(verbose) LogInfo() << "waypoint: " << waypoint;

        int32_t action = vm.popDataValue();
        int32_t stop_m = vm.popDataValue(); if(verbose) LogInfo() << "stop_m: " << stop_m;
        int32_t stop_h = vm.popDataValue(); if(verbose) LogInfo() << "stop_h: " << stop_h;
        int32_t start_m = vm.popDataValue(); if(verbose) LogInfo() << "start_m: " << start_m;
        int32_t start_h = vm.popDataValue(); if(verbose) LogInfo() << "start_h: " << start_h;
        uint32_t arr_self;
        int32_t self = vm.popVar(arr_self); if(verbose) LogInfo() << "self: " << self;

        // TODO: Need a better API for this
        Daedalus::GameState::NpcHandle hnpc = ZMemory::handleCast<Daedalus::GameState::NpcHandle>
                (vm.getDATFile().getSymbolByIndex(self).instanceDataHandle);
        Daedalus::GEngineClasses::C_Npc& npcData = vm.getGameState().getNpc(hnpc);

        if(verbose) LogInfo() << "Self: " << self << ", " << vm.getDATFile().getSymbolByIndex(self).name << "  ...  hdl idx:" << vm.getDATFile().getSymbolByIndex(self).instanceDataHandle.index << ", Valid: " << vm.getDATFile().getSymbolByIndex(self).instanceDataHandle.isValid();
        if(verbose) LogInfo() << " - Userdata: " << npcData.userPtr;

        if(npcData.userPtr && vm.getDATFile().getSymbolByIndex(self).instanceDataHandle.isValid())
        {
            VobTypes::ScriptInstanceUserData* userData = reinterpret_cast<VobTypes::ScriptInstanceUserData*>(npcData.userPtr);

            World::WorldInstance& world = engine->getWorldInstance(userData->world);
            VobTypes::NpcVobInformation vob = VobTypes::asNpcVob(world, userData->vobEntity);

            if(verbose) LogInfo() << "NPC " << npcData.name[0] << " going to wp: " << waypoint;

            if(World::Waynet::waypointExists(world.getWaynet(), waypoint))
            {
                vob.playerController->teleportToWaypoint(World::Waynet::getWaypointIndex(world.getWaynet(), waypoint));
                vob.playerController->addRoutineWaypoint(World::Waynet::getWaypointIndex(world.getWaynet(), waypoint));
            }
        }
    });

    /**
     * EquipItem
     */
    vm->registerExternalFunction("equipitem", [=](Daedalus::DaedalusVM& vm){
        uint32_t instance = static_cast<uint32_t>(vm.popDataValue());
        uint32_t arr_n0;
        uint32_t self = vm.popVar(arr_n0);

        // TODO: Need a better API for this
        Daedalus::GameState::NpcHandle hnpc = ZMemory::handleCast<Daedalus::GameState::NpcHandle>
                (vm.getDATFile().getSymbolByIndex(self).instanceDataHandle);

        Daedalus::GameState::ItemHandle item = vm.getGameState().addInventoryItem(instance, hnpc);

        // Get data of npc this belongs to
        Daedalus::GEngineClasses::C_Npc& npcData = vm.getGameState().getNpc(hnpc);
        VobTypes::ScriptInstanceUserData* userData = reinterpret_cast<VobTypes::ScriptInstanceUserData*>(npcData.userPtr);

		if(userData)
		{
			World::WorldInstance& world = engine->getWorldInstance(userData->world);
			VobTypes::NpcVobInformation vob = VobTypes::asNpcVob(world, userData->vobEntity);

			VobTypes::NPC_EquipWeapon(vob, item);
		}
		else{
			//LogWarn() << "No userptr on npc: " << npcData.name[0];
		}
    });

    /**
     * GetDistTo...
     */
    vm->registerExternalFunction("npc_getdisttonpc", [=](Daedalus::DaedalusVM& vm){
        uint32_t arr_npc2;
        uint32_t npc2 = vm.popVar(arr_npc2); if(verbose) LogInfo() << "npc2: " << npc2;
        uint32_t arr_npc1;
        uint32_t npc1 = vm.popVar(arr_npc1); if(verbose) LogInfo() << "npc1: " << npc1;

        VobTypes::NpcVobInformation vob1 = getNPCByInstance(npc1);
        VobTypes::NpcVobInformation vob2 = getNPCByInstance(npc2);

        // Calculate distance
        float dist = (Vob::getTransform(vob1).Translation() - Vob::getTransform(vob2).Translation()).length();

        // Convert to cm
        dist *= 100.0f;

        vm.setReturn(static_cast<int32_t>(dist));
    });

    vm->registerExternalFunction("npc_getdisttowp", [=](Daedalus::DaedalusVM& vm){
        std::string wpname = vm.popString(); if(verbose) LogInfo() << "wpname: " << wpname;
        uint32_t arr_self;
        uint32_t self = vm.popVar(arr_self); if(verbose) LogInfo() << "self: " << self;

        VobTypes::NpcVobInformation vob = getNPCByInstance(self);

        // Calculate distance
        Math::Matrix selfTransform = Vob::getTransform(vob);
        World::WorldInstance& world = Vob::getWorld(vob);
        size_t wpidx = World::Waynet::getWaypointIndex(world.getWaynet(), wpname);

        float dist = (selfTransform.Translation() - world.getWaynet().waypoints[wpidx].position).length();

        // Convert to cm
        dist *= 100.0f;

        vm.setReturn(static_cast<int32_t>(dist));
    });

    vm->registerExternalFunction("npc_getdisttoitem", [=](Daedalus::DaedalusVM& vm){

        uint32_t item = static_cast<uint32_t>(vm.popDataValue());
        uint32_t arr_npc;
        int32_t npc = vm.popVar(arr_npc); if(verbose) LogInfo() << "npc: " << npc;

        VobTypes::NpcVobInformation npcvob = getNPCByInstance(npc);
        Vob::VobInformation itemvob = getItemByInstance(item);

        float dist = (Vob::getTransform(npcvob).Translation() - Vob::getTransform(itemvob).Translation()).length();

        // Convert to cm
        dist *= 100.0f;

        vm.setReturn(static_cast<int32_t>(dist));
    });

    vm->registerExternalFunction("npc_getdisttoplayer", [=](Daedalus::DaedalusVM& vm){
        uint32_t arr_npc1;
        uint32_t npc1 = vm.popVar(arr_npc1); if(verbose) LogInfo() << "npc1: " << npc1;


        VobTypes::NpcVobInformation vob1 = getNPCByInstance(npc1);
        VobTypes::NpcVobInformation vob2 = getNPCByInstance(vm.getDATFile().getSymbolIndexByName("hero"));

        // Calculate distance
        float dist = (Vob::getTransform(vob1).Translation() - Vob::getTransform(vob2).Translation()).length();

        // Convert to cm
        dist *= 100.0f;

        vm.setReturn(static_cast<int32_t>(dist));
    });

    vm->registerExternalFunction("npc_exchangeroutine", [=](Daedalus::DaedalusVM& vm){

        std::string routinename = vm.popString();
        uint32_t arr_npc;
        int32_t npc = vm.popVar(arr_npc); if(verbose) LogInfo() << "npc: " << npc;

        VobTypes::NpcVobInformation npcvob = getNPCByInstance(npc);

        // TODO: Implement the others
        if(routinename == "FOLLOW")
        {
            // Follow player
            npcvob.playerController->setFollowTarget(npcvob.world->getScriptEngine().getPlayerEntity());
        }else
        {
            npcvob.playerController->setFollowTarget(Handle::EntityHandle::makeInvalidHandle());
            npcvob.playerController->setDailyRoutine({});
        }
    });

    vm->registerExternalFunction("printdebuginstch", [=](Daedalus::DaedalusVM& vm){

        uint32_t arr_npc;
        std::string s = vm.popString();
        int32_t ch = vm.popDataValue();

        LogInfo() << "DEBUG: " << s;
    });

    vm->registerExternalFunction("ai_turntonpc", [=](Daedalus::DaedalusVM& vm){
        uint32_t arr_n1;
        int32_t target = vm.popVar(arr_n1); if(verbose) LogInfo() << "target: " << target;
        uint32_t arr_n0;
        int32_t self = vm.popVar(arr_n0); if(verbose) LogInfo() << "self: " << self;

        VobTypes::NpcVobInformation selfvob = getNPCByInstance(self);
        VobTypes::NpcVobInformation targetvob = getNPCByInstance(target);

        // Fill turn message
        EventMessages::MovementMessage msg;
        msg.subType = EventMessages::MovementMessage::ST_TurnToVob;
        msg.targetVob = targetvob.entity;

        // Push the message
        selfvob.playerController->getEM().onMessage(msg);
    });

    /*vm->registerExternalFunction("snd_getdisttosource", [=](Daedalus::DaedalusVM& vm){
        uint32_t arr_self;
        int32_t self = vm.popVar(arr_self); if(verbose) LogInfo() << "self: " << self;
        vm.setReturn(0);
    });*/

    vm->registerExternalFunction("printscreen", [=](Daedalus::DaedalusVM& vm){
        int32_t timesec = vm.popDataValue(); if(verbose) LogInfo() << "timesec: " << timesec;
        std::string font = vm.popString(); if(verbose) LogInfo() << "font: " << font;
        int32_t posy = vm.popDataValue(); if(verbose) LogInfo() << "posy: " << posy;
        int32_t posx = vm.popDataValue(); if(verbose) LogInfo() << "posx: " << posx;
        std::string msg = vm.popString(); if(verbose) LogInfo() << "msg: " << msg;
        int32_t dialognr = vm.popDataValue(); if(verbose) LogInfo() << "dialognr: " << dialognr;

        pWorld->getPrintScreenManager().printMessageTimed(posx / 100.0f,
                                                        posy / 100.0f,
                                                        msg,
                                                        static_cast<double>(timesec));
    });

    vm->registerExternalFunction("npc_isplayer", [=](Daedalus::DaedalusVM& vm){
        uint32_t arr_player;
        uint32_t player = vm.popVar(arr_player); if(verbose) LogInfo() << "player: " << player;

        VobTypes::NpcVobInformation npc = getNPCByInstance(player);

        if(npc.world)
        {
            int r = npc.world->getScriptEngine().getPlayerEntity() == npc.entity;
            vm.setReturn(r);
        }else
        {
            vm.setReturn(0);
        }
    });
}






