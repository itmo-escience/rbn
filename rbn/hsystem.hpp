#ifndef HSYSTEM_HPP
#define HSYSTEM_HPP

/**
 * This class represents the hierarchical network. It consists of small networks that connect with each other.
*/
#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "network.hpp"
#include "hparams.hpp"
#include "node.hpp"
#include "my_random.hpp"

using namespace std;

typedef vector<int> ints;
typedef vector<ints> ints2;
typedef vector<double> doubles;
typedef boost::shared_ptr<network> network_ptr;
typedef vector<network_ptr> networks;

class hsystem {
public:
	/**
	 * Default constructor. Does not generate any nodes or networks.
	 *
	 * @param seed (Optional) A number that is used to seed the hsystem's pRNG.
	 */
	explicit hsystem(unsigned seed = my_random::default_seed());

    /**
	 * Constructor that accepts simulation parameters. Generates nodes, but does not split them into networks.
	 *
	 * @param p Simulation parameters, see hparams.hpp for description.
	 * @param seed (Optional) A number that is used to seed the hsystem's pRNG. 
	 */
	hsystem(hparams p, unsigned seed = my_random::default_seed());

	void generate_nodes(int N); //generates nodes
	void generate_nodes();
	void generate_hnetwork(int nets_number, int Kin_); //creates net_number network objects dividing nodes equally (or almost) into them. Then initial structure is created with identical in-connectivity - Kin_.
	void generate_hnetwork(int nets_nr, ints Kins_);
	void generate_hnetwork(int l); //same as above, takes info from field params, variable l is level of hierarchy (params.levels[l]).
	void generate_interconnections(int); //generuje polaczenia miedzysieciowe,

	int find_attractor(void); //znajduje atraktor calej sieci
	int find_attractor(execution_policy::gpu_tag);
	template<class ExecutionPolicy> int find_attractor(ExecutionPolicy);
	
	void iterate(void); //iteracja sieci
	template<class ExecutionPolicy> void iterate(ExecutionPolicy);

	ints2 get_network_state(void); //zwraca stan calej sieci
	ints get_network_state(unsigned int n); //zwraca stan czasteczki n (wszystkich wezlow)
	double get_Kin(unsigned int n) const; //zraca srednie Kin
	double get_Kout(unsigned int n) const; //zraca srednie Kout
	double get_Kin() const; //zraca srednie Kin
	double get_CC() const; //returns average CC

	int get_count() const {return nets.size();}
	nodes& get_all(unsigned int n);
	nodes& get_all() {return all;}
	const nodes& get_all() const {return all;}
	long get_period() const {return T;}
	long get_period(unsigned int n) const;
	long get_basin() const {return it;}

	/*
	Prints network in pajek format
	*/
	string print_net();

	friend std::ostream& operator<<(std::ostream&, const hsystem&);

private:
	void iterate_v2(void);
	void iterate_v2(execution_policy::openmp_parallel_tag);
	void iterate_v2(execution_policy::nested_openmp_parallel_tag);
	template<class ExecutionPolicy>	void iterate_v2(ExecutionPolicy);

	nodes all;
	networks nets;
	hparams params;

	long T; //period of system
	long it; //period and basin length
	ints2 state; //state of networks

	my_random rand;

public:
	enum INTER_CON_MODELS{ //generating interconnections models
		CONST_PROB,
		CONST_NUM_UNIFORM //stala liczba polaczen, rozlozona rownomiernie (w miare mozliwosci)
	};
};

std::ostream& operator<<(std::ostream&, const hsystem&);

#include "hsystem.inl"

#endif
