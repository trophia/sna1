#pragma once

#include <ctime>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/lognormal_distribution.hpp>
#include <boost/random/exponential_distribution.hpp>

/**
 * The mt11213b generator is fast and has a reasonable cycle length
 * See http://www.boost.org/doc/libs/1_60_0/doc/html/boost_random/reference.html#boost_random.reference.generators
 */
struct random_generator : boost::mt11213b {
    random_generator(void){
        seed(static_cast<unsigned int>(std::time(0)));
    }
} random_generator;


template<
	class Type
> struct Distribution {
	Type distribution;
	boost::variate_generator<decltype(random_generator),Type> variate_generator;

	template<class... Args>
	Distribution(Args... args):
		variate_generator(random_generator,Type(args...)) {}

    double random(void) {
    	return variate_generator();
    }
};


typedef Distribution< boost::uniform_01<> > Uniform01;
typedef Distribution< boost::exponential_distribution<> > Exponential;
typedef Distribution< boost::lognormal_distribution<> > Lognormal;


Uniform01 chance_distr;
double chance(void){
	return chance_distr.random();
}
