
#include <stdio.h>
#include <windows.h>
#include <winuser.h>

// struct handle_data accessible by all functions
struct handle_data {
    unsigned long process_id;
    HWND window_handle;
};

// do nothing
__declspec(dllexport) void DoNothing()
{
    return;
}

BOOL is_main_window(HWND handle)
{
    return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}
BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
    struct handle_data data = *(struct handle_data*)lParam;
    unsigned long process_id = 0;
    GetWindowThreadProcessId(handle, &process_id);
    if (data.process_id != process_id || !is_main_window(handle))
        return TRUE;
    data.window_handle = handle;
    return FALSE;
}

HWND find_main_window(unsigned long process_id)
{
    struct handle_data data;
    data.process_id = process_id;
    data.window_handle = 0;
    EnumWindows(enum_windows_callback, (LPARAM)&data);
    return data.window_handle;
}


// we want to receive custom events, and from that simulate key presses
__declspec(dllexport) void TestDisplayDialog()
{
    // get process name
    char processName[256];
    GetModuleFileName(NULL, processName, 256);
    // target string ("Hello from ",processName)
    char target[256];
    strcpy(target, "Hello from ");
    strcat(target, processName);
    MessageBox(NULL, "Hello World", target, MB_OK);
}

// message to confirm that the dll has been loaded (returns 1)
__declspec(dllexport) int TestDll()
{
    return 1;
}

// simulate a key press
__declspec(dllexport) void SimulateKeyPress(LPCSTR key)
{
    // message to confirm we received the key press
    int virtualKeyCode = VkKeyScanEx(key[0], GetKeyboardLayout(0));
    DWORD processId = GetCurrentProcessId();
    HWND hwnd = find_main_window(processId);
    // modifiers: ctrl shift
    keybd_event(VK_CONTROL, 0, 0, 0);
    keybd_event(VK_SHIFT, 0, 0, 0);
    keybd_event(virtualKeyCode, 0, 0, 0);
    keybd_event(virtualKeyCode, 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
    // also, bonus: get out of the WH_KEYBOARD_LL hook (still accessible to keybd_event)
    // get main window thread id
    DWORD threadId = GetWindowThreadProcessId(hwnd, NULL);
    // Declare a hook handle
    HHOOK hookHandle = NULL;

    // Set the hook using SetWindowsHookEx
    hookHandle = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)DoNothing, NULL, threadId);

    // Unhook the hook using UnhookWindowsHookEx
    UnhookWindowsHookEx(hookHandle);
}

