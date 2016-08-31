#include <boost/test/unit_test.hpp>

#include "../fishes.hpp"


BOOST_AUTO_TEST_SUITE(fish)

/**
 * @brief      Test the construction of fish at birth
 */
BOOST_AUTO_TEST_CASE(birth){
	Fish fish;
	fish.born(HG);

	BOOST_CHECK(fish.alive());

	BOOST_CHECK_EQUAL(fish.region, HG);

	BOOST_CHECK_EQUAL(fish.age(), 0);
	BOOST_CHECK_EQUAL(fish.age_bin(), 0);

	BOOST_CHECK_EQUAL(fish.length, 0);
	BOOST_CHECK_EQUAL(fish.length_bin(), 0);
}

/**
 * @brief      Test the construction of fish through seeding
 */
BOOST_AUTO_TEST_CASE(seed){
	Fish fish;
	fish.seed();

	BOOST_CHECK(fish.alive());

	BOOST_CHECK(fish.age() > 0);
	BOOST_CHECK(fish.length > 0);
}

// Runs fish movement over many time steps and many 
// fish and calculates the resulting distribution of fish 
// across regions for each home region
Array<double, Regions, RegionTos> movement_run(void) {
	Fishes fishes(5000);

	int count = 0;
	for (auto& fish : fishes) {
		fish.born(Region(count++ % 3));
	}

	for (int t=0; t<100; t++) {
		for (auto& fish : fishes) {
			fish.movement();
		}
	}

	Array<double, Regions, RegionTos> dist;
	for (auto& fish : fishes) {
		dist(fish.home, fish.region)++;
	}
	dist /= fishes.size()/3;

	return dist;
}

BOOST_AUTO_TEST_CASE(movement_none){
	parameters.fishes_movement_type = 'n';
	parameters.fishes_movement = {};

	auto dist = movement_run();

	BOOST_CHECK_CLOSE(dist(EN, EN), 1.0, 1);
	BOOST_CHECK(dist(EN, HG) < 0.00001);
	BOOST_CHECK(dist(EN, BP) < 0.00001);

	BOOST_CHECK(dist(HG, EN) < 0.00001);
	BOOST_CHECK_CLOSE(dist(HG, HG), 1.0, 1);
	BOOST_CHECK(dist(HG, BP) < 0.00001);

	BOOST_CHECK(dist(BP, EN) < 0.00001);
	BOOST_CHECK(dist(BP, HG) < 0.00001);
	BOOST_CHECK_CLOSE(dist(BP, BP), 1.0, 1);
	
	parameters.initialise();
}

BOOST_AUTO_TEST_CASE(movement_markov){
	parameters.fishes_movement_type = 'm';
	parameters.fishes_movement = {
		0.8, 0.1, 0.1,
		0.1, 0.8, 0.1,
		0.1, 0.1, 0.8
	};

	auto dist = movement_run();

	BOOST_CHECK_SMALL(dist(EN, EN) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(EN, HG) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(EN, BP) - 0.333, 0.05);

	BOOST_CHECK_SMALL(dist(HG, EN) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(HG, HG) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(HG, BP) - 0.333, 0.05);

	BOOST_CHECK_SMALL(dist(BP, EN) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(BP, HG) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(BP, BP) - 0.333, 0.05);

	parameters.initialise();
}


BOOST_AUTO_TEST_CASE(movement_home){
	parameters.fishes_movement_type = 'h';
	parameters.fishes_movement = {
		0.8, 0.1, 0.1,
		0.1, 0.7, 0.2,
		0.1, 0.3, 0.6
	};

	auto dist = movement_run();

	BOOST_CHECK_SMALL(dist(EN, EN) - 0.8, 0.05);
	BOOST_CHECK_SMALL(dist(EN, HG) - 0.1, 0.05);
	BOOST_CHECK_SMALL(dist(EN, BP) - 0.1, 0.05);

	BOOST_CHECK_SMALL(dist(HG, EN) - 0.1, 0.05);
	BOOST_CHECK_SMALL(dist(HG, HG) - 0.7, 0.05);
	BOOST_CHECK_SMALL(dist(HG, BP) - 0.2, 0.05);

	BOOST_CHECK_SMALL(dist(BP, EN) - 0.1, 0.05);
	BOOST_CHECK_SMALL(dist(BP, HG) - 0.3, 0.05);
	BOOST_CHECK_SMALL(dist(BP, BP) - 0.6, 0.05);

	parameters.initialise();
}

BOOST_AUTO_TEST_SUITE_END()
