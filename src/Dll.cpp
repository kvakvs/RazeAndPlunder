#if 0
#include <BWAPI.h>

#include "RnpBot.h"

extern "C" __declspec(dllexport) void gameInit(BWAPI::Game* game) {
  BWAPI::BroodwarPtr = game;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    break;
  case DLL_PROCESS_DETACH:
    break;
  default:
    break;
  }
  return TRUE;
}

extern "C" __declspec(dllexport) BWAPI::AIModule* newAIModule() {
  return RnpBot::setup();
}
#endif //0
