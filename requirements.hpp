/**
 * A header file to include all external dependencies
 */

#pragma once

// C/C++ standard library
#include <fstream>
#include <vector>
#include <thread>
#include <map>
#include <iostream>

// Boost library
#include <boost/filesystem.hpp>

// Stencila structure, array and query classes
#include <stencila/dimension.hpp>
using Stencila::Dimension;
#include <stencila/array-static.hpp>
using Stencila::Array;
#include <stencila/query.hpp>
using namespace Stencila::Queries;
#include <stencila/structure.hpp>
using Stencila::Structure;

// FSL probability distributions etc
#include <fsl/math/probability/uniform.hpp>
using Fsl::Math::Probability::Uniform;
#include <fsl/math/probability/truncated.hpp>
using Fsl::Math::Probability::Truncated;
#include <fsl/math/probability/normal.hpp>
using Fsl::Math::Probability::Normal;
#include <fsl/math/probability/exponential.hpp>
using Fsl::Math::Probability::Exponential;
#include <fsl/math/probability/discrete.hpp>
using Fsl::Math::Probability::Discrete;
#include <fsl/population/growth/von-bert.hpp>
using Fsl::Population::Growth::VonBert;
#include <fsl/math/functions/double-normal-plateau.hpp>
using Fsl::Math::Functions::DoubleNormalPlateau;
