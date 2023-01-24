#' Create a new KMedoids object
#' @param k number of medoids to use and clusters to create, default 5
#' @param max_iter the maximum number of SWAP steps the algorithm runs, default 1000
#' @param build_conf parameter that affects the width of BUILD confidence intervals, default 1000
#' @param swap_conf parameter that affects the width of SWAP confidence intervals, default 10000
#' @return a KMedoids object which can be used to fit the BanditPAM algorithm to data
#' @export k_medoids
k_medoids <- function(k = 5L, max_iter = 1000L, build_conf = 1000, swap_conf = 10000L) {
  xptr <- .Call('_BanditPAM_KMedoids__new', PACKAGE = 'BanditPAM', k, max_iter, build_conf, swap_conf)
  obj <- list(xptr = xptr, k = k, max_iter = max_iter, build_conf = build_conf, swap_conf = swap_conf)
  class(obj) <- "KMedoid"
}

#' Get the number of medoids specified for the object
#' @param x the k_medoid object
#' @param ... further arguments
#' @export
get_k <- function(x, ...) {
  UseMethod("get_k", x)
}

#' Get the number of medoids specified for the object
#' @param x the k_medoid object
#' @method get_k KMedoid
#' @export
get_k.KMedoid <- function(x) {
  invisible(.Call('_BanditPAM_KMedoids__get_k', PACKAGE = 'BanditPAM', x$xptr))
}

#' Fit clustering method to data
#' @param x the k_medoid object
#' @param data the data matrix
#' @param loss the loss function
#' @param ... further arguments
fit <- function(x, data, loss, ...) {
  UseMethod("fit", x)
}

#' Fit the KMedoids algorthm given the data and loss
#' @param x the k_medoid object
#' @param data the data matrix
#' @param loss the loss function, either a positive integer p (indicating L_p  loss) or one of "manhattan", "cosine", "inf" or "euclidean"
#' @method fit KMedoid
#' @export
fit.KMedoid <- function(x, data, loss) {
  if (is.numeric(loss)) {
    p <- as.integer(loss)
    if ((p <= 0) || is.na(p)) stop("L_p loss should be indicated with appropriate value for p > 0!")
    loss <- paste0("l", p)
  } else {
    loss <- match.arg(loss, c("manhattan", "cosine", "inf", "euclidean"))
  }
  invisible(.Call('_BanditPAM_KMedoids__fit', PACKAGE = 'BanditPAM', x$xptr, data, loss))
}

#' Return final mediod indices after clustering
#' @param x the k_medoid object
#' @param data the data matrix
#' @param loss the loss function
#' @param ... further arguments
#' @export
get_medoids_final <- function(x, ...) {
  UseMethod("get_medoids_final", x)
}

#' Return the final medoid indices after clustering
#' @param x the k_medoid object
#' @method get_medoids_final KMedoid
#' @export
get_medoids_final.KMedoid <- function(x) {
    .Call('_BanditPAM_KMedoids__get_medoids_final', PACKAGE = 'BanditPAM', x$xptr)
}

