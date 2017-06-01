# R script for running a CASAL model on outputs from a SNA1 IBM run
# This script should be run in the top /sna1 directory

library(casal)
library(tidyr)
library(dplyr)
library(ggplot2)

CASAL_HOME = '~/trophia/code/sna1/casal'


#################################################################
# Load IBM outputs

# Load catches
catch_ibm <- read.table('output/monitor/casal/catch.tsv', header = T, as.is = T)
catch_ibm$method <- ifelse(catch_ibm$method == 'RE', 'REC', catch_ibm$method)
catch_ibm$fishery <- paste(catch_ibm$region, catch_ibm$method, sep = '_')

## Load CPUE
cpue_ibm <- read.table('output/monitor/casal/cpue.tsv', header = T, as.is = T)
cpue_ibm <- subset(cpue_ibm, cpue_ibm != 0)
cpue_ibm$method <- ifelse(cpue_ibm$method == 'RE', 'REC', cpue_ibm$method)
cpue_ibm$fishery <- paste(cpue_ibm$region, cpue_ibm$method, sep = '_')

# Load parameters
parameters_ibm <- read.table('output/monitor/casal/parameters.tsv', header = T, as.is = T)


#################################################################
# Update population.csl

population <- extract.csl.file(file.path(CASAL_HOME, 'length/population.csl'))

# Insert initial, current and final years
population$initial$value <- unique(catch_ibm$year)[1]
current <- unique(catch_ibm$year)[length(unique(catch_ibm$year))]
population$current$value <- current
population$final$value <- current + 5

# Insert growth parameters from IBM
# Currently uses the same parameters for all substocks
# EN
population$`growth[1]`$g <- subset(parameters_ibm, par %in% c('growth_20', 'growth_50'))$value
population$`growth[1]`$cv <- subset(parameters_ibm, par == 'growth_cv')$value
population$`growth[1]`$minsigma <- subset(parameters_ibm, par == 'growth_sdmin')$value
# HG
population$`growth[2]`$g <- subset(parameters_ibm, par %in% c('growth_20', 'growth_50'))$value
population$`growth[2]`$cv <- subset(parameters_ibm, par == 'growth_cv')$value
population$`growth[2]`$minsigma <- subset(parameters_ibm, par == 'growth_sdmin')$value
# BP
population$`growth[3]`$g <- subset(parameters_ibm, par %in% c('growth_20', 'growth_50'))$value
population$`growth[3]`$cv <- subset(parameters_ibm, par == 'growth_cv')$value
population$`growth[3]`$minsigma <- subset(parameters_ibm, par == 'growth_sdmin')$value

## Insert catches from IBM
for (fish in population$annual_cycle$fishery_names) {
  ## put catch
  sub.catch <- subset(catch_ibm, fishery == fish)
  i <- grep(fish, names(population))
  population[[i]]$years <- sub.catch$year
  population[[i]]$catches <- sub.catch$catch
  if (!length(grep('future_constant_catches', names(population[[i]])))) next
  population[[i]]$future_constant_catches <- sub.catch$catch[nrow(sub.catch)]
}

## Update YCS (because year may change)
for (stock in population$stock_names$value) {
  i <- paste('recruitment[', stock, ']', sep = '')
  y.start <- population$initial$value
  y.current <- population$current$value
  population[[i]]$YCS_years <- (y.start:y.current) - 1
  population[[i]]$YCS <- rep(1, length(y.start:y.current))
}

# Write to file
write.csl.file(population, 'population.csl')


#################################################################
# Update estimation.csl

estimation <- extract.csl.file(file.path(CASAL_HOME, 'length/estimation.csl'))

## Insert CPUEs from model
for (fish in c('EN_LL', 'HG_LL', 'BP_LL')) {
  sub.cpue <- subset(cpue_ibm, fishery == fish)
  sub.cpue$cpue <- sub.cpue$cpue/sub.cpue$cpue[1]
  
  i <- grep(paste(fish, 'cpue', sep = ''), names(estimation))
  estimation[[i]]$years <- sub.cpue$year
  
  ## remove years not in data
  csl.years <- as.numeric(names(estimation[[i]]))
  years.to.remove <- csl.years[!is.na(csl.years)]
  for (y in years.to.remove) {
    estimation[[i]][[as.character(y)]] <- NULL
  }
  
  ## delete cv and add it in later to make it look nice
  cv <- estimation[[i]]$cv
  estimation[[i]]$cv <- NULL

  ## put data in
  for (k in 1:nrow(sub.cpue)) {
    j <- as.character(sub.cpue$year[k])
    estimation[[i]][[j]] <- sub.cpue$cpue[k]
  }
  estimation[[i]]$cv <- cv
}

# Write to file
write.csl.file(estimation, 'estimation.csl')

#################################################################
# Update output.csl

output <- extract.csl.file(file.path(CASAL_HOME, 'length/output.csl'))

# Write to file
write.csl.file(output, 'output.csl')


#################################################################
# Run CASAL

if (Sys.info()['sysname'] == 'Windows') {
  system(paste(CASAL_HOME, 'run_CASAL.bat', sep='/'))
} else {
  system(paste(CASAL_HOME, '/casal -r -q -O mpd.out 1> casal.out 2> casal.err', sep='/'))
}


#################################################################
# Get CASAL output

output <- extract.quantities('casal.out')

B0 <- data.frame(variable = 'B0', year = NA, stock = names(output$B0), 
                 estimate = as.numeric(output$B0))

R0 <- data.frame(variable = 'R0', year = NA, stock = names(output$R0), 
                 estimate = as.numeric(output$R0))

years <- output$SSBs$year
output$SSBs$year <- NULL
SSB <- data.frame(variable = 'SSB', year = years, 
                  stock = rep(names(output$SSBs), each = length(years)),
                  estimate = unlist(output$SSBs))

years <- names(output$BP_Biom2)
biom <- data.frame(variable = 'Biomass', year = as.numeric(names(output$BP_Biom2)), 
                   stock = rep(names(output$SSBs), each = length(years)), 
                   estimate = unlist(c(output$EN_Biom2, output$HG_Biom2, output$BP_Biom2)))

write.table(rbind(B0, R0, SSB), 'estimates.txt', row.names = F, quote = F, sep = '\t')


## plot SSBs from IBM and CASAL
biomass <- read.table('output/monitor/casal/biomass.tsv', header = T, as.is = T)

stocks <- c('ENLD', 'HAGU', 'BOP')
areas <- c('EN', 'HG', 'BP')

pdf('SSBs.pdf', width = 9, height = 7, pointsize = 11)
par(mfrow = c(2, 2), mar = c(2, 2, 2, 2), oma = c(3, 3, 1, 1))

for (i in 1:length(stocks)) {
  sub.ssb <- subset(SSB, stock == stocks[i])
  sub.biom <- subset(biom, stock == stocks[i])
  sub.biomass <- subset(biomass, region == areas[i])
  plot(sub.biomass$year, sub.biomass$biomass, type = 'l', xlab = '', ylab = '', 
       ylim=c(0, max(biomass$biomass, SSB$estimate)))
  lines(sub.biom$year, sub.biom$estimate, lty = 2, col = 2)
  lines(sub.ssb$year, sub.ssb$estimate, lty = 3, col = 4)
  legend('topright', stocks[i], bty = 'n', inset = 0.02)
}
legend('bottomleft', c('IBM biomass', 'CASAL biom', 'CASAL SSB'), lty = 1:3, col = c(1:2, 4), 
       bty = 'n', inset = 0.02)
mtext('Year', 1, outer = T, line = 1)
mtext('Biomass', 2, outer = T, line = 1)
dev.off()

# -----------------------------------------------------------------------------
# Comparision of catch at lengths

# Wrangle IBM lengths into long format
lengths_ibm <- read.table('output/monitor/casal/length.tsv', header=T, as.is=T)
names(lengths_ibm)[4:103] <- seq(0, 99, 1)
lengths_ibm <- lengths_ibm %>%
  gather('length', 'count', -(1:3)) %>%
  arrange(year, region, method) %>%
  within({
    length <- as.integer(length)
    length_bin = floor((length-1)/2)*2
  }) %>%
  group_by(year, region, method, length_bin) %>% 
  summarise(count_ = sum(count)) %>%
  rename(length = length_bin, count = count_) %>%
  arrange(year, region, method)

# Wrangle CASAL lengths into long format
temp <- function(region, method) {
  data <- do.call(rbind.data.frame, output[[paste(region, method, sep='_')]])
  names(data) <- 1:100
  data$year <- as.integer(row.names(data))
  data$region <- region
  data$method <- method
  data <- gather(data, 'length', 'count', -(101:103))
  data$length <- as.integer(data$length)
  data
}
lengths_casal <- rbind(
  temp('EN', 'LL'), temp('EN', 'BT'), temp('EN', 'DS'), temp('EN', 'REC'), temp('EN', 'pop'),
  temp('HG', 'LL'), temp('HG', 'BT'), temp('HG', 'DS'), temp('HG', 'REC'), temp('HG', 'pop'),
  temp('BP', 'LL'), temp('BP', 'BT'), temp('BP', 'DS'), temp('BP', 'REC'), temp('BP', 'pop')
)
lengths_casal <- lengths_casal %>%
  mutate(length_bin = floor((length-1)/2)*2) %>%
  group_by(year, region, method, length_bin) %>% 
  summarise(count_ = sum(count)) %>%
  rename(length = length_bin, count = count_) %>%
  arrange(year, region, method)

# Join them together
lengths <- left_join(
  lengths_casal, 
  lengths_ibm, 
  by=c('year', 'region', 'method', 'length'), 
  suffix=c('_casal', '_ibm')
)
# Convert some column types
lengths <- within(lengths, {
  year <- as.factor(year)
  region <- factor(region, levels=c('EN', 'HG', 'BP'), ordered=T)
  method <- factor(method, levels=c('pop', 'LL', 'BT', 'DS', 'REC'), ordered=T)
  length <- as.integer(length)
})
# Add proportions
lengths <- lengths %>% 
  group_by(year, region, method) %>% 
  summarise(
    total_casal = sum(count_casal, na.rm=T),
    total_ibm = sum(count_ibm, na.rm=T)
  ) %>%
  right_join(lengths) %>%
  mutate(
    prop_casal = count_casal/total_casal,
    prop_ibm = count_ibm/total_ibm
  )

# Plot over years
p <- ggplot(lengths %>% 
  group_by(region,method,length) %>%
  summarise(
    prop_casal = mean(prop_casal, na.rm=T),
    prop_ibm = mean(prop_ibm, na.rm=T)
  )
) +
  geom_line(aes(x=length,y=prop_casal, colour='CASAL')) + 
  geom_line(aes(x=length,y=prop_ibm, colour='IBM')) + 
  facet_grid(region~method)
png(paste0('lengths-region-method.png'), width = 20, height = 20, units = "cm", res=250)
print(p)
dev.off()
p

# Plots by year for specific region & method
temp <- function(region_, method_){
  p <- ggplot(filter(lengths, region==region_ & method==method_), aes(x=length)) + 
    geom_line(aes(y=prop_casal, colour='CASAL')) +
    geom_line(aes(y=prop_ibm, colour='IBM')) + 
    facet_wrap(~year)
  png(paste0('lengths-year-', region_, '-', method_, '.png'), width = 20, height = 20, units = "cm", res=250)
  print(p)
  dev.off()
  p
}
temp('HG', 'pop')
temp('EN', 'LL')
temp('BP', 'LL')
temp('HG', 'BT')
temp('HG', 'REC')
