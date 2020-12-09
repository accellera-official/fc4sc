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

class cvg : public covergroup {
public:
  CG_CONS(cvg) { }

  int sample_point = 0;

  COVERPOINT(int, cvp, sample_point) {
    bin<int> ("overlapping_bin",
    interval(4,7),
    1,
    interval(15,8),
    interval(2,4))
  };
};

TEST(bin_overlap,signed_integer) {

  auto cntxt = fc4sc::global::create_new_context();

  cvg cg("cg",__FILE__,__LINE__,cntxt);
  EXPECT_EQ(cg.get_inst_coverage(),0);
  cg.sample_point = 4;
  cg.sample();
  cg.sample_point = 9;
  cg.sample();
  EXPECT_EQ(cg.get_inst_coverage(),100);
  xml_printer::coverage_save("bin_overlap_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);
}

class slice_test : public covergroup {
public:
  CG_CONS(slice_test) { }

  int val = 0;

  COVERPOINT(int, cvp_int, val) {
    bin<int>("first",interval(1,5),interval(7,9)),
    bin<int>("second",interval(4,8)),
    bin<int>("third",interval(8,9)),
    bin<int>("fourth",interval(9,100))
  };

};

TEST(flat_map,slice) {
  auto cntxt = fc4sc::global::create_new_context();
  slice_test cvg("cvg",__FILE__,__LINE__,cntxt);
  auto bins = cvg.cvp_int.get_bins_base();
  EXPECT_EQ(bins[0]->name(),"first");
  EXPECT_EQ(bins[1]->name(),"second");
  EXPECT_EQ(bins[2]->name(),"third");
  EXPECT_EQ(bins[3]->name(),"fourth");

  cvg.val = 1;
  cvg.sample();

  EXPECT_EQ(bins[0]->interval_hits()[0],1);
  EXPECT_EQ(cvg.get_inst_coverage(),25);

  cvg.val = 6;
  cvg.sample();

  EXPECT_EQ(bins[1]->interval_hits()[0],1);
  EXPECT_EQ(cvg.get_inst_coverage(),50);

  cvg.val = 7;
  cvg.sample();

  EXPECT_EQ(bins[0]->interval_hits()[1],1);
  EXPECT_EQ(bins[0]->get_hitcount(),2);
  EXPECT_EQ(bins[1]->interval_hits()[0],2);
  EXPECT_EQ(cvg.get_inst_coverage(),50);

  cvg.val = 9;
  cvg.sample();

  EXPECT_EQ(bins[0]->interval_hits()[1],2);
  EXPECT_EQ(bins[2]->interval_hits()[0],1);
  EXPECT_EQ(bins[3]->interval_hits()[0],1);
  EXPECT_EQ(cvg.get_inst_coverage(),100);


  cvg.val = 8;
  cvg.sample();

  EXPECT_EQ(bins[0]->interval_hits()[1],3);
  EXPECT_EQ(bins[1]->interval_hits()[0],3);
  EXPECT_EQ(bins[2]->interval_hits()[0],2);
  EXPECT_EQ(bins[3]->interval_hits()[0],1);
  EXPECT_EQ(cvg.get_inst_coverage(),100);

  cvg.val = 10;
  cvg.sample();

  EXPECT_EQ(bins[0]->interval_hits()[1],3);
  EXPECT_EQ(bins[1]->interval_hits()[0],3);
  EXPECT_EQ(bins[2]->interval_hits()[0],2);
  EXPECT_EQ(bins[3]->interval_hits()[0],2);
  EXPECT_EQ(cvg.get_inst_coverage(),100);
 
  xml_printer::coverage_save("sample_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}
