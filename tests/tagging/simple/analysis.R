# Analysis for simple tagging experiment with:
# - no size selectivity above 25cm
# - instantaneous recapture of all fish

library(dplyr)

# Read in file produced by IBM
# Population number of fish > 25cm
pop <- within(read.table('population.tsv', header= T), {
  region <- factor(c('EN','HG','BP')[region+1])
})
# Release schedule by year
releases <- within(read.table('releases.tsv', header= T), {
  region <- factor(c('EN','HG','BP')[region+1])
})
# Number of fish scanned for tags by year, region, method, length
scanned <- within(read.table('scanned.tsv', header= T), {
  region <- factor(c('EN','HG','BP')[region+1])
})
# Tag release-recapture 'database' one row for each tagged fish
tags <- read.table('tags.tsv', header= T)

# Get actual numbers in each region
pop_r <- pop %>%
  filter(year == 2000) %>%
  rename(actual = value)

# Get release numbers in each region
released_r <- releases %>%
  filter(year == 2000) %>%
  group_by(region) %>%
  summarise(released = sum(value))

# Get scanned numbers in each region
scanned_r <- scanned %>%
  filter(year == 2000) %>%
  group_by(region) %>%
  summarise(scanned = sum(value))

# Get recaptures by region
recovered_r <- tags %>%
  filter(time_rec == 2000) %>%
  group_by(region_rec) %>%
  summarise(recovered = length(tag))

temp <- pop_r %>%
  left_join(released_r) %>%
  left_join(scanned_r) %>%
  left_join(recovered_r, by=c('region'='region_rec'))

temp <- within(temp, {
  rate <- recovered/scanned
  expected <- released / rate
  error <- expected/actual
})

temp

  
temp <- scanned %>%
  group_by(region, length) %>%
  summarise(count = sum(value)) %>%
  left_join(
    tags %>%
      mutate(length_rec_bin=floor(length_rec/2)) %>%
      group_by(region_rec, length_rec_bin) %>%
      count,
    by = c('region'='region_rec', 'length'='length_rec_bin')
  )
ggplot(temp, aes(x=length)) + 
  geom_line(aes(y=count, colour='Scanned')) +
  geom_line(aes(y=n, colour='Recovered')) + 
  facet_wrap(~region)

  