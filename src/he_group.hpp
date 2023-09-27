// parse Haken Editor group file
#pragma once
#include <rack.hpp>
using namespace ::rack;

namespace he_group {

/* -- sample group file --
16,"SineSpray Rain via Surface.mid"
 15,"Zipper.mid"
 14,"Mojo of FDN.mid"
 13,"Marlin Perkins 1.mid"
 11,"Bowed Mood.mid"
 10,"Living Pad.mid"
 9,"Shimmer.mid"
 8,"Cumulus.mid"
 7,"Choir.mid"
 6,"Bass Monster.mid"
 5,"Snap Bass.mid"
 4,"Jaymar Toy Piano.mid"
 3,"Woodwind.mid"
 2,"Tin Whistle.mid"
 1,"Vln Vla Cel Bass 2.mid"
# put comments at end. Anything invalid stops import, so 
anything after this won't get imported.
12,"Ishango Bone.mid"
*/

struct HEPresetItem
{
    int index;
    std::string name;
};

std::vector<HEPresetItem> ReadGroupFile(const std::string& path);

}