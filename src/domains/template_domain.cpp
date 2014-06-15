#include "template_domain.h"

#include <iostream>

#include <util/find_symbols.h>
#include <util/arith_tools.h>
#include <util/ieee_float.h>
#include <util/simplify_expr.h>
#include <langapi/languages.h>

void template_domaint::bottom(valuet &value)
{
  value.resize(templ.size());
  for(unsigned row = 0; row<templ.size(); row++)
  {
    value[row] = false_exprt(); //marker for -oo
  }
}

void template_domaint::set_to_top(const var_listt &top_vars, valuet &value)
{
  assert(value.size()==templ.size());
  
  find_symbols_sett top_symbols;
  for(var_listt::const_iterator 
      it=top_vars.begin();
      it!=top_vars.end(); 
      ++it)
  {
    top_symbols.insert(it->get_identifier());
  }
  
  for(unsigned row = 0; row<templ.size(); row++)
  {
    const exprt &row_expr=templ.rows[row];
  
    if(has_symbol(row_expr, top_symbols))
    {
      value[row] = true_exprt(); //get_max_row_value(row); //marker for oo
    }
  }
}

template_domaint::row_valuet template_domaint::between(
  const row_valuet &lower, const row_valuet &upper)
{
  if(lower.type()==upper.type() && 
     (lower.type().id()==ID_signedbv || lower.type().id()==ID_unsignedbv))
  {
    mp_integer vlower, vupper;
    to_integer(lower, vlower);
    to_integer(upper, vupper);
    assert(vupper>=vlower);
    return from_integer((vupper-vlower)/2,lower.type());
  }
  if(lower.type().id()==ID_floatbv && upper.type().id()==ID_floatbv)
  {
    ieee_floatt vlower(to_constant_expr(lower));
    ieee_floatt vupper(to_constant_expr(upper));
    if(vlower.get_sign()==vupper.get_sign()) 
    {
      mp_integer plower = vlower.pack(); //compute "median" float number
      mp_integer pupper = vupper.pack();
      //assert(pupper>=plower);
      ieee_floatt res;
      res.unpack((plower-pupper)/2); //...by computing integer mean
      return res.to_expr();
    }
    ieee_floatt res;
    res.make_zero();
    return res.to_expr();
  }
  assert(false); //types do not match or are not supported
}

bool template_domaint::leq(const row_valuet &v1, const row_valuet &v2)
{
  if(v1.type()==v2.type() && 
     (v1.type().id()==ID_signedbv || v1.type().id()==ID_unsignedbv))
  {
    mp_integer vv1, vv2;
    to_integer(v1, vv1);
    to_integer(v2, vv2);
    return vv1<=vv2;
  }
  if(v1.type().id()==ID_floatbv && v2.type().id()==ID_floatbv)
  {
    ieee_floatt vv1(to_constant_expr(v1));
    ieee_floatt vv2(to_constant_expr(v2));
    return vv1<=vv2;
  }
  assert(false); //types do not match or are not supported
}

exprt template_domaint::get_row_constraint(const rowt &row, const row_valuet &row_value)
{
  assert(row<templ.size());
  if(is_row_value_neginf(row_value)) return implies_exprt(templ.guards[row], false_exprt());
  if(is_row_value_inf(row_value)) return true_exprt();
  return implies_exprt(templ.guards[row], binary_relation_exprt(templ.rows[row],ID_le,row_value));
}

exprt template_domaint::get_row_constraint(const rowt &row, const valuet &value)
{
  assert(value.size()==templ.size());
  return get_row_constraint(row,value[row]);
}

exprt template_domaint::to_constraints(const valuet &value)
{
  assert(value.size()==templ.size());
  exprt::operandst c; 
  for(unsigned row = 0; row<templ.size(); row++)
  {
    c.push_back(get_row_constraint(row,value[row]));
  }
  return conjunction(c); 
}

void template_domaint::make_not_constraints(const valuet &value,
  exprt::operandst &cond_exprs, 
  exprt::operandst &value_exprs)
{
  assert(value.size()==templ.size());
  cond_exprs.resize(templ.size());
  value_exprs.resize(templ.size());

  exprt::operandst c; 
  for(unsigned row = 0; row<templ.size(); row++)
  {
    value_exprs[row] = templ.rows[row];
    cond_exprs[row] = not_exprt(get_row_constraint(row,value));
    c.push_back(cond_exprs[row]);
  }
}

template_domaint::row_valuet template_domaint::get_row_value(
  const rowt &row, const valuet &value)
{
  assert(row<value.size());
  assert(value.size()==templ.size());
  return value[row];
}

void template_domaint::set_row_value(
  const rowt &row, const template_domaint::row_valuet &row_value, valuet &value)
{
  assert(row<value.size());
  assert(value.size()==templ.size());
  value[row] = row_value;
}

template_domaint::row_valuet template_domaint::get_max_row_value(
  const template_domaint::rowt &row)
{
  if(templ.rows[row].type().id()==ID_signedbv)
  {
    return to_signedbv_type(templ.rows[row].type()).largest_expr();
  }
  if(templ.rows[row].type().id()==ID_unsignedbv)
  {
    return to_unsignedbv_type(templ.rows[row].type()).largest_expr();
  }
  if(templ.rows[row].type().id()==ID_floatbv) 
  {
    ieee_floatt max;
    max.make_fltmax();
    return max.to_expr();
  }
  assert(false); //type not supported
}


void template_domaint::output_value(std::ostream &out, const valuet &value, 
  const namespacet &ns) const
{
  for(unsigned row = 0; row<templ.size(); row++)
  {
    out << from_expr(ns,"",templ.guards[row]) << " ===> ";
    out << " ( " << from_expr(ns,"",templ.rows[row]) << " <= ";
    if(is_row_value_neginf(value[row])) out << "-oo";
    else if(is_row_value_inf(value[row])) out << "oo";
    else out << from_expr(ns,"",value[row]);
    out << " )" << std::endl;
  }
}

void template_domaint::output_template(std::ostream &out, const namespacet &ns) const
{
  for(unsigned row = 0; row<templ.size(); row++)
  {
    out << from_expr(ns,"",templ.guards[row]) << " ===> (";
    out << from_expr(ns,"",templ.rows[row]) << " <= CONST )" << std::endl;
  }
}

unsigned template_domaint::template_size()
{
  return templ.size();
}

bool template_domaint::is_row_value_neginf(const row_valuet & row_value) const
{
  return row_value.get(ID_value)==ID_false;
}

bool template_domaint::is_row_value_inf(const row_valuet & row_value) const
{
  return row_value.get(ID_value)==ID_true;
}



void make_interval_template(template_domaint::templatet &templ, 
  const template_domaint::var_listt &vars,
  const template_domaint::var_guardst &guards,
  const namespacet &ns)
{
  assert(vars.size() == guards.size());
  unsigned size = 2*vars.size();
  templ.rows.clear(); templ.rows.reserve(size);
  templ.guards.clear(); templ.guards.reserve(size);
  
  template_domaint::var_guardst::const_iterator g = guards.begin();
  for(template_domaint::var_listt::const_iterator v = vars.begin(); 
      v!=vars.end(); v++, g++)
  {
    templ.rows.push_back(*v);
    templ.guards.push_back(*g);
    templ.rows.push_back(unary_minus_exprt(*v,v->type()));
    templ.guards.push_back(*g);
  }
  assert(templ.rows.size() == templ.guards.size());
}

void make_zone_template(template_domaint::templatet &templ, 
  const template_domaint::var_listt &vars,
  const template_domaint::var_guardst &guards,
  const namespacet &ns)
{ 
  assert(vars.size() == guards.size());
  unsigned size = 2*vars.size()+vars.size()*(vars.size()-1);
  templ.rows.clear(); templ.rows.reserve(size);
  templ.guards.clear(); templ.guards.reserve(size);
  template_domaint::var_guardst::const_iterator g1 = guards.begin();
  for(template_domaint::var_listt::const_iterator v1 = vars.begin(); 
      v1!=vars.end(); v1++, g1++)
  {
    templ.rows.push_back(*v1); 
    templ.guards.push_back(*g1);
    templ.rows.push_back(unary_minus_exprt(*v1,v1->type()));
    templ.guards.push_back(*g1);
    template_domaint::var_guardst::const_iterator g2 = g1; g2++; 
    template_domaint::var_listt::const_iterator v2 = v1; v2++;
    for(;v2!=vars.end(); v2++, g2++)
    {
      exprt g = and_exprt(*g1,*g2);
      simplify(g,ns);
      templ.rows.push_back(minus_exprt(*v1,*v2));
      templ.guards.push_back(g);
      templ.rows.push_back(minus_exprt(*v2,*v1));
      templ.guards.push_back(g);
    }
  }
  assert(templ.rows.size() == templ.guards.size());
}

void make_octagon_template(template_domaint::templatet &templ,
  const template_domaint::var_listt &vars,
  const template_domaint::var_guardst &guards,
  const namespacet &ns)
{
  assert(vars.size() == guards.size());
  unsigned size =  2*vars.size()+2*vars.size()*(vars.size()-1);
  templ.rows.clear(); templ.rows.reserve(size);
  templ.guards.clear(); templ.guards.reserve(size);
  template_domaint::var_guardst::const_iterator g1 = guards.begin();
  for(template_domaint::var_listt::const_iterator v1 = vars.begin(); 
      v1!=vars.end(); v1++, g1++)
  {
    templ.rows.push_back(*v1); 
    templ.guards.push_back(*g1);
    templ.rows.push_back(unary_minus_exprt(*v1,v1->type()));
    templ.guards.push_back(*g1);
    template_domaint::var_guardst::const_iterator g2 = g1; g2++; 
    template_domaint::var_listt::const_iterator v2 = v1; v2++;
    for(;v2!=vars.end(); v2++, g2++)
    {
      exprt g = and_exprt(*g1,*g2);
      simplify(g,ns);
      templ.rows.push_back(minus_exprt(*v1,*v2));
      templ.guards.push_back(g);
      templ.rows.push_back(minus_exprt(*v2,*v1));
      templ.guards.push_back(g);
      templ.rows.push_back(plus_exprt(*v1,*v2));
      templ.guards.push_back(g);
      templ.rows.push_back(minus_exprt(unary_minus_exprt(*v1,v1->type()),*v2));
      templ.guards.push_back(g);
    }
  }
  assert(templ.rows.size() == templ.guards.size());
}

mp_integer simplify_const_int(const exprt &expr)
{
  if(expr.id()==ID_constant) 
  {
    mp_integer v;
    to_integer(expr, v);
    return v;
  }
  if(expr.id()==ID_unary_minus) return -simplify_const_int(expr.op0());
  if(expr.id()==ID_plus) return simplify_const_int(expr.op0())+simplify_const_int(expr.op1());
  if(expr.id()==ID_minus) return simplify_const_int(expr.op0())-simplify_const_int(expr.op1());
  if(expr.id()==ID_mult) return simplify_const_int(expr.op0())*simplify_const_int(expr.op1());  
  assert(false); //not implemented
}

ieee_floatt simplify_const_float(const exprt &expr)
{
  if(expr.id()==ID_constant) 
  {
    ieee_floatt v(to_constant_expr(expr));
    return v;
  }
  if(expr.id()==ID_unary_minus) 
  {
    ieee_floatt v = simplify_const_float(expr.op0());
    v.set_sign(!v.get_sign());
    return v; 
  }
  if(expr.id()==ID_plus) 
  {
    ieee_floatt v1 = simplify_const_float(expr.op0());
    ieee_floatt v2 = simplify_const_float(expr.op1());
    v1 += v2;
    return v1; 
  }
  if(expr.id()==ID_minus)
  {
    ieee_floatt v1 = simplify_const_float(expr.op0());
    ieee_floatt v2 = simplify_const_float(expr.op1());
    v1 -= v2;
    return v1; 
  }
  if(expr.id()==ID_mult)
  {
    ieee_floatt v1 = simplify_const_float(expr.op0());
    ieee_floatt v2 = simplify_const_float(expr.op1());
    v1 *= v2;
    return v1; 
  }
  assert(false); //not implemented
}

constant_exprt simplify_const(const exprt &expr)
{
  if(expr.id()==ID_constant) return to_constant_expr(expr);
  if(expr.type().id()==ID_signedbv || expr.type().id()==ID_unsignedbv)
  {
    return to_constant_expr(from_integer(simplify_const_int(expr),expr.type()));
  }
    if(expr.type().id()==ID_floatbv)
  {
    return to_constant_expr(simplify_const_float(expr).to_expr());
  }
  assert(false); //type not supported
}