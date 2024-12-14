// Copyright 2023 Nesterov Alexander
#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>
#include <random>
#include <vector>

#include "mpi/leontev_n_gather/include/ops_mpi.hpp"

namespace leontev_n_mat_vec_mpi {
std::vector<int> getRandomVector(int sz) {
  std::random_device dev;
  std::mt19937 gen(dev());
  std::vector<int> vec(sz);
  for (int i = 0; i < sz; i++) {
    vec[i] = gen() % 100;
  }
  return vec;
}
}  // namespace leontev_n_mat_vec_mpi

inline void taskEmplacement(std::shared_ptr<ppc::core::TaskData>& taskDataPar, std::vector<int>& global_vec,
                            std::vector<int>& global_mat,
                            std::vector<int32_t>& global_res) {
  taskDataPar->inputs.emplace_back(reinterpret_cast<uint8_t*>(global_mat.data()));
  taskDataPar->inputs_count.emplace_back(global_mat.size());
  taskDataPar->inputs.emplace_back(reinterpret_cast<uint8_t*>(global_vec.data()));
  taskDataPar->inputs_count.emplace_back(global_vec.size());
  taskDataPar->outputs.emplace_back(reinterpret_cast<uint8_t*>(global_res.data()));
  taskDataPar->outputs_count.emplace_back(global_res.size());
}

TEST(leontev_n_mat_vec_mpi, mul_mpi_50elem) {
  const int vector_size = 50;
  boost::mpi::communicator world;
  std::vector<int> global_vec;
  std::vector<int> global_mat;
  std::vector<int> global_res(vector_size);
  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataPar = std::make_shared<ppc::core::TaskData>();
  if (world.rank() == 0) {
    global_vec = leontev_n_mat_vec_mpi::getRandomVector(vector_size);
    global_mat = leontev_n_mat_vec_mpi::getRandomVector(vector_size * vector_size);
    taskEmplacement(taskDataPar, global_vec, global_mat, global_res);
  }
  leontev_n_mat_vec_mpi::MPIMatVecParallel MPIMatVecParallel(taskDataPar);
  ASSERT_TRUE(MPIMatVecParallel.validation());
  MPIMatVecParallel.pre_processing();
  MPIMatVecParallel.run();
  MPIMatVecParallel.post_processing();
  if (world.rank() == 0) {
    // Create data
    std::vector<int> reference_vec(vector_size);
    // Create TaskData
    std::shared_ptr<ppc::core::TaskData> taskDataSeq = std::make_shared<ppc::core::TaskData>();
    taskEmplacement(taskDataSeq, global_vec, global_mat, reference_vec);
    // Create Task
    leontev_n_mat_vec_mpi::MPIMatVecSequential MPIMatVecSequential(taskDataSeq);
    ASSERT_TRUE(MPIMatVecSequential.validation());
    MPIMatVecSequential.pre_processing();
    MPIMatVecSequential.run();
    MPIMatVecSequential.post_processing();
    for (size_t i = 0; i < vector_size; i++) {
      ASSERT_EQ(reference_vec[i], global_res[i]);
    }
  }
}

TEST(leontev_n_mat_vec_mpi, mul_mpi_0elem) {
  boost::mpi::communicator world;
  std::vector<int> global_vec;
  std::vector<int> global_mat;
  std::vector<int> global_res;
  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataPar = std::make_shared<ppc::core::TaskData>();
  if (world.rank() == 0) {
    taskEmplacement(taskDataPar, global_vec, global_mat, global_vec);
  }
  leontev_n_mat_vec_mpi::MPIMatVecParallel MPIMatVecParallel(taskDataPar);
  ASSERT_FALSE(MPIMatVecParallel.validation());
}

TEST(leontev_n_mat_vec_mpi, mul_mpi_1000elem) {
  const int vector_size = 1000;
  boost::mpi::communicator world;
  std::vector<int> global_vec;
  std::vector<int> global_mat;
  std::vector<int> global_res(vector_size);
  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataPar = std::make_shared<ppc::core::TaskData>();
  if (world.rank() == 0) {
    global_vec = leontev_n_mat_vec_mpi::getRandomVector(vector_size);
    global_mat = leontev_n_mat_vec_mpi::getRandomVector(vector_size * vector_size);
    taskEmplacement(taskDataPar, global_vec, global_mat, global_res);
  }
  leontev_n_mat_vec_mpi::MPIMatVecParallel MPIMatVecParallel(taskDataPar);
  ASSERT_TRUE(MPIMatVecParallel.validation());
  MPIMatVecParallel.pre_processing();
  MPIMatVecParallel.run();
  MPIMatVecParallel.post_processing();
  if (world.rank() == 0) {
    // Create data
    std::vector<int> reference_vec(vector_size);
    // Create TaskData
    std::shared_ptr<ppc::core::TaskData> taskDataSeq = std::make_shared<ppc::core::TaskData>();
    taskEmplacement(taskDataSeq, global_vec, global_mat, reference_vec);
    // Create Task
    leontev_n_mat_vec_mpi::MPIMatVecSequential MPIMatVecSequential(taskDataSeq);
    ASSERT_TRUE(MPIMatVecSequential.validation());
    MPIMatVecSequential.pre_processing();
    MPIMatVecSequential.run();
    MPIMatVecSequential.post_processing();
    for (size_t i = 0; i < vector_size; i++) {
      ASSERT_EQ(reference_vec[i], global_res[i]);
    }
  }
}

TEST(leontev_n_mat_vec_mpi, mul_mpi_5000elem) {
  const int vector_size = 5000;
  boost::mpi::communicator world;
  std::vector<int> global_vec;
  std::vector<int> global_mat;
  std::vector<int> global_res(vector_size);
  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataPar = std::make_shared<ppc::core::TaskData>();
  if (world.rank() == 0) {
    global_vec = leontev_n_mat_vec_mpi::getRandomVector(vector_size);
    global_mat = leontev_n_mat_vec_mpi::getRandomVector(vector_size * vector_size);
    taskEmplacement(taskDataPar, global_vec, global_mat, global_res);
  }
  leontev_n_mat_vec_mpi::MPIMatVecParallel MPIMatVecParallel(taskDataPar);
  ASSERT_TRUE(MPIMatVecParallel.validation());
  MPIMatVecParallel.pre_processing();
  MPIMatVecParallel.run();
  MPIMatVecParallel.post_processing();
  if (world.rank() == 0) {
    // Create data
    std::vector<int> reference_vec(vector_size);
    // Create TaskData
    std::shared_ptr<ppc::core::TaskData> taskDataSeq = std::make_shared<ppc::core::TaskData>();
    taskEmplacement(taskDataSeq, global_vec, global_mat, reference_vec);
    // Create Task
    leontev_n_mat_vec_mpi::MPIMatVecSequential MPIMatVecSequential(taskDataSeq);
    ASSERT_TRUE(MPIMatVecSequential.validation());
    MPIMatVecSequential.pre_processing();
    MPIMatVecSequential.run();
    MPIMatVecSequential.post_processing();
    for (size_t i = 0; i < vector_size; i++) {
      ASSERT_EQ(reference_vec[i], global_res[i]);
    }
  }
}

TEST(leontev_n_mat_vec_mpi, mul_mpi_1elem) {
  const int vector_size = 1;
  boost::mpi::communicator world;
  std::vector<int> global_vec;
  std::vector<int> global_mat;
  std::vector<int> global_res(vector_size);
  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataPar = std::make_shared<ppc::core::TaskData>();
  if (world.rank() == 0) {
    global_vec = leontev_n_mat_vec_mpi::getRandomVector(vector_size);
    global_mat = leontev_n_mat_vec_mpi::getRandomVector(vector_size * vector_size);
    taskEmplacement(taskDataPar, global_vec, global_mat, global_res);
  }
  leontev_n_mat_vec_mpi::MPIMatVecParallel MPIMatVecParallel(taskDataPar);
  ASSERT_TRUE(MPIMatVecParallel.validation());
  MPIMatVecParallel.pre_processing();
  MPIMatVecParallel.run();
  MPIMatVecParallel.post_processing();
  if (world.rank() == 0) {
    // Create data
    std::vector<int> reference_vec(vector_size);
    // Create TaskData
    std::shared_ptr<ppc::core::TaskData> taskDataSeq = std::make_shared<ppc::core::TaskData>();
    taskEmplacement(taskDataSeq, global_vec, global_mat, reference_vec);
    // Create Task
    leontev_n_mat_vec_mpi::MPIMatVecSequential MPIMatVecSequential(taskDataSeq);
    ASSERT_TRUE(MPIMatVecSequential.validation());
    MPIMatVecSequential.pre_processing();
    MPIMatVecSequential.run();
    MPIMatVecSequential.post_processing();
    for (size_t i = 0; i < vector_size; i++) {
      ASSERT_EQ(reference_vec[i], global_res[i]);
    }
  }
}
