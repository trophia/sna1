# A test of alternative formulations for growth
# 
# How to allow for individual variability in growth and still maintain population level growth pars


# Population level growth pars
k <- 0.1
linf <- 60

# Convert to slope and intercept
b <- -k
a <- -linf*b
plot(0:100,a+b*(0:100),type='l',ylab='Length increment (cm)',xlab='Length (cm)')
abline(h=0)

# Convert to g1 and g2
l1 <- 20
l2 <- 40
g1 <- a + b*l1
g2 <- a + b*l2

# Convert back to k and Linf as a check
k_ <- -(g1-g2)/(l1-l2)
linf_ <- (l2*g1-l1*g2)/(g1-g2)
if(k_!=k & linf_!=linf) stop('Back conversion to k and Linf not correct')


# Simulate a population of fish with different g1 and g2s
n <- 1000
cv <- 0.1
g1s <- rnorm(n,g1,g1*cv)
g2s <- rnorm(n,g2,g2*cv)

# For each fish calculate simulate growth
fishes <- NULL
ages <- 0:50
steps <- 4
for(fish in 1:n){
  # Calculate a and b for this fish
  b <- (g1s[fish]-g2s[fish])/(l1-l2)
  a <- g1s[fish]-(b*l1)
  # Simulate growth
  lengths <- 0
  for(age in ages[-1]) {
    length_now <- lengths[length(lengths)]
    for(step in 1:steps) {
      length_incr <- max(0,a + b*length_now)/steps
      length_now <- length_now + length_incr
    }
    lengths <- c(lengths, length_now)
  }
  fishes <- if(is.null(fishes)) lengths else rbind(fishes,lengths)
}

# Plot lengths at age
plot(ages,rep(NA,length(ages)),ylim=c(0,80))
for(fish in 1:n) lines(fishes[fish,],col=hsv(0,0,0,0.2))

# Calculate distributions at age
length_mean_age <- apply(fishes,2,mean)
length_sd_age <- apply(fishes,2,sd)

# Calculate vonB lengths at age
length_vb_age <- linf * (1-exp(-k*ages))

plot(ages,length_mean_age,ylab='Mean length (cm)',xlab='Age (y)')
segments(ages,length_mean_age-length_sd_age,ages,length_mean_age+length_sd_age)
lines(ages,length_vb_age)

# Histograms of length at age
par(mfrow=c(3,1))
hist(fishes[,5],breaks=30,main='5+',xlab='Length (cm)')
hist(fishes[,10],breaks=30,main='10+',xlab='Length (cm)')
hist(fishes[,25],breaks=30,main='25+',xlab='Length (cm)')
