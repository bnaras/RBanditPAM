// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>
using namespace Rcpp;


/**
 * @file main.cpp
 * @date 2020-06-10
 *
 * Defines a command line program that can be used
 * to run the BanditPAM KMedoids algorithm.
 *
 * Usage (from home repo directory):
 * ./src/build/BanditPAM -f [path/to/input] -k [number of clusters]
 */

#include <unistd.h>
#include <fstream>
#include <exception>
#include <filesystem>

#include "kmedoids_algorithm.hpp"

//' Create a new KMedoids object.
//'
//' @return an external ptr (Rcpp::XPtr) to a KMedoids object instance.
// [[Rcpp::export(.KMedoids__new)]]
SEXP KMedoids__new(IntegerVector k, IntegerVector max_iter, IntegerVector build_confidence, IntegerVector swap_confidence) {
  
  // create a pointer to an KMedoids object and wrap it
  // as an external pointer
  XPtr<km::KMedoids> ptr( new km::KMedoids((size_t) k[0], "BanditPAM", (size_t) max_iter[0], (size_t) build_confidence[0], (size_t) swap_confidence[0]), true );
  // return the external pointer to the R side
  return ptr;
}

//' Fit the KMedoids algorthm given the data and loss
//'
//' @param xp the km::KMedoids Object XPtr
//' @param data the data matrix
//' @param loss the loss indicator
// [[Rcpp::export(.KMedoids__fit)]]
void KMedoids__fit(SEXP xp, arma::mat data, std::vector< std::string > loss ) {
  // grab the object as a XPtr (smart pointer)
  XPtr<km::KMedoids> ptr(xp);
  ptr->fit(data, loss[0]);
}

//' Return the final medoids
//'
//' @param xp the km::KMedoids Object XPtr
// [[Rcpp::export(.KMedoids__get_medoids_final)]]
SEXP KMedoids__get_medoids_final(SEXP xp) {
  // grab the object as a XPtr (smart pointer)
  XPtr<km::KMedoids> ptr(xp);
  return wrap(ptr->getMedoidsFinal());
}
