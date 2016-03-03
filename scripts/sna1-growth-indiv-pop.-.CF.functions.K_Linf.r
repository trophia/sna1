### Confidence interval calc function
ci<- function(x,ci) {
  
  x<-x[order(x)]
  y<-(1-ci)/2
  lci <-round(length(x)*y,0)
  uci<- round(length(x)*(1-y),0)
  return(c(x[lci],x[uci]))
}
####################################




# A test of alternative formulations for growth
# 
# How to allow for individual variability in growth and still maintain population level growth pars


# Population level growth pars
k <- 0.1
linf <- 60
#number of annual time steps in annual cycle
steps=1


t=1/steps
#max age
max_age<-80

ages= seq(0,max_age,t)


# Simulate a population of fish with different g1 and g2s
n <- 1000
cv <- 0.2


## need to generate k and Linf as longnormal random deviates assumes k comes from a lognormal distribution with mean = k and variance = v  
# in log space log(k) come from a normal distribution with mean = 'u' and standard deviation 's' calculated as follows:


v=(k*cv)^2 #varaiance of k

u = log(k^2/sqrt(v +k^2)) # mean of log(k)
s= sqrt(log(v/k^2 +1)) # stdev of log(k)

Ks <-exp(rnorm(n,u,s)) # k lognormal random deviates

#Test you get log mean and stdev back for K 
nsamp = 10000000
Ktest<- exp(rnorm(nsamp,u,s))
mean(Ktest) #mean
sd(Ktest)/mean(Ktest) #cv


# Linf
v=(linf*cv)^2 #varaiance of Linf

u = log(linf^2/sqrt(v +linf^2))  # mean of log(Linf)
s= sqrt(log(v/linf^2 +1))
Linfs <-exp(rnorm(n,u,s)) # linf lognormal random deviates



#Test you get log mean and stdev back for  Linf
Ltest<- exp(rnorm(nsamp,u,s))
mean(Ltest) #mean
sd(Ltest)/mean(Ltest)  #cv

#generate starting survey bootstraps



# For each fish calculate simulate growth
fishes <- NULL


for(fish in 1:n){
  # Calculate a and b for this fish
  

  b<- -(1-exp(-Ks[fish]*t))

  a <- -Linfs[fish]*b
  
  # Simulate growth
  lengths <- 0
  for(age in ages[-1]) {
    length_now <- lengths[length(lengths)]

      length_incr <- a + b*length_now
      length_now <- length_now + length_incr
    lengths <- c(lengths, length_now)
  }
  fishes <- if(is.null(fishes)) lengths else rbind(fishes,lengths)
}



# Calculate distributions at age





length_mean_age<-apply(fishes,2,mean)
#95% CI of boots
lci<- function(x) return(ci(x,0.95)[1])
uci<- function(x) return(ci(x,0.95)[2])
upper_sd<- apply(fishes,2,uci)
lower_sd<-apply(fishes,2,lci)

par(mfrow=c(2,1))
# Calculate vonB lengths at age
length_vb_age <- linf * (1-exp(-k*ages))
par(mfrow=c(2,1))
plot(ages,length_mean_age,ylab='Mean length (cm)',xlab='Age (y)',ylim=c(0,100))
segments(ages,lower_sd ,ages,upper_sd)
lines(ages,length_vb_age, col="blue",lwd=2)



# Plot lengths at age
plot(ages,rep(NA,length(ages)),ylim=c(0,150))
for(fish in 1:n) lines(ages,fishes[fish,],col=hsv(0,0,0,0.2))
lines(ages,length_vb_age, col="blue",lwd=2)


# Histograms of length at age
par(mfrow=c(3,1))
hist(fishes[,(5*steps+1)],breaks=30,main='5+',xlab='Length (cm)')
hist(fishes[,(10*steps+1)],breaks=30,main='10+',xlab='Length (cm)')
hist(fishes[,(25*steps+1)],breaks=30,main='25+',xlab='Length (cm)')
