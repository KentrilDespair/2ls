/*******************************************************************\

Module: ACDL Worklist Heuristics (Order SSA elements)

Author: Rajdeep Mukherjee, Peter Schrammel

\*******************************************************************/

#ifndef CPROVER_ACDL_WORKLIST_ORDERED_H
#define CPROVER_ACDL_WORKLIST_ORDERED_H

#include "acdl_worklist_base.h"

class acdl_worklist_orderedt : public acdl_worklist_baset
{
public:
  explicit acdl_worklist_orderedt()
    : 
  acdl_worklist_baset()
  {
  }
    
  virtual void initialize(const local_SSAt &SSA, const exprt &assertion, const exprt& additional_constraint);

  virtual void dec_update(const local_SSAt &SSA, const acdl_domaint::meet_irreduciblet &stmt, const exprt& assertion);
 
  virtual acdl_domaint::varst pop_from_map (const acdl_domaint::statementt &statement);

  
  virtual void update(const local_SSAt &SSA,
		      const acdl_domaint::varst &new_vars,
		      const acdl_domaint::statementt &statement=nil_exprt(), const exprt& assertion=true_exprt());

  // virtual void initialize_live_variables();
protected:
  //typedef std::list<acdl_domaint::statementt> assert_listt;
  typedef std::list<acdl_domaint::statementt> listt;
  
  void push_into_list (listt &lexpr,
				 const acdl_domaint::statementt &statement);

  const acdl_domaint::statementt pop_from_list (listt &lexpr);


  void update (const local_SSAt &SSA,
	       const acdl_domaint::varst &vars,
	       listt &lexpr, 
	       const acdl_domaint::statementt &current_statement,
         const exprt& assertion);
};

#endif
