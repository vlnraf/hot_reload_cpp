#include "live_reload_code/reload_code.h"

#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

#include <windows.h>

struct ReloadableCpp
{
    std::wstring DLLPath; // it shoulw be wstring because CreateFileW accept only wstring
    std::vector<std::string> procsToLoad;
    
    HMODULE module = nullptr;
    uint64_t lastWrite = 0;
    std::map<std::string, void*> procs;
};

// Checks if the DLL has been changed, and if so, reloads the functions from it.
void Poll(ReloadableCpp& rcpp){
    //printf("%s",rcpp.DLLPath.c_str());
    // Try opening the DLL's file
    HANDLE hDLLFile = CreateFileW(rcpp.DLLPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDLLFile == INVALID_HANDLE_VALUE)
    {
        return;
    }
    // Check if the DLL has changed since we last loaded it.
    uint64_t lastWriteTime;
    if (GetFileTime(hDLLFile, NULL, NULL, (FILETIME*)&lastWriteTime) &&
        rcpp.lastWrite < lastWriteTime){

        // Remove all references to the old module
        if (rcpp.module){
            FreeLibrary((HMODULE)rcpp.module);
            rcpp.module = NULL;
            rcpp.procs.clear();
        }

        // Copy the DLL so we don't hold a lock on the original file.
        std::wstring tmpname = rcpp.DLLPath + L".rcpp.dll";
        if (CopyFileW(rcpp.DLLPath.c_str(), tmpname.c_str(), FALSE))
        {
            // Load the copied DLL and get the functions from it.
            rcpp.module = LoadLibraryW(tmpname.c_str());
            if (rcpp.module)
            {
                for (const auto& p : rcpp.procsToLoad)
                {
                    rcpp.procs[p] = (void*) GetProcAddress((HMODULE)rcpp.module, p.c_str());
                }
                rcpp.lastWrite = lastWriteTime;
            }
        }
    }
    // Boy Scout Rule!
    CloseHandle(hDLLFile);
}

int main(int argc, char** argv){
    // Get the name of the directory the .exe is in.
    // We are assuming that the .dll to live-reload is in the same directory.
    std::wstring directory(argv[0], argv[0] + strlen(argv[0]));
    directory.erase(directory.find_last_of(L'\\') + 1);

    // Specify the path to the DLL and the functions we want to load from it.
    ReloadableCpp rcpp;
    rcpp.DLLPath = directory + L"reload_code.dll";
    rcpp.procsToLoad = { "reload_code" };

    while (true)
    {
        // Poll the DLL and update any live-reloaded function from it.
        Poll(rcpp);

        // Get the pointer to the live-reloaded function,
        // and cast it to a function pointer type with the right signature.
        auto reload_code_ptr = (decltype(reload_code)*) rcpp.procs["reload_code"];

        // If loading the function pointer worked properly, we can call it.
        if (reload_code_ptr)
        {
            reload_code_ptr();
        }
    }
}