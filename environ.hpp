#pragma once

#include <fsl/math/probability/uniform.hpp>
using Fsl::Math::Probability::Uniform;

/**
 * Represents stochasticity
 *
 * Used in numerous places for adding probabalistic 
 * behavious to dynamics
 */
static Uniform chance = {0, 1};

/**
 * The "environment"
 *
 * Currently just a placeholder
 */
class Environ {
 public:
    Environ& initialise(void) {
        return *this;
    }

    void update(void) {
    }

    Environ& finalise(void) {
        return *this;
    }
};  // class Environ
