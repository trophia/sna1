## R script for modifying CASAL input files and run CASAL 
## and output B0s and SSBs


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
population <- extract.csl.file('tests/casal-files/length/SNA1_len.population.csl')
fishiery.names <- population$annual_cycle$fishery_names

## put initial, current and final years in csl
population$initial$value <- unique(catch$year)[1]
current <- unique(catch$year)[length(unique(catch$year))]
population$current$value <- current
population$final$value <- current + 5

# Insert growth parameters from IBM
population_ibm <- read.table('tests/casal-files/length/population.tsv', header = T, as.is = T)
# Currently uses the same parameters for all substocks
population$`growth[1]`$g <- subset(population_ibm, par %in% c('growth_20', 'growth_50'))$value
population$`growth[1]`$cv <- subset(population_ibm, par == 'growth_cv')$value
population$`growth[1]`$minsigma <- subset(population_ibm, par == 'growth_min_sd')$value
population$`growth[2]`$g <- subset(population_ibm, par %in% c('growth_20', 'growth_50'))$value
population$`growth[2]`$cv <- subset(population_ibm, par == 'growth_cv')$value
population$`growth[2]`$minsigma <- subset(population_ibm, par == 'growth_min_sd')$value
population$`growth[3]`$g <- subset(population_ibm, par %in% c('growth_20', 'growth_50'))$value
population$`growth[3]`$cv <- subset(population_ibm, par == 'growth_cv')$value
population$`growth[2]`$minsigma <- subset(population_ibm, par == 'growth_min_sd')$value

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
estimation <- extract.csl.file('tests/casal-files/length/SNA1_len.estimation.csl')

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

write.csl.file(estimation, 'tests/estimation.csl')


# Copy template `output.csl` over to `test` directory
file.copy('tests/casal-files/length/SNA1_len.output.csl', 'tests/output.csl', overwrite=T)


## run CASAL
if (Sys.info()['sysname'] == 'Windows') {
  system('tests/run_CASAL.bat')
} else {
  system('cd tests && ./casal -r -q -O mpd.out 1> casal.out 2> casal.err')
}


#### get CASAL output and write to files ####
output <- extract.quantities('tests/casal.out')

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

write.table(rbind(B0, R0, SSB), 'tests/casal-estimates.txt', 
            row.names = F, quote = F, sep = '\t')


## plot SSBs from IBM and CASAL
biomass <- read.table('output/biomass.tsv', header = T, as.is = T)

stocks <- c('ENLD', 'HAGU', 'BOP')
areas <- c('EN', 'HG', 'BP')

pdf('tests/SSBs.pdf', width = 9, height = 7, pointsize = 11)
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


## load catch at length data from IBM
len <- read.table('output/monitor/length.tsv', header = F, as.is = T)
colnames(len) <- c('year', 'region', 'method', 'length', 'freq')
len$method <- ifelse(len$method == 'RE', 'REC', len$method)
len$fishery <- paste(len$region, len$method, sep = '_')


## plot catch at length by LL for 2004
ibm.cal <- subset(len, year == 2004 & method == 'LL' & length < 50)

tmp <- data.frame(fishery = rep(c('EN_LL', 'HG_LL', 'BP_LL'), each = 100), 
                        length = rep(0:99, times = 3), 
                        freq = unlist(c(output$EN_LL, output$HG_LL, output$BP_LL)))

tmp$len_bin <- floor(tmp$length/2)
casal.cal <- aggregate(freq ~ fishery + len_bin, data = tmp, sum)

## plot numbers at length
pdf('tests/catch at length.pdf', width = 9, height = 7, pointsize = 11)
par(mfrow = c(2, 2), mar = c(2, 2, 2, 2), oma = c(3, 3, 1, 1))

for (fish in unique(ibm.cal$fishery)) {
  sub.ibm <- subset(ibm.cal, fishery == fish)
  sub.casal <- subset(casal.cal, fishery == fish)
  plot(sub.ibm$length, sub.ibm$freq/sum(sub.ibm$freq), type = 'l', xlab = '', ylab = '', 
       ylim=c(0, 0.1))
  lines(sub.casal$len_bin, sub.casal$freq/sum(sub.casal$freq), lty = 2, col = 2)
  legend('topright', fish, bty = 'n', inset = 0.02)
  if (fish == 'EN_LL')
    legend('topleft', c('IBM CAL', 'CASAL CAL'), lty = 1:2, col = 1:2, 
           bty = 'n', inset = 0.02)
  }
mtext('Length (cm)', 1, outer = T, line = 1)
mtext('Frequency', 2, outer = T, line = 1)
dev.off()


## plot cumulative proportions at length
pdf('tests/catch at length cum prop.pdf', width = 9, height = 7, pointsize = 11)
par(mfrow = c(2, 2), mar = c(2, 2, 2, 2), oma = c(3, 3, 1, 1))

for (fish in unique(ibm.cal$fishery)) {
  sub.ibm <- subset(ibm.cal, fishery == fish)
  sub.ibm$prop <- cumsum(sub.ibm$freq)/sum(sub.ibm$freq)
  sub.casal <- subset(casal.cal, fishery == fish)
  sub.casal$prop <- cumsum(sub.casal$freq)/sum(sub.casal$freq)
  plot(sub.ibm$length, sub.ibm$prop, type = 'l', xlab = '', ylab = '', ylim=c(0, 1))
  lines(sub.casal$len_bin, sub.casal$prop, lty = 2, col = 2)
  legend('bottomright', fish, bty = 'n', inset = 0.02)
}
legend('topleft', c('IBM CAL', 'CASAL CAL'), lty = 1:2, col = 1:2, 
       bty = 'n', inset = 0.02)
mtext('Length (cm)', 1, outer = T, line = 1)
mtext('Cumulative proportional frequency', 2, outer = T, line = 1)
dev.off()
