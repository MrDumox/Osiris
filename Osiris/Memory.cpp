#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Psapi.h>

#include "Memory.h"

Memory::Memory()
{
    present = findPattern("gameoverlayrenderer.dll", "FF 15 ? ? ? ? 8B F8 85 DB") + 2;
    reset = findPattern("gameoverlayrenderer.dll", "C7 45 ? ? ? ? ? FF 15 ? ? ? ? 8B F8") + 9;
    localPlayer = findPattern("client_panorama.dll", "8B 0D ? ? ? ? 83 FF FF 74 07") + 2;
    globalVars = findPattern("client_panorama.dll", "A1 ? ? ? ? 5E 8B 40 10") + 1;


    isReloading = findPattern("client_panorama.dll", "C6 87 ? ? ? ? ? 8B 06 8B CE FF 90") + 2;
    isReloading2 = findPattern("client_panorama.dll", "83 BE ? ? ? ? ? 7F 67") + 2;
    currentCommand = findPattern("client_panorama.dll", "C6 86 ? ? ? ? ? FF 50 04") + 2;
    currentCommand2 = findPattern("client_panorama.dll", "89 BE ? ? ? ? E8 ? ? ? ? 85 FF") + 2;
}

std::uintptr_t Memory::findPattern(std::string module, std::string pattern)
{
    MODULEINFO moduleInfo{ };

    if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(module.c_str()), &moduleInfo, sizeof(moduleInfo)))
        return 0;

    const uintptr_t start_address = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
    const uintptr_t end_address = start_address + moduleInfo.SizeOfImage;
    const char* scanPattern = pattern.c_str();

    uintptr_t first_match = 0;

    for (uintptr_t position = start_address; position < end_address; position++) {
        if (!*scanPattern)
            return first_match;

        const uint8_t pattern_current = *reinterpret_cast<const uint8_t*>(scanPattern);
        const uint8_t memory_current = *reinterpret_cast<const uint8_t*>(position);

        if (pattern_current == '\?' || memory_current == ((((scanPattern[0] & (~0x20)) >= 'A' && (scanPattern[0] & (~0x20)) <= 'F') ? ((scanPattern[0] & (~0x20)) - 'A' + 0xA) : ((scanPattern[0] >= '0' && scanPattern[0] <= '9') ? scanPattern[0] - '0' : 0)) << 4 | (((scanPattern[1] & (~0x20)) >= 'A' && (scanPattern[1] & (~0x20)) <= 'F') ? ((scanPattern[1] & (~0x20)) - 'A' + 0xA) : ((scanPattern[1] >= '0' && scanPattern[1] <= '9') ? scanPattern[1] - '0' : 0)))) {
            if (!first_match)
                first_match = position;

            if (!scanPattern[2])
                return first_match;

            scanPattern += pattern_current != '\?' ? 3 : 2;
        }
        else {
            scanPattern = pattern.c_str();
            first_match = 0;
        }
    }
    return 0;
}
