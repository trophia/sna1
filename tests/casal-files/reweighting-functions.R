#################################################################################################

#                 FRANCIS METHOD FOR REWEIGHTING MULTINOMIAL DATA SETS FOR INPUT TO CASAL CSL FILES 
# ALLOWING FOR AUTOCORELATION (equation TA 1.8 Francis 2011) 
# This script uses Chris Francis's iterative reweighting process for optimisation of multinomial 
# error after first fitting the model with using unscaled multinomial errors.
# After Francis 2011 Data weighting in statistical fisheries stock assessment models Can J Fish 
# Aquat Sci 68 p1124-1138

# The code produces an output file "AdjustedMN.txt" containing summary stats and a table of 
# reweighted multinomail sample sizes in a form suitable to be pasted back into CASAL.


#################################################################################################
francis.reweighting <- function(mpd.file, dsets) {
  mpd<-extract.mpd(mpd.file)
  
  # dsets<-cbind(Tables=names(mpd$fits[grep("len",names(mpd$fits))]))
  
  weightings <- NULL
  
  write("                 Adjusted Multinomial sample sizes","AdjustedMN.txt",ncol=2,append = FALSE)
  write(" ","AdjustedMN.txt",ncol=2,append = T)
  write(" ","AdjustedMN.txt",ncol=2,append = T)
  
  for(ns in 1:length(dsets)) {
    
    Mtables<-cbind(Tables=as.vector(dsets[ns]))
    
    for(i in 1:length(Mtables[,1])) {
      
      name<-paste("mpd$fits$",as.character(Mtables[i,]),sep = "")
      x <- eval(parse(text = name))$obs
      Fintadd <- matrix(as.numeric(rownames(x)), nrow = nrow(x), ncol=1)
      colnames(Fintadd)<-c("Year")
      rownames(Fintadd)<-  replicate(nrow(x),as.character(Mtables[i,]))
      
      ages<-as.numeric(substr(names(x),2,max(nchar(names(x)))))
      mdat <- t(matrix(ages, nrow = ncol(x), ncol=nrow(x)))
      Obsrv<- t(rowsum(t(x*mdat),replicate(ncol(x),nrow(x))))
      colnames(Obsrv)<-c("Obs")
      
      Fintable <- NULL
      x <- eval(parse(text = name))$fits
      ages<-as.numeric(substr(names(x),2,max(nchar(names(x)))))
      mdat <- t(matrix(ages, nrow = ncol(x), ncol=nrow(x)))
      Expd<- t(rowsum(t(x*mdat),replicate(ncol(x),nrow(x))))
      vary  <- t(rowsum(t(x*mdat^2),replicate(ncol(x),nrow(x))))
      vary<- vary-(Expd^2)
      colnames(Expd)<-c("Exp") 
      colnames(vary)<-c("vyb") 
      mlt <- eval(parse(text = name))$error.value
      Fintadd <- cbind(Fintadd, Obsrv, Expd, vary, mult = mlt[,1])
      Fintable <- rbind(Fintable,Fintadd)  
    }
    
    fnames<-rownames(Fintable)
    rownames(Fintable)<-NULL 
    Fintable<-as.data.frame(Fintable)
    Fintable<-cbind(Fintable, datasets=fnames )
    
    unscaledVar<- with(Fintable, var((Obs-Exp)/sqrt(vyb/mult), na.rm = T))
    
    scalar <- 1/unscaledVar
    scaledVar<- with(Fintable, round(var((Obs-Exp)/sqrt(vyb/(mult*scalar))), 8))
    
    
    Fintable<-cbind(Fintable, Var_unscl = with(Fintable, (Obs-Exp)/sqrt(vyb/mult)), 
                    Var_scl = with(Fintable, (Obs-Exp)/sqrt(vyb/(mult*scalar))))
    
    outfile<- as.data.frame(matrix(fnames, nrow = nrow(Fintable), ncol=1))
    colnames(outfile)<-c("Dataset")
    
    outfile<-cbind(outfile, Year = paste("N_", Fintable$Year, sep = ""), 
                   newMult = Fintable$mult*scalar)
    
    weightings <- rbind(weightings, outfile)
    
    write(" ","AdjustedMN.txt",ncol=2,append = T)
    write(" ","AdjustedMN.txt",ncol=2,append = T)  
    write(paste('DATASET   ',as.vector(Mtables[1,1])),"AdjustedMN.txt",append = T)
    write(" ","AdjustedMN.txt",ncol=2,append = T)
    write(" ","AdjustedMN.txt",ncol=2,append = T)
    
    write(t(rbind(c("unscaled variance"),as.numeric(unscaledVar))),"AdjustedMN.txt",ncol=2,append = T)
    write(t(rbind(c("scaled variance"),as.numeric(scaledVar))),"AdjustedMN.txt",ncol=2,append = T)
    write(" ","AdjustedMN.txt",ncol=2,append = T)
    write(t(rbind(c("Optimised scalar"),as.numeric(scalar))),"AdjustedMN.txt",ncol=2,append = T)
    
    write(" ","AdjustedMN.txt",ncol=2,append = T)
    write(" ","AdjustedMN.txt",ncol=2,append = T)
    write("          Optimisation variance table ","AdjustedMN.txt",ncol=2,append = T)
    write(" ","AdjustedMN.txt",ncol=2,append = T)
    
    write(colnames(Fintable),"AdjustedMN.txt",ncol=8,append = T,sep = "     ")
    write(t(as.matrix(Fintable)),"AdjustedMN.txt",ncol=8,append = T,sep = "\t")
    write(" ","AdjustedMN.txt",ncol=2,append = T)
    write(" ","AdjustedMN.txt",ncol=2,append = T)
    write("          Adjusted Multinomial sample size table ","AdjustedMN.txt",ncol=2,append = T)
    write(" ","AdjustedMN.txt",ncol=2,append = T)
    write(colnames(outfile),"AdjustedMN.txt",ncol=3,append = T,sep = "     ")
    write(t(as.matrix(outfile)),"AdjustedMN.txt",ncol=3,append = T,sep = "\t")
    write(" ","AdjustedMN.txt",ncol=2,append = T)
    write("_________________________________________________________________________________________________","AdjustedMN.txt",ncol=2,append = T)
    ##############################
  }
  return(weightings)
}


## this function is to over-write the function in casal package
## fix the problem of dealing with emply or comment lines
extract.csl.file <- function (file, path = "") 
{
  csl.commands <- csl.commands
  csl.commands$count <- rep(0, length(csl.commands$command))
  set.class <- function(object, new.class) {
    attributes(object)$class <- c(new.class, attributes(object)$class[attributes(object)$class != 
                                                                        new.class])
    object
  }
  if (missing(path)) 
    path <- ""
  filename <- casal.make.filename(path = path, file = file)
  res <- casal.convert.to.lines(filename)
  while (any(casal.regexpr(" ", res) == 1)) {
    index <- casal.regexpr(" ", res) == 1
    res <- ifelse(index, substring(res, 2), res)
  }
  res <- res[substring(res, 1, 1) != "#"]
  index1 <- ifelse(substring(res, 1, 1) == "{", 1:length(res), 
                   0)
  index2 <- ifelse(substring(res, 1, 1) == "}", 1:length(res), 
                   0)
  index1 <- index1[index1 != 0]
  index2 <- index2[index2 != 0]
  if (length(index1) != length(index2)) 
    stop(paste("Error in the file ", filename, ". Cannot find a matching '{' or '}'", 
               sep = ""))
  if (length(index1) > 0 || length(index2) > 0) {
    index <- unlist(apply(cbind(index1, index2), 1, function(x) seq(x[1], 
                                                                    x[2])))
    res <- res[!casal.isin(1:length(res), index)]
  }
  res <- ifelse(casal.regexpr("#", res) > 0, substring(res, 
                                                       1, casal.regexpr("#", res) - 1), res)
  res <- res[res != ""]
  if (substring(res[1], 1, 1) != "@") 
    stop(paste("Error in the file ", filename, ". Cannot find a '@' at the begining of the file", 
               sep = ""))
  res <- as.vector(tapply(res, 1:length(res), function(x) {
    tmp <- unlist(casal.unpaste(x, sep = "\t"))
    tmp <- tmp[!casal.isin(tmp, c("", " ", "\t"))]
    return(as.vector(paste(tmp, collapse = " ")))
  }))
  res <- as.vector(tapply(res, 1:length(res), function(x) {
    tmp <- unlist(casal.unpaste(x, sep = " "))
    tmp <- tmp[!casal.isin(tmp, c("", " ", "\t"))]
    return(as.vector(paste(tmp, collapse = " ")))
  }))
  ans <- list()
  print(paste("The 'csl' input parameter file has", length(res[substring(res, 
                                                                         1, 1) == "@"]), "commands, and", length(res), "lines"))
  CommandCount <- 0
  for (i in 1:length(res)) {
    temp <- casal.string.to.vector.of.words(res[i])
    if (!length(temp)) next
    if (substring(temp[1], 1, 1) == "@") {
      CommandCount <- CommandCount + 1
      Command <- substring(temp[1], 2)
      if (length(csl.commands$type[csl.commands$command == 
                                   Command]) > 0) {
        if (csl.commands$type[csl.commands$command == 
                              Command][1] == "autonumber") {
          counter <- csl.commands$count[csl.commands$command == 
                                          Command] + 1
          csl.commands$count[csl.commands$command == 
                               Command] <- counter
          ans[[paste(Command, "[", counter, "]", sep = "")]] <- list(command = Command, 
                                                                     value = temp[-1])
        }
        else if (csl.commands$type[csl.commands$command == 
                                   Command][1] == "argument") {
          ans[[Command]] <- list(command = Command, value = temp[-1])
        }
        else if (csl.commands$type[csl.commands$command == 
                                   Command][1] == "label") {
          ans[[paste(Command, "[", temp[2], "]", sep = "")]] <- list(command = Command, 
                                                                     value = temp[-1])
        }
        else if (csl.commands$type[csl.commands$command == 
                                   Command][1] == "labelifrepeated") {
          if (length(temp) > 1) 
            ans[[paste(Command, "[", temp[2], "]", sep = "")]] <- list(command = Command, 
                                                                       value = temp[-1])
          else ans[[Command]] <- list(command = Command, 
                                      value = temp[-1])
        }
        else if (csl.commands$type[csl.commands$command == 
                                   Command][1] == "nolabel") {
          ans[[Command]] <- list(command = Command, value = temp[-1])
        }
        else {
          print(paste("Warning: Unknown CASAL command '", 
                      Command, "' found in '", filename, "'\n", 
                      sep = ""))
          ans[[Command]] <- list(command = Command, value = temp[-1])
        }
      }
      else {
        print(paste("Warning: Unknown CASAL command '", 
                    Command, "' found in '", filename, "'\n", sep = ""))
        ans[[Command]] <- list(command = Command, value = temp[-1])
      }
    }
    else {
      ans[[CommandCount]][[temp[1]]] <- temp[-1]
    }
  }
  ans <- set.class(ans, "casalCSLfile")
  return(ans)
}

update.csl <- function(csl.prefix) {
  ## estimation csl file
  est.csl <- paste(csl.prefix, 'estimation.csl', sep = '')
  
  ## load estimation csl file
  estimation <- extract.csl.file(est.csl)
  
  ## modify weightings
  for (obs in obs.names) {
    sub.weight <- subset(weightings, Dataset == obs)
    i <- grep(obs, names(estimation))
    for (y in sub.weight$Year) {
      j <- grep(y, names(estimation[[i]]))
      estimation[[i]][[j]] <- round(sub.weight$newMult[sub.weight$Year == y], 6)
    }
  }
  
  ## write the updated estimation in to csl file
  write.csl.file(estimation, est.csl)
}


