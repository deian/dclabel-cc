#include <iostream>
#include <set>
#include <algorithm>

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
	// Create clause form a set of principals
	Clause(std::set<Principal>&);
	// Create cluase form a list of principals; the list size is
  // given by the second argument.
	Clause(Principal[], size_t);

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

void Clause::setTo(const std::set<Principal>& c) {
	this->clause = c;
}

Clause::Clause(std::set<Principal>& c) {
	clause = c;
}

Clause::Clause(Principal ps[], size_t len) {
	std::set<Principal> c(ps,ps+len);
	clause = c;
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


// A component is either the value |False or a set of clauses.
// The clauses are treated as conjunctions.
// The default constructor sets the value to |False.
class Component {

public:

	// Default constructor to |False
	Component();
	// Construct new component with value |False
	static Component dcFalse();
	// Construct new component with value |True
	static Component dcTrue();
	// Construct new component with value of the provided singleton formula
	static Component dcFormula(const Clause&);
	// Construct new component with value of the provided formula
	static Component dcFormula(const std::set<Clause>&);

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

Component::Component() {
	DCFalse = true;
}


inline Component Component::dcFalse() {
	Component c;
	c.setToDCFalse();
	return c;
}

inline Component Component::dcTrue() {
	Component c;
	c.setToDCTrue();
	return c;
}

inline Component Component::dcFormula(const Clause& clause) {
	std::set<Clause> cs;
	cs.insert(clause);
	return dcFormula(cs);
}

inline Component Component::dcFormula(const std::set<Clause>& f) {
	Component c;
	c.DCFalse = false;
	c.DCFormula = f;
	return c;
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
//
//


int main(void) {
	Principal ps1[] = {"a","b", "c", "f"};
	Principal ps2[] = {"g", "h"};
	Principal ps3[] = {"a","b", "c", "f", "g", "h"};
	Principal ps4[] = {"x","y", "z"};

	Clause clause1(ps1, 4);
	Clause clause2(ps2, 2);
	Clause clause3(ps3, 6);
	Clause clause4(ps4, 3);

	std::set<Clause> c12;
	c12.insert(clause1);
	c12.insert(clause2);
	
	std::cout << clause1.implies(clause1) << std::endl;
	std::cout << clause1.implies(clause2) << std::endl;
	std::cout << clause1.implies(clause3) << std::endl;

	std::cout << clause2.implies(clause1) << std::endl;
	std::cout << clause2.implies(clause2) << std::endl;
	std::cout << clause2.implies(clause3) << std::endl;

	std::cout << clause3.implies(clause1) << std::endl;
	std::cout << clause3.implies(clause2) << std::endl;
	std::cout << clause3.implies(clause3) << std::endl;
	
	Component c1;
	Component c2 = Component::dcTrue();
	Component c3 = Component::dcFormula(clause1);
	Component c4 = Component::dcFormula(c12);
	Component c5 = Component::dcFormula(clause4);
	
	
	std::cout << "c1: " << c1 << std::endl;
	std::cout << "c2: " << c2 << std::endl;
	std::cout << "c3: " << c3 << std::endl;
	std::cout << "c4: " << c4 << std::endl;
	std::cout << "c5: " << c5 << std::endl;

	std::cout << c1.implies(c1) << std::endl;
	std::cout << c1.implies(c2) << std::endl;
	std::cout << c1.implies(c3) << std::endl;
	std::cout << c1.implies(c4) << std::endl << std::endl;

	std::cout << c2.implies(c1) << std::endl;
	std::cout << c2.implies(c2) << std::endl;
	std::cout << c2.implies(c3) << std::endl;
	std::cout << c2.implies(c4) << std::endl << std::endl;

	std::cout << c3.implies(c1) << std::endl;
	std::cout << c3.implies(c2) << std::endl;
	std::cout << c3.implies(c3) << std::endl;
	std::cout << c3.implies(c4) << std::endl << std::endl;

	std::cout << c4.implies(c1) << std::endl;
	std::cout << c4.implies(c2) << std::endl;
	std::cout << c4.implies(c3) << std::endl;
	std::cout << c4.implies(c4) << std::endl << std::endl;


	c4.dcOr(c5); std::cout << c4 << std::endl;
	c3.dcOr(c5); std::cout << c3 << std::endl;
	c1.dcOr(c4); std::cout << c1 << std::endl;
	c2.dcOr(c4); std::cout << c2 << std::endl;

	
	{
		Principal ps1[] = {"a","b"};
		Principal ps2[] = {"a","b","c"};
		Principal ps3[] = {"a","b","c","d"};
		Principal ps4[] = {"a","q","c","d"};
		Clause c1 = Clause(ps1,2);
		Clause c2 = Clause(ps2,3);
		Clause c3 = Clause(ps3,4);
		Clause c4 = Clause(ps4,4);

		std::set<Clause> c;
	  c.insert(c1);
	  c.insert(c2);
	  c.insert(c3);
	  c.insert(c4);
		
		Component cc = Component::dcFormula(c);
		std::cout << "cc = " << cc << std::endl;
		cc.dcReduce();
		std::cout << "cc = " << cc << std::endl;
	}


	return 0;
}
