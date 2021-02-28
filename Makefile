BUILD_DIR := build
DYNAMIC_BUILD_DIR := build-dynamic
STATIC_BUILD_DIR := build-static
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

COMMON_CXX_FLAGS = -std=c++2a -Os -DUPLOAD_PLUGIN_DIR=$(INSTALL_PLUGIN_DIR) -DCPPHTTPLIB_OPENSSL_SUPPORT
#$(GCC_WARNING_FLAGS)
release: COMMON_CXX_FLAGS += -DINTEGRATED_CERTIFICATES
export COMMON_CXX_FLAGS

export COMMON_LD_FLAGS := -Os -Wl,-as-needed -Wl,-z,relro,-z,now

BACKENDS_DIR = backends
BACKENDS_BUILD_DIR = ../../$(BUILD_DIR)
BACKENDS += nullpointer transfersh oshi
STATIC_BACKEND_LIBS = $(BACKENDS:%=$(BUILD_DIR)/lib%.a)
SHARED_BACKEND_LIBS = $(BACKENDS:%=$(BUILD_DIR)/lib%.so)

# TODO improve this
# If set to yeah, the targets are build as shared libraries
DYNAMIC := nay

UPLOAD := upload
INCLUDE_FLAGS += -Iinclude
INCLUDE_FLAGS += -isystem $(LIB_DIR)/cxxopts/include
INCLUDE_FLAGS += -isystem $(LIB_DIR)/miniz-cpp
INCLUDE_FLAGS += -I$(SRC_DIR)
STATIC_INCLUDE_FLAGS += $(INCLUDE_FLAGS)
STATIC_INCLUDE_FLAGS += -isystem $(LIB_DIR)/cpp-httplib
STATIC_INCLUDE_FLAGS += $(BACKENDS:%=-I$(BACKENDS_DIR)/%)

STATIC_CXX_FLAGS := $(COMMON_CXX_FLAGS) -MMD -MP -isystem $(LIB_DIR)/cpp-httplib $(STATIC_INCLUDE_FLAGS) -DSTATIC_LOADER
CXX_FLAGS := $(STATIC_CXX_FLAGS)
DYNAMIC_CXX_FLAGS := $(COMMON_CXX_FLAGS) -MMD -MP $(INCLUDE_FLAGS)

LD_FLAGS := $(COMMON_LD_FLAGS) -lssl -lcrypto -pthread -lpthread
STATIC_LD_FLAGS := $(COMMON_LD_FLAGS) -static -lrt -lssl -lcrypto -pthread -lpthread
DYNAMIC_LD_FLAGS := $(COMMON_LD_FLAGS) -pthread -ldl -rdynamic

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
STATIC_OBJS := $(SRCS:%=$(STATIC_BUILD_DIR)/%.o)
DYNAMIC_OBJS := $(SRCS:%=$(DYNAMIC_BUILD_DIR)/%.o)

DEPS := $(OBJS:.o=.d)

all: $(BUILD_DIR)/$(UPLOAD)
static: $(BUILD_DIR)/$(UPLOAD)
dynamic: $(BUILD_DIR)/$(UPLOAD)

$(BUILD_DIR)/$(UPLOAD): $(OBJS) $(STATIC_BACKEND_LIBS)
	$(MKDIR) $(dir $@)
	$(CXX) $(OBJS) -o $@ $(STATIC_BACKEND_LIBS) $(LD_FLAGS)

$(STATIC_BUILD_DIR)/$(UPLOAD): $(STATIC_OBJS) $(STATIC_BACKEND_LIBS)
	$(MKDIR) $(dir $@)
	$(CXX) $(STATIC_OBJS) -o $@ $(STATIC_BACKEND_LIBS) $(STATIC_LD_FLAGS)

$(DYNAMIC_BUILD_DIR)/$(UPLOAD): $(DYNAMIC_OBJS) $(SHARED_BACKEND_LIBS)
	$(MKDIR) $(dir $@)
	$(CXX) $(DYNAMIC_OBJS) -o $@ $(DYNAMIC_LD_FLAGS)

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(STATIC_BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(STATIC_CXX_FLAGS) -c $< -o $@

$(DYNAMIC_BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(DYNAMIC_CXX_FLAGS) -c $< -o $@

$(STATIC_BACKEND_LIBS): $(BUILD_DIR)/lib%.a :
	$(MAKE) -C $(BACKENDS_DIR)/$* $(BACKENDS_BUILD_DIR)/lib$*.a

$(SHARED_BACKEND_LIBS): $(BUILD_DIR)/lib%.so :
	$(MAKE) -C $(BACKENDS_DIR)/$* $(BACKENDS_BUILD_DIR)/lib$*.so

.PHONY: clean $(SHARED_BACKEND_LIBS) $(STATIC_BACKEND_LIBS)

-include $(DEPS)

clean:
	$(RM) -r $(BUILD_DIR) $(STATIC_BUILD_DIR) $(DYNAMIC_BUILD_DIR) upload generator include/cacert.hpp

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

## Section for optaining header with cert bundle data

cacert.pem:
	wget https://curl.se/ca/cacert.pem

generator: generator.cpp
	g++ generator.cpp -o generator

include/cacert.hpp: cacert.pem generator
	./generator cacert.pem cacert.hpp
	mv cacert.hpp include

## Section for generating compressed and stripped release binary

release: include/cacert.hpp upload

upload: $(STATIC_BUILD_DIR)/$(UPLOAD)
	cp $(STATIC_BUILD_DIR)/$(UPLOAD) upload
	strip upload
	upx --lzma upload
