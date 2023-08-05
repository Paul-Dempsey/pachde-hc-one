RACK_DIR ?= ../..

FLAGS +=

SOURCES += src/colors.cpp
SOURCES += src/components.cpp
SOURCES += src/em_midi.cpp
SOURCES += src/plugin.cpp
SOURCES += src/port.cpp
SOURCES += src/small_push.cpp
SOURCES += src/text.cpp
SOURCES += src/HC-1/preset_widget.cpp
SOURCES += src/HC-1/HC-1.cpp
SOURCES += src/HC-1/HC-1-ui.cpp

DISTRIBUTABLES += res
# DISTRIBUTABLES += presets
# DISTRIBUTABLES += selections

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk