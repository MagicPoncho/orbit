#pragma once
#include <memory>
#include <thread>
#include <vector>

class OrbitTest {
 public:
  OrbitTest() = default;
  OrbitTest(uint32_t num_threads, uint32_t recurse_depth, uint32_t sleep_us);
  ~OrbitTest();

  void Start();

 private:
  void Loop();
  void TestFunc(uint32_t a_Depth = 0);
  void TestFunc2(uint32_t a_Depth = 0);
  void BusyWork(uint64_t microseconds);

 private:
  bool m_ExitRequested = false;
  std::vector<std::shared_ptr<std::thread>> m_Threads;
  uint32_t num_threads_ = 10;
  uint32_t recurse_depth_ = 10;
  uint32_t sleep_us_ = 100'000;
};
