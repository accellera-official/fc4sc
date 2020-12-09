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

class decl_def_cvg : public covergroup {
public:
  CG_CONS_DECL(decl_def_cvg);
  int x;
  COVERPOINT(int,cvp_x,x) { bin<int>("one",1) };
};

class top_scope : public fc4sc::scope {
public:
SCOPE_DECL(top_scope)

  class scoped_decl_def_cvg : public covergroup {
  public:
    CG_SCOPED_CONS_DECL(scoped_decl_def_cvg,top_scope);
    int x;
    COVERPOINT(int,cvp_x,x) { bin<int>("one",1) };
  };

  top_scope(std::string file_name = "", int inst_line = 1, fc4sc::global* cntxt = fc4sc::global::getter()) : fc4sc::scope("top_scope_inst",__FILE__,__LINE__,file_name,inst_line,cntxt), CG_SCOPED_INST(cvg) { }
  scoped_decl_def_cvg cvg;
};

