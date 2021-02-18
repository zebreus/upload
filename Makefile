TARGET := upload

BUILD_DIR := build
SRC_DIR := src
LIB_DIR := libs

CXX := g++
MKDIR := mkdir -p

SRCS := $(shell find $(SRC_DIR) -name *.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INCLUDE_DIRS := $(shell find $(SRC_DIR) -type d)
INCLUDE_DIRS += $(LIB_DIR)/cxxopts/include
INCLUDE_FLAGS := $(addprefix -I,$(INCLUDE_DIRS))

CXX_FLAGS := $(INCLUDE_FLAGS) -MMD -MP -std=c++2a

LD_FLAGS := 

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LD_FLAGS)

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

.PHONY: clean

-include $(DEPS)

clean:
	$(RM) -r $(BUILD_DIR)
