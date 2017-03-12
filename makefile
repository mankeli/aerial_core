HASM=~/src/hashembler/hasm.sh

.PHONY: all
all: extzoo.d64
	@true

%.prg: %.hasm.cpp memconfig.h
	$(HASM) $<

valosrc=valo.hasm.cpp lode/lodepng.cpp
valo.prg valosprites.prg pallo.hires kallo.hires: $(valosrc) hires.h memconfig.h
	$(HASM) $(valosrc)

lasersrc=laser.hasm.cpp lode/lodepng.cpp
laser.prg: $(lasersrc) hires.h memconfig.h
	$(HASM) $(lasersrc)

hiressrc=tohires.hasm.cpp lode/lodepng.cpp
tohires: $(hiressrc) hires.h
	clang++ $(hiressrc) -o $@

laser.hires: assets/laser_intro_lopullinen2.png tohires
	./tohires $<  $@

laser2.hires: assets/uusi_tekstuuri_2.png tohires
	./tohires $<  $@

kiss_the_bride.hires: assets/kiss_the_bride.png tohires
	./tohires $<  $@

logo1.petscii: to_petscii_logo1.hasm.cpp memconfig.h
	$(HASM) $<

logo2.petscii: to_petscii_logo2.hasm.cpp memconfig.h
	$(HASM) $<

pumpumsrc=pumpum.hasm.cpp lode/lodepng.cpp
pumpum.prg pumpum.hires: $(pumpumsrc) hires.h memconfig.h
	$(HASM) $(pumpumsrc)

spriteoverlaysrc=spriteoverlay.hasm.cpp lode/lodepng.cpp
spriteoverlay.prg mohkosprites.bin mohko.hires: $(spriteoverlaysrc) hires.h memconfig.h
	$(HASM) $(spriteoverlaysrc)

BANKS = \
	irqfukker.prg \
	msdos.prg msdos2.prg \
	pallo.hires valo.prg kallo.hires valosprites.prg \
	pumpum.prg pumpum.hires \
	laser.prg laser.hires laser2.hires \
	spriteoverlay.prg mohkosprites.bin mohko.hires \
	kiss_the_bride.hires \
	rotozoom.prg twist.prg \
	logo1.petscii logo2.petscii \
	assets/m5betaplanet.prg

spin=~/src/spindle-2.3/spindle/spin

DIRART=assets/dirart.txt
extzoo.d64: $(BANKS) script.spindle $(DIRART)
	$(spin) -vv -o extzoo.d64 -t "EXTEND ZOO DEMO" -a $(DIRART) -d 6 --my-magic 0xDEAD32 --next-magic 0xDEAD32 script.spindle


#x64=/Applications/vice/x64sc.app/Contents/MacOS/x64sc
x64=/Applications/vice/x64.app/Contents/MacOS/x64

.PHONY: run
run: extzoo.d64
	$(x64) -truedrive $<

.PHONY: runsilent
runsilent: extzoo.d64
	$(x64) -truedrive $< +sound

.PHONY: clean
clean:
	rm *.prg *.hires extzoo.d64
