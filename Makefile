dll:
	g++ -c -o live_reload_code/reload_code.o live_reload_code/reload_code.cpp -D LIVE_RELOADED_CODE_EXPORTS
	g++ -o reload_code.dll live_reload_code/reload_code.o -s -shared -Wl,--subsystem,windows
all:
	g++ -o main main.cpp -L./ -lreload_code