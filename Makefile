CC := C:/Users/joshr/Documents/Gaming/MKWii/Pulsar/CodeWarrior/Command_Line_Tools/mwcceppc.exe
ENGINE := ./KamekInclude
GAMESOURCE := ./GameSource
PULSAR := ./PulsarEngine
RIIVO := "G:/Emulation/GCN and Wii/DolphinTesting/User/Load/Riivolution/GrillinsWTPTest"
CFLAGS := -I- -i $(ENGINE) -i $(GAMESOURCE) -i $(PULSAR) -opt all -inline auto -enum int -proc gekko -fp hard -sdata 0 -sdata2 0 -maxerrors 1 -func_align 4
KAMEK := ./KamekLinker/Kamek.exe
EXTERNALS := -externals=$(GAMESOURCE)/symbols.txt -externals=$(GAMESOURCE)/AntiCheat.txt -versions=$(GAMESOURCE)/versions.txt

SRCS := $(shell find $(PULSAR) -type f -name "*.cpp")
OBJS := $(patsubst $(PULSAR)/%.cpp, build/%.o, $(SRCS))

all: build force_link

.PHONY: all force_link

test:
	@echo "$(SRCS)"

build:
	@mkdir -p build

build/kamek.o: $(ENGINE)/kamek.cpp | build
	@$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: $(PULSAR)/%.cpp | build
	@echo Compiling $<...
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

force_link: build/kamek.o $(OBJS)
	@echo Linking...
	@$(KAMEK) $^ -dynamic $(EXTERNALS) -output-combined=build/Code.pul

install: force_link
	@echo Copying binaries to $(RIIVO)/Binaries...
	@mkdir -p $(RIIVO)/Binaries
	@cp build/Code.pul $(RIIVO)/Binaries

clean:
	@echo Cleaning...
	@rm -rf build
