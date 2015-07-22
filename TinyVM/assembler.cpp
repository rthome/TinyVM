#include "assembler.hpp"

#include "assembler_details.hpp"

#include "vm.hpp"
#include "instruction.hpp"
#include "instruction_support.hpp"

namespace
{


    struct ASContext
    {
        FileMapping *f;
    };
}
