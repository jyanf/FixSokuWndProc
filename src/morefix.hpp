//thanks to @Co1umbium and @fishshapedapple
#include <SokuLib.hpp>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "imm32.lib")
#include <dinput.h>

template<typename T>
static T* TamperIndirectCall(DWORD addr, T* target) {
	DWORD old;
	T* org = NULL;
	auto& address = *reinterpret_cast<unsigned char**>(&addr);
	VirtualProtect(address, 6, PAGE_EXECUTE_WRITECOPY, &old);
	if (*address == 0xFF && address[1] == 0x15) {//rel32 call
		DWORD old2;
		T** &addr2 = *reinterpret_cast<T***>(address + 2);
		VirtualProtect(addr2, 4, PAGE_READONLY, &old2);
		org = *addr2;
		VirtualProtect(addr2, 4, old2, &old2);
		unsigned char call[] = { 0xE8, 0, 0, 0, 0, 0x90 };
		*(DWORD*)(call + 1) = DWORD(target)-DWORD(address + 5);
		memcpy(address, call, sizeof(call));
	} else if (*address == 0xE8) {//near call
		SokuLib::TamperNearCall(addr, target);
	} else {
		// unknown
	}
	VirtualProtect(address, 6, old, &old);
	return org;
}

inline void FakePress()
{
	INPUT input[2] = { 0 };

	// Ctrl down
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = VK_MENU;
	input[0].ki.dwFlags = 0;

	// Ctrl up
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = VK_MENU;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(2, input, sizeof(INPUT));
}

/*
static WNDPROC originalWndProc = nullptr;
static LRESULT CALLBACK PatchedWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	unsigned short key = LOWORD(wParam);
	switch (uMsg) {
	case WM_IME_SETCONTEXT:

		break;

	case WM_KEYDOWN:
		if ((key == VK_LWIN || key == VK_RWIN)
		&& !(HIWORD(lParam) & KF_REPEAT)//preiviously not pressed
		) {
			FakePress();
			puts("WIN UP blocked");
			return 0;
		}
		break;
	case WM_SYSCOMMAND:
		if (key == SC_KEYMENU || key == SC_TASKLIST) {
			puts("WM_SYSCOMMAND blocked");
			return 0;
		}
		break;
	case WM_DESTROY:
		//HookRemove();
		break;
	}


	return CallWindowProc(originalWndProc, hWnd, uMsg, wParam, lParam);
}
*/


inline void setEngIME(HWND hwnd) {
	MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
	HKL hkl = LoadKeyboardLayoutW(L"00000409", KLF_ACTIVATE);
	if (hkl) {
		ActivateKeyboardLayout(hkl, KLF_SETFORPROCESS);
		puts("KeyboradLayout switched to EN-US. ");
	}
	else {
		HIMC hIMC = ImmGetContext(hwnd);
		if (hIMC) {
			ImmSetOpenStatus(hIMC, FALSE);
			ImmReleaseContext(hwnd, hIMC);
			puts("Tried to close Input Method. ");
		}
		else {

		}
	}
}

/*
static BOOL(__stdcall* orgUpdateWindow)(HWND) = NULL;
static BOOL __stdcall afterWndCreated(HWND hwnd) {
	auto ret = orgUpdateWindow(hwnd);
	originalWndProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
	if (originalWndProc)
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)PatchedWndProc);

	setEngIME(hwnd);

	return ret;
}*/
//static void __cdecl afterWindowStart(void* _ignored)
//{
//	HWND hGameWnd = nullptr;
//	while (!hGameWnd) {
//		hGameWnd = SokuLib::window;
//		Sleep(200);
//		puts("Thread tried");
//	}
//	puts("Inited");
//	// Ìæ»»´°¿Ú¹ý³Ì
//	originalWndProc = (WNDPROC)GetWindowLongPtr(hGameWnd, GWLP_WNDPROC);
//	if (originalWndProc)
//		SetWindowLongPtr(hGameWnd, GWLP_WNDPROC, (LONG_PTR)PatchedWndProc);
//	_endthread();
//}
static unsigned char* const ADDR_DINPUT_SETCOOPERATIVE_SPEC = (unsigned char*)0x40d8b4;
inline void tamperDInputSetup() {
	DWORD old;
	VirtualProtect(ADDR_DINPUT_SETCOOPERATIVE_SPEC, 1, PAGE_WRITECOPY, &old);
	printf("Original DInput SetCooperativeLevel spec: %#02X changed to ", *ADDR_DINPUT_SETCOOPERATIVE_SPEC);
	*ADDR_DINPUT_SETCOOPERATIVE_SPEC = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;// | DISCL_NOWINKEY;
	printf("%#02X\n", *ADDR_DINPUT_SETCOOPERATIVE_SPEC);
	VirtualProtect(ADDR_DINPUT_SETCOOPERATIVE_SPEC, 1, old, &old);
}

/*
extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
#ifdef _DEBUG
	FILE *_;
	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
	puts("Hello, world! by FixSokuWndProc");
#endif
	DWORD old;
	//_beginthread(afterWindowStart, 0, nullptr);
	orgUpdateWindow = TamperIndirectCall(0x7fb735, afterWndCreated);
	
	tamperDInputSetup();
	
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return true;
}

//check Soku 1.10a
extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return memcmp(hash, SokuLib::targetHash, sizeof(SokuLib::targetHash)) == 0;
}
extern "C" __declspec(dllexport) int getPriority()
{
	return -1;
}
*/