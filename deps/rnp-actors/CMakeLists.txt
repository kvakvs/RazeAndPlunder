SET(CMAKE_CXX_STANDARD 14)

PROJECT(RNPActors-${CMAKE_BUILD_TYPE} CXX)

INCLUDE_DIRECTORIES(
    src
    )
ADD_DEFINITIONS(${RNP_COMPILE_FLAGS})

SET(SOURCE_FILES
    Actors/Actor.cpp
    Actors/Actor.h
    Actors/Scheduler.cpp
    Actors/Scheduler.h
    Actors/ActorId.h 
    Actors/Algorithm.h
    Actors/Timer.h
    Actors/Timer.cpp
    )

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	SET(RNP_ACTORS_TARGET_NAME	"rnpactors-d")
else()
	SET(RNP_ACTORS_TARGET_NAME	"rnpactors")
endif()

ADD_LIBRARY(${RNP_ACTORS_TARGET_NAME} STATIC ${SOURCE_FILES})
SET_TARGET_PROPERTIES(${RNP_ACTORS_TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
