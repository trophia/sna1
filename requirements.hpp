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
#include <string>

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
