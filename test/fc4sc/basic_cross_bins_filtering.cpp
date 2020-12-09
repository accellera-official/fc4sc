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
   Original Authors: Dragos Dospinescu,
                     AMIQ Consulting s.r.l. (contributors@amiq.com)

               Date: 2018-Feb-20
******************************************************************************/
#include "fc4sc.hpp"
#include "gtest/gtest.h"

class basic_cross_bins_filtering_test: public covergroup {
public:
  CG_CONS(basic_cross_bins_filtering_test){
    cvp1.option().weight = 0;
    cvp2.option().weight = 0;
  };

  int SAMPLE_POINT(sample_point_1, cvp1);
  int SAMPLE_POINT(sample_point_2, cvp2);

  void sample(const int& x, const int& y) {
    this->sample_point_1 = x;
    this->sample_point_2 = y;
    covergroup::sample();
  }
  coverpoint<int> cvp1 = coverpoint<int> (this, "cvp1",
    bin<int>("one", 1),
    bin<int>("two", 2)
  );
  coverpoint<int> cvp2 = coverpoint<int> (this, "cvp2",
    bin<int>("one", 1),
    bin<int>("two", 2)
  );
  cross<int,int> cross1 = cross<int,int> (this, "cross1", &cvp1, &cvp2);
};

TEST(cross_bins_filtering, binsof) {
    auto cntxt = fc4sc::global::create_new_context();

    basic_cross_bins_filtering_test basic_cg_1("basic_cg_1",__FILE__,__LINE__,cntxt);

    // TODO: update testcase to check the functionality of binsof, || and && operators
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 0);

    basic_cg_1.sample(0, 0);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 0);

    basic_cg_1.sample(1, 0);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 0);

    basic_cg_1.sample(0, 1);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 0);

    basic_cg_1.sample(1, 2);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 25);

    basic_cg_1.sample(1, 2);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 25);

    basic_cg_1.cross1.option().goal = 25;
    EXPECT_EQ(basic_cg_1.cross1.get_inst_coverage(), 100);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 100);

    basic_cg_1.cross1.option().goal = 100;

    basic_cg_1.sample(2, 2);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 50);

    basic_cg_1.sample(2, 1);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 75);

    basic_cg_1.sample(1, 1);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 100);

    xml_printer::coverage_save("basic_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);

    fc4sc::global::delete_context(cntxt);

}

class cross_cvg : public covergroup {
public:
  int value = 0;
  CG_CONS(cross_cvg) {};

  COVERPOINT(int, cvp1, value) {
    bin<int>("filled", 1),
    bin<int>("value",  2)
  };

  cross<int> cvp_cross = cross<int> (this, &cvp1);

};


TEST(cross_bins_filtering, edge_cross) {

  auto cntxt = fc4sc::global::create_new_context();

  cross_cvg cvg("cvg",__FILE__,__LINE__,cntxt);

  EXPECT_EQ(cvg.get_inst_coverage(), 0);
  
  cvg.stop();
  cvg.value = 1;
  cvg.sample();

  EXPECT_EQ(cvg.get_inst_coverage(), 0);

  cvg.start();
  cvg.cvp1.stop();
  cvg.value = 1;
  cvg.sample();

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 0);
  EXPECT_EQ(cvg.cvp_cross.get_inst_coverage(), 0);  
  EXPECT_EQ(cvg.get_inst_coverage(), 0);

  cvg.cvp1.start();
  cvg.value = 1;
  cvg.sample();
  
  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 50);
  EXPECT_EQ(cvg.cvp_cross.get_inst_coverage(), 50);  
  EXPECT_EQ(cvg.get_inst_coverage(), 50);

  xml_printer::coverage_save("basic_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}


