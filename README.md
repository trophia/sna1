# `sna1`

A simulation model of the snapper (_Pagrus auratus_) population and fishery in the Auckland (East) Fisheries Management Area (SNA 1).

## Background

This model is being developed to aid in the evaluation alternative monitoring and management strategies for the SNA 1 fishery. For example: is it better to conduct a large tagging study every 10 years, or a smaller one every 3 years? What are the consequences of sampling age frequencies every three years as opposed to every year? What are the likely long term effects of changes in the minimum legal size?

This work is being undertaken by [NIWA](http://niwa.co.nz) and [Trophia](http://trophia.com) as part of a project funded by the [New Zealand Ministry of Primary Industries](http://mpi.govt.nz).

## Model structure

The model is an [individual-based (aka agent-based)](https://en.wikipedia.org/wiki/Agent-based_model)(IBM). IBMs have been used for some time in ecology (see Grimm & Railsback (2005) for a review) but their use in fisheries science has been limited (although see Thorson et al (2012) for a recent example). We chose to use an IBM because it has a number of advantages for simulating detailed temporal and spatial dynamics.

The SNA 1 fishery is one of New Zealand's most important. Commensurate with the fishery's importance, we wanted to use a model structure that could accommodate the types of questions that would be asked of it. While a coarse single-area, age-structured model is more than adequate to evaluate many types of monitoring and management strategies it could not answer questions around multiple area, multiple substocks or size-based processes that are of interest for SNA 1.

We considered using a partition-based model (i.e one based on a multi-dimensional array of fish numbers) but the number of partitions necessary quickly becomes very large when complexity is added. For example, a model with 3 substocks, 3 areas, 30 ages, 50 sizes (2cm size bins) and a tagged/untagged split would involve `3*3*30*50*2 = 27000` partitions. Increasing the precision of size to 1cm (to be able to better evaluate changes in MLS) and increasing the spatial precision by adding two additional area (e.g. inner HG, western BP) increases the number of partitions dramatically to `3*5*30*100*2 = 90000` partitions.

Instead of having a model that performs it's mathematical accounting on 20000 partitions, a model can instead perform it's accounting on a sample of 20000 fish. It is important to remember that an IBM does not necessarily have to model all the millions of individuals in a population. Rather, it just uses a "sample" population of individuals to reflect the distribution of attributes (e.g. age, length, location) amongst the whole population. An IBM has a number of features that are advantageous for a model of SNA 1.

Perhaps most importantly, the nature of IBM models, and in particular how they model detailed population processes, is likely to be more easily explained and understood by stakeholders than complicated partition based models.

It is generally easy to add additional complexity to an IBM. This is important in the current context; we wanted a model that could start off relatively simple but be flexible enough to have complexity added to in in response to questions asked of it by stakeholders. For example, in an IBM, explicit modeling of "trap shyness" just involves adding an attribute to each individual which counts the number of times it has previously been caught and then making it's probability of capture partly dependent on that count. 

IBMs are inherently easy to parrallelize and thus take advantage of multi-core and multi-machine computing environments.

## Status

The model is in early stages of development. The basic architecture has been setup but much of the dynamics are yet to be implemented.

## References

Grimm, V., and Railsback, S. F. 2005. Individual-based Modeling andEcology. Princeton University Press, Princeton, NJ. 428 pp.

Thorson, J. T., Stewart, I. J., & Punt, A. E. (2012). Development and application of an agent-based model to evaluate methods for estimating relative abundance indices for shoaling fish such as Pacific rockfish (Sebastes spp.). ICES Journal of Marine Science: Journal du Conseil, 69(4), 635-647.
