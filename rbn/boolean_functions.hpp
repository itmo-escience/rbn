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
	/**
	 * Constructor.
	 * 
	 * @param rand_ Reference to a pRNG that will be used to generate random functions.
	 * @param p     Probability of generating ones.
	 * @param con   Number of connections.
	 */
	boolean_functions(my_random& rand_, double p_ = 0.5, int con = 0);
	~boolean_functions();

	/**
	 * Updates all connections (generates all of them once again).
	 */
	void update_connections();

	/**
	 * Updates all connections with new connections (con) number.
	 *
	 * @param con Number of connections.
	 */
	void update_connections(int con);

	/**
	 * Returns the result of given RBF.
	 *
	 * @param fun Number of a function.
	 * @returns The result of given RBF.
	 */
	const int get_value(int fun); //zwraca wynik danej funkcji boolowskiej

	/**
	 * Returns the result of given RBF.
	 *
	 * @param inputs Table of ones and zeros.
	 * @returns The result of given RBF.
     */
	const int get_value(int* inputs); //zwraca wynik dla danej serii wejsc

	/**
	 * Returns the result of given RBF.
	 *
	 * @param inputs Set of pointers to nodes.
	 * @returns The result of given RBF.
	 */
	const int get_value(const nodes& inputs); //zwraca wynik dla danej serii wejsc

private:
//probability of generating ones
	const double p;
//number of connections of certain node
	int connections; //liczba polaczen danego wezla
//vector of functions
	ints functions; //tablica zawierajaca funkcje
// random generator
	my_random& rand;

//function that creates functions
	void generate();//funkcja losujaca
};

#endif
