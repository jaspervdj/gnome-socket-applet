APPLET=gnome-socket-applet
FLAGS=$(shell pkg-config --cflags --libs libpanelapplet-2.0 gthread-2.0)

$(APPLET): $(APPLET).c
	gcc $(FLAGS) -o $(APPLET) $(APPLET).c

install: $(APPLET)
	cp $(APPLET).server /usr/lib/bonobo/servers/
	cp $(APPLET)        /usr/lib/gnome-panel/
	cp $(APPLET).png    /usr/share/pixmaps/

uninstall:
	rm /usr/lib/gnome-panel/$(APPLET)
	rm /usr/share/pixmaps/$(APPLET).png
