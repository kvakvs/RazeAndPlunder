cmake_minimum_required(VERSION 3.6)
#add_subdirectory("deps/bwapi")

project(RnP_bot)

set(CMAKE_CXX_STANDARD 14)
set(BOOST_DIR f:/boost_1_63_0)
set(BOOST_GEOM_EXT j:/Projects/boost_geometry_ext)

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${BOOST_DIR}/
        ${BOOST_GEOM_EXT}/include
        )

add_definitions(
	/MP /Oi /Zi
	/MD /Gy
	/DNOMINMAX
	)
# /DDEBUG_DRAW=1

set(SOURCE_FILES
        src/AIloop.cpp
        src/AIloop.h
        src/Commander/Commander.cpp
        src/Commander/Commander.h
        src/Commander/ExplorationSquad.cpp
        src/Commander/ExplorationSquad.h
        src/Commander/Protoss/ProtossMain.cpp
        src/Commander/Protoss/ProtossMain.h
        src/Commander/RushSquad.cpp
        src/Commander/RushSquad.h
        src/Commander/Squad.cpp
        src/Commander/Squad.h
        src/Commander/StrategySelector.cpp
        src/Commander/StrategySelector.h
        src/Commander/Terran/TerranMain.cpp
        src/Commander/Terran/TerranMain.h
        src/Commander/UnitSetup.cpp
        src/Commander/UnitSetup.h
        src/Commander/Zerg/LurkerRush.cpp
        src/Commander/Zerg/LurkerRush.h
        src/Commander/Zerg/ZergMain.cpp
        src/Commander/Zerg/ZergMain.h
        src/Dll.cpp
        src/Influencemap/MapManager.cpp
        src/Influencemap/MapManager.h
        src/MainAgents/AgentFactory.cpp
        src/MainAgents/AgentFactory.h
        src/MainAgents/BaseAgent.cpp
        src/MainAgents/BaseAgent.h
        src/MainAgents/TargetingAgent.cpp
        src/MainAgents/TargetingAgent.h
        src/MainAgents/WorkerAgent.cpp
        src/MainAgents/WorkerAgent.h
        src/Managers/AgentManager.cpp
        src/Managers/AgentManager.h
        src/Managers/Agentset.cpp
        src/Managers/Agentset.h
        src/Managers/BuildingPlacer.cpp
        src/Managers/BuildingPlacer.h
        src/Managers/BuildplanEntry.cpp
        src/Managers/BuildplanEntry.h
        src/Managers/Constructor.cpp
        src/Managers/Constructor.h
        src/Managers/ExplorationManager.cpp
        src/Managers/ExplorationManager.h
        src/Managers/ResourceManager.cpp
        src/Managers/ResourceManager.h
        src/Managers/SpottedObject.cpp
        src/Managers/SpottedObject.h
        src/Managers/Upgrader.cpp
        src/Managers/Upgrader.h
        src/OpprimoBot.cpp
        src/OpprimoBot.h
        src/Pathfinding/NavigationAgent.cpp
        src/Pathfinding/NavigationAgent.h
        src/Pathfinding/Pathfinder.cpp
        src/Pathfinding/Pathfinder.h
        src/Pathfinding/PathObj.cpp
        src/Pathfinding/PathObj.h
        src/Pathfinding/PFFunctions.cpp
        src/Pathfinding/PFFunctions.h
        src/StructureAgents/Protoss/NexusAgent.cpp
        src/StructureAgents/Protoss/NexusAgent.h
        src/StructureAgents/RefineryAgent.cpp
        src/StructureAgents/RefineryAgent.h
        src/StructureAgents/StructureAgent.cpp
        src/StructureAgents/StructureAgent.h
        src/StructureAgents/Terran/CommandCenterAgent.cpp
        src/StructureAgents/Terran/CommandCenterAgent.h
        src/StructureAgents/Terran/ComsatAgent.cpp
        src/StructureAgents/Terran/ComsatAgent.h
        src/StructureAgents/Zerg/HatcheryAgent.cpp
        src/StructureAgents/Zerg/HatcheryAgent.h
        src/UnitAgents/Protoss/CarrierAgent.cpp
        src/UnitAgents/Protoss/CarrierAgent.h
        src/UnitAgents/Protoss/CorsairAgent.cpp
        src/UnitAgents/Protoss/CorsairAgent.h
        src/UnitAgents/Protoss/HighTemplarAgent.cpp
        src/UnitAgents/Protoss/HighTemplarAgent.h
        src/UnitAgents/Protoss/ReaverAgent.cpp
        src/UnitAgents/Protoss/ReaverAgent.h
        src/UnitAgents/Terran/BattlecruiserAgent.cpp
        src/UnitAgents/Terran/BattlecruiserAgent.h
        src/UnitAgents/Terran/FirebatAgent.cpp
        src/UnitAgents/Terran/FirebatAgent.h
        src/UnitAgents/Terran/GhostAgent.cpp
        src/UnitAgents/Terran/GhostAgent.h
        src/UnitAgents/Terran/MarineAgent.cpp
        src/UnitAgents/Terran/MarineAgent.h
        src/UnitAgents/Terran/MedicAgent.cpp
        src/UnitAgents/Terran/MedicAgent.h
        src/UnitAgents/Terran/ScienceVesselAgent.cpp
        src/UnitAgents/Terran/ScienceVesselAgent.h
        src/UnitAgents/Terran/SiegeTankAgent.cpp
        src/UnitAgents/Terran/SiegeTankAgent.h
        src/UnitAgents/Terran/VultureAgent.cpp
        src/UnitAgents/Terran/VultureAgent.h
        src/UnitAgents/Terran/WraithAgent.cpp
        src/UnitAgents/Terran/WraithAgent.h
        src/UnitAgents/TransportAgent.cpp
        src/UnitAgents/TransportAgent.h
        src/UnitAgents/UnitAgent.cpp
        src/UnitAgents/UnitAgent.h
        src/UnitAgents/Zerg/DefilerAgent.cpp
        src/UnitAgents/Zerg/DefilerAgent.h
        src/UnitAgents/Zerg/HydraliskAgent.cpp
        src/UnitAgents/Zerg/HydraliskAgent.h
        src/UnitAgents/Zerg/LurkerAgent.cpp
        src/UnitAgents/Zerg/LurkerAgent.h
        src/UnitAgents/Zerg/MutaliskAgent.cpp
        src/UnitAgents/Zerg/MutaliskAgent.h
        src/UnitAgents/Zerg/QueenAgent.cpp
        src/UnitAgents/Zerg/QueenAgent.h
        src/Utils/Config.cpp
        src/Utils/Config.h
        src/Utils/cthread.cpp
        src/Utils/cthread.h
        src/Utils/FileReaderUtils.cpp
        src/Utils/FileReaderUtils.h
        src/Utils/Profiler.cpp
        src/Utils/Profiler.h
        src/Utils/ProfilerObj.cpp
        src/Utils/ProfilerObj.h
        src/Utils/Sets.h
        src/Utils/Statistics.cpp
        src/Utils/Statistics.h
        )
source_group("Source" FILES ${SOURCE_FILES})

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

set(BWTA2_FILES
        include/BWTA.h
        include/BWTA/BaseLocation.h
        include/BWTA/Chokepoint.h
        include/BWTA/Polygon.h
        include/BWTA/RectangleArray.h
        include/BWTA/Region.h
        src/BWTA2/BalanceMetrics.cpp
        src/BWTA2/BalanceMetrics.h
        src/BWTA2/BaseLocationGenerator.cpp
        src/BWTA2/BaseLocationGenerator.h
        src/BWTA2/BaseLocationImpl.cpp
        src/BWTA2/BaseLocationImpl.h
        src/BWTA2/BWTA.cpp
        src/BWTA2/BWTA_Result.cpp
        src/BWTA2/BWTA_Result.h
        src/BWTA2/ChokepointImpl.cpp
        src/BWTA2/ChokepointImpl.h
        src/BWTA2/ClosestObjectMap.cpp
        src/BWTA2/ClosestObjectMap.h
        src/BWTA2/distanceTransform.cpp
        src/BWTA2/DistanceTransform.h
        src/BWTA2/GraphColoring.cpp
        src/BWTA2/GraphColoring.h
        src/BWTA2/Heap.h
        src/BWTA2/LoadData.cpp
        src/BWTA2/LoadData.h
        src/BWTA2/MapData.cpp
        src/BWTA2/MapData.h
        src/BWTA2/Painter.cpp
        src/BWTA2/Painter.h
        src/BWTA2/pathfinding.cpp
        src/BWTA2/Pathfinding.h
        src/BWTA2/PolygonGenerator.cpp
        src/BWTA2/PolygonGenerator.h
        src/BWTA2/PolygonImpl.cpp
        src/BWTA2/PolygonImpl.h
        src/BWTA2/RegionGenerator.cpp
        src/BWTA2/RegionGenerator.h
        src/BWTA2/RegionImpl.cpp
        src/BWTA2/RegionImpl.h
        src/BWTA2/stdafx.cpp
        src/BWTA2/stdafx.h
        src/BWTA2/TerrainAnalysis.cpp
        src/BWTA2/TileType.h
        src/BWTA2/Timer.h
        src/BWTA2/Utils.cpp
        src/BWTA2/Utils.h
        )
source_group("BWTA2" FILES ${BWTA2_FILES})

link_directories(
        ${BOOST_DIR}/lib32-msvc-12.0
        lib
        )

set(DEBUG_D_SUFFIX d)
set(DEBUG_GD_SUFFIX -gd)
set(BOT_TARGET_NAME rnpbot${DEBUG_D_SUFFIX})
set(BOOST_VER 1_63)

add_library(${BOT_TARGET_NAME} SHARED
        ${SOURCE_FILES} ${BWAPI_FILES} ${BWTA2_FILES})

target_link_libraries(${BOT_TARGET_NAME}
        BWAPI${DEBUG_D_SUFFIX}
        BWAPIClient${DEBUG_D_SUFFIX}
        libCGAL-vc120-mt-4.4
        libboost_filesystem-vc120-mt${DEBUG_GD_SUFFIX}-${BOOST_VER}
        libboost_system-vc120-mt${DEBUG_GD_SUFFIX}-${BOOST_VER}
        libboost_thread-vc120-mt${DEBUG_GD_SUFFIX}-${BOOST_VER}
        )

#        BWTA${DEBUG_D_SUFFIX}
#        gmpd
#        libmpfr-4

#install(TARGETS ${BOT_TARGET_NAME}
#        RUNTIME DESTINATION f:/Games/Starcraft/bwapi-data/
#        )
