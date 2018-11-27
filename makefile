main: client-bins
	gcc statusbar-networkmanager-daemon.c -o bin/statusbar-networkmanager -pthread

client-bins:
	mkdir -p bin
	gcc connection-info.c -o bin/connection-info
	gcc connection-unit.c -o bin/connection-unit
	gcc connection-next.c -o bin/connection-next
	gcc connection-prev.c -o bin/connection-prev
	gcc connection-disc.c -o bin/connection-disc
