#ifndef RELOAD_CODE
#define RELOAD_CODE 

#ifdef LIVE_RELOADED_CODE_EXPORTS
#define LIVE_RELOADED_CODE_API __declspec(dllexport)
#else
#define LIVE_RELOADED_CODE_API __declspec(dllimport)
#endif

#define CALL __cdecl
 
extern "C" LIVE_RELOADED_CODE_API void CALL reload_code();

#endif