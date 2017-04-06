#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include "../model.hpp"


BOOST_AUTO_TEST_SUITE(model)

BOOST_AUTO_TEST_CASE(tagging){
	Model model;
	model.initialise();

	// Set up movement
	parameters.fishes_movement_type = 'h';
	parameters.fishes_movement = {
		0.8, 0.1, 0.1,
		0.1, 0.7, 0.2,
		0.1, 0.3, 0.6
	};
	
	// Set up tagging program
	auto& tagging = model.monitor.tagging;

	tagging.release_targets = 0;
	tagging.release_targets(2000, EN, LL) =  7500;
	tagging.release_targets(2000, HG, LL) = 15000;
	tagging.release_targets(2000, BP, LL) =  7500;

	tagging.release_years = false;
	tagging.release_years(2000) = true;

	tagging.recovery_years = false;
	tagging.recovery_years(2000) = true;
	tagging.recovery_years(2001) = true;
	tagging.recovery_years(2002) = true;
	tagging.recovery_years(2003) = true;

	model.run(2000, 2004);

	// Do checks
	BOOST_CHECK(tagging.number > 0);
	BOOST_CHECK(tagging.tags.size() > 0);

	// Output files for checking
	model.finalise();
}

BOOST_AUTO_TEST_CASE(tagging_simple){
	Model model;
	model.initialise();

	// No movement
	parameters.fishes_movement_type = 'n';

	// Set up tagging program
	auto& monitor = model.monitor;
	auto& tagging = model.monitor.tagging;

	// Tag releases are not affected by the size selectivity of the gear
	tagging.release_length_selective = false;
	
	// Release schedule
	tagging.release_targets = 0;
	tagging.release_targets(2000, EN, LL) = 100000;
	tagging.release_targets(2000, HG, LL) = 100000;
	tagging.release_targets(2000, BP, LL) = 100000;

	tagging.release_years = false;
	tagging.release_years(2000) = true;

	// Recovery schedule
	tagging.recovery_years = false;
	tagging.recovery_years(2000) = true;
	tagging.recovery_years(2001) = true;
	tagging.recovery_years(2002) = true;
	tagging.recovery_years(2003) = true;
	tagging.recovery_years(2004) = true;

	// Record population size in each year
	Array<int, Years, Regions> pop = 0;
	std::function<void()> callback([&](){
		if (year(now) >= 2000) {
			for (const auto& fish : model.fishes) {
				if (fish.alive() and (fish.length > tagging.release_length_min)) {
					pop(year(now), fish.region)++;
				}
			}
			std::cout 
				<< year(now) << "\t" 
				<< model.fishes.number(false) << "\t" 
				<< pop(year(now), EN) << "\t" 
				<< pop(year(now), HG) << "\t" 
				<< pop(year(now), BP) << std::endl;
		}
	});

	// Run the model
	model.run(2000, 2005, &callback);

	// Do checks
	BOOST_CHECK(tagging.number > 0);
	BOOST_CHECK(tagging.tags.size() > 0);

	// Output files for R script
	tagging.write("tests/tagging/simple");
	pop.write("tests/tagging/simple/population.tsv");

	// Run analysis script
	auto ok = std::system("cd tests/tagging/simple && Rscript analysis.R");
	BOOST_CHECK(ok==0);

	// Read in mean error
	std::ifstream file("tests/tagging/simple/error.txt");
	double error;
	file >> error;

	// Check error <5%
	BOOST_CHECK(error < 0.05);
}

BOOST_AUTO_TEST_CASE(casal){
	// Create an initialise model
	Model model;
	model.initialise();

	// Ensure parameters are set to match the assumptions of CASAL
	// Only temporal variation in growth
	Fish::growth_variation = 't';
	// No movement
	parameters.fishes_movement_type = 'n';
	// No MLS
	parameters.harvest_mls = 0;

	// Generate files for CASAL
	model.generate_casal(1900, 2020, "tests/casal/ibm-outputs");

	// Run CASAL
	auto ok = std::system("cd tests/casal && Rscript length-runner.R");
	BOOST_CHECK(ok==0);

	// Read in output files containing CASAL estimates
	std::ifstream file("tests/casal/estimates.txt");
	std::map<std::string, double> estimates;
	std::string variable;
	std::string year;
	std::string stock;
	double estimate;
	std::string header;
	std::getline(file, header);
	while (file >> variable >> year >> stock >> estimate) {
		estimates[variable + "-" + year + "-" + stock] = estimate;
	}

	// Check estimates of B0 by region are within 5%
	BOOST_CHECK_CLOSE(estimates["B0-NA-ENLD"], parameters.fishes_b0(EN), 5);
	BOOST_CHECK_CLOSE(estimates["B0-NA-HAGU"], parameters.fishes_b0(HG), 5);
	BOOST_CHECK_CLOSE(estimates["B0-NA-BOP"], parameters.fishes_b0(BP), 5);

	// Check estimates of R0 by region are within 10%
	BOOST_CHECK_CLOSE(estimates["R0-NA-ENLD"], model.fishes.recruitment_pristine(EN), 10);
	BOOST_CHECK_CLOSE(estimates["R0-NA-HAGU"], model.fishes.recruitment_pristine(HG), 10);
	BOOST_CHECK_CLOSE(estimates["R0-NA-BOP"], model.fishes.recruitment_pristine(BP), 10);
}

BOOST_AUTO_TEST_SUITE_END()