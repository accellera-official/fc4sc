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
#ifndef FC4SC_SCOPE_HPP
#define FC4SC_SCOPE_HPP

#include "fc4sc_base.hpp"
#include "fc4sc_covergroup.hpp"

#define GET_SCP_MACRO(_1,_2,NAME,...) NAME

#define SCOPE_DECL(...) GET_SCP_MACRO(__VA_ARGS__,SCOPE_2ARG,SCOPE_1ARG)(__VA_ARGS__)

/*!
 * \brief Macro to declare scope for registration.
 *
 * This macro expands to function definitions that ensure the macro is used for a class that extends fc4sc::scope
 * and implements type_name() for run-time introspection. This is for scopes with no parent type (i.e. top type scopes).
 */
#define SCOPE_1ARG(t_name) \
std::string type_name() { return t_name::scp_type_name(); } \
static std::string scp_type_name() { return #t_name; } \
void* fc4sc_t_name_initialize() { \
  if(this->fc4sc::scp_base::parent_scp == nullptr) { \
    fc4sc::global::register_data(this->get_scp_data(),this->type_name(),fc4sc::global::get_file_id(__FILE__,this->get_scp_data()->cntxt),__LINE__,this->get_scp_data()->cntxt); \
  } \
  else { \
     parent_scp->get_scp_data()->name_check(this->fc4sc::scope::name(),type_name()); \
     parent_scp->register_child_scope(this); \
     parent_scp->get_scp_data()->add_scp_data(this->get_scp_data(),this->type_name(),fc4sc::global::get_file_id(__FILE__,this->get_scp_data()->cntxt),__LINE__); \
  } \
  return nullptr;\
} \
void* fc4sc_t_name_init = fc4sc_t_name_initialize();

/*!
 * \brief Macro to declare scope for registration. This macro is when macro has two args: the scope type and the parent scopetype.
 *
 * This macro expands to function definitions that ensure the macro is used for a class that extends fc4sc::scope
 * and implements type_name() for run-time introspection. This is for scopes with a parent type.
 */
#define SCOPE_2ARG(t_name,p_name) \
std::string type_name() { return t_name::scp_type_name(); } \
static std::string scp_type_name() { return p_name::scp_type_name() + "::" + #t_name; } \
void* fc4sc_t_name_initialize() { \
  if(this->fc4sc::scp_base::parent_scp == nullptr) { \
    fc4sc::global::register_data(this->get_scp_data(),this->type_name(),fc4sc::global::get_file_id(__FILE__,this->get_scp_data()->cntxt),__LINE__,this->get_scp_data()->cntxt); \
  } \
  else { \
     parent_scp->get_scp_data()->name_check(this->fc4sc::scope::name(),type_name()); \
     parent_scp->register_child_scope(this); \
     parent_scp->get_scp_data()->add_scp_data(this->get_scp_data(),this->type_name(),fc4sc::global::get_file_id(__FILE__,this->get_scp_data()->cntxt),__LINE__); \
  } \
  return nullptr;\
} \
void* fc4sc_t_name_init = fc4sc_t_name_initialize();


namespace fc4sc
{

/*!
 *  \class scope_data_model fc_scope.hpp
 *  \brief Data container class for scopes
 *
 *  Holds the instance specific data
 */
class scope_data_model : public scp_base_data_model
{
public:

  void add_cvg_data(cvg_base_data_model* cvg_data, std::string cvg_type_name, std::string scp_type_name, unsigned int cvg_file_id, uint32_t cvg_line)
  {
    cvg_insts[cvg_data->name] = cvg_data;
    cvgs[scp_type_name + "::" + cvg_type_name].push_back(cvg_data);

    cvg_data->parent_scp = this;

    scp_metadata* type_data;
    if(fc4sc::global::get_scopes_data(this->cntxt).count(scp_type_name) > 0) {
      type_data = fc4sc::global::get_scopes_data(this->cntxt)[scp_type_name];
    }
    else {
      type_data = new scp_metadata;
      type_data->type_name = scp_type_name;
      fc4sc::global::get_scopes_data(this->cntxt)[scp_type_name] = type_data;
    }
    
    if(type_data->cvg_type_table.count(cvg_type_name) > 0)
    {
      type_data->cvg_type_table[cvg_type_name]->cvg_insts.push_back(cvg_data);
    }
    else 
    {
      cvg_metadata* tmp = new cvg_metadata;
      tmp->type_name = cvg_type_name;
      tmp->scp_type_name = scp_type_name;
      tmp->file_id = cvg_file_id;
      tmp->line = cvg_line;
      tmp->cvg_insts.push_back(cvg_data);
      type_data->cvg_type_table[cvg_type_name] = tmp;
    }
    cvg_data->type_data = type_data->cvg_type_table[cvg_type_name];
  }

  void add_scp_data(scp_base_data_model* scp_data, std::string scp_type_name, unsigned int scp_file_id, uint32_t scp_line)
  {
    child_scp_insts[scp_data->name] = scp_data;
    child_scps[scp_type_name].push_back(scp_data);
    scp_data->parent_scp = this;
    if(fc4sc::global::get_scopes_data(this->cntxt).count(scp_type_name) > 0)
    {
      scp_metadata* type_data = fc4sc::global::get_scopes_data(this->cntxt)[scp_type_name];
      if(type_data->scp_insts.size() == 0) {
        type_data->file_id = scp_file_id;
	type_data->line = scp_line;
      }
      fc4sc::global::get_scopes_data(this->cntxt)[scp_type_name]->scp_insts.push_back(scp_data);
    }
    else
    {
      scp_metadata* tmp = new scp_metadata;
      tmp->type_name = scp_type_name;
      tmp->file_id = scp_file_id;
      tmp->line = scp_line;
      tmp->scp_insts.push_back(scp_data);
      fc4sc::global::get_scopes_data(this->cntxt)[scp_type_name] = tmp;
    }
    scp_data->type_data = fc4sc::global::get_scopes_data(this->cntxt)[scp_type_name];
  }
  
};

/*!
 *  \class dynamic_scope_factory fc_scope.hpp
 *  \brief Factory for creating dynamic scopes
 *
 *  Defines facilities to create scope type dynamically and add sub scope types or covergroup types
 */
class dynamic_scope_factory : public scope_factory_base
{
public:
  
  dynamic_scope_factory(dynamic_scope_factory& p_scope, std::string type_name, std::string filename = "", int line = 1)
  {
    this->type_name = type_name;
    this->filename = filename;
    this->line = line;
    p_scope.register_sub_scp_type(this);
  }

  dynamic_scope_factory(std::string type_name, std::string filename = "", int line = 1)
  {
    this->type_name = type_name;
    this->filename = filename;
    this->line = line;
  }

  void add_child(dynamic_scope_factory& c_scope, std::string inst_name)
  {
    child_scp_insts.push_back({&c_scope,inst_name});
  }

  void add_covergroup(dynamic_covergroup_factory& cvg, std::string name)
  {
    cvgs.push_back({&cvg,name});
  }

  void register_sub_cvg_type(dynamic_covergroup_factory* cvg) {
    if (this->cvg_types.count(cvg->type_name) > 0)
      throw ("Covergroup already registered in this scope!");
    this->cvg_types[cvg->type_name] = cvg;
    cvg->scp_type_factory = this;
  }

  void register_sub_scp_type(dynamic_scope_factory* scp) {
    if(this->child_scp_types.count(scp->type_name) > 0)
      throw("scope " + scp->type_name + " already exists in scope " + this->type_name);
    this->child_scp_types[scp->type_name] = scp;
    scp->parent_scp_type = this;
  }

  std::string get_scp_type_name()
  {
    if(parent_scp_type != nullptr) {
      return parent_scp_type->get_scp_type_name() + "::" + type_name;
    }
    else {
      return type_name;
    }
  }

  std::string type_name;

  uint32_t line;

  std::string filename;

  dynamic_scope_factory* parent_scp_type = nullptr;

  std::vector<std::pair<dynamic_scope_factory*,std::string>> child_scp_insts;

  std::unordered_map<std::string,dynamic_scope_factory*> child_scp_types;

  std::vector<std::pair<dynamic_covergroup_factory*,std::string>> cvgs;

  std::unordered_map<std::string,dynamic_covergroup_factory*> cvg_types;

};


/*!
 *  \class scope fc_scope.hpp
 *  \brief Scope class for coverage model definition
 *
 *  Defines the behavior of a scope during simulation
 */
class scope : public scp_base
{
  friend class dynamic_scope;

  scp_base_data_model* scp_data = new scope_data_model;

  std::weak_ptr<bool> valid_data = scp_data->valid;

  scope(dynamic_scope_factory& scp_fact, scp_base& p_scope, std::string inst_name, const char *inst_file_name, int inst_line)
  {
    scp_data->cntxt = p_scope.get_scp_data()->cntxt;
    scp_data->name = inst_name;
    scp_data->inst_file_id = fc4sc::global::get_file_id(inst_file_name,scp_data->cntxt);
    scp_data->inst_line = inst_line;
    scp_data->instance_id = fc4sc::global::create_instance_id(scp_data->cntxt);
    this->parent_scp = &p_scope;
    p_scope.get_scp_data()->name_check(scp_data->name,p_scope.type_name());
    p_scope.register_child_scope(this);
    p_scope.get_scp_data()->add_scp_data(this->scp_data,scp_fact.get_scp_type_name(),fc4sc::global::get_file_id(scp_fact.filename,scp_data->cntxt),scp_fact.line);
  }

  scope(dynamic_scope_factory& scp_fact,std::string inst_name,const char *inst_file_name, int inst_line, fc4sc::global* cntxt)
  {
    scp_data->cntxt = cntxt;
    scp_data->name = inst_name;
    scp_data->inst_file_id = fc4sc::global::get_file_id(inst_file_name,scp_data->cntxt);
    scp_data->inst_line = inst_line;
    scp_data->instance_id = fc4sc::global::create_instance_id(scp_data->cntxt);
    fc4sc::global::register_data(this->scp_data,scp_fact.get_scp_type_name(),fc4sc::global::get_file_id(scp_fact.filename,scp_data->cntxt),scp_fact.line,scp_data->cntxt);
  }

public:

  scope(std::string name = "", std::string file_name = "", int line = 1, std::string inst_file_name = "", int inst_line = 1, fc4sc::global* cntxt = fc4sc::global::getter())
  {
    scp_data->cntxt = cntxt;
    scp_data->name = name;
    scp_data->inst_file_id = fc4sc::global::get_file_id(inst_file_name,scp_data->cntxt);
    scp_data->inst_line = inst_line;
    scp_data->instance_id = fc4sc::global::create_instance_id(scp_data->cntxt);
  }

  scope(scp_base& p_scope, std::string name = "", std::string file_name = "", int line = 1, std::string inst_file_name = "", int inst_line = 1)
  {
    scp_data->cntxt = p_scope.get_scp_data()->cntxt;
    scp_data->name = name;
    scp_data->inst_file_id = fc4sc::global::get_file_id(inst_file_name,scp_data->cntxt);
    scp_data->inst_line = inst_line;
    scp_data->instance_id = fc4sc::global::create_instance_id(scp_data->cntxt);
    this->parent_scp = &p_scope;
  }

  /*! Destructor */
  virtual ~scope() { }

  scp_base_data_model* get_scp_data()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return scp_data;
  }

  std::string& name()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return scp_data->name;
  }

  unsigned int& file_id()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return scp_data->type_data->file_id;
  }

  uint32_t& line()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return scp_data->type_data->line;
  }

  unsigned int& inst_file_id()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return scp_data->inst_file_id;
  }

  uint32_t& inst_line()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return scp_data->inst_line;
  }

  unsigned int& instance_id()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return scp_data->instance_id;
  }

};

/*!
 *  \class dynamic_scope fc_scope.hpp
 *  \brief dynamic scope class for dynamic scope instances
 *
 *  Defines coverage model object for interacting with dynamic scopes
 */
class dynamic_scope : public scope
{

  dynamic_scope(dynamic_scope_factory& scp_fact, dynamic_scope& p_scope, std::string inst_name, const char *inst_file_name = "", int inst_line = 1) :
  scope(scp_fact, p_scope, inst_name, inst_file_name, inst_line)
  {
    for(auto cvg_it : scp_fact.cvgs)
    {
      new dynamic_covergroup(*this, (*cvg_it.first),cvg_it.second,inst_file_name,inst_line);
    }
    for(auto scp_it : scp_fact.child_scp_insts)
    {
      new dynamic_scope(*(scp_it.first),*this,scp_it.second); //child scope registers itself to parent
    }
  }

public:

  dynamic_scope(dynamic_scope_factory& scp_fact,std::string inst_name,const char *inst_file_name = "", int inst_line = 1, fc4sc::global* cntxt = fc4sc::global::getter()) :
    scope(scp_fact, inst_name, inst_file_name, inst_line, cntxt)
  {
    for(auto cvg_it : scp_fact.cvgs)
    {
      new dynamic_covergroup(*this, (*cvg_it.first),cvg_it.second,inst_file_name,inst_line);
    }
    for(auto scp_it : scp_fact.child_scp_insts)
    {
      new dynamic_scope(*(scp_it.first),*this,scp_it.second); //child scope registers itself to parent
    }
  }

  std::string type_name()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return scp_data->type_data->type_name;
  }

  /*! Destructor */
  virtual ~dynamic_scope()
  {
    for(auto scp_it : this->child_scp_insts) {
      delete scp_it.second;
    }
    for(auto cvg_it : this->cvg_insts) {
      delete cvg_it.second;
    }
  }
};

}

#endif
