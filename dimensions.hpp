#pragma once

#include "requirements.hpp"

/**
 * Time
 */
typedef uint Time;
Time now;

const uint times_per_year = 1;

uint year(Time t) {
    // TODO
    return t;
}

uint quarter(Time t) {
    // TODO
    return 0;
}

/**
 * Years
 */
const uint Years_min = 1900;
const uint Years_max = 2025;
STENCILA_DIM_RANGE(Years,years,year,Years_min,Years_max)

/**
 * Region
 */
enum Region {
    EN = 0,
    HG = 1,
    BP = 2
};
STENCILA_DIM(Regions,regions,region,3);
STENCILA_DIM(RegionTos,region_tos,region_to,3);  // For defining and accessing movement matrix

std::string region_code(int region){
    const char* codes[] = {"EN","HG","BP"};
    return codes[region];
}

std::string region_code(Stencila::Level<Regions>& region){
    return region_code(region.index());
}


/**
 * Sex of fish
 */
enum Sex {
    male = 0,
    female = 1
};
STENCILA_DIM(Sexes,sexes,sex,2);

/**
 * Age of fish
 */
STENCILA_DIM(Ages,ages,age,31);

int age_bin(double age){
    return std::min(age,30.0);
}

/**
 * Length of fish
 */
STENCILA_DIM(Lengths,lengths,length,100);

const double length_bin_width = 1; // cm

int length_bin(double length){
    return std::min(length, 99.99)/length_bin_width;
}

int length_mid(Stencila::Level<Lengths>& length_bin){
    return  (length_bin.index() + 0.5) * length_bin_width;
}

/**
 * Harvest methods
 */
enum Method {
    LL = 0,
    BT = 1,
    DS = 2,
    RE = 3
};
STENCILA_DIM(Methods,methods,method,4);

std::string method_code(int method){
    const char* codes[] = {"LL","BT","DS","RE"};
    return codes[method];
}

std::string method_code(Stencila::Level<Methods>& method){
    return method_code(method.index());
}
