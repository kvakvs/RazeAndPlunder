cmake_minimum_required(VERSION 3.6)

##--- begin config ---
##
##
##--- end config ---

IF(CMAKE_CROSSCOMPILING)
    INCLUDE(${CMAKE_TOOLCHAIN_FILE})
ENDIF()


SET(CMAKE_CXX_STANDARD 14)

SET(RNP_VERSION 0.1)

if (RNP_MSVC)
    # Parallel build /MP, debug info /Zi
    SET(ENV{CL} /MP)
    SET(RNP_COMPILE_FLAGS ${RNP_COMPILE_FLAGS}
            /DRNP_VERSION="${RNP_VERSION}"
            /MP /Zi /Gy /DNOMINMAX
            /fp:precise
            /std:c++latest
            )

    MESSAGE(STATUS ${CMAKE_BUILD_TYPE})

    # Debug checks, MT Debug DLL, no opt /Od, Omit frame pointers disable /Oy-, /GS security check
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        SET(RNP_COMPILE_FLAGS ${RNP_COMPILE_FLAGS}
                /RTC1 /MTd /DRNP_DEBUG=1 /Od /Oy- /GS
                )
    	SET(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)
    else ()
        # intrinsic funs /Oi, inline any suitable /Ob2, favour fast /Ot
        # string pooling /GF, Omit frame pointers /Oy
        SET(RNP_COMPILE_FLAGS ${RNP_COMPILE_FLAGS}
                /Oi /Ob2 /Ot /GF /Oy /GS-
                /MT /DRNP_DEBUG=0
                )
        SET(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
    endif ()
    SET(BWAPI_CUSTOM_COMPILE_FLAGS ${RNP_COMPILE_FLAGS})
ELSE ()
    SET(RNP_COMPILE_FLAGS ${RNP_COMPILE_FLAGS}
            -DRNP_VERSION="${RNP_VERSION}"
            -DNOMINMAX -std=c++17
            -g -ggdb -O0
            -Weffc++
            )
    SET(BWAPI_CUSTOM_COMPILE_FLAGS ${RNP_COMPILE_FLAGS} -fpermissive)
ENDIF ()

ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/deps/bwapi/CMake/BWAPI)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/deps/bwapi/CMake/Client)

ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/deps/rnp-actors)

##
## Bot Main Project
##

PROJECT(RNPBot-${CMAKE_BUILD_TYPE} CXX)

GET_FILENAME_COMPONENT(BWAPI_ROOT           ${CMAKE_SOURCE_DIR}/deps/bwapi/bwapi    ABSOLUTE)
GET_FILENAME_COMPONENT(RNP_BWAPI_LINKDIR    ${BWAPI_ROOT}/lib                       ABSOLUTE)

INCLUDE_DIRECTORIES(
        ${BWAPI_ROOT}/include
        ${BWAPI_ROOT}/Util/Source
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/deps/rnp-actors
        ${CMAKE_SOURCE_DIR}/deps/spdlog/include
        )
ADD_DEFINITIONS(${RNP_COMPILE_FLAGS})

SET(SRC_SRC
        src/BotAILoop.cpp
        src/BotAILoop.h
        src/ClientMain.cpp
        src/Dll.cpp
        src/Glob.cpp
        src/Glob.h
        src/Influencemap/MapManager.cpp
        src/Influencemap/MapManager.h
        src/PathAB.cpp
        src/PathAB.h
        src/RnpBot.cpp
        src/RnpBot.h
        src/RnpConst.h
        src/RnpRandom.cpp
        src/RnpRandom.h
        src/RnpUtil.cpp
        src/RnpUtil.h
        src/RnpStateMachine.h
        )
SET(SRC_PATHING
        src/Pathfinding/NavigationAgent.cpp
        src/Pathfinding/NavigationAgent.h
        src/Pathfinding/Pathfinder.cpp
        src/Pathfinding/Pathfinder.h
        src/Pathfinding/PathObj.cpp
        src/Pathfinding/PathObj.h
        src/Pathfinding/PFFunctions.cpp
        src/Pathfinding/PFFunctions.h
        )
SET(SRC_MAINAGENTS
        src/MainAgents/AgentFactory.cpp
        src/MainAgents/AgentFactory.h
        src/MainAgents/BaseAgent.cpp
        src/MainAgents/BaseAgent.h
        src/MainAgents/BaseAgentMsg.h
        src/MainAgents/TargetingAgent.cpp
        src/MainAgents/TargetingAgent.h
        src/MainAgents/RnpMovementMonitor.cpp
        src/MainAgents/RnpMovementMonitor.h
        src/MainAgents/WorkerAgent.cpp
        src/MainAgents/WorkerAgent.h
        src/MainAgents/WorkerAgentMsg.h
        )
SET(SRC_MANAGERS
        src/Managers/AgentManager.cpp
        src/Managers/AgentManager.h
        src/Managers/AgentManagerMsg.h
        src/Managers/BuildingPlacer.cpp
        src/Managers/BuildingPlacer.h
        src/Managers/BuildplanEntry.cpp
        src/Managers/BuildplanEntry.h
        src/Managers/Constructor.cpp
        src/Managers/Constructor.h
        src/Managers/ConstructorMsg.h
        src/Managers/ExplorationManager.cpp
        src/Managers/ExplorationManager.h
        src/Managers/ResourceManager.cpp
        src/Managers/ResourceManager.h
        src/Managers/SpottedObject.cpp
        src/Managers/SpottedObject.h
        src/Managers/Upgrader.cpp
        src/Managers/Upgrader.h
        )
SET(SRC_COMMANDER
        src/Commander/Commander.cpp
        src/Commander/Commander.h
        src/Commander/ExplorationSquad.cpp
        src/Commander/ExplorationSquad.h
        src/Commander/Protoss/ProtossMain.cpp
        src/Commander/Protoss/ProtossMain.h
        src/Commander/RnpArmy.cpp
        src/Commander/RnpArmy.h
        src/Commander/RushSquad.cpp
        src/Commander/RushSquad.h
        src/Commander/Squad.cpp
        src/Commander/Squad.h
        src/Commander/SquadMsg.h
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
        )
SET(SRC_STRUCTAGENTS
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
        )
SET(SRC_UNITAGENTS
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
        )
SET(SRC_UTILS
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

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/deps/spdlog/include)
SET(SPDLOG_DIR ${CMAKE_SOURCE_DIR}/deps/spdlog/include/spdlog)
SET(SRC_SPDLOG 
        ${SPDLOG_DIR}/async_logger.h
        ${SPDLOG_DIR}/common.h
        ${SPDLOG_DIR}/formatter.h
        ${SPDLOG_DIR}/logger.h
        ${SPDLOG_DIR}/spdlog.h
        ${SPDLOG_DIR}/tweakme.h
        ${SPDLOG_DIR}/details/async_log_helper.h
        ${SPDLOG_DIR}/details/async_logger_impl.h
        ${SPDLOG_DIR}/details/file_helper.h
        ${SPDLOG_DIR}/details/log_msg.h
        ${SPDLOG_DIR}/details/logger_impl.h
        ${SPDLOG_DIR}/details/mpmc_bounded_q.h
        ${SPDLOG_DIR}/details/null_mutex.h
        ${SPDLOG_DIR}/details/os.h
        ${SPDLOG_DIR}/details/pattern_formatter_impl.h
        ${SPDLOG_DIR}/details/registry.h
        ${SPDLOG_DIR}/details/spdlog_impl.h
        ${SPDLOG_DIR}/fmt/fmt.h
        ${SPDLOG_DIR}/fmt/ostr.h
        ${SPDLOG_DIR}/fmt/bundled/time.h
        ${SPDLOG_DIR}/sinks/android_sink.h
        ${SPDLOG_DIR}/sinks/ansicolor_sink.h
        ${SPDLOG_DIR}/sinks/base_sink.h
        ${SPDLOG_DIR}/sinks/dist_sink.h
        ${SPDLOG_DIR}/sinks/file_sinks.h
        ${SPDLOG_DIR}/sinks/msvc_sink.h
        ${SPDLOG_DIR}/sinks/null_sink.h
        ${SPDLOG_DIR}/sinks/ostream_sink.h
        ${SPDLOG_DIR}/sinks/sink.h
        ${SPDLOG_DIR}/sinks/stdout_sinks.h
        ${SPDLOG_DIR}/sinks/syslog_sink.h
        ${SPDLOG_DIR}/sinks/wincolor_sink.h
        )
SOURCE_GROUP("Bot" FILES ${SRC_SRC})
SOURCE_GROUP("Bot/Commander" FILES ${SRC_COMMANDER})
SOURCE_GROUP("Bot/MainAgents" FILES ${SRC_MAINAGENTS})
SOURCE_GROUP("Bot/Managers" FILES ${SRC_MANAGERS})
SOURCE_GROUP("Bot/Pathing" FILES ${SRC_PATHING})
SOURCE_GROUP("Bot/StructureAgents" FILES ${SRC_STRUCTAGENTS})
SOURCE_GROUP("Bot/UnitAgents" FILES ${SRC_UNITAGENTS})
SOURCE_GROUP("Bot/Utils" FILES ${SRC_UTILS})
SOURCE_GROUP("SpdLog" FILES ${SRC_SPDLOG})

SET(SOURCE_FILES
        ${SRC_COMMANDER}
        ${SRC_MAINAGENTS}
        ${SRC_MANAGERS}
        ${SRC_PATHING}
        ${SRC_SRC}
        ${SRC_STRUCTAGENTS}
        ${SRC_UNITAGENTS}
        ${SRC_UTILS}
        ${SRC_SPDLOG}
        )

SET(BWEM_ROOT deps/BWEM-1.3.1)
SET(BWEM_DIR ${BWEM_ROOT}/BWEM)
SET(BWEM_EASYBMP ${BWEM_ROOT}/EasyBMP106)
INCLUDE_DIRECTORIES(${BWEM_ROOT})
SET(BWEM_FILES
        ${BWEM_DIR}/area.cpp
        ${BWEM_DIR}/area.h
        ${BWEM_DIR}/base.cpp
        ${BWEM_DIR}/base.h
        ${BWEM_DIR}/bwapiExt.cpp
        ${BWEM_DIR}/bwapiExt.h
        ${BWEM_DIR}/bwem.cpp
        ${BWEM_DIR}/bwem.h
        ${BWEM_DIR}/cp.cpp
        ${BWEM_DIR}/cp.h
        ${BWEM_DIR}/defs.h
        ${BWEM_DIR}/graph.cpp
        ${BWEM_DIR}/graph.h
        ${BWEM_DIR}/gridMap.cpp
        ${BWEM_DIR}/gridMap.h
        ${BWEM_DIR}/map.cpp
        ${BWEM_DIR}/map.h
        ${BWEM_DIR}/mapDrawer.cpp
        ${BWEM_DIR}/mapDrawer.h
        ${BWEM_DIR}/mapImpl.cpp
        ${BWEM_DIR}/mapImpl.h
        ${BWEM_DIR}/mapPrinter.cpp
        ${BWEM_DIR}/mapPrinter.h
        ${BWEM_DIR}/neutral.cpp
        ${BWEM_DIR}/neutral.h
        ${BWEM_DIR}/tiles.cpp
        ${BWEM_DIR}/tiles.h
        ${BWEM_DIR}/utils.cpp
        ${BWEM_DIR}/utils.h
        ${BWEM_DIR}/winutils.cpp
        ${BWEM_DIR}/winutils.h
        ${BWEM_EASYBMP}/EasyBMP.cpp
        ${BWEM_EASYBMP}/EasyBMP.h
        ${BWEM_EASYBMP}/EasyBMP_BMP.h
        ${BWEM_EASYBMP}/EasyBMP_DataStructures.h
        ${BWEM_EASYBMP}/EasyBMP_VariousBMPutilities.h
        )
SOURCE_GROUP("BWEM" FILES ${BWEM_FILES})

LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib)

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(RNP_BWEM_LIB                "rnp-bwem131-d")
    SET(RNP_DLL_NAME                "rnpbot-d")
    SET(RNP_CLIENT_NAME             "rnpbot-client-d")
    SET(RNP_ACTORS_TARGET_NAME      "rnpactors-d")
ELSE ()
    SET(RNP_BWEM_LIB                "rnp-bwem131")
    SET(RNP_DLL_NAME                "rnpbot")
    SET(RNP_CLIENT_NAME             "rnpbot-client")
    SET(RNP_ACTORS_TARGET_NAME      "rnpactors")
ENDIF ()

ADD_LIBRARY(${RNP_BWEM_LIB} STATIC ${BWEM_FILES})

ADD_EXECUTABLE(${RNP_CLIENT_NAME}
        ${SOURCE_FILES}
        ${BWAPI_FILES}
        )

ADD_DEPENDENCIES(${RNP_CLIENT_NAME} "BWAPI-Static")
ADD_DEPENDENCIES(${RNP_CLIENT_NAME} "BWAPI-Client-Static")

LINK_DIRECTORIES(${RNP_BWAPI_LINKDIR})
ADD_DEPENDENCIES(${RNP_CLIENT_NAME} ${RNP_BWEM_LIB})
ADD_DEPENDENCIES(${RNP_CLIENT_NAME} ${RNP_ACTORS_TARGET_NAME})

TARGET_LINK_LIBRARIES(${RNP_CLIENT_NAME}
        ${RNP_BWEM_LIB}
        "BWAPI-Static" 
        "BWAPI-Client-Static"
        ${RNP_ACTORS_TARGET_NAME}
        )
