#pragma once

#include <fsl/math/probability/uniform.hpp>
using Fsl::Math::Probability::Uniform;

static Uniform chance = {0,1};

/**
 * The "environment"
 *
 * Currently just a placeholder
 */
class Environ {
public:
    void start(void){
    }
    void update(void){
    }
}; //end class Environ
