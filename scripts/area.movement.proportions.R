## R script for getting movement model for 9 areas to 9 areas in 3 stocks
## (3 areas in each stock) to make the stock movement as close as possible
## the model assumes the movement proportion from area i to j 
## when j != i, sigma^(abs(i - j)*power) 
## when j == i, 1 - sum of all proportions k, k = 1, 2, ... i - 1, i + 1, ... 9

## function for the sigma and power search
func <- function(p, stock) {
  sigma = p[1]
  power = p[2]
  for (area in unique(sub.area.move$from_area)) {
    sub.area.move$move_prop[sub.area.move$from_area == area] <- 
      with(sub.area.move[sub.area.move$from_area == area, ], 
           ifelse(from_area != to_area, 
                  sigma^(abs(from_area - to_area)*power), 
                  2 - sum(sigma^(abs(from_area - to_area)*power))))
  }
  sub.area.move$move <- with(sub.area.move, alloc*move_prop)
  est = aggregate(move ~ to_stock, data = sub.area.move, sum)
  obs <- stock.move[stock.move$from_stock == stock, ]
  obs$est <- est$move[match(obs$to_stock, est$to_stock)]
  obs$var2 <- (obs$move - obs$est)^2
  sum(obs$var2)
}


## stock movement published in FAR
## we want our aggregated area movement to be close to this as much as possible
stock.move <- data.frame(from_stock = rep(c('ENLD', 'HAGU', 'BPLE'), each = 3), 
                         to_stock = rep(c('ENLD', 'HAGU', 'BPLE'), times = 3), 
                         move = c(0.94, 0.04, 0.02, 0.07, 0.89, 0.04, 0.03, 0.27, 0.7), 
                         stringsAsFactors = F)


## define the data frame for area movement 
area_stock <- c('ENLD', 'ENLD', 'ENLD', 'HAGU', 'HAGU', 'HAGU', 'BPLE', 'BPLE', 'BPLE')
area.move <- data.frame(from_area = rep(1:9, each = 9), to_area = rep(1:9, times = 9))
area.move$from_stock <- area_stock[area.move$from_area]
area.move$to_stock <- area_stock[area.move$to_area]

## proportional biomass allocation in areas within each stock now all 1/3
area.move$alloc <- 1/3

## search for the best sigma and power
ests <- NULL
for (stock in c('ENLD', 'HAGU', 'BPLE')) {
  sub.area.move <- subset(area.move, from_stock == stock)
  par <- nlm(func, c(0.12, 1), stock)
  ests <- rbind(ests, data.frame(stock = stock, sigma = par$estimate[1], power = par$estimate[2]))
}

## est sigma and power for BPLE is not sensible, 
## so assign a value to sigma and power for BPLE
ests$sigma[ests$stock == 'BPLE'] <- 0.22
ests$power[ests$stock == 'BPLE'] <- 0.7


## get area movement model
for (stock in c('ENLD', 'HAGU', 'BPLE')) {
  sigma <- ests$sigma[ests$stock == stock]
  power <- ests$power[ests$stock == stock]
  for (area in unique(area.move$from_area)) {
    area.move$prop[area.move$from_stock == stock & area.move$from_area == area] <- 
      with(area.move[area.move$from_stock == stock & area.move$from_area == area, ], 
           ifelse(from_area != to_area, 
                  sigma^(abs(from_area - to_area)*power), 
                  2 - sum(sigma^(abs(from_area - to_area)*power))))
  }
}

## cal stock movement from the area movement
area.move$move <- with(area.move, alloc*prop)
est.stock.move = aggregate(move ~ from_stock + to_stock, data = area.move, sum)
est.stock.move$from_stock <- factor(est.stock.move$from_stock, levels = c('ENLD', 'HAGU', 'BPLE'))
est.stock.move <- est.stock.move[order(est.stock.move$from_stock), ]

with(area.move, tapply(move, list(from_stock, to_stock), sum))

## manually change areas 7, 8 and 9 in BPLE 
## divid by 2 for the move to jasent area and add half to the stay
## area 7
area.move$prop[area.move$from_area == 7 & area.move$to_area == 7] <- 
  area.move$prop[area.move$from_area == 7 & area.move$to_area == 7] + 
  area.move$prop[area.move$from_area == 7 & area.move$to_area == 6]

area.move$prop[area.move$from_area == 7 & area.move$to_area == 6] <- 
  area.move$prop[area.move$from_area == 7 & area.move$to_area == 6]/2

area.move$prop[area.move$from_area == 7 & area.move$to_area == 8] <- 
  area.move$prop[area.move$from_area == 7 & area.move$to_area == 8]/2

## area 8
area.move$prop[area.move$from_area == 8 & area.move$to_area == 8] <- 
  area.move$prop[area.move$from_area == 8 & area.move$to_area == 8] + 
  area.move$prop[area.move$from_area == 8 & area.move$to_area == 7]

area.move$prop[area.move$from_area == 8 & area.move$to_area == 7] <- 
  area.move$prop[area.move$from_area == 8 & area.move$to_area == 7]/2

area.move$prop[area.move$from_area == 8 & area.move$to_area == 9] <- 
  area.move$prop[area.move$from_area == 8 & area.move$to_area == 9]/2

## area 9
area.move$prop[area.move$from_area == 9 & area.move$to_area == 9] <- 
  area.move$prop[area.move$from_area == 9 & area.move$to_area == 9] + 
  area.move$prop[area.move$from_area == 9 & area.move$to_area == 8]/2

area.move$prop[area.move$from_area == 9 & area.move$to_area == 8] <- 
  area.move$prop[area.move$from_area == 9 & area.move$to_area == 8]/2


## calculate and display stock move again
area.move$move <- with(area.move, alloc*prop)
est.stock.move <- est.stock.move[order(est.stock.move$from_stock), ]
with(area.move, tapply(move, list(from_stock, to_stock), sum))

with(area.move, tapply(prop, list(from_area, to_area), sum))


