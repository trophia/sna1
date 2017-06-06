library(ggplot2)

pars <- read.table('../../output/fishes/growth_pars.tsv', header=T)

ggplot(pars, aes(x=intercept, y=..density..)) + geom_histogram()
ggplot(pars, aes(x=slope, y=..density..)) + geom_histogram()

trajs <- within(read.table('../../output/fishes/growth_trajs.tsv', header=T), {
  length_incr <- length_new - length
})
ggplot(trajs, aes(x=time, y=length, group=fish)) +
  geom_line(alpha=0.3)

ggplot(trajs, aes(x=length, y=length_incr)) +
  geom_point(alpha=0.3) + 
  geom_line(aes(group=fish), alpha=0.2)
