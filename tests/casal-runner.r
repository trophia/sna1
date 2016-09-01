## R script for modifying CASAL input files and run CASAL 
## and output B0s and SSBs

# ## run the two lines to install casal package
# if (Sys.info()['sysname'] == 'Windows') {
#   install.packages("casal",contriburl="file://niwa.local/Groups/wellington/niwafisheries/R/",
#                    type="win.binary",repos=NULL)
# } else {
#   install.packages('/data01/fisheries-modeling/bianrr/Rlib/casal_2.30.tar.gz', repos=NULL)
# }

library(casal)

# #### Initialisation ####
# file.remove('estimation.csl')
# file.remove('output.csl')
# file.remove('population.csl')
# file.remove('casal.par')
# file.remove('casal.prj')

#### update population csl file ####
catch <- read.table('catch.tsv', header = T, as.is = T)
catch$method <- ifelse(catch$method == 'RE', 'REC', catch$method)
catch$fishery <- paste(catch$region, catch$method, sep = '_')

population <- extract.csl.file('SNA1_age.population.csl')
fishiery.names <- population$annual_cycle$fishery_names

current <- unique(catch$year)[length(unique(catch$year))]
population$current <- current
population$final <- current + 5

## put catches from catch
for (fish in fishiery.names) {
  sub.catch <- subset(catch, fishery == fish)
  i <- grep(fish, names(population))
  population[[i]]$years <- sub.catch$year
  population[[i]]$catches <- sub.catch$catch
  if (!length(grep('future_constant_catches', names(population[[i]])))) next
  population[[i]]$future_constant_catches <- sub.catch$catch[nrow(sub.catch)]
}
write.csl.file(population, 'population.csl')


#### update estimation csl file ####
## load CPUE data
cpue <- read.table('cpue.tsv', header = T, as.is = T)

cpue$method <- ifelse(cpue$method == 'RE', 'REC', cpue$method)
cpue$fishery <- paste(cpue$region, cpue$method, sep = '_')

estimation <- extract.csl.file('SNA1_age.estimation.csl')

## put CPUEs from model
cpue.fisheries <- c('EN_LL', 'HG_LL', 'BP_LL')

for (fish in cpue.fisheries) {
  sub.cpue <- subset(cpue, fishery == fish)
  sub.cpue$cpue <- sub.cpue$cpue/sub.cpue$cpue[1]
  
  i <- grep(paste(fish, 'cpue', sep = ''), names(estimation))
  estimation[[i]]$years <- sub.cpue$year
  
  ## remove years not in data
  csl.years <- as.numeric(names(estimation[[i]]))
  years.to.remove <- csl.years[!csl.years %in% sub.cpue$year]
  years.to.remove <- years.to.remove[!is.na(years.to.remove)]
  for (y in years.to.remove) {
    estimation[[i]][[as.character(y)]] <- NULL
  }
  
  ## put data in
  for (k in 1:nrow(sub.cpue)) {
    j <- as.character(sub.cpue$year[k])
    estimation[[i]][[j]] <- sub.cpue$cpue[k]
  }
}

## load catch at age data
age <- read.table('age.tsv', header = T, as.is = T)
age$method <- ifelse(age$method == 'RE', 'REC', age$method)
age$fishery <- paste(age$region, age$method, sep = '_')
cols <- paste('age', 1:30, sep = '')
age$N <- apply(age[, cols], 1, sum)
age[, cols] <- age[, cols]/age$N

## put catch at age in csl
for (fish in fishiery.names) {
  sub.age <- subset(age, fishery == fish)
  i <- grep(paste(fish, 'age', sep = '_'), names(estimation))
  estimation[[i]]$years <- sub.age$year

  ## remove years not in data
  csl.years <- as.numeric(names(estimation[[i]]))
  years.to.remove <- csl.years[!csl.years %in% sub.age$year]
  years.to.remove <- years.to.remove[!is.na(years.to.remove)]
  for (y in years.to.remove) {
    estimation[[i]][[as.character(y)]] <- NULL
  }

  ## put data in
  for (k in 1:nrow(sub.age)) {
    j <- as.character(sub.age$year[k])
    estimation[[i]][[j]] <- sub.age[k, cols]
    estimation[[i]][[paste('N', j, sep = '_')]] <- sub.age$N[k]
  }
}

write.csl.file(estimation, 'estimation.csl')


#### run CASAL with Francis reweighting ####
source('reweighting functions.R')

## provide the MPD output file from CASAL run
mpd.file <- 'casal.out'

## provide prefix of your CSL file names
csl.prefix <- ''

obs.names <- c('EN_LL_age', 'EN_BT_age', 'EN_REC_age', 
               'HG_LL_age', 'HG_BT_age', 'HG_DS_age','HG_REC_age', 
               'BP_LL_age', 'BP_BT_age', 'BP_DS_age','BP_REC_age')

## reweighting times
rewrite = 0

for (i in 1:10) {
  ## run CASAL
  if (Sys.info()['sysname'] == 'Windows') {
    system('run_CASAL.bat')
  } else {
    system('casal -e -q -O mpd.out > casal.out')
  }
  
  ## calculate reweightings
  weightings <- francis.reweighting(mpd.file, obs.names)
  
  ## if yes, save weightings to history file and update estimation.csl
  rewrite <- rewrite + 1
  write.table(cbind(rewrite, weightings), 'rewriting history.txt', append = rewrite != 1, 
              row.names = F, col.names = rewrite == 1, quote = F, sep = '\t')
  
  update.csl(csl.prefix)
}


#### get CASAL output and write to files ####
output <- extract.quantities('casal.out')

B0 <- data.frame(variable = 'B0', year = NA, stock = names(output$B0), 
                 estimate = as.numeric(output$B0))

years <- output$SSBs$year
output$SSBs$year <- NULL
SSB <- data.frame(variable = 'SSB', year = years, 
                  stock = rep(names(output$SSBs), each = length(years)),
                  estimate = unlist(output$SSBs))

write.table(rbind(B0, SSB), 'CASAL outputs.txt', 
            row.names = F, quote = F, sep = '\t')


## plot SSBs from simulator and CASAL
biomass <- read.table('biomass.tsv', header = T, as.is = T)

stocks <- c('ENLD', 'HAGU', 'BOP')
areas <- c('EN', 'HG', 'BP')

windows(9, 7, 11)
par(mfrow = c(2, 2), mar = c(2, 2, 2, 2), oma = c(3, 3, 1, 1))

for (i in 1:length(stocks)) {
  sub.ssb <- SSB[, c('year', stocks[i])]
  sub.biomass <- subset(biomass, region == areas[i])
  plot(sub.ssb$year, sub.ssb[, stocks[i]], type = 'l', xlab = '', ylab = '')
  lines(sub.biomass$year, sub.biomass$biomass, lty = 2, col = 2)
  legend('topright', stocks[i], bty = 'n', inset = 0.02)
}
legend('bottomleft', c('CASAL SSB', 'Biomass'), lty = 1:2, col = 1:2, bty = 'n', inset = 0.02)
mtext('Year', 1, outer = T, line = 1)
mtext('Biomass', 2, outer = T, line = 1)


