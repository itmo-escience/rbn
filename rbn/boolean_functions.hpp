#ifndef BOOLEAN_FUNCTIONS_HPP
#define BOOLEAN_FUNCTIONS_HPP

//#include <boost/random/uniform_real.hpp>
//#include <boost/random/mersenne_twister.hpp>
#include <ctime>
#include <vector>
#include "node.hpp"
#include "my_random.hpp"

class node;
typedef boost::shared_ptr<node> node_ptr;
typedef std::set<node_ptr> nodes;
typedef std::vector<int> ints;

using namespace std;

/*
Class for generating random boolean functions (RBFs). There are 2 parameters:
-p: probability of generating ones,
-connections: number of connections of a node having the specific instance of the object of this class.
The RBFs are stored in vector<int>. They are stored in natural order, that is if a node has 3 inputs, then 8 RBFs need to be generated. Then the first value in vector is for input (0 0 0), the 2nd - (0 0 1), the 3rd - (0 1 0) and so on... 
The usage have several possibilities:
-constructing: calling the constructor, setting 
parameters, creating functions in function generate(),
-getting value: looking at the vector. The vector of old states from nodes that have a link to this node is converted into a number in base 10 (i.e. (1 0 1)_2 -> (5)_{10}) and then the relevant value from vector is taken,
-changing or updating with new connections number: creating functions in function generate().
Function generate() takes 2^connections random numbers from {0,1} and saves it to the vector.

This class works if the Kin is not bigger than 28.
*/

class boolean_functions{
public:
/* constructor
p - probability of generating ones,
con - number of connections
*/
	boolean_functions(double p, int con);
	~boolean_functions();

//	void set_rand_gen(boost::mt19937 gen_){rnd_gen = gen_;}
//updates all connections (generates all of them ones again)
	void update_connections();
//updates all connections with new connections (con) number
	void update_connections(int con);
/*returns the result of given RBF
fun - number of a function
*/
	const int get_value(int fun); //zwraca wynik danej funkcji boolowskiej
/*returns the result of given RBF
inputs - table of ones and zeros
*/
	const int get_value(int* inputs); //zwraca wynik dla danej serii wejsc
/*returns the result of given RBF
inputs - set of pointers to nodes
*/
	const int get_value(const nodes& inputs); //zwraca wynik dla danej serii wejsc

private:
//probability of generating ones
	const double p;
//number of connections of certain node
	int connections; //liczba polaczen danego wezla
//vector of functions
	ints functions; //tablica zawierajaca funkcje
//pointer to random generator
	my_random rand;

//function that creates functions
	void generate();//funkcja losujaca
};

#endif
