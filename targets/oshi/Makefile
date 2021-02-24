BASE_DIR := ../..
BUILD_DIR := $(BASE_DIR)/build
LIB_DIR := $(BASE_DIR)/libs
SRC_DIR := .

CXX := g++
MKDIR := mkdir -p

TARGET := oshi

INCLUDE_DIRS += $(BASE_DIR)/include
INCLUDE_DIRS += $(LIB_DIR)/cpp-httplib
INCLUDE_DIRS += $(SRC_DIR)
INCLUDE_FLAGS := $(addprefix -I,$(INCLUDE_DIRS))
CXX_FLAGS := $(INCLUDE_FLAGS) -MMD -MP -std=c++2a -O3 -pthread -fPIC -fno-use-cxa-atexit -DCPPHTTPLIB_OPENSSL_SUPPORT
LD_FLAGS := -std=c++2a -O3 -pthread

TARGET_STATIC := lib$(TARGET).a
TARGET_DYNAMIC := lib$(TARGET).so
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

all: $(BUILD_DIR)/$(TARGET_DYNAMIC) $(BUILD_DIR)/$(TARGET_STATIC)

$(BUILD_DIR)/$(TARGET_STATIC): $(OBJS)
	$(MKDIR) $(dir $@)
	ar rvs $@ $<

$(BUILD_DIR)/$(TARGET_DYNAMIC): $(OBJS)
	$(MKDIR) $(dir $@)
	g++ --shared $< -o $@

$(OBJS): $(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

.PHONY: clean

-include $(DEPS)

clean:
	$(RM) -r $(OBJS) $(BUILD_DIR)/$(TARGET_DYNAMIC)  $(BUILD_DIR)/$(TARGET_STATIC)
