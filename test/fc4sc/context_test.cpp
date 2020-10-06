/******************************************************************************

   Copyright 2020 NVIDIA Corporation

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

******************************************************************************/
#include "fc4sc.hpp"
#include "gtest/gtest.h"

class cvg_context_test : public covergroup {
  public:
  CG_CONS(cvg_context_test) { }
};

TEST(context,error) {
  auto cntxt = fc4sc::global::create_new_context();
  cvg_context_test inst("inst",__FILE__,__LINE__,cntxt);
  fc4sc::global::delete_context(cntxt);
  bool test_pass = false;
  try {
    inst.sample();
  } catch(...) {
    test_pass = true;
  }
  EXPECT_EQ(true,test_pass);
}
