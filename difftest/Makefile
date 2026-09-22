CC  = clang
CXX = clang++

GUEST_ISA = riscv64
BUILD_DIR = $(abspath ./build)
TARGET    = $(BUILD_DIR)/$(GUEST_ISA)-spike-so

XIANGSHAN_BUILD_DIR ?= $(abspath ./build-xiangshan-spike)
XIANGSHAN_SPIKE_TARGET = $(XIANGSHAN_BUILD_DIR)/spike
XIANGSHAN_SPIKE_MAKEFILE = $(XIANGSHAN_BUILD_DIR)/Makefile
XIANGSHAN_CONFIG_STAMP = $(XIANGSHAN_BUILD_DIR)/.config-signature

# Dependencies for building Spike
SPIKE_PATH     = $(abspath ..)
SPIKE_TARGET   = $(BUILD_DIR)/spike
SPIKE_MAKEFILE = $(BUILD_DIR)/Makefile

XIANGSHAN_CONFIG_INPUTS = $(SPIKE_PATH)/configure $(SPIKE_PATH)/configure.ac \
    $(SPIKE_PATH)/aclocal.m4 $(SPIKE_PATH)/config.h.in $(SPIKE_PATH)/Makefile.in \
    $(wildcard $(SPIKE_PATH)/*/*.mk.in) $(abspath ./difftest-def.h)

SPIKE_CFLAGS    = -O3 -DDIFFTEST -include $(abspath .)/difftest-def.h
SPIKE_CXXFLAGS  = -O3 -DDIFFTEST -include $(abspath .)/difftest-def.h -Wno-c99-designator
SPIKE_LDFLAGS   =

ifneq ($(CPU),)
SPIKE_CFLAGS   += -DCPU_$(CPU)
SPIKE_CXXFLAGS += -DCPU_$(CPU)
endif

ifeq ($(DEBUG),)
SPIKE_CFLAGS   += -g0
SPIKE_CXXFLAGS += -g0
endif

ifeq ($(SANCOV),1)
SPIKE_CFLAGS   += -fsanitize-coverage=trace-pc-guard -fsanitize-coverage=pc-table -g
SPIKE_CXXFLAGS += -fsanitize-coverage=trace-pc-guard -fsanitize-coverage=pc-table -g
SPIKE_LDFLAGS  += -fsanitize-coverage=trace-pc-guard -fsanitize-coverage=pc-table -g
endif

CONFIGURE_FLAGS = -q CC=$(CC) CXX=$(CXX)         \
    --with-boost=no	--with-boost-asio=no --with-boost-regex=no \
    CFLAGS="$(SPIKE_CFLAGS)" CXXFLAGS="$(SPIKE_CXXFLAGS)"      \
    LDFLAGS="$(SPIKE_LDFLAGS)"

XIANGSHAN_SPIKE_CFLAGS ?= -O3
XIANGSHAN_SPIKE_CXXFLAGS ?= -O3 -Wno-c99-designator
XIANGSHAN_SPIKE_LDFLAGS ?=
override XIANGSHAN_SPIKE_CFLAGS += -DCPU_XIANGSHAN -include $(abspath .)/difftest-def.h
override XIANGSHAN_SPIKE_CXXFLAGS += -DCPU_XIANGSHAN -include $(abspath .)/difftest-def.h

ifeq ($(DEBUG),)
XIANGSHAN_SPIKE_CFLAGS += -g0
XIANGSHAN_SPIKE_CXXFLAGS += -g0
endif

XIANGSHAN_CONFIGURE_FLAGS ?= -q --with-boost=no --with-boost-asio=no --with-boost-regex=no

SPIKE_INCS = fesvr riscv disasm customext fdt softfloat spike_main spike_dasm
INC_PATH   = -I$(SPIKE_PATH) -I$(BUILD_DIR) $(addprefix -I$(SPIKE_PATH)/, $(SPIKE_INCS))

SPIKE_SRCS = $(shell find $(SPIKE_PATH) -maxdepth 2 -name '*.cc') \
             $(shell find $(SPIKE_PATH) -maxdepth 3 -name '*.h')
SPIKE_LIBS = libriscv.a libdisasm.a libsoftfloat.a libfesvr.a libfdt.a
INC_LIBS   = $(addprefix $(BUILD_DIR)/, $(SPIKE_LIBS))

# We need some utilities from spike
CXX_UTILS = $(BUILD_DIR)/difftest_utils.cc

$(CXX_UTILS): $(SPIKE_PATH)/spike_main/spike.cc
	cp $< $(CXX_UTILS)
	@sed -i 's/static //g' $@
	@sed -i 's/int main/static int spike_main/g' $@

# Dependencies for building the difftest dynamic library
DIFFTEST_SOURCES = $(shell find . -maxdepth 1 -name '*.cc') $(CXX_UTILS)
DIFFTEST_HEADERS = $(shell find . -maxdepth 1 -name '*.h')
CXXFLAGS = $(SPIKE_CXXFLAGS) --std=c++17 -shared -fPIC
ifneq ($(SANCOV),1)
CXXFLAGS += -flto
endif

ifneq ($(DEBUG),)
CONFIGURE_FLAGS += --enable-commitlog
CXXFLAGS += -DDIFFTEST_LOG_FILE=\"$(NOOP_HOME)/build/spike.log\"
endif

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SPIKE_MAKEFILE): $(SPIKE_PATH)/configure | $(BUILD_DIR)
	cd $(@D) && $(abspath $(SPIKE_PATH))/configure $(CONFIGURE_FLAGS)

$(SPIKE_TARGET): $(SPIKE_MAKEFILE) $(SPIKE_SRCS)
	$(MAKE) -s -C $(BUILD_DIR)

$(TARGET): $(SPIKE_TARGET) $(DIFFTEST_SOURCES) $(DIFFTEST_HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INC_PATH) $(DIFFTEST_SOURCES) $(INC_LIBS) -o $@

xiangshan-config:
	@set -eu; \
	tmp=$$(mktemp); \
	trap 'rm -f "$$tmp"' EXIT; \
	{ \
	  printf '%s\n' 'CC=$(CC)' 'CXX=$(CXX)' \
	    'CONFIGURE_FLAGS=$(XIANGSHAN_CONFIGURE_FLAGS)' \
	    'CFLAGS=$(XIANGSHAN_SPIKE_CFLAGS)' \
	    'CXXFLAGS=$(XIANGSHAN_SPIKE_CXXFLAGS)' \
	    'LDFLAGS=$(XIANGSHAN_SPIKE_LDFLAGS)'; \
	  sha256sum $(XIANGSHAN_CONFIG_INPUTS); \
	} > "$$tmp"; \
	if ! test -r "$(XIANGSHAN_CONFIG_STAMP)" || \
	   ! test -r "$(XIANGSHAN_SPIKE_MAKEFILE)" || \
	   ! cmp -s "$$tmp" "$(XIANGSHAN_CONFIG_STAMP)"; then \
	  rm -rf "$(XIANGSHAN_BUILD_DIR)"; \
	  mkdir -p "$(XIANGSHAN_BUILD_DIR)"; \
	  mv "$$tmp" "$(XIANGSHAN_CONFIG_STAMP)"; \
	  cd "$(XIANGSHAN_BUILD_DIR)" && $(abspath $(SPIKE_PATH))/configure \
	    $(XIANGSHAN_CONFIGURE_FLAGS) CC=$(CC) CXX=$(CXX) \
	    CFLAGS="$(XIANGSHAN_SPIKE_CFLAGS)" \
	    CXXFLAGS="$(XIANGSHAN_SPIKE_CXXFLAGS)" \
	    LDFLAGS="$(XIANGSHAN_SPIKE_LDFLAGS)"; \
	fi

$(XIANGSHAN_SPIKE_TARGET): xiangshan-config $(SPIKE_SRCS)
	$(MAKE) -s -C $(XIANGSHAN_BUILD_DIR) spike

xiangshan-spike: $(XIANGSHAN_SPIKE_TARGET)

clean:
	rm -rf $(BUILD_DIR)

clean-xiangshan-spike:
	rm -rf $(XIANGSHAN_BUILD_DIR)

.DEFAULT_GOAL = $(TARGET)

.PHONY: xiangshan-config xiangshan-spike clean clean-xiangshan-spike
