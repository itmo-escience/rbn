#ifndef MY_RANDOM_HPP
#define MY_RANDOM_HPP

#include <boost/random/uniform_real.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <ctime>

using namespace boost;

//klasa do generowania liczb pseudolosowych
class my_random{
public:
	static my_random* get_instance();

	int next_int(const int right_inc); //losuje int z podanego przedzialu zamknietego z rozkladu jednorodnego od 0 wlacznie
	int next_int(const int left_inc, const int right_inc); //losuje int z podanego przedzialu zamknietego z rozkladu jednorodnego
	double next_double(); //losuje double z przedzialu [0,1] z rozkladu normalnego

private:
	static my_random* rand;

	mt19937 rnd_gen; //generator

	uniform_real<> double_gen; //do liczb double

	my_random();
};

#endif