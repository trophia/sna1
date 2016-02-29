#pragma once

#include <fsl/math/probability/uniform.hpp>
using Fsl::Math::Probability::Uniform;

/**
 * Represents stochasticity
 *
 * Used in numerous places for adding probabalistic 
 * behaviours to dynamics
 */
static Uniform chance = {0, 1};

/**
 * The "environment"
 *
 * Currently just a placeholder
 */
class Environ {
 public:
    void initialise(void) {
    }

    void finalise(void) {
    }
};  // class Environ
