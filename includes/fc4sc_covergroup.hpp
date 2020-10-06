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

/*!
 \file covergroup.hpp
 \brief Covergroup implementation

   This file contains the covergroup implementation
 with all the non-user functions
 */

#ifndef FC4SC_COVERGROUP_HPP
#define FC4SC_COVERGROUP_HPP

#include "fc4sc_base.hpp"

#include <unordered_map>
#include <typeinfo>
#include <tuple>

namespace fc4sc
{

class dynamic_covergroup;
class dynamic_covergroup_factory;

class covergroup_data_model : public cvg_base_data_model
{
public:
};

/*!
 * \class covergroup covergroup.hpp
 * \brief Covergroup option declaration
 */
class covergroup : public cvg_base
{

  friend class dynamic_covergroup_factory;
  template<typename T,typename U>
  friend class dynamic_coverpoint_factory;

  /*! Pointer to this object's data */
  covergroup_data_model* cvg_data = new covergroup_data_model;

  /*! For ensuring the covergroup's data has not been deallocated */
  std::weak_ptr<bool> valid_data = cvg_data->valid;

protected:
  /*
   * This function registers a coverpoint instance inside this covergroup.
   * It receives as arguments a pointer to the coverpoint to be registered,
   * the coverpoint name, the sample expression lambda function and string,
   * and finally, the sample condition lambda function and string.
   * Finally, it returns a coverpoint constructed with the given arguments.
   * The purpose of this function is to be used for coverpoint instantiation
   * via the COVERPOINT macro and should not be explicitly used!
   */
  template<typename T>
  coverpoint <T> register_cvp(coverpoint <T>* cvp, std::string&& cvp_name,
    std::function<T()>&& sample_expr, std::string&& sample_expr_str,
    std::function<bool()>&& sample_cond, std::string&& sample_cond_str) {

    // NOTE: VERY important! Do not attempt to dereference the cvp pointer in
    // any way because it points to uninitialized memory!
    cvg_base::register_cvp(cvp);
    coverpoint<T> cvp_structure;
    cvp_structure.has_sample_expression = true;
    cvp_structure.sample_expression = sample_expr;
    cvp_structure.sample_condition = sample_cond;
    cvp_structure.cvp_data->sample_expression_str = sample_expr_str;
    cvp_structure.cvp_data->sample_condition_str = sample_cond_str;
    cvp_structure.name() = cvp_name;
    cvg_data->add_cvp_data(cvp_structure.cvp_data);
    return cvp_structure;
  }

  std::unordered_map<cvp_base *, cvp_metadata_t> cvp_strings;

  /*!
   * \brief Registers an instance WITH NO SCOPE and some info to \link fc4sc::global_access \endlink
   * \param type_name Type of covergroup stringified
   * \param file_name File of declaration
   * \param line  Line of declaration
   * \param inst_name Name of the instance
   */
  covergroup(const char *type_name, const char *file_name = "", int line = 1, const std::string &inst_name = "", const char *inst_file_name = "", int inst_line = 1, fc4sc::global* cntxt = fc4sc::global::getter())
  {
    cvg_data->name = inst_name;
    cvg_data->inst_file_id = fc4sc::global::get_file_id(inst_file_name,cntxt);
    cvg_data->inst_line = inst_line;
    fc4sc::global::get_default_scope(cntxt)->get_scp_data()->name_check(cvg_data->name,type_name);
    fc4sc::global::get_default_scope(cntxt)->get_scp_data()->add_cvg_data(this->cvg_data,type_name,_FC4SC_DEFAULT_SCOPE_TYPE_,fc4sc::global::get_file_id(file_name,cntxt),line);
    fc4sc::global::get_default_scope(cntxt)->register_cvg(this);
  }

  /*!
   * \brief Registers an instance WITH NO SCOPE and some info to \link fc4sc::global_access \endlink
   * \param type_name Type of covergroup stringified
   * \param file_name File of declaration
   * \param line  Line of declaration
   * \param inst_name Name of the instance
   */
  covergroup(const char *type_name, std::string scp_type_name, const char *file_name = "", int line = 1, const std::string &inst_name = "", const char *inst_file_name = "", int inst_line = 1, fc4sc::global* cntxt = fc4sc::global::getter())
  {
    cvg_data->name = inst_name;
    cvg_data->inst_file_id = fc4sc::global::get_file_id(inst_file_name,cntxt);
    cvg_data->inst_line = inst_line;
    fc4sc::global::get_default_scope(cntxt)->get_scp_data()->name_check(cvg_data->name,type_name);
    fc4sc::global::get_default_scope(cntxt)->get_scp_data()->add_cvg_data(this->cvg_data,type_name,scp_type_name,fc4sc::global::get_file_id(file_name,cntxt),line);
    fc4sc::global::get_default_scope(cntxt)->register_cvg(this);
  }

   /*!
   * \brief Registers an instance and some info to \link fc4sc::global_access \endlink
   * \param mod Reference to parent scope instance
   * \param type_name Type of covergroup stringified
   * \param scp_type_name Stringified type of the scope in which the covergroup is defined
   * \param file_name File of declaration
   * \param line  Line of declaration
   * \param inst_name Name of the instance
   */
  covergroup(scp_base& mod, const char *type_name, std::string scp_type_name, const char *file_name = "", int line = 1, const std::string &inst_name = "", const char *inst_file_name = "", int inst_line = 1)
  {
    cvg_data->name = inst_name;
    cvg_data->inst_file_id = fc4sc::global::get_file_id(inst_file_name,mod.get_scp_data()->cntxt);
    cvg_data->inst_line = inst_line;
    mod.get_scp_data()->name_check(cvg_data->name,type_name);
    mod.get_scp_data()->add_cvg_data(this->cvg_data,type_name,scp_type_name,fc4sc::global::get_file_id(file_name,mod.get_scp_data()->cntxt),line);
    mod.register_cvg(this);
  }

  uint32_t set_strings(cvp_base *cvp, void *sample_point, const std::string& cvp_name, const std::string& expr_name) {
    cvp_strings[cvp] = cvp_metadata_t(sample_point, cvp_name, expr_name);
    return 0;
  }

  virtual cvp_metadata_t get_strings(cvp_base *cvp) {
    return cvp_strings[cvp];
  }

  /*! Re-enabled default constructor for dynamic_covergroup_factory */
  //covergroup() = delete;
  /*! Disabled */
  covergroup &operator=(const covergroup &other) = delete;
  /*! Disabled */
  covergroup(const covergroup &other) = delete;
  /*! Disabled */
  covergroup(covergroup &&other) = delete;
  // ! Disabled
  covergroup &operator=(covergroup &&other) = delete;

  /*! Destructor */
  virtual ~covergroup() { }

public:

  virtual void sample() {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    if(this->is_enabled()) {
      for (auto& cvp : this->cvps) {
        try {
          cvp->sample();
        }
        catch(illegal_bin_sample_exception &e) {
          e.update_cvg_info(this->name());
          std::cerr << e.what() << std::endl;
#ifndef FC4SC_NO_THROW // By default the simulation will stop
          std::cerr << "Stopping simulation\n";
	  throw(e);
#endif
        }
      }
    }
    else {
      std::cerr << "Warning: attempted to sample a disabled covergroup\n";
    }
  }

  virtual void disable_cvg() {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    throw("Cannot dynamically disable statically defined covergroup " + this->name());
  }

  /*!
   * \brief Computes coverage of an instance
   * \returns Coverage percentage of this instance
   */
  double get_inst_coverage()  const 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    if(this->is_enabled()) {
      double res = 0;
      double weights = 0;

      for (auto &cvp : cvps) {
        res += cvp->get_inst_coverage() * cvp->get_weight();
        weights += cvp->get_weight();

      }

      if (weights == 0 || cvps.size() == 0 || res == 0)
        return (this->option().weight == 0) ? 100 : 0;

      double real = res / weights;
      return (real >= this->option().goal) ? 100 : real;
    }
    else {
      std::cerr << "Warning: called get_inst_coverage on disabled covergroup\n";
      return 100;
    }
  }

  /*!
   * \brief Computes coverage of an instance
   * \param bins_covered Number of covered bins
   * \param total Total number of bins in this covergroup
   * \returns Coverage percentage of this instance
   */
  double get_inst_coverage(int &bins_covered, int &total) const 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    if(this->is_enabled()) {
      double res = 0;
      double weights = 0;
      int bins_aux = 0;
      int total_aux = 0;

      bins_covered = 0;
      total = 0;

      for (auto &cvp : cvps)
      {
        res += cvp->get_inst_coverage(bins_aux, total_aux) * cvp->get_weight();
        bins_covered += bins_aux;
        total += total_aux;
        weights += cvp->get_weight();
      }

      if (weights == 0 || cvps.size() == 0 || res == 0)
      {
        return (this->option().weight == 0) ? 100 : 0;
      }

      double real = res / weights;
      return (real >= this->option().goal) ? 100 : real;
    }
    else {
      std::cerr << "Warning: called get_inst_coverage on disabled covergroup\n";
      return 100;
    }
  }

  /*!
   *  \brief Computes coverage count across all instances of this type
   *  \returns Coverage percentage of this instance
   */
  double get_coverage(fc4sc::global* cvg_cntxt = nullptr)
  {
    if(cvg_cntxt == nullptr)
    {
      if(this->parent_scp == nullptr) {
        cvg_cntxt = fc4sc::global::getter();
      }
      else {
        cvg_cntxt = this->parent_scp->get_scp_data()->cntxt;
      }
    }
    return fc4sc::global::get_coverage(this->scp_type_name(), this->type_name(), cvg_cntxt);
  }

  /*!
   * \brief Computes coverage of an instance
   * \param bins_covered Number of covered bins
   * \param total Total number of bins in this covergroup
   * \returns Coverage percentage of this instance
   */
  double get_coverage(int &bins_covered, int &total, fc4sc::global* cvg_cntxt = nullptr)
  {
    if(cvg_cntxt == nullptr)
    {
      if(this->parent_scp == nullptr) {
        cvg_cntxt = fc4sc::global::getter();
      }
      else {
        cvg_cntxt = this->parent_scp->get_scp_data()->cntxt;
      }
    }
    return fc4sc::global::get_coverage(this->scp_type_name(), this->type_name(), bins_covered, total, cvg_cntxt);
  }

  /*!
   * \brief Enables sampling on all coverpoints/crosses
   */
  void start()
  {
    for (auto i : cvps)
      i->start();
  };

  /*!
   * \brief Enables sampling on all coverpoints/crosses
   */
  void stop()
  {
    for (auto i : cvps)
      i->stop();
  };

  cvg_base_data_model* get_cvg_data()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return cvg_data;
  }

  std::string& name() 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return cvg_data->name;
  }

  cvg_option& option() const
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return cvg_data->option;
  }

  cvg_type_option& type_option()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return cvg_data->type_data->type_option;
  }

  std::string& type_name()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return cvg_data->type_data->type_name;
  }

  std::string& scp_type_name()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return cvg_data->type_data->scp_type_name;
  }
  
  unsigned int& file_id()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return cvg_data->type_data->file_id;
  }

  uint32_t& line()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return cvg_data->type_data->line;
  }

  unsigned int& inst_file_id()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return cvg_data->inst_file_id;
  }

  uint32_t& inst_line()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return cvg_data->inst_line;
  }

  bool is_enabled() const
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return cvg_data->enable;
  }

};

/*!
 * \class dynamic_covergroup_factory covergroup.hpp
 * \brief class for creating dynamic covergroup definition.
 */
class dynamic_covergroup_factory {
public:

  std::string type_name;
  std::string file_name;
  uint32_t line;

  /*! Array of associated coverpoints and crosses */
  std::vector<cvp_base *> cvps;

  scope_factory_base* scp_type_factory = nullptr;

  dynamic_covergroup_factory(const char *type_name, const char *file_name = "", int line = 1)
  {
    this->type_name = type_name;
    this->file_name = file_name;
    this->line = line;
  }

  dynamic_covergroup_factory(scope_factory_base& scp,const char *type_name, const char *file_name = "", int line = 1) 
  {
    this->type_name = type_name;
    this->file_name = file_name;
    this->line = line;
    scp.register_sub_cvg_type(this);
  }

  std::string get_scp_type_name()
  {
    if(scp_type_factory != nullptr) {
      return scp_type_factory->get_scp_type_name();
    }
    else {
      return _FC4SC_DEFAULT_SCOPE_TYPE_;
    }
  }

  /*!
   * \brief Create a coverpoint without specifying a sample condition expression. Adds a coverpoint to the dynamic
   * covergroup factory which will be used again when the dynamic covergroup is instanced. Returns a dynamic coverpoint
   * factory object for the user to add bins to the created coverpoint. template specifies the type of sample expression
   */
  template<typename T>
  dynamic_coverpoint_factory<T,bool()> create_coverpoint(std::string cvp_name, std::function<T> sample_expr) 
  {
    typedef typename std::function<T>::result_type ret_type;

    for(auto cvp_it : this->cvps) {
      if(cvp_it->name() == cvp_name)
        throw ("Cannot have multiple coverpoints with same name " + cvp_name + " in covergroup" + " " + this->type_name);
    }

    coverpoint<ret_type>* dyn_cvp = new coverpoint<ret_type>;
    dyn_cvp->name() = cvp_name;
    dyn_cvp->has_sample_expression = true;
    dyn_cvp->cvp_data->sample_expression_str = "dynamic sample";
    dyn_cvp->cvp_data->sample_condition_str = "dynamic sample";
    dyn_cvp->sample_condition = [](){ return true; };
    this->cvps.push_back(dyn_cvp);

    dynamic_coverpoint_factory<T,bool()> dyn_cvp_fact;
    dyn_cvp_fact.cvp_fact = dyn_cvp;
    dyn_cvp_fact.sample_expression = sample_expr;
    dyn_cvp_fact.sample_condition = [](){ return true; };
    return dyn_cvp_fact;
  }

  /*!
   * \brief Create a coverpoint. Adds a coverpoint to the dynam covergroup factory 
   * which will be used again when the dynamic covergroup is instanced. Returns a dynamic coverpoint
   * factory object for the user to add bins to the created coverpoint. Template args needed to specify sample
   * expression type and sample condition expression type
   */
  template<typename T,typename U>
  dynamic_coverpoint_factory<T,U> create_coverpoint(std::string cvp_name, std::function<T> sample_expr, std::function<U> cond_expr ) 
  {
    typedef typename std::function<T>::result_type ret_type;

    for(auto cvp_it : this->cvps) {
      if(cvp_it->name() == cvp_name)
        throw ("Cannot have multiple coverpoints with same name " + cvp_name + " in covergroup" + " " + this->type_name);
    }

    coverpoint<ret_type>* dyn_cvp = new coverpoint<ret_type>;
    dyn_cvp->name() = cvp_name;
    dyn_cvp->has_sample_expression = true;
    dyn_cvp->cvp_data->sample_expression_str = "dynamic sample";
    dyn_cvp->cvp_data->sample_condition_str = "dynamic sample";
    this->cvps.push_back(dyn_cvp);

    dynamic_coverpoint_factory<T,U> dyn_cvp_fact;
    dyn_cvp_fact.cvp_fact = dyn_cvp;
    dyn_cvp_fact.sample_expression = sample_expr;
    dyn_cvp_fact.sample_condition = cond_expr;
    return dyn_cvp_fact;
  }

  /*!
   * \brief Add cross to dynamic_covergroup type
   * \param cross name strig
   * \param pointers to dynamic_coverpoint_factory of crossed coverpoints
   */
  template<typename ... Args>
  void add_cross(std::string crs_name, Args&... args)
  {
    internal_add_cross(crs_name,(args.cvp_fact)...);
  }

  /*!
   * \brief Helper function to add cross to covergroup
   */
  template<typename ... Args>
  void internal_add_cross(std::string& crs_name, coverpoint<Args>*... args)
  {
    cross<Args...>* crs = new cross<Args...>(crs_name,args...);
    this->cvps.push_back(crs);
  }

  ~dynamic_covergroup_factory()
  {
    for (auto cvp : this->cvps)
    {
      delete cvp->get_data();
      delete cvp;
    }
  }
};


/*!
 * \class dynamic_covergroup covergroup.hpp
 * \brief instances dynamic covergroup types. Constructor requires a dynamic_covergroup_factory
 * to specify the type of the dynamic covergroup instance
 */
class dynamic_covergroup : public covergroup {
public:

  dynamic_covergroup(dynamic_covergroup_factory& cvg_type, std::string inst_name = "",const char *inst_file_name = "", int inst_line = 1, fc4sc::global* cntxt = fc4sc::global::getter()) : covergroup(cvg_type.type_name.c_str(),cvg_type.get_scp_type_name(),cvg_type.file_name.c_str(),cvg_type.line,inst_name,inst_file_name,inst_line,cntxt)
  {
    //iterate through each coverpoint of the covergroup type and create a dynamic instance of the coverpoint
    for (auto cvp_it : cvg_type.cvps) 
    {
      cvp_base* next_cvp = cvp_it->create_instance(this);
      this->cvg_base::register_cvp(next_cvp);
      this->get_cvg_data()->add_cvp_data(next_cvp->get_data());
    }

  }

  dynamic_covergroup(scp_base& scp,dynamic_covergroup_factory& cvg_type, std::string inst_name = "",const char *inst_file_name = "", int inst_line = 1) : covergroup(scp,cvg_type.type_name.c_str(),cvg_type.get_scp_type_name(),cvg_type.file_name.c_str(),cvg_type.line,inst_name,inst_file_name,inst_line)
  {
    //iterate through each coverpoint of the covergroup type and create a dynamic instance of the coverpoint
    for (auto cvp_it : cvg_type.cvps) 
    {
      cvp_base* next_cvp = cvp_it->create_instance(this);
      this->cvg_base::register_cvp(next_cvp);
      this->get_cvg_data()->add_cvp_data(next_cvp->get_data());
    }

  }
  
  void disable_cvg() {
    get_cvg_data()->enable = false;
  }

  ~dynamic_covergroup()
  {
    for (auto cvp : cvps)
    {
      delete cvp;
    }
  }

};

} // namespace fc4sc

#endif /* FC4SC_COVERGROUP_HPP */
