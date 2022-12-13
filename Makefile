
PROJ = stm32-gpio

PYTHON := python3

export ROOT := $(CURDIR)

.PHONY: FORCE

.DEFAULT_GOAL := all

prod-%: FORCE
	$(MAKE) target mcu=$* target=stm32-gpio level=prod

debug-%: FORCE
	$(MAKE) target mcu=$* target=stm32-gpio level=debug

all-%: FORCE prod-% debug-% ;

all: FORCE all-stm32f1 ;

clean: FORCE
	rm -rf out

out: FORCE
	+mkdir -p out/$(mcu)/$(level)/$(target)

target: FORCE out
	$(MAKE) -C out/$(mcu)/$(level)/$(target) -f $(ROOT)/Rules.mk target.bin target.hex $(mcu)=y $(level)=y $(target)=y

dist: level := prod
dist: t := $(ROOT)/out/$(PROJ)
dist: FORCE all
	rm -rf out/$(PROJ)-*
	mkdir -p $(t)/hex
	cd out/stm32f1/$(level)/stm32-gpio; \
	  cp -a target.hex $(t)/hex/$(PROJ)-f1.hex;
	cp -a COPYING $(t)/
	cd out && zip -r $(PROJ).zip $(PROJ)
