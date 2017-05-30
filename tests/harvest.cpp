#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include "../harvest.hpp"


BOOST_AUTO_TEST_SUITE(harvest)

BOOST_AUTO_TEST_CASE(selectivity){
	Harvest harvest;

	parameters.harvest_sel_mode = {20, 25, 30, 35};
	parameters.harvest_sel_steep1 = {1, 3, 5, 10};
	parameters.harvest_sel_steep2 = {1000, 100, 10, 5};

	harvest.initialise();

	// Output selectivity at length
	harvest.selectivity_at_length.write("tests/harvest/selectivity_at_length.tsv");
	
	// Output parameters
	std::ofstream pars("tests/harvest/selectivity_pars.tsv");
	pars << "method\tmode\tsteep1\tsteep2\n";
	for (auto method : methods) {
		pars << method.index() << "\t"
			 << parameters.harvest_sel_mode(method) << "\t"
			 << parameters.harvest_sel_steep1(method) << "\t"
			 << parameters.harvest_sel_steep2(method) << "\n";
	}
	pars.close();

	// Calculate selectivity at length in R; output graph anda sum of abs differences
	auto ok = std::system("cd tests/harvest && Rscript selectivity.R");
	BOOST_CHECK(ok==0);

	// Read in difference and check it is less than a certain amount
	double diff;
	std::ifstream file("tests/harvest/selectivity-diff.txt");
	file >> diff;
	BOOST_CHECK(diff < 0.001);
}

BOOST_AUTO_TEST_SUITE_END()
