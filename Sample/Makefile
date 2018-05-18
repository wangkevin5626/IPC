all:
	@for i in `ls Makefile.*` ; do $(MAKE) -s -f $$i CPU=local || exit 1; done

local: clean
	@for i in `ls Makefile.*` ; do $(MAKE) -s -f $$i CPU=local || exit 1; done

target: clean
	@for i in `ls Makefile.*` ; do $(MAKE) -s -f $$i CPU=target || exit 1; done

clean:
	@echo clean
	@for i in `ls Makefile.*` ; do $(MAKE) -s -f $$i clean || exit 1; done

