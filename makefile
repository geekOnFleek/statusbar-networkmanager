main: client-bins
	gcc statusbar-networkmanager-daemon.c -o bin/statusbar-networkmanager -pthread
	systemctl stop network-management.service
	cp bin/statusbar-networkmanager ~/.uscripts/wifiservice/network-handler-daemon
	systemctl start network-management.service

client-bins:
	mkdir -p bin
	gcc connection-info.c -o bin/connection-info
	gcc connection-unit.c -o bin/connection-unit
	gcc connection-next.c -o bin/connection-next
	gcc connection-prev.c -o bin/connection-prev
	gcc connection-disc.c -o bin/connection-disc
