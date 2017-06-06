# `sna1`

A simulation model of the snapper (_Pagrus auratus_) population and fishery in the Auckland (East) Fisheries Management Area (SNA 1).

[![Travis](https://travis-ci.org/trophia/sna1.svg?branch=master)](https://travis-ci.org/trophia/sna1)
[![codecov](https://codecov.io/gh/trophia/sna1/branch/master/graph/badge.svg)](https://codecov.io/gh/trophia/sna1)

## Background

This model is being developed to aid in the evaluation alternative monitoring and management strategies for the SNA 1 fishery. For example: is it better to conduct a large tagging study every 10 years, or a smaller one every 3 years? What are the consequences of sampling age frequencies every three years as opposed to every year? What are the likely long term effects of changes in the minimum legal size?

This work is being undertaken by [NIWA](http://niwa.co.nz) and [Trophia](http://trophia.com) as part of a project funded by the [New Zealand Ministry of Primary Industries](http://mpi.govt.nz).

## Usage

The `input` folder contains several files that can be used for setting parameters of the model. All parameters have default values hardwired into the code, so these input files provide a way of optionally overriding these values. All of these parameters are then dumped into the corresponding file in the `output` folder so that you can check the values read in and actually used.

Some of these files require the use of numeric codes for certain dimensions:

- `sex`: male = `0`, female = `1`
- `region`: EN = `0`, HG = `1`, BP = `2`
- `method`: LL = `0`, BT = `1`, DS = `2`, RE = `3`

#### [`input/parameters.json`](input/parameters.json)

A JSON file containing single-valued parameters. The format is fairly self explanatory, but note that string parameters need to be quoted e.g.

```json
{
    "fishes_seed_number": 1000000,
    "fishes_steepness": 0.85,
    "fishes_movement_type": "h",
}
```

The file `output/fishes/values.tsv` contains summary values related to the fish population simulated:

- size: the size of the vector of simulated fish (this may be above `fishes_seed_number` for example due to recruitment variation causing the population size to grow above the seed size)
- alive : the simulated number of *alive* fish in the population in the *last year* e.g. `492813`
- scalar : the scalar used to scale the simulated population to the real population e.g. `244.498`
- number : the scaled number of fish in the populaion in the *last year* e.g. `1.20492e+08`

#### Pristine spawning biomass

The pristine spawning biomass (B0) for each region can be set in the file [`input/fishes_b0.tsv`](input/fishes_b0.tsv) e.g.

```
region	value
0	100000
1	200000
2	100000
```

#### Recruitment year class strengths

Year class strengths (multipliers of deterministic recruitment) can be set for each year and region using the file [`input/fishes_rec_strengths.tsv`](input/fishes_rec_strengths.tsv). Use `-1` for random recruitment strength (having mean `1` and a c.v. of `fishes_rec_var`). Use values of zero or greater to specify a recruitment strength. You don't have to specify a value for each year; the default recruitment strength is `1` (i.e. deterministic). 

For example, a recruitment strength of `1.3` in 1990, random recruitment strength in 1991, and deterministic in all other years for Hauraki Gulf would be specified using:

```
year	region	value
1990	1	1.3
1991	1	-1
```

#### Growth

The model allows for two forms of the relation between fish length and the increment in fish length:

- linear (`l`)
- exponential (`e`)

Both models are parameterized using the von Bertalanffy parameters _k_ and _Linf_ (for the exponential model these are converted into equivalent increments at length 25cm and 50cm and then into the _lamda_ and _kappa_ parameters of the exponential model). For example, a von Bertalanffy growth curve is defined as:

```json
{
    "fishes_growth_model": "l",
    "fishes_k_mean": 0.1,
    "fishes_linf_mean": 60
}
```

But can be converted to an exponential model by changing the `fishes_growth_model` code to `e`:

```json
{
    "fishes_growth_model": "e",
    "fishes_k_mean": 0.1,
    "fishes_linf_mean": 60
}
```


#### [`input/fishes_movement.tsv`](input/fishes_movement.tsv)

A TSV file with the probability of a fish moving from `region` to `region_to`. You don't have to specify a value for all combinations of regions; the default value is `0` (i.e. no movement). But you should be careful to ensure that the probabilities for a region sum to 1. e.g.

```
region	region_to	value
0	0	0.8
0	1	0.1
0	2	0.1
1	0	0
1	1	0.9
1	2	0.1
2	0	0
2	1	0
2	2	1
```

Also note that `fishes_movement_type` can be set in `parameters.json`: 

- `"h"` = home fidelity
- `"m"` = markov

#### [`input/harvest_catch_history.tsv`](input/harvest_catch_history.tsv)

A TSV file with catches (in tonnes) for each `year`, `region` and `method`. You don't have to specify a value for each year; the default value is `0` (i.e. no catch). 

For example, if you only wanted to simulate a single catch event of 100t taken by BT in EN in 2017:

```
year	region	method	value
2017	0	1	100
```

#### Monitoring

The file [`input/monitoring_programme.tsv`](input/monitoring_programme.tsv) allows you to specify an annual monitoring programme. Each value is a character string with each character specifying if a type of monitoring will be conducted and outputted in `output/monitoring`:

- `C`: catch-per-unit-effort index in `output/monitoring/cpue.tsv`
- `L`: length sampling of the catch in `output/monitoring/length_samples.tsv`
- `A`: age sampling of the catch in `output/monitoring/age_samples.tsv`

The default value is an empty string i.e. no monitoring.

For example, to specify CPUE and length sampling every year, and age sampling every second year, between 1990 and 1994:

```
year	value
1990	CL
1991	CLA
1992	CL
1993	CLA
1994	CL
```

The output file `output/monitoring/programme.tsv` provides an alternative representation of the inputted programme with `0`s and `1`s in each column for every year e.g.

```
year	cpue	lengths	ages
1900	0	0	0
1901	0	0	0
1902	0	0	0
1903	0	0	0
...
```

#### Tagging

Two TSV files allow you to specify release and recapture schedules for tagging programmes:

- [`input/tagging_releases.tsv`](input/tagging_releases.tsv): the number of tags to release by `year`, `region` and `method`
- [`input/tagging_scanning.tsv`](input/tagging_scanning.tsv): the proportion of catch to be scanned by `year`, `region` and `method`

For example, to specify 30,000 longline tag releases in 2018, spread over the three regions, followed by 2 years of scanning 50% of LL and BT catches:

`tagging_releases.tsv`:

```
year	region	method	value
2018	0	0	10000
2018	1	0	10000
2018	2	0	10000
```

`tagging_scanning.tsv`:

```
year	region	method	value
2019	0	0	0.5
2019	1	0	0.5
2019	2	0	0.5
2019	0	1	0.5
2019	1	1	0.5
2019	2	1	0.5
2020	0	0	0.5
2020	1	0	0.5
2020	2	0	0.5
2020	0	1	0.5
2020	1	1	0.5
2020	2	1	0.5
```

When doing simulations involving tagging it may be appropriate to set the `fishes_seed_number` (i.e. the number of fish in the pristine, unfished, poulation) to a high value e.g. 50000000

The simulated tagging programme outputs five files to the folder `output/monitor/tagging/`:

- `population_numbers.tsv` : the numbers of fish in the population that are above `release_length_min` by year and region
- `released.tsv` : the number of fish scanned by year, region and method
- `scanned.tsv` : the number of fish scanned by year, region, method and length bin
- `releases.tsv` : the individual tag releases with fields `tag`, `time_rel`, `time_rec`, `region_rel`
- `recaptures.tsv`: the individual tag recaptures with fields `tag`, `time_rel`, `time_rec`, `region_rel`, `region_rec`, `method_rel`, `method_rec`, `length_rel`, `length_rec`


#### Shyness

The file [`input/fishes_shyness.tsv`](input/fishes_shyness.tsv) controls the degree of shyness of a fish to the last fishing method that it was caught by (assuming it was subsequently released because it was undersized or tagged). This is used to mediate it vulnerability to that fishing method in subsequent time steps. Shyness to a method should be a value between 0 and 1:

- 1 = complete shyness, will never get caught by the method again
- 0 = no shyness, normal vulnerability/selectivity applies

Note that although shyness is of most interest for it's implications for tagging estimates, it also applies to undersized fish that have been returned to the sea.

## Structure

The model is an [individual-based](https://en.wikipedia.org/wiki/Agent-based_model) (IBM, aka agent-based). IBMs have been used for some time in ecology (see Grimm & Railsback (2005) for a review) but their use in fisheries science has been limited (although see Thorson et al (2012) for a recent example). We chose to use an IBM because it has a number of advantages for simulating detailed temporal and spatial dynamics.

The SNA 1 fishery is one of New Zealand's most important. Commensurate with the fishery's importance, we wanted to use a model structure that could accommodate the types of questions that would be asked of it. While a coarse single-area, age-structured model is more than adequate to evaluate many types of monitoring and management strategies it could not answer questions around multiple area, multiple substocks or size-based processes that are of interest for SNA 1.

We considered using a partition-based model (i.e one based on a multi-dimensional array of fish numbers) but the number of partitions necessary quickly becomes very large when complexity is added. For example, a model with 3 substocks, 3 areas, 30 ages, 50 sizes (2cm size bins) and a tagged/untagged split would involve `3*3*30*50*2 = 27000` partitions. Increasing the precision of size to 1cm (to be able to better evaluate changes in MLS) and increasing the spatial precision by adding two additional area (e.g. inner HG, western BP) increases the number of partitions dramatically to `3*5*30*100*2 = 90000` partitions.

Instead of having a model that performs it's mathematical accounting on 20000 partitions, a model can instead perform it's accounting on a sample of 20000 fish. It is important to remember that an IBM does not necessarily have to model all the millions of individuals in a population. Rather, it just uses a "sample" population of individuals to reflect the distribution of attributes (e.g. age, length, location) amongst the whole population. An IBM has a number of features that are advantageous for a model of SNA 1.

Perhaps most importantly, the nature of IBM models, and in particular how they model detailed population processes, is likely to be more easily explained and understood by stakeholders than complicated partition based models.

It is generally easy to add additional complexity to an IBM. This is important in the current context; we wanted a model that could start off relatively simple but be flexible enough to have complexity added to in response to questions asked of it by stakeholders. For example, in an IBM, explicit modeling of "trap shyness" just involves adding an attribute to each individual which counts the number of times it has previously been caught and then making it's probability of capture partly dependent on that count. 

IBMs are inherently easy to parrallelize and thus take advantage of multi-core and multi-machine computing environments.

## References

Grimm, V., and Railsback, S. F. 2005. Individual-based modeling and ecology. Princeton University Press, Princeton, NJ. 428 pp.

Thorson, J. T., Stewart, I. J., & Punt, A. E. (2012). Development and application of an agent-based model to evaluate methods for estimating relative abundance indices for shoaling fish such as Pacific rockfish (Sebastes spp.). ICES Journal of Marine Science: Journal du Conseil, 69(4), 635-647.
