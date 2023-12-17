#include "em_priority.hpp"

namespace eagan_matrix {

const char * NotePriorityName(NotePriorityType pri)
{
    switch (pri)
    {
    case NotePriorityType::LRU: return "Oldest";
    case NotePriorityType::LRR: return "Same note";
    case NotePriorityType::LCN: return "Lowest";
    case NotePriorityType::HI1: return "Highest";
    case NotePriorityType::HI2: return "Highest 2";
    case NotePriorityType::HI3: return "Highest 3";
    case NotePriorityType::HI4: return "Highest 4";
    default: return "(unknown)";
    }
}


}