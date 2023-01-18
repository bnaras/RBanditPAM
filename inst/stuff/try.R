library(BanditPAM)
library(MASS)
# Generate data from a Gaussian Mixture Model with the given means:
set.seed(10)
n_per_cluster <- 40

means <- list(c(0, 0), c(-5, 5), c(5, 5))

set.seed(129)
X <- do.call(rbind, lapply(means, mvrnorm, n = n_per_cluster, Sigma = diag(2)))

k <- 3
max_iter <- 100

build_confidence <- 1000
swap_confidence <- 10000
loss <- "L2"

obj <- BanditPAM:::.KMedoids__new(k, max_iter, build_confidence, swap_confidence)

BanditPAM:::.KMedoids__fit(obj, X, loss)
meds <- BanditPAM:::.KMedoids__get_medoids_final(obj)
plot(X[, 1], X[, 2])
points(X[meds, 1], X[meds, 2], col = "red", pch = 19)

## MNIST
k <- 10
obj <- BanditPAM:::.KMedoids__new(k, max_iter, build_confidence, swap_confidence)

d <- as.matrix(data.table::fread("~/tmp/BanditPAM/data/MNIST_100.csv"))


BanditPAM:::.KMedoids__fit(obj, d, loss)
BanditPAM:::.KMedoids__get_medoids_final(obj)
