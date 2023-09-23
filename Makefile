RACK_DIR ?= ../..

FLAGS +=

SOURCES += src/colors.cpp
SOURCES += src/components.cpp
SOURCES += src/em_midi.cpp
SOURCES += src/em_pedal.cpp
SOURCES += src/em_types.cpp
SOURCES += src/misc.cpp
SOURCES += src/HcOne.cpp
SOURCES += src/open_file.cpp
SOURCES += src/plugin.cpp
SOURCES += src/preset_meta.cpp
SOURCES += src/preset_widget.cpp
SOURCES += src/presets.cpp
SOURCES += src/port.cpp
SOURCES += src/small_push.cpp
SOURCES += src/text.cpp

SOURCES += src/HC-1/HC-1.cpp
SOURCES += src/HC-1/HC-1-draw.cpp
SOURCES += src/HC-1/HC-1-menu.cpp
SOURCES += src/HC-1/HC-1-midi.cpp
SOURCES += src/HC-1/HC-1-presets.cpp
SOURCES += src/HC-1/HC-1-process.cpp
SOURCES += src/HC-1/HC-1-ui.cpp

SOURCES += src/HC-2/HC-2.cpp
SOURCES += src/HC-2/HC-2-ui.cpp

SOURCES += src/HC-3/HC-3.cpp
SOURCES += src/HC-3/HC-3-ui.cpp

DISTRIBUTABLES += res
# DISTRIBUTABLES += presets
# DISTRIBUTABLES += selections

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk