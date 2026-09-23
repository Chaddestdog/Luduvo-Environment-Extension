#include <Windows.h>
#include <thread>
#include <iostream>
#include <scripting/Scheduler.h>
#include <Offsets.h>

int Entry() {
#ifdef _DEBUG
    AllocConsole(); SetConsoleTitleA("Debug");
    FILE* fp; freopen_s(&fp, "CONOUT$", "w", stdout); freopen_s(&fp, "CONOUT$", "w", stderr);
#endif

    if (not lee::Offsets::GetSingleton()->Initalise()) {
        printf("Offsets->Initalise() feiald\n");
        return EXIT_FAILURE;
    }

    auto Scheduler = lee::Scheduler::GetSingleton();
    if (not Scheduler->Initalise()) {
        printf("Scheduler->Initalise() feiald\n");
        return EXIT_FAILURE;
    }

    printf("done :3\n");

    while (true) {
        Sleep(16);
        Scheduler->Step();
    }
    
    return EXIT_SUCCESS;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        std::thread(Entry).detach();
    return TRUE;
}