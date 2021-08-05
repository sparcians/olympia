// <RISCVInst.cpp> -*- C++ -*-

#include "RISCVInst.hpp"

namespace core_example
{
    // This pipeline supports around 135 outstanding example instructions.
    sparta::SpartaSharedPointer<RISCVInst>::SpartaSharedPointerAllocator example_inst_allocator(300, 250);
}