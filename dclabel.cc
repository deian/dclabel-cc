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

public:
	Clause(std::set<Principal>&);
	Clause(Principal[], size_t);
	
	friend std::ostream& operator<< (std::ostream&, const Clause&);
	friend bool operator<  (const Clause&, const Clause&);
	friend bool operator<= (const Clause&, const Clause&);
	friend bool operator== (const Clause&, const Clause&);
	friend bool operator!= (const Clause&, const Clause&);
	
private:
	std::set<Principal> clause;

};

Clause::Clause(std::set<Principal>& c) {
	clause = c;
}

Clause::Clause(Principal ps[], size_t len) {
	std::set<Principal> c(ps,ps+len);
	clause = c;
}

bool operator<  (const Clause& c1, const Clause& c2) {
	return c1 <= c2 && c1 != c2;
}
	
bool operator<= (const Clause& c1, const Clause& c2) {
	if(c1.clause.size() ==  c2.clause.size()) {
		return c1.clause <= c2.clause;
	} else {
		return c1.clause.size() <  c2.clause.size();
	}
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
// Componenets
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
	
	// Is the component |False
	bool isFalse() const;
	// Is the component |True
	bool isTrue() const;

	friend std::ostream& operator<< (std::ostream&, const Component&);

private:
	bool DCFalse;
	std::set<Clause> DCFormula; // empty set encodes |True
		
};

Component::Component() {
	DCFalse = true;
}

inline Component Component::dcFalse() {
	return Component();
}

inline Component Component::dcTrue() {
	Component c;
	c.DCFalse = false;
	c.DCFormula.clear();
	return c;
}

inline Component Component::dcFormula(const Clause& clause) {
	std::set<Clause> cs;
/*
	cs.insert(clause);
	f.insert(clause);
	return dcFormula(f);
*/
	Component c;
	return c;
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


int main(void) {
	Principal ps[] = {"a","b", "c", "f"};
	Principal ps2[] = {"a","b", "c", "g", "f"};

	Clause c(ps, 4);
	Clause cz(ps2, 4);
	std::cout << (c < cz) << std::endl;
	
	
	Component c1;
	Component c2 = Component::dcTrue();
	Component c3 = Component::dcFormula(c);
	
	
	std::cout << c1 << std::endl;
	std::cout << c2 << std::endl;
	std::cout << c3 << std::endl;

	return 0;
}
