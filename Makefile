RACK_DIR ?= ../..

ifdef BUILD_DEBUG
FLAGS += -O0
else
FLAGS += -O3
FLAGS += -DNDEBUG
endif

SOURCES += src/colors.cpp
SOURCES += src/em_device.cpp
SOURCES += src/em_midi.cpp
SOURCES += src/em_types/em_pedal.cpp
SOURCES += src/em_types/em_priority.cpp
SOURCES += src/em_types/em_rounding.cpp
SOURCES += src/em_types/em_tuning.cpp
SOURCES += src/he_group.cpp
SOURCES += src/misc.cpp
SOURCES += src/module_broker.cpp
SOURCES += src/open_file.cpp
SOURCES += src/plugin.cpp
SOURCES += src/preset_meta.cpp
SOURCES += src/presets.cpp
SOURCES += src/text.cpp

SOURCES += src/widgets/components.cpp
SOURCES += src/widgets/port.cpp
SOURCES += src/widgets/preset_widget.cpp
SOURCES += src/widgets/small_push.cpp
SOURCES += src/widgets/vert_slider.cpp

SOURCES += src/HC-1/midi_input_worker.cpp
SOURCES += src/HC-1/init_phase.cpp

SOURCES += src/HC-1/HC-1.cpp
SOURCES += src/HC-1/HC-1-draw.cpp
SOURCES += src/HC-1/HC-1-menu.cpp
SOURCES += src/HC-1/HC-1-midi.cpp
SOURCES += src/HC-1/HC-1-midi-out.cpp
SOURCES += src/HC-1/HC-1-presets.cpp
SOURCES += src/HC-1/HC-1-process.cpp
SOURCES += src/HC-1/HC-1-ui.cpp

SOURCES += src/HC-2/HC-2.cpp
SOURCES += src/HC-2/HC-2-ui.cpp
SOURCES += src/HC-2/cc_map_widget.cpp

SOURCES += src/Favorites/Favorites.cpp
SOURCES += src/Favorites/Favorites-ui.cpp

#SOURCES += src/HC-4/HC-4.cpp
#SOURCES += src/HC-4/HC-4-ui.cpp

SOURCES += src/Pedals/Pedals.cpp
SOURCES += src/Pedals/Pedals-ui.cpp

SOURCES += src/Round/Round.cpp
SOURCES += src/Round/Round-ui.cpp

SOURCES += src/Compress/Compress.cpp
SOURCES += src/Compress/Compress-ui.cpp

SOURCES += src/PolyMidi/PolyMidi.cpp
SOURCES += src/PolyMidi/PolyMidi-ui.cpp

DISTRIBUTABLES += res
# DISTRIBUTABLES += presets
# DISTRIBUTABLES += selections

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk