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

bool flag1, flag2;

class general_expression_cvg : public covergroup {
public:
  CG_CONS(general_expression_cvg) {
    //set options or type options
   }

  COVERPOINT(bool,flag1_cvp,flag1) {
    bin<bool>("LOW",false),
    bin<bool>("HIGH",true)
  };
  COVERPOINT(bool,flag2_cvp,flag2) {
    bin<bool>("LOW",false),
    bin<bool>("HIGH",true)
  };

  cross<bool,bool> flag_crs = cross<bool,bool>(this, "flag_crs", &flag1_cvp, &flag2_cvp);
};

TEST(cvp_general_expression,global_var) {
  auto cntxt = fc4sc::global::create_new_context();
  //instantiate statically defined covergroup
  general_expression_cvg inst("inst",__FILE__,__LINE__,cntxt); 

  flag1 = true;
  flag2 = false;

  inst.sample();

  flag2 = true;
  inst.sample();

  EXPECT_EQ((int)inst.get_inst_coverage(),66);

  xml_printer::coverage_save("cvp_general_expression_"+ std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}

