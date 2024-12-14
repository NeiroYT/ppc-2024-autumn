// Copyright 2023 Nesterov Alexander
#include "mpi/leontev_n_gather/include/ops_mpi.hpp"

#include <cstdlib>
#include <string>

bool leontev_n_mat_vec_mpi::MPIMatVecSequential::pre_processing() {
  internal_order_test();
  int* vec_ptr1;
  int* vec_ptr2;
  if (taskData->inputs.size() >= 2) {
    vec_ptr1 = reinterpret_cast<int*>(taskData->inputs[0]);
    vec_ptr2 = reinterpret_cast<int*>(taskData->inputs[1]);
  } else {
    return false;
  }
  mat_ = std::vector<int>(taskData->inputs_count[0]);
  vec_ = std::vector<int>(taskData->inputs_count[1]);
  for (size_t i = 0; i < taskData->inputs_count[0]; i++) {
    mat_[i] = vec_ptr1[i];
  }
  for (size_t i = 0; i < taskData->inputs_count[1]; i++) {
    vec_[i] = vec_ptr2[i];
  }
  res = std::vector<int>(vec_.size(), 0);
  return true;
}

bool leontev_n_mat_vec_mpi::MPIMatVecSequential::validation() {
  internal_order_test();
  // Matrix+Vector input && vector input
  if (taskData->inputs.size() != 2 || taskData->outputs.size() != 1) {
    return false;
  }
  // square matrix
  if (taskData->inputs_count[0] != taskData->inputs_count[1] * taskData->inputs_count[1]) {
    return false;
  }
  if (taskData->inputs_count[0] == 0) {
    return false;
  }
  return true;
}

bool leontev_n_mat_vec_mpi::MPIMatVecSequential::run() {
  internal_order_test();
  for (size_t i = 0; i < res.size(); i++) {
    for (size_t j = 0; j < res.size(); j++) {
      res[i] += mat_[i * res.size() + j] * vec_[j];
    }
  }
  return true;
}

bool leontev_n_mat_vec_mpi::MPIMatVecSequential::post_processing() {
  internal_order_test();
  std::copy(res.begin(), res.end(), reinterpret_cast<int*>(taskData->outputs[0]));
  return true;
}

template <class InOutType>
void leontev_n_mat_vec_mpi::MPIMatVecParallel::my_gather(const boost::mpi::communicator& wrld,
                                                         const std::vector<InOutType>& input, InOutType* output,
                                                         const std::vector<int>& sizes, int root) {
  std::vector<InOutType> local_container = input;
  int rank = wrld.rank();
  int size = wrld.size();
  int parentNode;
  int leftNode;
  int rightNode;
  int newrank = (rank - root) % size;
  std::vector<InOutType> childTemp;
  if (2 * newrank + 1 < size) {
    leftNode = (2 * rank + 1 - root) % size;
  } else {
    leftNode = -1;
  }
  if (2 * newrank + 2 < size) {
    rightNode = (2 * rank + 2 - root) % size;
  } else {
    rightNode = -1;
  }
  if (rank == root) {
    parentNode = -1;
  } else {
    parentNode = (root + (newrank - 1) / 2) % size;
  }
  if (leftNode != -1) {
    size_t leftNodeSize;
    wrld.recv(leftNode, 0, &leftNodeSize, 1);
    std::vector<InOutType> leftChild(leftNodeSize);
    wrld.recv(leftNode, 0, leftChild.data(), leftChild.size());
    local_container.insert(local_container.end(), leftChild.begin(), leftChild.begin() + sizes[leftNode]);
    childTemp.insert(childTemp.end(), leftChild.begin() + sizes[leftNode], leftChild.end());
  }
  if (rightNode != -1) {
    size_t rightNodeSize;
    wrld.recv(rightNode, 0, &rightNodeSize, 1);
    std::vector<InOutType> rightChild(rightNodeSize);
    wrld.recv(rightNode, 0, rightChild.data(), rightChild.size());
    local_container.insert(local_container.end(), rightChild.begin(), rightChild.begin() + sizes[rightNode]);
    childTemp.insert(childTemp.end(), rightChild.begin() + sizes[rightNode], rightChild.end());
  }
  local_container.insert(local_container.end(), childTemp.begin(), childTemp.end());
  if (rank != root) {
    size_t localContainerSize = local_container.size();
    wrld.send(parentNode, 0, &localContainerSize, 1);
    wrld.send(parentNode, 0, local_container.data(), localContainerSize);
  } else {
    std::copy(local_container.begin(), local_container.end(), output);
  }
}

bool leontev_n_mat_vec_mpi::MPIMatVecParallel::pre_processing() {
  internal_order_test();
  if (world.rank() == 0) {
    int* vec_ptr1;
    int* vec_ptr2;
    if (taskData->inputs.size() >= 2) {
      vec_ptr1 = reinterpret_cast<int*>(taskData->inputs[0]);
      vec_ptr2 = reinterpret_cast<int*>(taskData->inputs[1]);
    } else {
      return false;
    }
    mat_ = std::vector<int>(taskData->inputs_count[0]);
    vec_ = std::vector<int>(taskData->inputs_count[1]);
    for (size_t i = 0; i < taskData->inputs_count[0]; i++) {
      mat_[i] = vec_ptr1[i];
    }
    for (size_t i = 0; i < taskData->inputs_count[1]; i++) {
      vec_[i] = vec_ptr2[i];
    }
    res = std::vector<int>(vec_.size(), 0);
  }
  return true;
}

bool leontev_n_mat_vec_mpi::MPIMatVecParallel::validation() {
  internal_order_test();
  if (world.rank() == 0) {
    if (taskData->inputs.size() != 2 || taskData->outputs.size() != 1) {
      return false;
    }
    // square matrix
    if (taskData->inputs_count[0] != taskData->inputs_count[1] * taskData->inputs_count[1]) {
      return false;
    }
    if (taskData->inputs_count[0] == 0) {
      return false;
    }
  }
  return true;
}

bool leontev_n_mat_vec_mpi::MPIMatVecParallel::run() {
  internal_order_test();
  div_t divres;
  if (world.rank() == 0) {
    divres = std::div(taskData->inputs_count[1], world.size());
  }
  std::vector<int> local_input(mat_.size());
  broadcast(world, divres.quot, 0);
  broadcast(world, divres.rem, 0);
  std::vector<int> local_tmp(divres.quot + divres.rem, 0);
  std::vector<int> sizes(world.size(), divres.quot * res.size());
  sizes[0] += divres.rem * res.size();
  if (world.rank() == 0) {
    boost::mpi::scatterv(world, mat_, sizes, local_input.data(), 0);
  }
  if (world.rank() == 0) {
    for (size_t i = 0; i < divres.quot + divres.rem; i++) {
      for (size_t j = 0; j < res.size(); j++) {
        local_tmp[i] += local_input[i * res.size() + j] * vec_[j];
      }
    }
  } else {
    for (size_t i = 0; i < divres.quot; i++) {
      for (size_t j = 0; j < res.size(); j++) {
        local_tmp[i] += local_input[i * res.size() + j] * vec_[j];
      }
    }
  }
  //boost::mpi::gatherv(world, local_tmp, res.data(), sizes, 0);
  my_gather(world, local_tmp, res.data(), sizes, 0);
  return true;
}

bool leontev_n_mat_vec_mpi::MPIMatVecParallel::post_processing() {
  internal_order_test();
  if (world.rank() == 0) {
    std::copy(res.begin(), res.end(), reinterpret_cast<int*>(taskData->outputs[0]));
  }
  return true;
}