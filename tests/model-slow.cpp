#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include "../model.hpp"


BOOST_AUTO_TEST_SUITE(model)


BOOST_AUTO_TEST_CASE(casal){
	Model model;
	model.initialise();

	// Generate files for CASAL
	model.generate_casal(1900, 2020);

	// Run CASAL
	auto ok = std::system("Rscript tests/casal-files/runner.R");
	BOOST_CHECK(ok==0);

	// Read in output files containing CASAL estimates
	std::ifstream file("tests/casal-estimates.txt");
	std::map<std::string, double> estimates;
	std::string name;
	double value;
	while (file >> name >> value) estimates[name] = value;

	// Check estimates of B0 by region are within 5%
	BOOST_CHECK_CLOSE(estimates["B0_EN"], parameters.fishes_b0(EN), 5);
	BOOST_CHECK_CLOSE(estimates["B0_HG"], parameters.fishes_b0(HG), 5);
	BOOST_CHECK_CLOSE(estimates["B0_BP"], parameters.fishes_b0(BP), 5);
}

BOOST_AUTO_TEST_SUITE_END()
