#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <winuser.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_PATH 260

// dllimport for the Windows API functions we will use
// import using __declspec(dllimport) for C

__declspec(dllimport) HANDLE WINAPI OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
__declspec(dllimport) FARPROC WINAPI GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE hObject);


// Structure for enumerating the modules (DLLs) in a process
typedef struct _MODULEENTRY32
{
    DWORD dwSize;
    DWORD th32ModuleID;
    DWORD th32ProcessID;
    DWORD GlblcntUsage;
    DWORD ProccntUsage;
    BYTE *modBaseAddr;
    LPDWORD modBaseSize;
    HMODULE hModule;
    char szModule[MAX_PATH];
    char szExePath[MAX_PATH];// moduleentry32 from tlhelp32.h
} _MODULEENTRY32, *_LPMODULEENTRY32;

// Constants for the CreateToolhelp32Snapshot function
#define TH32CS_SNAPMODULE 0x00000008
#define TH32CS_SNAPMODULE32 0x00000010

// Constants for the Module32First/Module32Next functions
#define TH32CS_INHERIT 0x80000000


char key = 'z';
char dllpath[MAX_PATH];
bool refresh = false;


BOOL WINAPI EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    // Get the process ID of the window
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0)
        return TRUE;
    // Open the process
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess == NULL)
        return TRUE;
    // Create a snapshot of the modules (DLLs) in the process
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return TRUE;
    // Fill in the size of the structure before using it
    MODULEENTRY32 me;
    me.dwSize = sizeof(MODULEENTRY32);
    if (Module32First(hSnapshot, &me))
    {
        do
        {
            // Get the address of any exported functions in the DLL
            FARPROC pFunc = GetProcAddress(me.hModule, "SetWindowsHookExA");
            if (pFunc)
            {
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
                // is the next hook vtuber maker?
                TCHAR szProcName[MAX_PATH];
                GetModuleFileNameEx(hProcess, NULL, szProcName, MAX_PATH);
                if (strstr(szProcName, "VTuber Maker.exe") != NULL)
                {
                    char classname[256];
                    GetClassName(hwnd, classname, 256);
                    // is class UnityWndClass?
                    if(strcmp(classname, "UnityWndClass") != 0)
                        continue;
                    printf("Found vtuber maker process: %s\n", szProcName);
                    // load library
                    HMODULE hModule = LoadLibrary(dllpath);
                    if(hModule == NULL)
                    {
                        printf("Failed to load library: %s\n", dllpath);
                        return FALSE;
                    }
                    HANDLE hThread;
                    long result = 0;
                    // get their dll list
                    HMODULE hMods[1024];
                    DWORD cbNeeded;
                    if(EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
                    {
                        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
                        {
                            TCHAR szModName[MAX_PATH];
                            if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
                            {
                                // is it the same as ours?
                                if(strcmp(szModName, dllpath) == 0)
                                {
                                    printf("Found our dll in target process\n");
                                    result = 1;
                                }
                            }
                        }
                    }
                    // if it isn't handled, inject
                    if(result == 0 || refresh)
                    {
                        if(result != 0){
                            printf("Refreshing dll in vtuber maker...\n");
                            refresh = false;
                            // we want to refresh the dll, so we need to unhook it first
                            FARPROC UnhookWindowsHookExAddr = GetProcAddress(me.hModule, "UnhookWindowsHookEx");
                            hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)UnhookWindowsHookExAddr, NULL, 0, NULL);
                            WaitForSingleObject(hThread, INFINITE);
                            CloseHandle(hThread);
                            // exit code?
                            GetExitCodeThread(hThread, &result);
                            if(result == 0)
                            {
                                printf("Failed to unhook dll\n");
                                return FALSE;
                            }
                            else
                                printf("Unhooked dll\n");
                        }
                        printf("Injecting dll into vtuber maker...\n");
                        // inject the dll
                        LPVOID mydll = VirtualAllocEx(hProcess, NULL, strlen(dllpath), MEM_COMMIT, PAGE_READWRITE);
                        WriteProcessMemory(hProcess, mydll, dllpath, strlen(dllpath), NULL);
                        LPVOID lpStartAddress = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
                        hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpStartAddress, mydll, 0, NULL);
                        if(hThread == NULL)
                        {
                            printf("Failed to unhook dll\n");
                            return FALSE;
                        }
                        // free stuff
                        CloseHandle(hThread);
                        VirtualFreeEx(hProcess, mydll, strlen(dllpath), MEM_RELEASE);
                    }
                    else
                        printf("Dll already injected\n");
                    // call the dll's SimulateKeyPress function, for now we test with TestDisplayDialog
                    printf("Simulating %c key press...\n", key);
                    char keyL[2];
                    strcpy(keyL, &key);
                    FARPROC SimulateKeyPressAddr = GetProcAddress(hModule, "SimulateKeyPress");
                    LPVOID convertedKey = VirtualAllocEx(hProcess, NULL, strlen(keyL), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
                    WriteProcessMemory(hProcess, convertedKey, keyL, strlen(keyL), NULL);
                    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)SimulateKeyPressAddr, convertedKey, 0, NULL);
                    if(hThread == NULL)
                    {
                        printf("Failed to simulate key press\n");
                        printf("Error: %d\n", GetLastError());
                        return FALSE;
                    }
                    WaitForSingleObject(hThread, INFINITE);
                    // get the result
                    GetExitCodeThread(hThread, &result);
                    // free stuff
                    FreeLibrary(hModule);
                    VirtualFreeEx(hProcess, convertedKey, sizeof(char), MEM_RELEASE);
                    CloseHandle(hThread);
                }
                // free stuff
                CloseHandle(hProcess);
            }
        } while (Module32Next(hSnapshot, &me));
    }
    // Close the snapshot handle
    CloseHandle(hSnapshot);
    // Close the process handle
    CloseHandle(hProcess);
    return TRUE;
}

int main(int argc, char *argv[])
{
    // if no argument, set to "t"
    if (argc < 2)
        key = 't';
    else
        key = (char)toupper((int)argv[1][0]);
    if(argc < 3){
        char executablePath[256];
        GetModuleFileName(NULL, executablePath, 256);
        char *lastSlash = strrchr(executablePath, '\\');
        if(lastSlash != NULL)
            lastSlash[1] = '\0';
        strcpy(dllpath, executablePath);
        strcat(dllpath, "keyboard.dll");
    }
    else
        strcpy(dllpath, argv[2]);
    if(argc < 4)
        refresh = false;
    else
        refresh = argv[3][0] == '1';
    // Enumerate all windows
    printf("Enumerating windows...\n");
    EnumWindows(EnumWindowsProc, 0);
    // prompt done, wait for user input
    printf("Press any key to exit...\n");
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
    //getch();
    #pragma GCC diagnostic pop
    return 0;
}
