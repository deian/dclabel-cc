#include <iostream>
#include <set>
#include <algorithm>
#include "dclabel.h"

//
// Clauses
//
//

Clause::Clause() {
  clause.clear();
}

Clause::Clause(std::set<Principal>& c) {
  clause = c;
}

Clause::Clause(Principal ps[], size_t len) {
  std::set<Principal> c(ps,ps+len);
  clause = c;
}

Clause::Clause(const Clause& c) {
  clause = c.clause;
}

void Clause::setTo(const std::set<Principal>& c) {
  this->clause = c;
}

// c1 ==> c2 iff
// c1 `isSubsetOf` c2
bool Clause::implies(const Clause& that) const {
  //that is subset of this:
  return std::includes(that.clause.begin(), that.clause.end()
  									  ,this->clause.begin(), this->clause.end());
}

bool operator< (const Clause& c1, const Clause& c2) {
  return c1 <= c2 && c1 != c2;
}
  
bool operator<= (const Clause& c1, const Clause& c2) {
  if(c1.clause.size() ==  c2.clause.size()) {
  	return c1.clause <= c2.clause;
  } else {
  	return c1.clause.size() <  c2.clause.size();
  }
}

bool operator> (const Clause& c1, const Clause& c2) {
  return !(c1 <= c2);
}
  
bool operator>= (const Clause& c1, const Clause& c2) {
  return !(c1 <  c2);
}

bool operator== (const Clause& c1, const Clause& c2) {
  return c1.clause == c2.clause;
}

bool operator!= (const Clause& c1, const Clause& c2) {
  return c1.clause != c2.clause;
}

std::ostream& operator<< (std::ostream& o, const Clause& c) {
  o << "[";
  int flag = c.clause.size() - 1;
  for( std::set<Principal>::iterator i = c.clause.begin()
  	 ; i != c.clause.end(); --flag, ++i ) {
  	o << *i;
    if(flag) { o << " \\/ "; }
  }
  o << "]";
  return o;
}

//
// Components
//


Component::Component() {
  DCFalse = true;
}

Component::Component(const Component &c) {
  DCFalse = c.DCFalse;
  DCFormula = c.DCFormula;
}

Component Component::dcFalse() {
  Component c;
  c.setToDCFalse();
  return c;
}

Component Component::dcTrue() {
  Component c;
  c.setToDCTrue();
  return c;
}

Component Component::dcFormula(const Clause& clause) {
  std::set<Clause> cs;
  cs.insert(clause);
  return dcFormula(cs);
}

Component Component::dcFormula(const std::set<Clause>& f) {
  Component c;
  c.DCFalse = false;
  c.DCFormula = f;
  return c;
}

bool operator== (const Component& c1, const Component& c2) {
    return (c1.DCFalse == c2.DCFalse) && (c1.DCFormula == c2.DCFormula);
}

bool operator!= (const Component& c1, const Component& c2) {
    return !(c1==c2);
}

bool Component::isFalse() const {
  return DCFalse;
}

bool Component::isTrue() const {
  return !DCFalse && DCFormula.empty();
}

bool Component::implies(const Component& that) const {
  // special cases:
  if( this->isFalse() ) { return true;  }
  if( that.isFalse()  ) { return false; }
  if( that.isTrue()   ) { return true;  }
  if( this->isTrue()  ) { return false; }
  
  // for all clauses in that there must be at least one in
  // this that implies it.
  for( std::set<Clause>::iterator cs2 = that.DCFormula.begin()
  	 ; cs2 != that.DCFormula.end(); cs2++) {
    bool flag = false;
  	for( std::set<Clause>::iterator cs1 = this->DCFormula.begin()
  		 ; cs1 != this->DCFormula.end(); cs1++) {
  		flag |= cs1->implies(*cs2);
  	}
  	if(!flag) return false;
  }
  return true;
}

void Component::dcAnd(const Component& that) {

  if( this->isFalse() || that.isFalse() ) { 
  	this->setToDCFalse();
  	return;
  }

  this->DCFormula.insert(that.DCFormula.begin(),that.DCFormula.end());
}

void Component::dcOr(const Component& that) {

  if( this->isTrue() || that.isTrue() ) {
  	this->setToDCTrue();
  	return;
  }
  if( that.isFalse() ) {
  	return;
  }
  if( this->isFalse() ) {
  	this->setTo(that);
  	return;
  }
  	
  Component component = dcTrue();
  
  for( std::set<Clause>::iterator c1 = this->DCFormula.begin()
  	 ; c1 != this->DCFormula.end(); ++c1 ) {

  	Clause c = *c1;
  	for( std::set<Clause>::iterator c2 = that.DCFormula.begin()
  		 ; c2 != that.DCFormula.end(); ++c2 ) {

  		c.clause.insert(c2->clause.begin(),c2->clause.end());

  	}
  	component.DCFormula.insert(c);

  }
  this->setTo(component);
}

void Component::dcReduce() {

  if ( this->isFalse() || this->isTrue() ) return;

  std::set<Clause> rmList;

  for( std::set<Clause>::iterator c1 = this->DCFormula.begin()
  	 ; c1 != this->DCFormula.end(); ++c1 ) {

  	for( std::set<Clause>::reverse_iterator c2 = this->DCFormula.rbegin()
  		 ; *c2 != *c1; ++c2 ) {
  		if(c1->implies(*c2)) {
  		  rmList.insert(*c2);
  		}
  	}

  }
  for( std::set<Clause>::iterator r = rmList.begin() ; r != rmList.end(); ++r ) {
  	this->DCFormula.erase(*r);
  }
}

std::ostream& operator<< (std::ostream& o, const Component& c) {
  if( c.isFalse() ) {
  	o << "|False";
  } else if( c.isTrue() ) {
  	o << "|True";
  } else {
  	o << "{";
  	std::set<Clause> s = c.DCFormula;
  	int flag = s.size() - 1;
  	for( std::set<Clause>::iterator i = s.begin()
  		 ; i != s.end(); ++i, flag-- ) {
  		o << *i;
      if(flag) { o << " /\\ "; }
  	}
  	o << "}";
  }
  return o;
}

void Component::setToDCFalse() {
  this->DCFalse = true;
  this->DCFormula.clear();
}

void Component::setToDCTrue() {
  this->DCFalse = false;
  this->DCFormula.clear();
}

void Component::setTo(const Component &c) {
  this->DCFalse = c.DCFalse;
  this->DCFormula = c.DCFormula;
}

//
// Labels
//

DCLabel::DCLabel() {
  secrecy = Component::dcTrue();
  integrity = Component::dcTrue();
}

DCLabel::DCLabel(const Component& s, const Component& i) {
  secrecy = s;
  integrity = i;
  secrecy.dcReduce();
  integrity.dcReduce();
}

DCLabel::DCLabel(const DCLabel& l) {
  secrecy = l.secrecy;
  integrity = l.integrity;
}

bool operator== (const DCLabel& l1, const DCLabel& l2) {
  return l1.secrecy == l2.secrecy && l1.integrity == l2.integrity;
}

bool operator!= (const DCLabel& l1, const DCLabel& l2) {
  return !(l1 == l2);
}

void DCLabel::dcReduce() {
  this->secrecy.dcReduce();
  this->integrity.dcReduce();
}

DCLabel DCLabel::dcBottom() {
  return DCLabel(Component::dcTrue(),Component::dcFalse());
}

DCLabel DCLabel::dcTop() {
  return DCLabel(Component::dcFalse(),Component::dcTrue());
}

DCLabel DCLabel::dcPub() {
  return DCLabel(Component::dcTrue(),Component::dcTrue());
}

bool DCLabel::canFlowTo(const DCLabel& that) const {
  return that.secrecy.implies(this->secrecy)
  		&& this->integrity.implies(that.integrity);
}

bool DCLabel::canFlowTo(const DCLabel& l1, const DCLabel& l2) {
  return l2.secrecy.implies(l1.secrecy) && l1.integrity.implies(l2.integrity);
}

void DCLabel::lub(const DCLabel& that) {
  this->secrecy.dcAnd(that.secrecy);
  this->integrity.dcOr(that.integrity);
  this->secrecy.dcReduce();
  this->integrity.dcReduce();
}

DCLabel DCLabel::lub(const DCLabel& l1, const DCLabel& l2) {
  DCLabel l = l1;
  l.lub(l2);
  return l;
}

void DCLabel::glb(const DCLabel& that) {
  this->secrecy.dcOr(that.secrecy);
  this->integrity.dcAnd(that.integrity);
  this->secrecy.dcReduce();
  this->integrity.dcReduce();
}

DCLabel DCLabel::glb(const DCLabel& l1, const DCLabel& l2) {
  DCLabel l = l1;
  l.glb(l2);
  return l;
}
