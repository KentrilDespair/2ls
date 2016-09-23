/*******************************************************************\

Module: ACDL Worklist Management Base

Author: Rajdeep Mukherjee, Peter Schrammel

 \*******************************************************************/

#include <util/find_symbols.h>
#include "acdl_worklist_base.h"

#define DEBUG


#ifdef DEBUG
#include <iostream>
#endif

/*******************************************************************\

Function: acdl_worklist_baset::push_into_assertion_list()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

void
acdl_worklist_baset::push_into_assertion_list (assert_listt &aexpr,
				  const acdl_domaint::statementt &statement)
{
  aexpr.push_back(statement);
}

/*******************************************************************	\

Function: acdl_worklist_baset::check_statement()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

bool
acdl_worklist_baset::check_statement (const exprt &expr,
                               const acdl_domaint::varst &vars)
{

  std::set<symbol_exprt> symbols;
  // find all variables in a statement
  find_symbols (expr, symbols);
  // check if vars appears in the symbols set,
  // if there is a non-empty intersection, then insert the
  // equality statement in the worklist
  for (acdl_domaint::varst::const_iterator it = vars.begin ();
      it != vars.end (); it++)
  {
    if (symbols.find (*it) != symbols.end ())
    {
      // find_symbols here may be required to 
      // find transitive dependencies
      // in which case make vars non-constant
      //find_symbols(expr, vars);
      return true;
    }
  }
  return false;
}

/*******************************************************************\

Function: acdl_worklist_baset::push_into_worklist()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

void
acdl_worklist_baset::push (const acdl_domaint::statementt &statement)
{
  std::cout << "Pushing into worklist " << from_expr(statement) << std::endl;
  for(worklistt::const_iterator it = worklist.begin();
      it != worklist.end(); ++it) {
    if(statement == *it)
      return;
  }
#ifdef DEBUG  
  std::cout << "Push element into worklist " << from_expr(statement) << std::endl;
#endif  
  worklist.push_back(statement);
}

/*******************************************************************\

  Function: acdl_worklist_baset::push_into_map()

  Inputs:

  Outputs:

  Purpose: 

 \*******************************************************************/

void
acdl_worklist_baset::push_into_map (const acdl_domaint::statementt &statement, const acdl_domaint::varst &lvars)
{
#ifdef DEBUG  
  std::cout << "Pushing in to map" << std::endl;
  std::cout << "Statement is " << from_expr(statement) << " ===> "; 
  for(acdl_domaint::varst::const_iterator it1 = 
      lvars.begin(); it1 != lvars.end(); ++it1)
    std::cout << from_expr(*it1) << ", "; 
  std::cout << std::endl;
#endif

  std::map<acdl_domaint::statementt,acdl_domaint::varst>::iterator itf; 
  itf = svpair.find(statement);
  if(itf != svpair.end()) {} // handle later
  else {
    svpair.insert(make_pair(statement, lvars));   
  }

  // iterate over whole map and 
  // update the live varaibles
  for(std::map<acdl_domaint::statementt, acdl_domaint::varst>::iterator
    it=svpair.begin(); it!=svpair.end(); ++it) {
#if 0   
   // check if statement already exists
   if(it->first == statement) {
     // check the live var list 
     // only add live variables not present
     acdl_domaint::varst live_vars = it->second;
     for(acdl_domaint::varst::const_iterator it1 = 
         lvars.begin(); it1 != lvars.end(); ++it1)
     {
       acdl_domaint::varst::iterator lit = live_vars.find(*it1); 
       if(lit == live_vars.end()) live_vars.insert(*it1);
       else continue;
     }
   }
   // for other statements, simply 
   // update the live varaibles if not present
   else {
#endif
#ifdef DEBUG          
     std::cout << "Looping over map statement: " << from_expr(it->first) << std::endl; 
     
#endif
     acdl_domaint::varst live_vars = it->second;
     for(acdl_domaint::varst::const_iterator it1 = 
         lvars.begin(); it1 != lvars.end(); ++it1)
     {
       std::cout << "Checking deduction vars" << from_expr(*it1) << " ,";
       acdl_domaint::varst::iterator lit = live_vars.find(*it1); 
       if(lit == live_vars.end()) {
         std::cout << "insert in to live vars" << std::endl;
         it->second.insert(*it1);
       }
       else continue;
     }
   //}
  }
#if 0  
  std::map<acdl_domaint::statementt,acdl_domaint::varst>::iterator itf; 
  itf = svpair.find(statement);
  if(itf != svpair.end()) {
    // check the live var list 
    // only add live variables not present
    acdl_domaint::varst live_vars = itf->second;
    for(acdl_domaint::varst::const_iterator it1 = 
        lvars.begin(); it1 != lvars.end(); ++it1)
    {
      acdl_domaint::varst::iterator lit = live_vars.find(*it1); 
      if(lit == live_vars.end()) live_vars.insert(*it1);
      else continue;
    }
  }
  else
  {
    svpair.insert(make_pair(statement, lvars));   
  }
#endif  
}

/*******************************************************************\

  Function: acdl_worklist_baset::delete_map()

  Inputs:

  Outputs:

  Purpose:

\*******************************************************************/

void acdl_worklist_baset::delete_map()
{    
  while(!svpair.empty())
   svpair.erase(svpair.begin());
}

/*******************************************************************\

  Function: acdl_worklist_baset::delete_from_map()

  Inputs:

  Outputs:

  Purpose:

\*******************************************************************/

void acdl_worklist_baset::delete_from_map (const acdl_domaint::statementt &statement)
{
  std::map<acdl_domaint::statementt,acdl_domaint::varst>::iterator itf; 
  itf = svpair.find(statement);
  svpair.erase(itf);
}


/*******************************************************************\

Function: acdl_worklist_baset::check_var_liveness()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

acdl_domaint::varst acdl_worklist_baset::check_var_liveness (acdl_domaint::varst &vars)
{
  
  for(acdl_domaint::varst::const_iterator it = 
    vars.begin(); it != vars.end(); ++it)
  {
    acdl_domaint::varst::iterator it1 = live_variables.find(*it);
    if(it1 == live_variables.end()) vars.erase(*it);
  }
  return vars;
}


/*******************************************************************\

Function: acdl_worklist_baset::remove_live_variables()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

void acdl_worklist_baset::remove_live_variables 
  (const local_SSAt &SSA, const acdl_domaint::statementt &statement)
{
#ifdef DEBUG
        std::cout << "Popped Statement for live variables: " 
        << from_expr (SSA.ns, "", statement) << std::endl;
#endif
  
  // remove variables in statement from live variables
  acdl_domaint::varst del_vars;
  find_symbols(statement, del_vars);  
#ifdef DEBUG
  std::cout << "Variables in Popped Statement: "; 
  for(acdl_domaint::varst::const_iterator it = 
    del_vars.begin(); it != del_vars.end(); ++it)
      std::cout << from_expr (SSA.ns, "", *it) << " ";
  std::cout << " " << std::endl;
#endif

#ifdef DEBUG   
  std::cout << "Live variables list are as follows: ";
  for(acdl_domaint::varst::const_iterator it = 
    live_variables.begin(); it != live_variables.end(); ++it)
   std::cout << from_expr(SSA.ns, "", *it); 
   std::cout << " " << std::endl;
#endif
  bool found = false;
  for(acdl_domaint::varst::const_iterator it = 
    del_vars.begin(); it != del_vars.end(); ++it) {
   found = false;
   for(std::list<acdl_domaint::statementt>::const_iterator 
      it1 = worklist.begin(); it1 != worklist.end(); ++it1)
   {
     acdl_domaint::varst find_vars;
     find_symbols(*it1, find_vars);
     acdl_domaint::varst::const_iterator it2 = find_vars.find(*it);   
     if(it2 != find_vars.end()) {found = true; break; }
   }  
   if(found == false)   
   {
#ifdef DEBUG
        std::cout << "Deleted live variables are: " 
        << from_expr (SSA.ns, "", *it) << std::endl;
#endif
       live_variables.erase(*it);
   }
  }
}


/*******************************************************************\

Function: acdl_worklist_baset::pop_from_worklist()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

const acdl_domaint::statementt
acdl_worklist_baset::pop ()
{
#if 1
  const acdl_domaint::statementt statement = worklist.front();
  worklist.pop_front();
#else
  worklistt::iterator it = worklist.begin ();
  const exprt statement = *it;
  worklist.erase (it);
#endif
  return statement;
}

/*******************************************************************

 Function: acdl_worklist_baset::select_vars()

 Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

void
acdl_worklist_baset::select_vars (const exprt &statement, acdl_domaint::varst &vars)
{
#if 0 //TODO: this was an attempt to implement a forward iteration strategy,
      //      but we would also need to consider execution order 
  // If it is an equality, then select the lhs for post-condition computation
  exprt lhs;
  if (statement.id () == ID_equal)
  {
    lhs = to_equal_expr (statement).lhs ();
    if (lhs.id () == ID_symbol)
    {
      vars.insert (to_symbol_expr (lhs));
    }
    else //TODO: more complex lhs
      assert(false);
  }
  else // for constraints
#endif
  {
    find_symbols(statement,vars);
  }
}

/*******************************************************************\

Function: acdl_worklist_orderedt::initialize()

  Inputs:

 Outputs:

 Purpose: Initialize the worklist

 \*******************************************************************/

void
acdl_worklist_baset::initialize_live_variables ()
{
  //Strategy 0: initialize live variables for each 
  //statement by adding all live variables
  for(std::list<acdl_domaint::statementt>::const_iterator 
      it = worklist.begin(); it != worklist.end(); ++it) {
    acdl_domaint::varst insert_vars;
    select_vars(*it, insert_vars);
    for(acdl_domaint::varst::const_iterator it1 = 
        insert_vars.begin(); it1 != insert_vars.end(); ++it1)
      live_variables.insert(*it1);   
  }
  // insert all live variables for each statement
  for(std::list<acdl_domaint::statementt>::const_iterator 
    it = worklist.begin(); it != worklist.end(); ++it)
  {
    svpair.insert(make_pair(*it, live_variables)); 
  }
  std::cout << "Printing the initialized map" << std::endl; 
  for(std::map<acdl_domaint::statementt, acdl_domaint::varst>::iterator
    it=svpair.begin(); it!=svpair.end(); ++it) {
    
    std::cout << "Statement is " << from_expr(it->first) << "==>"; 
    acdl_domaint::varst live_vars = it->second;
    for(acdl_domaint::varst::const_iterator it1 = 
        live_vars.begin(); it1 != live_vars.end(); ++it1)
      std::cout << from_expr(*it1) << ", "; 
      std::cout << std::endl;
  }
#if 0  
  //Strategy 1: initialize live variables by adding all vars
  for(std::list<acdl_domaint::statementt>::const_iterator 
      it = worklist.begin(); it != worklist.end(); ++it) {
    acdl_domaint::varst insert_vars;
    select_vars(*it, insert_vars);
    for(acdl_domaint::varst::const_iterator it1 = 
        insert_vars.begin(); it1 != insert_vars.end(); ++it1)
      live_variables.insert(*it1);   
  }
#endif
/* 
  // Strategy 2: initialize live variables by inserting only lhs vars 
  // from ID_equal statements for forward analysis 
  for(std::list<acdl_domaint::statementt>::const_iterator 
      it = worklist.begin(); it != worklist.end(); ++it) {
    if(it->id() == ID_equal) {
      exprt expr_lhs = to_equal_expr(*it).lhs();
      find_symbols(expr_lhs, live_variables);
    }
  }

#if 0
  // Strategy 3: initialize live variables by inserting only rhs vars 
  // from ID_equal statements for backward analysis 
  for(std::list<acdl_domaint::statementt>::const_iterator 
      it = worklist.begin(); it != worklist.end(); ++it) {
    if(it->id() == ID_equal) {
      exprt expr_rhs = to_equal_expr(*it).rhs();
      find_symbols(expr_rhs, live_variables);
    }
  }
#endif
*/
 
#if 0
  std::cout << "Printing all live variables" << std::endl;
  for(acdl_domaint::varst::const_iterator 
    it = live_variables.begin(); it != live_variables.end(); ++it)
      std::cout << it->get_identifier() << "," << std::endl;
#endif
}   
  
/*******************************************************************\

Function: acdl_worklist_orderedt::print()

 Inputs:

 Outputs:

 Purpose: Print the worklist

 \*******************************************************************/

void acdl_worklist_baset::print_worklist() 
{
  std::cout << "Printing the worklist" << std::endl; 
  for(std::list<acdl_domaint::statementt>::const_iterator 
      it = worklist.begin(); it != worklist.end(); ++it)
    std::cout << from_expr(*it) <<  "," << std::endl;
  std::cout << std::endl; 
}
