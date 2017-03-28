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

	explicit my_random(unsigned seed);
	static unsigned my_random::default_seed();

	int next_int(const int right_inc); //losuje int z podanego przedzialu zamknietego z rozkladu jednorodnego od 0 wlacznie
	int next_int(const int left_inc, const int right_inc); //losuje int z podanego przedzialu zamknietego z rozkladu jednorodnego
	unsigned next_unsigned();
	double next_double(); //losuje double z przedzialu [0,1] z rozkladu normalnego

private:

	mt19937 rnd_gen; //generator
	uniform_real<> double_gen; //do liczb double

};

#endif