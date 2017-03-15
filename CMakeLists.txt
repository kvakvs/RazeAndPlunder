cmake_minimum_required(VERSION 3.6)

project(RnP_bot)
set(CMAKE_CXX_STANDARD 14)

set(BOOST_VER       1_63)
set(BOOST_DIR       f:/boost_1_63_0)

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${BOOST_DIR}/
        )

add_definitions(
	/MP /Oi /Zi
	/Gy
	/DNOMINMAX
	)

# Debug checks, MT Debug DLL, /GS security check
add_definitions(
	/RTC1
	/MDd
	/GS
	/fp:fast
	)

set(SRC_SRC
        src/Glob.cpp src/Glob.h 
        src/BWEMUtil.cpp src/BWEMUtil.h
        src/BotAILoop.cpp src/BotAILoop.h
        src/Dll.cpp
        src/Influencemap/MapManager.cpp src/Influencemap/MapManager.h
        src/RnpBot.cpp src/RnpBot.h
        )
set(SRC_PATHING
        src/Pathfinding/NavigationAgent.cpp src/Pathfinding/NavigationAgent.h
        src/Pathfinding/Pathfinder.cpp src/Pathfinding/Pathfinder.h
        src/Pathfinding/PathObj.cpp src/Pathfinding/PathObj.h
        src/Pathfinding/PFFunctions.cpp src/Pathfinding/PFFunctions.h
        )
set(SRC_MAINAGENTS
        src/MainAgents/AgentFactory.cpp src/MainAgents/AgentFactory.h
        src/MainAgents/BaseAgent.cpp src/MainAgents/BaseAgent.h
        src/MainAgents/TargetingAgent.cpp src/MainAgents/TargetingAgent.h
        src/MainAgents/WorkerAgent.cpp src/MainAgents/WorkerAgent.h
        )
set(SRC_MANAGERS
        src/Managers/AgentManager.cpp src/Managers/AgentManager.h
        src/Managers/Agentset.cpp src/Managers/Agentset.h
        src/Managers/BuildingPlacer.cpp src/Managers/BuildingPlacer.h
        src/Managers/BuildplanEntry.cpp src/Managers/BuildplanEntry.h
        src/Managers/Constructor.cpp src/Managers/Constructor.h
        src/Managers/ExplorationManager.cpp src/Managers/ExplorationManager.h
        src/Managers/ResourceManager.cpp src/Managers/ResourceManager.h
        src/Managers/SpottedObject.cpp src/Managers/SpottedObject.h
        src/Managers/Upgrader.cpp src/Managers/Upgrader.h
        )
set(SRC_COMMANDER
        src/Commander/Commander.cpp src/Commander/Commander.h
        src/Commander/ExplorationSquad.cpp src/Commander/ExplorationSquad.h
        src/Commander/Protoss/ProtossMain.cpp src/Commander/Protoss/ProtossMain.h
        src/Commander/RushSquad.cpp src/Commander/RushSquad.h
        src/Commander/Squad.cpp src/Commander/Squad.h
        src/Commander/StrategySelector.cpp src/Commander/StrategySelector.h
        src/Commander/Terran/TerranMain.cpp src/Commander/Terran/TerranMain.h
        src/Commander/UnitSetup.cpp src/Commander/UnitSetup.h
        src/Commander/Zerg/LurkerRush.cpp src/Commander/Zerg/LurkerRush.h
        src/Commander/Zerg/ZergMain.cpp src/Commander/Zerg/ZergMain.h
        )
set(SRC_STRUCTAGENTS
        src/StructureAgents/Protoss/NexusAgent.cpp src/StructureAgents/Protoss/NexusAgent.h
        src/StructureAgents/RefineryAgent.cpp src/StructureAgents/RefineryAgent.h
        src/StructureAgents/StructureAgent.cpp src/StructureAgents/StructureAgent.h
        src/StructureAgents/Terran/CommandCenterAgent.cpp src/StructureAgents/Terran/CommandCenterAgent.h
        src/StructureAgents/Terran/ComsatAgent.cpp src/StructureAgents/Terran/ComsatAgent.h
        src/StructureAgents/Zerg/HatcheryAgent.cpp src/StructureAgents/Zerg/HatcheryAgent.h
        )
set(SRC_UNITAGENTS
        src/UnitAgents/Protoss/CarrierAgent.cpp src/UnitAgents/Protoss/CarrierAgent.h
        src/UnitAgents/Protoss/CorsairAgent.cpp src/UnitAgents/Protoss/CorsairAgent.h
        src/UnitAgents/Protoss/HighTemplarAgent.cpp src/UnitAgents/Protoss/HighTemplarAgent.h
        src/UnitAgents/Protoss/ReaverAgent.cpp src/UnitAgents/Protoss/ReaverAgent.h
        src/UnitAgents/Terran/BattlecruiserAgent.cpp src/UnitAgents/Terran/BattlecruiserAgent.h
        src/UnitAgents/Terran/FirebatAgent.cpp src/UnitAgents/Terran/FirebatAgent.h
        src/UnitAgents/Terran/GhostAgent.cpp src/UnitAgents/Terran/GhostAgent.h
        src/UnitAgents/Terran/MarineAgent.cpp src/UnitAgents/Terran/MarineAgent.h
        src/UnitAgents/Terran/MedicAgent.cpp src/UnitAgents/Terran/MedicAgent.h
        src/UnitAgents/Terran/ScienceVesselAgent.cpp src/UnitAgents/Terran/ScienceVesselAgent.h
        src/UnitAgents/Terran/SiegeTankAgent.cpp src/UnitAgents/Terran/SiegeTankAgent.h
        src/UnitAgents/Terran/VultureAgent.cpp src/UnitAgents/Terran/VultureAgent.h
        src/UnitAgents/Terran/WraithAgent.cpp src/UnitAgents/Terran/WraithAgent.h
        src/UnitAgents/TransportAgent.cpp src/UnitAgents/TransportAgent.h
        src/UnitAgents/UnitAgent.cpp src/UnitAgents/UnitAgent.h
        src/UnitAgents/Zerg/DefilerAgent.cpp src/UnitAgents/Zerg/DefilerAgent.h
        src/UnitAgents/Zerg/HydraliskAgent.cpp src/UnitAgents/Zerg/HydraliskAgent.h
        src/UnitAgents/Zerg/LurkerAgent.cpp src/UnitAgents/Zerg/LurkerAgent.h
        src/UnitAgents/Zerg/MutaliskAgent.cpp src/UnitAgents/Zerg/MutaliskAgent.h
        src/UnitAgents/Zerg/QueenAgent.cpp src/UnitAgents/Zerg/QueenAgent.h
        )
set(SRC_UTILS
        src/Utils/Config.cpp src/Utils/Config.h
        src/Utils/cthread.cpp src/Utils/cthread.h
        src/Utils/FileReaderUtils.cpp src/Utils/FileReaderUtils.h
        src/Utils/Profiler.cpp src/Utils/Profiler.h
        src/Utils/ProfilerObj.cpp src/Utils/ProfilerObj.h
        src/Utils/Sets.h
        src/Utils/Statistics.cpp src/Utils/Statistics.h
        )
source_group("Bot" FILES ${SRC_SRC})
source_group("Bot/MainAgents" FILES ${SRC_MAINAGENTS})
source_group("Bot/Utils" FILES ${SRC_UTILS})
source_group("Bot/Commander" FILES ${SRC_COMMANDER})
source_group("Bot/Pathing" FILES ${SRC_PATHING})
source_group("Bot/Managers" FILES ${SRC_MANAGERS})
source_group("Bot/UnitAgents" FILES ${SRC_UNITAGENTS})
source_group("Bot/StructureAgents" FILES ${SRC_STRUCTAGENTS})
set(SOURCE_FILES 
        ${SRC_SRC}
        ${SRC_COMMANDER}
        ${SRC_MAINAGENTS}
        ${SRC_MANAGERS}
        ${SRC_UTILS}
        ${SRC_PATHING}
        ${SRC_STRUCTAGENTS}
        ${SRC_UNITAGENTS}
        )  

set(BWAPI_FILES
        include/BWAPI.h
        include/BWAPI/Client/BulletData.h
        include/BWAPI/Client/BulletImpl.h
        include/BWAPI/Client/Client.h
        include/BWAPI/Client/Command.h
        include/BWAPI/Client/CommandType.h
        include/BWAPI/Client/Event.h
        include/BWAPI/Client/ForceData.h
        include/BWAPI/Client/ForceImpl.h
        include/BWAPI/Client/GameData.h
        include/BWAPI/Client/GameImpl.h
        include/BWAPI/Client/GameTable.h
        include/BWAPI/Client/PlayerData.h
        include/BWAPI/Client/PlayerImpl.h
        include/BWAPI/Client/RegionData.h
        include/BWAPI/Client/RegionImpl.h
        include/BWAPI/Client/Shape.h
        include/BWAPI/Client/ShapeType.h
        include/BWAPI/Client/UnitCommand.h
        include/BWAPI/Client/UnitData.h
        include/BWAPI/Client/UnitImpl.h
        include/BWAPI/AIModule.h
        include/BWAPI/ArithmaticFilter.h
        include/BWAPI/BestFilter.h
        include/BWAPI/Bullet.h
        include/BWAPI/Bulletset.h
        include/BWAPI/BulletType.h
        include/BWAPI/Client.h
        include/BWAPI/Color.h
        include/BWAPI/ComparisonFilter.h
        include/BWAPI/Constants.h
        include/BWAPI/CoordinateType.h
        include/BWAPI/DamageType.h
        include/BWAPI/Error.h
        include/BWAPI/Event.h
        include/BWAPI/EventType.h
        include/BWAPI/ExplosionType.h
        include/BWAPI/Filters.h
        include/BWAPI/Flag.h
        include/BWAPI/Force.h
        include/BWAPI/Forceset.h
        include/BWAPI/Game.h
        include/BWAPI/GameType.h
        include/BWAPI/Input.h
        include/BWAPI/Interface.h
        include/BWAPI/InterfaceEvent.h
        include/BWAPI/Latency.h
        include/BWAPI/Order.h
        include/BWAPI/Player.h
        include/BWAPI/Playerset.h
        include/BWAPI/PlayerType.h
        include/BWAPI/Position.h
        include/BWAPI/PositionUnit.h
        include/BWAPI/Race.h
        include/BWAPI/Region.h
        include/BWAPI/Regionset.h
        include/BWAPI/SetContainer.h
        include/BWAPI/TechType.h
        include/BWAPI/TournamentAction.h
        include/BWAPI/Type.h
        include/BWAPI/UnaryFilter.h
        include/BWAPI/Unit.h
        include/BWAPI/UnitCommand.h
        include/BWAPI/UnitCommandType.h
        include/BWAPI/Unitset.h
        include/BWAPI/UnitSizeType.h
        include/BWAPI/UnitType.h
        include/BWAPI/UpgradeType.h
        include/BWAPI/WeaponType.h
        include/BWAPI/WindowsTypes.h
        )
source_group("BWAPI" FILES ${BWAPI_FILES})

set(BWEM_ROOT deps/BWEM131)
set(BWEM_DIR  ${BWEM_ROOT}/src)
set(BWEM_EASYBMP ${BWEM_ROOT}/easybmp106)
include_directories(${BWEM_ROOT}/src ${BWEM_ROOT})
set(BWEM_FILES
        ${BWEM_EASYBMP}/EasyBMP.cpp ${BWEM_EASYBMP}/EasyBMP.h
        ${BWEM_EASYBMP}/EasyBMP_BMP.h
        ${BWEM_EASYBMP}/EasyBMP_DataStructures.h
        ${BWEM_EASYBMP}/EasyBMP_VariousBMPutilities.h

        ${BWEM_DIR}/area.cpp ${BWEM_DIR}/area.h
        ${BWEM_DIR}/base.cpp ${BWEM_DIR}/base.h
        ${BWEM_DIR}/bwapiExt.cpp ${BWEM_DIR}/bwapiExt.h
        ${BWEM_DIR}/bwem.cpp ${BWEM_DIR}/bwem.h
        ${BWEM_DIR}/cp.cpp ${BWEM_DIR}/cp.h
        ${BWEM_DIR}/defs.h
        ${BWEM_DIR}/graph.cpp ${BWEM_DIR}/graph.h
        ${BWEM_DIR}/gridMap.cpp ${BWEM_DIR}/gridMap.h
        ${BWEM_DIR}/map.cpp ${BWEM_DIR}/map.h
        ${BWEM_DIR}/mapDrawer.cpp ${BWEM_DIR}/mapDrawer.h
        ${BWEM_DIR}/mapImpl.cpp ${BWEM_DIR}/mapImpl.h
        ${BWEM_DIR}/mapPrinter.cpp ${BWEM_DIR}/mapPrinter.h
        ${BWEM_DIR}/neutral.cpp ${BWEM_DIR}/neutral.h
        ${BWEM_DIR}/tiles.cpp ${BWEM_DIR}/tiles.h
        ${BWEM_DIR}/utils.cpp ${BWEM_DIR}/utils.h
        ${BWEM_DIR}/winutils.cpp ${BWEM_DIR}/winutils.h
        )
source_group("BWEM" FILES ${BWEM_FILES})

link_directories(
        ${BOOST_DIR}/lib32-msvc-12.0
        lib
        )

set(DEBUG_D_SUFFIX d)
set(DEBUG_GD_SUFFIX -gd)
set(BOT_TARGET_NAME rnpbot${DEBUG_D_SUFFIX})

add_library(${BOT_TARGET_NAME} SHARED
        ${SOURCE_FILES}
        ${BWAPI_FILES}
        ${BWEM_FILES}
        )

target_link_libraries(${BOT_TARGET_NAME}
        BWAPI${DEBUG_D_SUFFIX}
        BWAPIClient${DEBUG_D_SUFFIX}
        libboost_filesystem-vc120-mt${DEBUG_GD_SUFFIX}-${BOOST_VER}
        libboost_system-vc120-mt${DEBUG_GD_SUFFIX}-${BOOST_VER}
        libboost_thread-vc120-mt${DEBUG_GD_SUFFIX}-${BOOST_VER}
        )

#install(TARGETS ${BOT_TARGET_NAME}
#        RUNTIME DESTINATION f:/Games/Starcraft/bwapi-data/
#        )
