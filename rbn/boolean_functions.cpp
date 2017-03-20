#include "boolean_functions.hpp"
#include <iostream>

boolean_functions::boolean_functions(my_random& rand_, double p_, int con)
	: p(p_)
	, connections(con)
	, rand(rand_)
{
//	rnd_gen.seed(static_cast<unsigned int>(std::time(0))); //seed generatora
	//if(connections == 0)
	//	return;
	functions.reserve(connections);
	generate();
}

boolean_functions::~boolean_functions(){}

void boolean_functions::update_connections(){
	generate();
}

void boolean_functions::update_connections(int con){
	connections = con;
	generate();
}

const int boolean_functions::get_value(int fun){
	return functions[fun];
}

//mozliwe ze to jest bledne
const int boolean_functions::get_value(int* inputs){
	unsigned long int i, sum = 0, mul = 1;
	for(i = connections; i > 0; --i){
		sum += mul*inputs[i];
		mul *= 2;
	}

	return functions[sum];
}

const int boolean_functions::get_value(const nodes& inputs){
	int i, sum = 0, mul = 1, c;
	nodes_it it = inputs.begin(), end = inputs.end();
	if(connections != inputs.size())
		cout << "aha";
	for(i = connections; i > 0; --i, ++it){
		if(it == end)
			std::cout << "akuku";
		c = (*it)->get_state_old();
		sum += mul*c;
		mul *= 2;
	}
//	if(connections == 0)
//		cout << sum << " " << functions.size() << " ";
	int r = functions[sum];
	return r;
}

void boolean_functions::generate(){
	
	functions.clear();
	functions.reserve(connections);
	int i = connections;
	int poss = 1;
	while(i-- > 0)
		poss *= 2;
	for(i = 0; i < poss; ++i){
			boolean_functions::functions.push_back(0);
		else boolean_functions::functions.push_back(1);
	}

}
