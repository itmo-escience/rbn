#include "my_random.hpp"
#include <iostream>

my_random::my_random(){
	rnd_gen.seed(static_cast<unsigned int>(std::time(0))); //seed generatora
}

int my_random::next_int(const int right_inc){ 
	return next_int(0, right_inc);
}

int my_random::next_int(const int left_inc, const int right_inc){ //losuje int z podanego przedzialu zamknietego z rozkladu jednorodnego
	if(right_inc < left_inc)
		std::cout << "error:my_random:next_int";
	random::uniform_int_distribution<> dist(left_inc, right_inc);
	return dist(rnd_gen);
}

double my_random::next_double(){ //losuje double z przedzialu [0,1] z rozkladu jednorodnego
	return double_gen(rnd_gen);
}

