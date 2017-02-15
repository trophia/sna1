# Analysis for simple tagging experiment with:
# - no size selectivity above 25cm
# - instantaneous recapture of all fish

library(dplyr)
library(ggplot2)

# Read in files produced by IBM
# True number of fish in population that are taggable (above e.g. 25cm)
pop <- within(read.table('population.tsv', header= T), {
  region <- factor(c('EN','HG','BP')[region+1])
})
# Number of releases by year, region, method.
released <- within(read.table('released.tsv', header= T), {
  region <- factor(c('EN','HG','BP')[region+1])
  method <- factor(c('LL','BT','DS','RE')[method+1])
})
# Number of fish scanned for tags by year, region, method, length
scanned <- within(read.table('scanned.tsv', header= T), {
  region <- factor(c('EN','HG','BP')[region+1])
  method <- factor(c('LL','BT','DS','RE')[method+1])
})
# Tag releases 'database' one row for each release of a tagged fish
releases <- read.table('releases.tsv', header= T)
# Tag release-recapture 'database' one row for each tagged fish recaptured
# (already linked with release info)
recaptures <- read.table('recaptures.tsv', header= T)

# Get actual numbers in each region
pop_r <- pop %>%
  filter(year == 2000) %>%
  rename(actual = value)

# Get release numbers in each region
released_r <- released %>%
  filter(year == 2000) %>%
  group_by(region) %>%
  summarise(released = sum(value))

# Get scanned numbers in each region
scanned_r <- scanned %>%
  filter(year == 2000) %>%
  group_by(region) %>%
  summarise(scanned = sum(value))

# Get recaptures by region
recovered_r <- recaptures %>%
  filter(time_rec == 2000) %>%
  group_by(region_rec) %>%
  summarise(recovered = length(tag))

# Join up
temp <- pop_r %>%
  left_join(released_r) %>%
  left_join(scanned_r) %>%
  left_join(recovered_r, by=c('region'='region_rec'))
# Calculate estimated population size and compare to actual
temp <- within(temp, {
  rate <- recovered/scanned
  expected <- released / rate
  error <- expected/actual-1
})
temp

# Output mean error
mean_error <- mean(abs(temp$error))
cat(mean_error, file='error.txt')

# Plot of scanned and recovered lengths
temp <- scanned %>%
  group_by(region, length) %>%
  summarise(count = sum(value)) %>%
  left_join(
    recaptures %>%
      mutate(length_rec_bin=floor(length_rec/2)) %>%
      group_by(region_rec, length_rec_bin) %>%
      count,
    by = c('region'='region_rec', 'length'='length_rec_bin')
  )
p = ggplot(temp, aes(x=length)) + 
  geom_line(aes(y=count, colour='Scanned')) +
  geom_line(aes(y=n, colour='Recovered')) + 
  facet_wrap(~region)
png(paste0('lengths.png'), width = 20, height = 20, units = "cm", res=250)
print(p)
dev.off()
p
  