/**
 * @file kmedoids_algorithm.cpp
 * @date 2020-06-10
 *
 * Contains the primary C++ implementation of the BanditPAM code.
 */

#include <omp.h>
#include <RcppArmadillo.h>
#include <unordered_map>
#include <regex>

#include "kmedoids_algorithm.hpp"
#include "fastpam1.hpp"
#include "pam.hpp"
#include "banditpam.hpp"

namespace km {
KMedoids::KMedoids(
  size_t nMedoids,
  const std::string& algorithm,
  size_t maxIter,
  size_t buildConfidence,
  size_t swapConfidence,
  size_t seed):
    nMedoids(nMedoids),
    algorithm(algorithm),
    maxIter(maxIter),
    buildConfidence(buildConfidence),
    swapConfidence(swapConfidence),
    seed(seed) {
  KMedoids::checkAlgorithm(algorithm);

  // Though we initialize seed from the given parameter,
  // we need to call setSeed to pass it to arma
  KMedoids::setSeed(seed);
}

KMedoids::~KMedoids() {}

void KMedoids::fit(const arma::mat& inputData, const std::string& loss) {
  batchSize = fmin(inputData.n_rows, batchSize);

  if (inputData.n_rows == 0) {
    throw std::invalid_argument("Dataset is empty");
  }
  try {
    KMedoids::setLossFn(loss);
    if (algorithm == "PAM") {
      static_cast<PAM*>(this)->fitPAM(inputData);
    } else if (algorithm == "BanditPAM") {
      static_cast<BanditPAM*>(this)->fitBanditPAM(inputData);
    } else if (algorithm == "FastPAM1") {
      static_cast<FastPAM1*>(this)->fitFastPAM1(inputData);
    }
  } catch (std::invalid_argument& e) {
    std::cout << e.what() << std::endl;
    std::cout << "Error: Clustering did not run." << std::endl;
    throw e;
  }
}

arma::urowvec KMedoids::getMedoidsBuild() const {
  return medoidIndicesBuild;
}

arma::urowvec KMedoids::getMedoidsFinal() const {
  return medoidIndicesFinal;
}

arma::urowvec KMedoids::getLabels() const {
  return labels;
}

size_t KMedoids::getSteps() const {
  return steps;
}

size_t KMedoids::getNMedoids() const {
  return nMedoids;
}

void KMedoids::setNMedoids(size_t newNMedoids) {
  nMedoids = newNMedoids;
}

std::string KMedoids::getAlgorithm() const {
  return algorithm;
}

void KMedoids::setAlgorithm(const std::string& newAlgorithm) {
  algorithm = newAlgorithm;
  KMedoids::checkAlgorithm(algorithm);
}

size_t KMedoids::getMaxIter() const {
  return maxIter;
}

void KMedoids::setMaxIter(size_t newMaxIter) {
  maxIter = newMaxIter;
}


size_t KMedoids::getBuildConfidence() const {
  return buildConfidence;
}

void KMedoids::setBuildConfidence(size_t newBuildConfidence) {
  if (algorithm != "BanditPAM") {
    // TODO(@motiwari): Better error type
    throw "Cannot set buildConfidence when not using BanditPAM";
  }
  buildConfidence = newBuildConfidence;
}

size_t KMedoids::getSwapConfidence() const {
  return swapConfidence;
}

void KMedoids::setSwapConfidence(size_t newSwapConfidence) {
  if (algorithm != "BanditPAM") {
    // TODO(@motiwari): Better error type
    throw "Cannot set buildConfidence when not using BanditPAM";
  }
  swapConfidence = newSwapConfidence;
}

void KMedoids::setSeed(size_t newSeed) {
  seed = newSeed;
  // In RcppArmadillo, the seed is set in R via set.seed(seed)!
  // arma::arma_rng::set_seed(seed);
}

size_t KMedoids::getSeed() const {
  return seed;
}

void KMedoids::setLossFn(std::string loss) {
  // TODO(@motiwari): On setting this, clear the cache and the average loss,
  // assignments, medoids, etc.
  std::for_each(loss.begin(), loss.end(), [](char& c){
    c = ::tolower(c);
  });
  // TODO(@motiwari): Change this to a switch
  if (std::regex_match(loss, std::regex("l\\d*"))) {
    lossFn = &KMedoids::LP;
    lp = stoi(loss.substr(1));
  } else if (loss == "manhattan") {
    lossFn = &KMedoids::manhattan;
  } else if (loss == "cos" || loss == "cosine") {
    lossFn = &KMedoids::cos;
  } else if (loss == "inf") {
    lossFn = &KMedoids::LINF;
  } else if (loss == "euclidean") {
    lossFn = &KMedoids::LP;
    lp = 2;
  } else {
    throw std::invalid_argument("Error: unrecognized loss function");
  }
}

std::string KMedoids::getLossFn() const {
  // TODO(@motiwari): make the strings constants
  if (lossFn == &KMedoids::manhattan) {
      return "manhattan";
  } else if (lossFn == &KMedoids::cos) {
    return "cosine";
  } else if (lossFn == &KMedoids::LINF) {
    return "L-infinity";
  } else if (lossFn == &KMedoids::LP) {
    return "L" + std::to_string(lp);
  } else {
    throw std::invalid_argument("Error: Loss Function Undefined!");
  }
}


void KMedoids::calcBestDistancesSwap(
  const arma::mat& data,
  const arma::urowvec* medoidIndices,
  arma::rowvec* bestDistances,
  arma::rowvec* secondBestDistances,
  arma::urowvec* assignments,
  const bool swapPerformed) {
  #pragma omp parallel for
  for (size_t i = 0; i < data.n_cols; i++) {
    double best = std::numeric_limits<double>::infinity();
    double second = std::numeric_limits<double>::infinity();
    for (size_t k = 0; k < medoidIndices->n_cols; k++) {
      double cost = KMedoids::cachedLoss(data, i, (*medoidIndices)(k));
      if (cost < best) {
        (*assignments)(i) = k;
        second = best;
        best = cost;
      } else if (cost < second) {
        second = cost;
      }
    }
    (*bestDistances)(i) = best;
    (*secondBestDistances)(i) = second;
  }

  if (!swapPerformed) {
    // We have converged; update the final loss
    averageLoss = arma::accu(*bestDistances) / data.n_cols;
  }
}

double KMedoids::calcLoss(
  const arma::mat& data,
  const arma::urowvec* medoidIndices) {
  double total = 0;
  // TODO(@motiwari): is this parallel loop accumulating properly?
  #pragma omp parallel for
  for (size_t i = 0; i < data.n_cols; i++) {
    double cost = std::numeric_limits<double>::infinity();
    for (size_t k = 0; k < nMedoids; k++) {
      double currCost = KMedoids::cachedLoss(data, i, (*medoidIndices)(k));
      if (currCost < cost) {
        cost = currCost;
      }
    }
    total += cost;
  }

  // Returns average distance
  return total/data.n_cols;
}

double KMedoids::cachedLoss(
  const arma::mat& data,
  const size_t i,
  const size_t j,
  const bool useCache) {
  if (!useCache) {
    return (this->*lossFn)(data, i, j);
  }

  size_t n = data.n_cols;
  size_t m = fmin(n, ceil(log10(data.n_cols) * cacheMultiplier));

  // test this is one of the early points in the permutation
  if (reindex.find(j) != reindex.end()) {
    // TODO(@motiwari): Potential race condition with shearing?
    // T1 begins to write to cache and then T2 access in the middle of write?
    if (cache[(m*i) + reindex[j]] == -1) {
      cache[(m*i) + reindex[j]] = (this->*lossFn)(data, i, j);
    }
    return cache[m*i + reindex[j]];
  }
  return (this->*lossFn)(data, i, j);
}

void KMedoids::checkAlgorithm(const std::string& algorithm) const {
  if ((algorithm != "BanditPAM") &&
      (algorithm != "PAM") &&
      (algorithm != "FastPAM1")) {
    // TODO(@motiwari): Better error type
    throw "unrecognized algorithm";
  }
}

double KMedoids::getAverageLoss() const {
  return averageLoss;
}

double KMedoids::LP(const arma::mat& data,
  const size_t i,
  const size_t j) const {
  return arma::norm(data.col(i) - data.col(j), lp);
}

double KMedoids::LINF(
  const arma::mat& data,
  const size_t i,
  const size_t j) const {
  return arma::max(arma::abs(data.col(i) - data.col(j)));
}

double KMedoids::cos(const arma::mat& data,
  const size_t i,
  const size_t j) const {
  return 1 - (arma::dot(data.col(i), data.col(j))
    / (arma::norm(data.col(i)) * arma::norm(data.col(j))));
}

double KMedoids::manhattan(const arma::mat& data,
  const size_t i,
  const size_t j) const {
  return arma::accu(arma::abs(data.col(i) - data.col(j)));
}
}  // namespace km
