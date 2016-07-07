# Routine to test agent based tagging process
# June 13th 2016 J McKenzie


#############
# FUNCTIONS #
#############               

#RMSD



RMSD<- function(x,y){
  
  
  CV<-round(sqrt( sum((x-y)^2)/length(y))/x,2)
  bias<-round((mean(y)/x)-1,2)
  return(cbind('true'=x,'est_mean'=round(mean(y),2),'bias'=bias,'RMCV'=CV,'obs'= length(y)))
}
# Resequence

reseq<- function(x){
  
  seq_ref<- 1
  
  for(i in 1:length(x[,1])){
    
    x[i,3]<-seq_ref
    seq_ref<-seq_ref+x[i,2]
    x[i,4]<-seq_ref-1
  }
  
  x[,1]<-c(1:length(x[,1]))
  return(x)
  
}

# point sequence

pointer<- function(x,y){

  point<-NULL
  for(i in x){
    
    point<-c(point,grep(1,(y[,3] <= i) * (y[,4] >= i)) )
    
    
  }
  point<-point[order(point)]
  point<-aggregate(point,list(point),length)
  colnames(point)<-c('point','freq') 
  return(point)
}







################
# Main Routine #
################

# Population size
pop = 100

# maximum agent scaler value
mscalr = 15


# Number of tags
tags = 10

# Number of catch
catch = 50

boot<- 1000
#######################

RunDet<-list()


RunDet[c('pop','mscalr','tags','catch','boot','runtime','stats')]<-c(pop,mscalr,tags,catch,boot,0,0)






pop_est<-NULL
startTime<-Sys.time()

for(bt in 1:boot){  
  # trap silly mscalar values
  if(mscalr >= pop ) mscalr <- pop-1
  if(mscalr <= 0 ) mscalr <- 1
  
  #### Create agent matrix
  agent_vec<- rep(mscalr,as.integer(pop/mscalr))
  if(pop %% mscalr > 0) agent_vec<-c(agent_vec,pop %% mscalr)
  
  agent_mat<-as.data.frame(cbind('agent'=1:length(agent_vec), 'number'=agent_vec, 'S_seq'=(c(0,cumsum(agent_vec)[1:(length(agent_vec)-1)]))+1,
                             'E_seq'=((c(0,cumsum(agent_vec)[1:(length(agent_vec)-1)]))+1)+(agent_vec-1), 'tag'=0))


  ##########################################
  
  # Tagging sequence
  
  tagseq<-sample(c(1:pop), tags, replace = FALSE, prob = NULL)
  
  point<-pointer(tagseq,agent_mat)

  for(i in 1:length(point[,1])){
  
      for(j in 1:point[i,2]){
        if(agent_mat[point[i,1],2]==1){
          agent_mat[point[i,1],5]= 1
        }else{
            zz<-agent_mat[point[i,1],]
            zz[,3]<-zz[,4]
            zz[,5]<-1
            zz[,2]<-1
            agent_mat[point[i,1],4]<- agent_mat[point[i,1],4]-1
            agent_mat[point[i,1],2]<- agent_mat[point[i,1],2]-1
            agent_mat<-rbind(agent_mat,zz)
        }
  
      }
      
  }
  
  agent_mat<-agent_mat[order(agent_mat[,4]),]
  
  agent_mat[,1]<-seq(1,length(agent_mat[,1]),1)
  
  pop<- sum(agent_mat[,2])
  tagsum<- sum(agent_mat[,5])
  

  #############################
  # Take catch routines 
  #############################
  

  # catch tag as individual  catch untagged as agent
  
  # recover tags
  
  catseq<-sample(c(1:pop), catch, replace = FALSE, prob = NULL)
  
  
  point_tag<-pointer(catseq,agent_mat)
  
  catch_tag<-NULL
  del_vec<-NULL
  for(i in 1:length(point_tag[,1])){
      if(agent_mat[point_tag[i,1],5]==1){
        catch_tag<-rbind(catch_tag,agent_mat[point_tag[i,1],])
        del_vec<-c(del_vec,-1*point_tag[i,1])
      }
  }
  norectags<- sum(catch_tag[,5])
  if(!is.null(del_vec)) agent_mat<-agent_mat[del_vec,]

  #print( catch_tag, row.names = F)
  
  # split out uncaught tagged fish
  Tags<- agent_mat[grep(1,agent_mat[,5]),]
  untagged<-agent_mat[grep(0,agent_mat[,5]),]
  untagged<-reseq(untagged)

  
  ############
  # untagged catch removal
  cat_untag<- catch-norectags
  while(cat_untag>0){
    catseq<-sample(sum(untagged[,2]), 1, replace = FALSE, prob = NULL)
    point<-pointer(catseq,untagged)
    cat<-untagged[point[1,1],2]
    if(cat_untag>=cat)
    {
      cat_untag<-cat_untag-cat
      untagged<-untagged[-point[1,1],]
      untagged<-reseq(untagged)
    }else{

      untagged[point[1,1],2]<-untagged[point[1,1],2]-cat_untag
      untagged[point[1,1],4]<-untagged[point[1,1],4]-cat_untag
      untagged<-reseq(untagged)
      cat_untag<-0
      
    }  
    
    
  }
  
  agent_mat<-reseq(rbind(Tags,untagged))
  # new_pop<-sum(agent_mat[,2])
  #stats
  tagrec<-sum(catch_tag[,5])
  pop_est<- rbind(pop_est,cbind('pop_est'= ((tags+1)*(catch+1)/(tagrec+1))-1, 'tagsrec'=tagrec)) # Petersen estimator with Chapman bias correction

  pop_est<-pop_est[order(pop_est[,2]),]
} #endboot
endTime<-Sys.time()
runTime<-endTime-startTime


# stats


RunDet[['runtime']]<-runTime
RunDet[['stats']]<-RMSD(pop,pop_est[,1])


RunDet









