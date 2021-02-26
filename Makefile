BUILD_DIR := build
LIB_DIR := libs
SRC_DIR := src

# Upload will load plugins from here
INSTALL_PLUGIN_DIR := $(abspath $(BUILD_DIR))

GCC_WARNING_FLAGS := -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wswitch-default -Wundef -Werror -Wno-unused -Wswitch-enum -Wzero-as-null-pointer-constant -Wuseless-cast

CLANG_WARNING_FLAGS := -Weverything

#Currently disabled: -Wunsafe-loop-optimizations -Wshadow 
# Additional -pedantic  -pedantic-errors -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wdisabled-optimization -Werror -Wfloat-equal -Wformat=2 -Wformat-nonliteral -Wformat-security  -Wformat-y2k -Wimplicit  -Wimport  -Winit-self  -Winline -Winvalid-pch   -Wlong-long -Wmissing-field-initializers -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wpacked  -Wpadded -Wpointer-arith -Wredundant-decls -Wshadow -Wstack-protector -Wstrict-aliasing=2 -Wswitch-default -Wswitch-enum -Wunreachable-code -Wunused -Wunused-parameter -Wvariadic-macros -Wwrite-strings

CXX=g++
MKDIR=mkdir -p
MAKEOVERRIDES += CXX:=$(CXX)

export COMMON_CXX_FLAGS := -std=c++2a -O3
#$(GCC_WARNING_FLAGS)
export COMMON_LD_FLAGS := -O3 -Wl,-as-needed -Wl,-z,relro,-z,now 

BACKENDS_DIR = backends
BACKENDS_BUILD_DIR = ../../$(BUILD_DIR)
BACKENDS += nullpointer transfersh oshi
STATIC_BACKEND_LIBS = $(BACKENDS:%=$(BUILD_DIR)/lib%.a)
SHARED_BACKEND_LIBS = $(BACKENDS:%=$(BUILD_DIR)/lib%.so)

# TODO improve this
# If set to yeah, the targets are build as shared libraries
DYNAMIC := yeah

UPLOAD := upload
INCLUDE_FLAGS += -Iinclude
INCLUDE_FLAGS += -isystem $(LIB_DIR)/cxxopts/include
INCLUDE_FLAGS += -isystem $(LIB_DIR)/miniz-cpp
INCLUDE_FLAGS += -I$(SRC_DIR)
CXX_FLAGS := $(COMMON_CXX_FLAGS) $(INCLUDE_FLAGS) -MMD -MP -pthread -DUPLOAD_PLUGIN_DIR=$(INSTALL_PLUGIN_DIR)
LD_FLAGS := $(COMMON_LD_FLAGS) -pthread
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

DEPS := $(OBJS:.o=.d)

all: $(BUILD_DIR)/$(UPLOAD)


ifeq ($(DYNAMIC),yeah)

# dynamic loading
LD_FLAGS += -ldl -rdynamic

$(BUILD_DIR)/$(UPLOAD): $(OBJS) $(SHARED_BACKEND_LIBS)
	$(CXX) $(OBJS) -o $@ $(LD_FLAGS)

else

# static linking
CXX_FLAGS += -isystem $(LIB_DIR)/cpp-httplib
LD_FLAGS += $(STATIC_BACKEND_LIBS) -pthread -lcrypto -lssl
CXX_FLAGS += $(BACKENDS:%=-I$(BACKENDS_DIR)/%)
CXX_FLAGS += -DSTATIC_LOADER -DCPPHTTPLIB_OPENSSL_SUPPORT

$(BUILD_DIR)/$(UPLOAD): $(OBJS) $(STATIC_BACKEND_LIBS)
	$(CXX) $(OBJS) -o $@ $(LD_FLAGS)

endif

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(STATIC_BACKEND_LIBS): $(BUILD_DIR)/lib%.a :
	$(MAKE) -C $(BACKENDS_DIR)/$* $(BACKENDS_BUILD_DIR)/lib$*.a

$(SHARED_BACKEND_LIBS): $(BUILD_DIR)/lib%.so :
	$(MAKE) -C $(BACKENDS_DIR)/$* $(BACKENDS_BUILD_DIR)/lib$*.so

.PHONY: clean $(SHARED_BACKEND_LIBS) $(STATIC_BACKEND_LIBS)

-include $(DEPS)

clean:
	$(RM) -r $(BUILD_DIR)

## Section for formatting

ALL_SOURCE_FOLDERS := $(SRC_DIR) include $(wildcard $(BACKENDS_DIR)/*)
ALL_CXX_FILES := $(wildcard $(ALL_SOURCE_FOLDERS:%=%/*.cpp)) $(wildcard $(ALL_SOURCE_FOLDERS:%=%/*.hpp))

format: 
	clang-format -style=file -i $(ALL_CXX_FILES)
	
## Section for documentation

.PHONY: manual
manual: upload.1

upload.1: upload.ronn
	ronn upload.ronn --roff
