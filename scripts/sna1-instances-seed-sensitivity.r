library(ggplot2)
library(plyr)
library(reshape)

data <- read.table('output/instances_seed_sensitivity.tsv')
times <- read.table('output/instances_seed_sensitivity_times.tsv')

names(data) <- c('seed','iter','time','biomass_spawner','length_mean')
data <- within(data,{
  seed <- factor(seed)
  iter <- factor(iter)  
})

names(times) <- c('seed','iter','duration')

ggplot(data,aes(x=time,group=iter)) +
  geom_line(aes(y=biomass_spawner/1000),alpha=0.2) +
  geom_hline(y=0,alpha=0) +
  facet_wrap(~seed) + 
  labs(x='',y='Spawner biomass (kt)')

ggplot(data,aes(x=time,group=iter)) +
  geom_line(aes(y=length_mean),alpha=0.2) +
  geom_hline(y=0,alpha=0) +
  facet_wrap(~seed) + 
  labs(x='',y='Mean length (cm)')

cv <- function(x) sd(x)/mean(x)
cvs <- ddply(data,.(seed),function(sub){
  data.frame(
    biomass_spawner = cv(subset(sub,time==2015)$biomass_spawner),
    length_mean = cv(subset(sub,time==2015)$length_mean)
  )
})
cvs <- merge(cvs, ddply(times,.(seed),summarise,duration=mean(duration)))

ggplot(melt(cvs,id.vars='seed'),aes(x=seed,y=value)) +
  geom_point(size=3,pch=1) + 
  facet_wrap(~variable,scales='free_y') + 
  labs(x='Seed instances',y='c.v.(Value in 2015)')

ggplot(times,aes(x=seed)) + 
  geom_point(aes(y=duration),size=3,alpha=0.3) + 
  scale_y_log10("Duration (s)",breaks=c(0.01,0.1,1,10)) + 
  scale_x_log10("Seed instances")

ggplot(cvs,aes(x=biomass_spawner,y=duration)) + 
  geom_text(aes(label=seed)) + 
  #scale_y_log10() + 
  #scale_x_log10() +
  labs(x='CV(Spawner biomass 2015)',y='Duration (s)')

