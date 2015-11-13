#ifndef HPARAMS_HPP
#define HPARAMS_HPP

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <cstdlib>

using namespace std;

typedef vector<int> ints;
typedef vector<double> doubles;
typedef vector<doubles> doubles2;
typedef vector<string> strings;
typedef map<string, doubles> map_str_dbl2;

class hparams {
public:
	hparams();
	hparams(string filename);
	hparams(ints levels_, doubles p_changes_);
	hparams(int count_, ints sizes_, double alpha_ = 0);
	~hparams();

	void update_file_info_names();

	//void initialize();

	int get_network_size(int n) const; //returns size of n network

//alpha functions
	void set_alpha(double alpha_); //ustawia parametr alpha
	void set_alpha(doubles alpha_); //ustawia parametry alpha
	void set_alpha(doubles alpha_, doubles prop); //ustawia parametry alpha i alpha_prop

//important fields

//-----------------BASIC MODEL PARAMETERS--------------
	int rbn_version; //RBN version the parameters should implement
	string structure; //filename with initial network structure

//-----------------NETWORK STRUCTURE DATA--------------
	int nodes; //nodes number
	int network_count; //number of small networks
	ints network_sizes;
	double prob; //ratio of interconnections;
	int tot_incon; //total number of interconnections

	ints Kins; //initial in-degree connectivity (may be different in subnetworks)
	doubles alpha; //parameters of Changing Connections Rule (explained in network.hpp)
	doubles alpha_prop; //see above
/**	Structure of expected integers in variable levels_.
 * It should contain information about sizes of hierarchy levels, starting from the bottom level up to the top.
 * The 1st integer (that is levels_[0]) should contain number of nodes in network
 * The 2nd integer should contain number of network groups (small group of nodes) in the 1st level
 * ...
 * The last integer should be 1.
*/
	ints levels;

	int ini_Kin; //initial in-degree connectivity
	double ini_p; //initial value of p node parameter
/**	vector structure responds to structure of field levels.
 * 	double are changes of parameter p from initial value.
*/
	doubles p_changes;
	int link_update; //choosing the update rule (0-normal, 1-PA, 2-PD, 3-PAD)

//---------------SIMULATION PARAMETERS-------------
	int iterations;
	int repetitions;

/**
 * the scheme of simulation:
 * 0 - update only intralinks,
 * 1 - update only interlinks,
 * 2 - update by turns intra- and interlinks
 */
	int scheme;
	int initial_iterations; //number of initial iterations in 2 simulation scheme (see readme file)
	int later_iterations; //number of iterations in 2 simulation scheme (see readme file)
	unsigned int max_attractor_length;

	//iteration management
	int inform_after; //after how many informations should the user be informed about the progress
	int save_hist_from; //start saving hist information after save_hist_from iterations


//--------------SAVING PARAMETERS------------------
	//flags:
	bool each_Kin; //save Kin after each iteration
	bool hist_Kin; //save histogram Kin data
	bool hist_Kout; //save histogram Kout data
	bool hist_info; //save histogram info data
	bool hist_last; //save histogram data after last iteration
	bool each_period; //save period of each iteration
	bool each_basin; //save period and basin length;
	bool pajek_structure;
	bool hist_cc; // save clustering coefficient histogram data

	//file info
	char delimiter;
	string f_each_Kin; //file names for different data
	string f_hist_Kin;
	string f_hist_Kout;
	string f_hist_info;
	string f_hist_last;
	string f_each_period;
	string f_each_basin;
	string f_pajek_structure;
	string f_hist_cc;
	string add_on; //this will be added to the end of the file
	string pajek_add_on;

private:
	void initialize(std::string);
	void read_from_file(std::string filename);
};


#endif
