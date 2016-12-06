## R script for modifying CASAL input files and run CASAL 
## and output B0s and SSBs

library(tidyr)
library(stringr)
library(dplyr)
library(ggplot2)
library(casal)

# #### Initialisation ####
# file.remove('estimation.csl')
# file.remove('output.csl')
# file.remove('population.csl')
# file.remove('casal.par')
# file.remove('casal.prj')

#### update population csl file ####
## load catch data
catch <- read.table('output/catch.tsv', header = T, as.is = T)
catch$method <- ifelse(catch$method == 'RE', 'REC', catch$method)
catch$fishery <- paste(catch$region, catch$method, sep = '_')

## load population csl 
population <- extract.csl.file('tests/casal-files/age/population.csl')
fishiery.names <- population$annual_cycle$fishery_names

## put initial, current and final years in csl
population$initial$value <- unique(catch$year)[1]
current <- unique(catch$year)[length(unique(catch$year))]
population$current$value <- current
population$final$value <- current + 5

## put catches in csl from IBM
for (fish in fishiery.names) {
  ## put catch
  sub.catch <- subset(catch, fishery == fish)
  i <- grep(fish, names(population))
  population[[i]]$years <- sub.catch$year
  population[[i]]$catches <- sub.catch$catch
  if (!length(grep('future_constant_catches', names(population[[i]])))) next
  population[[i]]$future_constant_catches <- sub.catch$catch[nrow(sub.catch)]
}

## update YCS (because year may change)
for (stock in population$stock_names$value) {
  i <- paste('recruitment[', stock, ']', sep = '')
  y.start <- population$initial$value
  y.current <- population$current$value
  population[[i]]$YCS_years <- (y.start:y.current) - 1
  population[[i]]$YCS <- rep(1, length(y.start:y.current))
}
write.csl.file(population, 'tests/population.csl')

#### update estimation csl file ####
## load CPUE data
cpue <- read.table('output/monitor/cpue.tsv', header = T, as.is = T)
cpue$method <- ifelse(cpue$method == 'RE', 'REC', cpue$method)
cpue$fishery <- paste(cpue$region, cpue$method, sep = '_')

## load estimation csl
estimation <- extract.csl.file('tests/casal-files/age/estimation.csl')

## put CPUEs from model
cpue.fisheries <- c('EN_LL', 'HG_LL', 'BP_LL')

for (fish in cpue.fisheries) {
  sub.cpue <- subset(cpue, fishery == fish)
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

## load catch at age data
age <- read.table('output/monitor/age.tsv', header = T, as.is = T)
age$method <- ifelse(age$method == 'RE', 'REC', age$method)
age$fishery <- paste(age$region, age$method, sep = '_')

## age need to be dynamic 
cols <- paste('age', 1:30, sep = '')
age$N <- apply(age[, cols], 1, sum)
age[, cols] <- age[, cols]/age$N

## temp:
age <- subset(age, !(region == 'BP' & method == 'DS' & year %in% c(1970:1973, 1987:1989)))

## put catch at age in csl
for (fish in fishiery.names) {
  sub.age <- subset(age, fishery == fish)
  i <- grep(paste(fish, 'age', sep = '_'), names(estimation))
  estimation[[i]]$years <- sub.age$year

  ## remove years not in data
  csl.years <- as.numeric(names(estimation[[i]]))
  years.to.remove <- csl.years[!is.na(csl.years)]
  for (y in years.to.remove) {
    estimation[[i]][[as.character(y)]] <- NULL
    estimation[[i]][[paste('N', y, sep = '_')]] <- NULL
  }

  ## put data in
  ## catch at age
  for (k in 1:nrow(sub.age)) {
    j <- as.character(sub.age$year[k])
    estimation[[i]][[j]] <- sub.age[k, cols]
  }
  ## N
  for (k in 1:nrow(sub.age)) {
    j <- as.character(sub.age$year[k])
    estimation[[i]][[paste('N', j, sep = '_')]] <- sub.age$N[k]
  }
}

write.csl.file(estimation, 'tests/estimation.csl')

# Copy template `output.csl` over to `test` directory
file.copy('tests/casal-files/age/output.csl', 'tests/output.csl')


#### run CASAL with Francis reweighting ####
source('tests/casal-files/reweighting-functions.R')

## provide the MPD output file from CASAL run
mpd.file <- 'tests/casal.out'

## provide prefix of your CSL file names
csl.prefix <- 'tests/'

obs.names <- c('EN_LL_age', 'EN_BT_age', 'EN_REC_age', 
               'HG_LL_age', 'HG_BT_age', 'HG_DS_age','HG_REC_age', 
               'BP_LL_age', 'BP_BT_age', 'BP_DS_age','BP_REC_age')

## reweighting times
rewrite = 0

for (i in 1:5) {
  ## run CASAL
  if (Sys.info()['sysname'] == 'Windows') {
    system('run_CASAL.bat')
  } else {
    system('cd tests && ./casal -e -q -O mpd.out 1> casal.out 2> casal.err')
  }
  
  ## calculate reweightings
  weightings <- francis.reweighting(mpd.file, obs.names)
  
  ## if yes, save weightings to history file and update estimation.csl
  rewrite <- rewrite + 1
  write.table(cbind(rewrite, weightings), 'tests/casal-rewriting-history.txt', append = rewrite != 1, 
              row.names = F, col.names = rewrite == 1, quote = F, sep = '\t')
  
  update.csl(csl.prefix)
}


#### get CASAL output and write to files ####
output <- extract.quantities('tests/casal.out')

B0 <- data.frame(variable = 'B0', year = NA, stock = names(output$B0), 
                 estimate = as.numeric(output$B0))

years <- output$SSBs$year
output$SSBs$year <- NULL
SSB <- data.frame(variable = 'SSB', year = years, 
                  stock = rep(names(output$SSBs), each = length(years)),
                  estimate = unlist(output$SSBs))

years <- names(output$BP_Biom2)
biom <- data.frame(variable = 'Biomass', year = as.numeric(names(output$BP_Biom2)), 
                   stock = rep(names(output$SSBs), each = length(years)), 
                   estimate = unlist(c(output$EN_Biom2, output$HG_Biom2, output$BP_Biom2)))

write.table(rbind(B0, SSB), 'tests/casal-estimates.txt', 
            row.names = F, quote = F, sep = '\t')

## plot SSBs from simulator and CASAL
biomass <- read.table('output/biomass.tsv', header = T, as.is = T)

stocks <- c('ENLD', 'HAGU', 'BOP')
areas <- c('EN', 'HG', 'BP')

#windows(9, 7, 11)
par(mfrow = c(2, 2), mar = c(2, 2, 2, 2), oma = c(3, 3, 1, 1))

for (i in 1:length(stocks)) {
  sub.ssb <- subset(SSB, stock == stocks[i])
  sub.biom <- subset(biom, stock == stocks[i])
  sub.biomass <- subset(biomass, region == areas[i])
  plot(sub.biomass$year, sub.biomass$biomass, type = 'l', xlab = '', ylab = '', ylim=c(0, max(biom$estimate)))
  lines(sub.biom$year, sub.biom$estimate, lty = 2, col = 2)
  lines(sub.ssb$year, sub.ssb$estimate, lty = 3, col = 4)
  legend('topright', stocks[i], bty = 'n', inset = 0.02)
}
legend('bottomleft', c('IBM biomass', 'CASAL biom', 'CASAL SSB'), lty = 1:3, col = c(1:2, 4), 
       bty = 'n', inset = 0.02)
mtext('Year', 1, outer = T, line = 1)
mtext('Biomass', 2, outer = T, line = 1)

# Compare stock status

temp <- merge(SSB, B0[,c('stock', 'estimate')], by='stock')
ggplot(temp, aes(x=year,y=estimate.x/estimate.y, color=stock)) +
  geom_point() + 
  geom_line(aes(x=year,y=biomass.x/biomass.y,color=region), data=merge(biomass,biomass[biomass$year==1900,c('region', 'biomass')],by='region')) +
  ylim(0,NA)

# Fits

fits <- extract.fits('tests/casal.out')

# CPUE 
par(mfrow=c(3,1))
plot(fits$EN_LLcpue$obs)
lines(fits$EN_LLcpue$fits)
plot(fits$HG_LLcpue$obs)
lines(fits$HG_LLcpue$fits)
plot(fits$BP_LLcpue$obs)
lines(fits$BP_LLcpue$fits)

# Catch at age

obs <- fits$BP_LL_age$obs
obs$year <- row.names(obs)
obs <- gather(subset(obs, year >= 2004), key, value, -year)
obs$age <- as.integer(str_sub(obs$key,2))

fits <- fits$BP_LL_age$fits
fits$year <- row.names(fits)
fits <- gather(subset(fits, year >= 2004), key, value, -year)
fits$age <- as.integer(str_sub(fits$key,2))

# Obs v Pred by year
ggplot() + 
  geom_point(aes(x=age,y=value), data=obs) + 
  geom_line(aes(x=age,y=value), data=fits) + 
  facet_wrap(~year)

# Obs v Pred
ggplot() + 
  geom_point(aes(x=age,y=value), obs %>% group_by(age) %>% summarise(value=mean(value))) + 
  geom_line(aes(x=age,y=value), fits %>% group_by(age) %>% summarise(value=mean(value)))



