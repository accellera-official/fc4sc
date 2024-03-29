/******************************************************************************

   Copyright 2003-2018 AMIQ Consulting s.r.l.
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
/******************************************************************************
   Original Authors: Teodor Vasilache and Dragos Dospinescu,
                     AMIQ Consulting s.r.l. (contributors@amiq.com)

               Date: 2018-Feb-20
******************************************************************************/

#include "fc4sc.hpp"
#include "gtest/gtest.h"

class basic_cvg_goal_test : public covergroup {
public:

  CG_CONS(basic_cvg_goal_test) {
  };

  int SAMPLE_POINT(sample_point, cvp1)

  void sample(const int& x) {
    this->sample_point = x;
    covergroup::sample();
  }
  
  coverpoint<int> cvp1 = coverpoint<int> (this,
    bin<int>("1", 1),
    bin<int>("2", 2)
  );

};


TEST(cvg_options, goal) {

  auto cntxt = fc4sc::global::create_new_context();

  basic_cvg_goal_test basic_cg_1("basic_cg_1",__FILE__,__LINE__,cntxt);
  basic_cvg_goal_test basic_cg_2("basic_cg_2",__FILE__,__LINE__,cntxt);

  basic_cg_1.option().goal = 50;
  basic_cg_2.option().goal = 100;

  EXPECT_TRUE(basic_cg_1.get_coverage() == 0);
  EXPECT_TRUE(basic_cg_2.get_coverage() == 0);
  
  basic_cg_1.sample(1);
  basic_cg_2.sample(1);

  EXPECT_EQ(basic_cg_1.get_inst_coverage(), 100);
  EXPECT_EQ(basic_cg_2.get_inst_coverage(), 50);
  EXPECT_EQ(basic_cg_1.get_coverage(), 75);
  EXPECT_EQ(basic_cg_1.get_coverage(), basic_cg_2.get_coverage());
  
  basic_cg_1.type_option().goal = 75;
  EXPECT_EQ(basic_cg_1.get_coverage(), 100);

  basic_cg_2.sample(2);

  EXPECT_EQ(basic_cg_1.get_inst_coverage(), 100);
  EXPECT_EQ(basic_cg_2.get_inst_coverage(), 100);
  EXPECT_EQ(basic_cg_1.get_coverage(), 100);
  EXPECT_EQ(basic_cg_1.get_coverage(), basic_cg_2.get_coverage());

  xml_printer::coverage_save("basic_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);
}
