#ifndef __LEARNING_GEM5_SIMPLE_OBJECT_HH__
#define __LEARNING_GEM5_SIMPLE_OBJECT_HH__

#include "params/SimpleObject.hh"
#include "sim/sim_object.hh"

namespace gem5
{

class SimpleObject : public SimObject
{
  public:
    SimpleObject(const SimpleObjectParams &p);
};

} // namespace gem5

#endif // __LEARNING_GEM5_SIMPLE_OBJECT_HH__