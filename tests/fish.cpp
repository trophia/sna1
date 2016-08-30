#include <boost/test/unit_test.hpp>

#include "../parameters.hpp"
#include "../fishes.hpp"


BOOST_AUTO_TEST_SUITE(fish)

/**
 * @brief      Test the construction of fish
 */
BOOST_AUTO_TEST_CASE(birth){
	Fish fish;
	fish.born(HG);

	BOOST_CHECK(fish.alive());

	BOOST_CHECK_EQUAL(fish.age(), 0);
	BOOST_CHECK_EQUAL(fish.age_bin(), 0);

	BOOST_CHECK_EQUAL(fish.length, 0);
	BOOST_CHECK_EQUAL(fish.length_bin(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
