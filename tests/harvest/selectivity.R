library(ggplot2)

props <- merge(
  read.table('selectivity_pars.tsv', header=T), 
  read.table('selectivity_at_length.tsv', header=T), 
  by = 'method'
)
props <- within(props, {
  length <- (length + 0.5) * 2
  expect <- ifelse(
    length <= mode,
    2^(-((length-mode)/steep1)^2),
    2^(-((length-mode)/steep2)^2)
  )
})

png('selectivity.png')
print(ggplot(props, aes(x=length)) + 
  geom_line(aes(y=value, colour='Produced'), linetype=1, alpha=0.5) + 
  geom_line(aes(y=expect, colour='Expected'), linetype=2, alpha=0.5) + 
  facet_wrap(~method) +
  labs(x='Length (cm)', y='Selectivity', colour=''))
dev.off()

cat(with(props, sum(abs(expect-value))),file='selectivity-diff.txt')
