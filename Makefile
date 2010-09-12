Ma.PHONY:all install clean
all:
	$(MAKE) -C ythtlib install
	$(MAKE) -C libythtbbs install
	$(MAKE) -C src depend
	$(MAKE) -C src
clean:
	$(MAKE) -C src clean
