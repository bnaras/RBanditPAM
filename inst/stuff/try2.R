library(BanditPAM)
library(MASS)

## MNIST
k <- 10

obj <- KMedoids$new(k = k)

d <- as.matrix(data.table::fread("~/tmp/BanditPAM/data/MNIST_1000.csv"))

set.seed(1783)
obj$fit(data = d, loss = "l2")
obj$get

obj <- (k, max_iter, build_confidence, swap_confidence)

d <- as.matrix(data.table::fread("~/tmp/BanditPAM/data/MNIST_100.csv"))


## MNIST
k <- 10
obj <- BanditPAM:::.KMedoids__new(k, max_iter, build_confidence, swap_confidence)




BanditPAM:::.KMedoids__fit(obj, d, loss)
BanditPAM:::.KMedoids__get_medoids_final(obj)



BanditPAM:::.KMedoids__fit(obj, d, loss)
BanditPAM:::.KMedoids__get_medoids_final(obj)
