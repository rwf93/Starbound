
#include <Winsock2.h>
#include <WinBase.h>
#include <Windows.h>
#include <stdio.h>
#include <intrin.h>

#pragma comment(lib, "ws2_32.lib")

typedef unsigned char byte;

#pragma intrinsic(_ReturnAddress)

WINBASEAPI
BOOL
WINAPI
GetModuleHandleExA(
    __in        DWORD    dwFlags,
    __in_opt    LPCSTR lpModuleName,
    __out HMODULE* phModule
    );

void WriteMem(DWORD addr, byte* mem, DWORD len)
{
	DWORD dwOldPr;
	if(VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &dwOldPr))
	{
		memcpy((void*)addr, mem, len);
		VirtualProtect((void*)addr, len, dwOldPr, &dwOldPr);
		FlushInstructionCache(GetCurrentProcess(), (void*)addr, len);
	}
}

void WriteJmp(DWORD from, DWORD to)
{
	byte jmp[5] = { 0xE9, 0,0,0,0 };
	*(DWORD*)(&jmp[1]) = to - (from + 5);
	WriteMem(from, jmp, 5);
}

byte* GetCodeBytes(DWORD len)
{
	return (byte*)VirtualAlloc(NULL, len, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

void FinalizeCodeBytes(byte* addr, DWORD len) {
	DWORD dwOldPr;
    VirtualProtect((void*)addr, len, PAGE_EXECUTE_READ, &dwOldPr);
	FlushInstructionCache(GetCurrentProcess(), (void*)addr, len);
}

DWORD GetAddr(const char* mod, const char* func)
{
	return (DWORD)GetProcAddress(GetModuleHandleA(mod), func);
}
HMODULE ghMod;

DWORD setlocale_addr;
typedef char*(__cdecl* t_setlocale)(int, const char*);
t_setlocale o_setlocale;
CRITICAL_SECTION setlocale_cs;
char* __cdecl hk_setlocale(int category, const char* locale)
{
	EnterCriticalSection(&setlocale_cs);
	char* r = o_setlocale(category, locale);
	LeaveCriticalSection(&setlocale_cs);
	if(r == NULL)
		r = "C";

	return r;
}

void PatchSetLocale() {
	HMODULE hmsvcrt;
	if ((hmsvcrt=LoadLibraryA("msvcrt")) == NULL)
		return;

	InitializeCriticalSection(&setlocale_cs);

	setlocale_addr = GetAddr("msvcrt", "setlocale");
	byte* origcode = GetCodeBytes(2+5+5);
	memcpy(origcode, (void*)setlocale_addr, 7);
	WriteJmp((DWORD)&origcode[7], setlocale_addr+7);
	o_setlocale = (t_setlocale)origcode;
	WriteJmp(setlocale_addr, (DWORD)&hk_setlocale);
	FinalizeCodeBytes(origcode, 2+5+5);
}

void ApplyPatches(void)
{ 
	OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);
	BOOL bIsWindowsXPOrEarlier = (osvi.dwMajorVersion <= 5);

	if (bIsWindowsXPOrEarlier)
		PatchSetLocale();
}


BOOL WINAPI DllMain(HMODULE hMod, DWORD fdwReason, LPVOID lpReserved)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		ghMod = hMod;
		ApplyPatches();
		break;
	}

	return TRUE;
}
