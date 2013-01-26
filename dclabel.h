#ifndef __DCLABEL__
#define __DCLABEL__

#include <iostream>
#include <set>

//
// Principals
//

// A principal is simply a string.
typedef std::string Principal;

//
// Clauses
//

// A clause is a set of principals.
// Specifically, it encodes a disjunction of principals.
class Clause {

  friend class Component;
  

public:
  // Empty constructor
  Clause();
  // Create clause form a set of principals
  Clause(std::set<Principal>&);
  // Create clause form a list of principals; the list size is
  // given by the second argument.
  Clause(Principal[], size_t);
  // Copy constructor
  Clause(const Clause&);

  // Set clause to supplied
  void setTo(const std::set<Principal>&);
  
  bool implies(const Clause&) const;

  friend bool operator<  (const Clause&, const Clause&);
  friend bool operator<= (const Clause&, const Clause&);
  friend bool operator>  (const Clause&, const Clause&);
  friend bool operator>= (const Clause&, const Clause&);
  friend bool operator== (const Clause&, const Clause&);
  friend bool operator!= (const Clause&, const Clause&);

  friend std::ostream& operator<< (std::ostream&, const Clause&);

  
private:
  std::set<Principal> clause;

};

//
// Components
//

// A component is either the value |False or a set of clauses.
// The clauses are treated as conjunctions.
// The default constructor sets the value to |False.
class Component {

public:
  // Default constructor to |False
  Component();
  // Copy constructor
  Component(const Component&);
  // Construct new component with value |False
  static Component dcFalse();
  // Construct new component with value |True
  static Component dcTrue();
  // Construct new component with value of the provided singleton formula
  static Component dcFormula(const Clause&);
  // Construct new component with value of the provided formula
  static Component dcFormula(const std::set<Clause>&);

  friend bool operator== (const Component&, const Component&);
  friend bool operator!= (const Component&, const Component&);

  // Does current component imply another
  bool implies(const Component&) const;

  // Perform conjunction with given component
  void dcAnd(const Component&);
  // Perform disjunction with given component
  void dcOr(const Component&);
  
  // Reduce component
  void dcReduce();
  
  // Is the component |False
  bool isFalse() const;
  // Is the component |True
  bool isTrue() const;

  friend std::ostream& operator<< (std::ostream&, const Component&);

private:
  bool DCFalse;
  std::set<Clause> DCFormula; // empty set encodes |True

  // Set componenet to |False
  void setToDCFalse();
  // Set componenet to |True
  void setToDCTrue();
  // Set componenet to the given one
  void setTo(const Component &);
  	
};

//
// Labels
//

class DCLabel {

public:
  // Default constructor sets label to the public label
  DCLabel();
  // Constructor that sets the secrecy and integrity component
  DCLabel(const Component&, const Component&);
  // Copy constructor
  DCLabel(const DCLabel&);

  friend bool operator== (const DCLabel&, const DCLabel&);
  friend bool operator!= (const DCLabel&, const DCLabel&);

  // Reduce components
  void dcReduce();

  // Bottom of lattice
  static DCLabel dcBottom();
  // Top of lattice
  static DCLabel dcTop();
  // Public label
  static DCLabel dcPub();

  // Can flow to relation
  bool canFlowTo(const DCLabel&) const;
  static bool canFlowTo(const DCLabel&, const DCLabel&);
  // Join, or least upper bound
  void lub(const DCLabel&);
  static DCLabel lub(const DCLabel&, const DCLabel&);
  // Meet, or greatest lower bound
  void glb(const DCLabel&);
  static DCLabel glb(const DCLabel&, const DCLabel&);

  
private:
  Component secrecy;
  Component integrity;
};


#endif
