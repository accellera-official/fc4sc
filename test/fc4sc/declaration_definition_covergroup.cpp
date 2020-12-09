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
#include "declaration_definition_covergroup.hpp"
#include "gtest/gtest.h"

CG_CONS_DEF(decl_def_cvg) { }

TEST(separate_cvg,test1) {
  auto cntxt = fc4sc::global::create_new_context();
  decl_def_cvg cvg("cvg",__FILE__,__LINE__,cntxt);
  EXPECT_EQ(cvg.get_inst_coverage(),0);
  cvg.x = 1;
  cvg.sample();
  EXPECT_EQ(cvg.get_inst_coverage(),100);
  xml_printer::coverage_save("declaration_definition_covergroup.xml",cntxt);
  fc4sc::global::delete_context(cntxt);
}

CG_SCOPED_CONS_DEF(scoped_decl_def_cvg,top_scope) { }

TEST(scoped_separate_cvg,test2) {
  auto cntxt = fc4sc::global::create_new_context();
  top_scope top_scope_inst(__FILE__,__LINE__,cntxt);
  auto& cvg = top_scope_inst.cvg;
  EXPECT_EQ(cvg.get_inst_coverage(),0);
  cvg.x = 1;
  cvg.sample();
  EXPECT_EQ(cvg.get_inst_coverage(),100);
  xml_printer::coverage_save("declaration_definition_covergroup_scoped.xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}
