#include "hparams.hpp"

hparams::hparams(){
	initialize(" ");
}

hparams::hparams(string filename){
	initialize(filename);
}

hparams::hparams(ints levels_, doubles p_changes_) : levels(levels_), p_changes(p_changes_) {
	initialize(" ");
}

hparams::hparams(int count_, ints sizes_, double alpha_){
	initialize(" ");

	network_count = count_;
	network_sizes = sizes_;
	set_alpha(alpha_);

	update_file_info_names();
}

hparams::~hparams(){
}

void hparams::initialize(std::string filename){

//-----------------BASIC MODEL PARAMETERS--------------
	rbn_version = 4; //RBN version the parameters should implement
	structure=""; //filename with initial network structure

//-----------------NETWORK STRUCTURE DATA--------------
	nodes = 0;
	network_count = 0; //number of small networks
	network_sizes.clear();
	prob = 0; //ratio of interconnections;
	tot_incon = 0; //total number of interconnections

	Kins.clear(); //initial in-degree connectivity
	alpha.clear(); //parameters of Changing Connections Rule (explained in network.hpp)
	alpha_prop.clear(); //see above

	ini_Kin = 2;
	ini_p = 0.5;
	set_alpha(0.0);

	levels.clear();
	p_changes.clear();

	link_update = 3;

//---------------SIMULATION PARAMETERS-------------
	iterations = 1000;
	repetitions = 1;
	max_attractor_length = 100000;

	scheme = 2;
	initial_iterations = 200; //number of initial iterations in 2 simulation scheme (see readme file)
	later_iterations = 100; //number of iterations in 2 simulation scheme (see readme file)

	//iteration management
	inform_after = 10; //after how many informations should the user be informed about the progress
	save_hist_from = 100; //start saving hist information after save_hist_from iterations

//--------------SAVING PARAMETERS------------------
	//flags:
	each_Kin = false; //save Kin after each iteration
	hist_Kin = false; //save histogram Kin data
	hist_Kout = false; //save histogram Kout data
	hist_info = false; //save histogram info data
	hist_last = false; //save histogram data after last iteration
	each_period = false; //save period of each iteration
	each_basin = false; //save period and basin length;
	pajek_structure = false;
	hist_cc = false;

	delimiter = ' ';
	add_on = ".txt"; //this will be added to the end of the file
	pajek_add_on = ".net";

	//file info
	string n = "hrbn_";
	stringstream h;
	if(rbn_version == 1)
		h << network_count << "_" << get_network_size(0);
	else if(rbn_version == 2)
		h << levels[0] << "_" << levels.size();
	else if(rbn_version == 3)
		h << network_count << "_" << "linkupdate" << link_update;
	else if(rbn_version == 4)
		h << network_count << "_" << get_network_size(0) << "linkupdate" << link_update;

	string h2 = h.str();

	f_each_Kin = (n + "data_" + h2); //file names for different data
	f_hist_Kin = (n + "hist_data_" + h2);
	f_hist_Kout = (n + "hist_outdata_" + h2);
	f_hist_info = (n + "hist_info_" + h2);
	f_hist_last = (n + "hist_end_" + h2);
	f_each_period = (n + "period_" + h2);
	f_each_basin = (n + "basin_" + h2);
	f_pajek_structure = (n + "pajek_" + h2);
	f_hist_cc = (n + "hist_cc_" + h2);

	if(filename != " ")
		read_from_file(filename);
}

void hparams::read_from_file(std::string filename){
	string line;
	ifstream file (filename.c_str());

	int i = 0;
	map_str_dbl2 params;

	if (file.is_open()){
		while ( getline (file,line) ){
			cout << line << endl;
			istringstream is(line);
			string sub, sub2;
			is >> sub;
			//names.push_back(sub);

			if(sub == "add_on"){
				sub.clear();
				is >> sub;

				add_on = sub + add_on;
				pajek_add_on = sub + pajek_add_on;
				++i;
				continue;
			}
			else if(sub == "structure"){
				sub.clear();
				is >> sub;

				structure = sub;
				++i;
				continue;
			}

			doubles dat;
			while(is){
				sub2.clear();
				is >> sub2;
				dat.push_back(atof(sub2.c_str()));
			}

			params[sub] = dat;
			++i;
		}

		file.close();
  	}
	else cout << "Unable to open file";
	/*
	strings pos_names;
	{
		pos_names.push_back("network_count");
		pos_names.push_back("network_sizes");
		pos_names.push_back("prob");
		pos_names.push_back("tot_incon");
		pos_names.push_back("Kins");
		pos_names.push_back("alpha");
		pos_names.push_back("alpha_prop");
		pos_names.push_back("iterations");
		pos_names.push_back("repetitions");
		pos_names.push_back("inform_after");
		pos_names.push_back("save_hist_from");
		pos_names.push_back("each_Kin");
		pos_names.push_back("hist_Kin");
		pos_names.push_back("hist_info");
		pos_names.push_back("hist_last");
		pos_names.push_back("each_period");
		pos_names.push_back("each_basin");
		pos_names.push_back("delimiter"); //nie dziala
		pos_names.push_back("add_on"); //slabo dziala, powinna byc klasa parameter
	}
	*/
	map_str_dbl2::iterator it, end = params.end();

	it = params.find("rbn_version");
	if(it == end)
		rbn_version = -1;
	else rbn_version = (it->second).at(0);

	if(rbn_version == 1){
		it = params.find("network_count");
		if(it == end)
			network_count = 0;
		else network_count = (it->second).at(0);

		nodes = 0;
		it = params.find("network_sizes");
		if(it == end)
			network_sizes.clear();
		else {
			doubles v = it->second;
			for(unsigned int i = 0; i < v.size(); ++i){
				network_sizes.push_back(int(v[i]));
				nodes += network_sizes[i];
			}
		}

		it = params.find("prob");
		if(it == end)
			prob = 0;
		else prob = (it->second).at(0);

		it = params.find("tot_incon");
		if(it == end)
			tot_incon = 0;
		else tot_incon = (int) (it->second).at(0);

		it = params.find("Kins");
		if(it != end){
			doubles v = it->second;
			for(unsigned int i = 0; i < v.size(); ++i){
				Kins.push_back(int(v[i]));

			}
		}

		it = params.find("alpha");
		if(it == end)
			set_alpha(0);
		else {
			alpha = it->second;

			it = params.find("alpha_prop");
			if(it == end)
				set_alpha(alpha);
			else alpha_prop = it->second;
		}
	}
	else if(rbn_version == 2){
		it = params.find("nodes");
		if(it != end)
			nodes = (it->second).at(0);

		unsigned int nlevels;
		it = params.find("nlevels");
		if(it == end)
			nlevels = 0;
		else {
			nlevels = (it->second).at(0);
		}

		it = params.find("levels");
		if(it == end)
			levels.clear();
		else {
			doubles v = it->second;
			//cout << v.size() << " " << v[2] << " " << nlevels;
			if(v.size() < nlevels)
				throw "param file ERROR\n Wrong number of levels.\n";
			levels.push_back(nodes);
			for(unsigned int i = 0; i < nlevels; ++i)
				levels.push_back(int(v[i]));
		}

		it = params.find("Kins");
		if(it != end)
			ini_Kin = int((it->second).at(0));

		it = params.find("p");
		if(it == end)
			p_changes.clear();
		else {
			doubles v = it->second;
			if(v.size() < nlevels)
				throw "param file ERROR\n Wrong number of p levels.\n";

			ini_p = v[0];
			p_changes.push_back(0.0);
			for(unsigned int i = 0; i < nlevels; ++i)
				p_changes.push_back(v[i] - p_changes[i] - ini_p);
		}
	}
	else if(rbn_version == 3){
		it = params.find("nodes");
		if(it != end)
			nodes = (it->second).at(0);

		it = params.find("Kins");
		if(it != end){
			doubles v = it->second;
			for(unsigned int i = 0; i < v.size(); ++i){
				Kins.push_back(int(v[i]));
			}
			ini_Kin = Kins[0];
		}

		it = params.find("alpha");
		if(it == end)
			set_alpha(0);
		else {
			alpha = it->second;

			it = params.find("alpha_prop");
			if(it == end)
				set_alpha(alpha);
			else alpha_prop = it->second;
		}

		it = params.find("link_update");
		if(it != end)
			link_update = (int) (it->second).at(0);
	}
	else if(rbn_version == 4){
		it = params.find("network_count");
		if(it == end)
			network_count = 0;
		else network_count = (it->second).at(0);

		nodes = 0;
		it = params.find("network_sizes");
		if(it == end)
			network_sizes.clear();
		else {
			doubles v = it->second;
			for(unsigned int i = 0; i < v.size(); ++i){
				network_sizes.push_back(int(v[i]));
				nodes += network_sizes[i];
			}
		}

		it = params.find("prob");
		if(it == end)
			prob = 0;
		else prob = (it->second).at(0);

		it = params.find("tot_incon");
		if(it == end)
			tot_incon = 0;
		else tot_incon = (int) (it->second).at(0);

		it = params.find("Kins");
		if(it != end){
			doubles v = it->second;
			for(unsigned int i = 0; i < v.size(); ++i){
				Kins.push_back(int(v[i]));

			}
		}

		it = params.find("alpha");
		if(it == end)
			set_alpha(0);
		else {
			alpha = it->second;

			it = params.find("alpha_prop");
			if(it == end)
				set_alpha(alpha);
			else alpha_prop = it->second;
		}

		it = params.find("link_update");
		if(it != end)
			link_update = (int) (it->second).at(0);

		it = params.find("scheme");
		if(it != end)
			scheme = (int) (it->second).at(0);

		it = params.find("initial_iterations");
		if(it != end)
			initial_iterations = (int) (it->second).at(0);

		it = params.find("later_iterations");
		if(it != end)
			later_iterations = (int) (it->second).at(0);
	}

	it = params.find("iterations");
	if(it != end)
		iterations = (int) (it->second).at(0);

	it = params.find("repetitions");
	if(it != end)
		repetitions = (int) (it->second).at(0);

	it = params.find("max_attractor_length");
	if (it != end)
		max_attractor_length = (unsigned int)(it->second).at(0);

	it = params.find("inform_after");
	if(it != end)
		inform_after = (int) (it->second).at(0);

	it = params.find("save_hist_from");
	if(it != end)
		save_hist_from = (int) (it->second).at(0);

	it = params.find("each_Kin");
	if(it != end)
		each_Kin = (bool) (it->second).at(0);

	it = params.find("hist_Kin");
	if(it != end)
		hist_Kin = (bool) (it->second).at(0);

	it = params.find("hist_Kout");
	if(it != end)
		hist_Kout = (bool) (it->second).at(0);

	it = params.find("hist_info");
	if(it != end)
		hist_info = (bool) (it->second).at(0);

	it = params.find("hist_last");
	if(it != end)
		hist_last = (bool) (it->second).at(0);

	it = params.find("each_period");
	if(it != end)
		each_period = (bool) (it->second).at(0);

	it = params.find("each_basin");
	if(it != end)
		each_basin = (bool) (it->second).at(0);

	it = params.find("pajek_structure");
	if(it != end)
		pajek_structure = (bool) (it->second).at(0);

	it = params.find("hist_cc");
	if(it != end)
		hist_cc = (bool) (it->second).at(0);

	update_file_info_names();
}


void hparams::update_file_info_names(){
	//file info
	string n = "hrbn_";
	stringstream h;
	if(rbn_version == 1)
		h << network_count << "_" << get_network_size(0);
	else if(rbn_version == 2)
		h << levels[0] << "_" << levels.size();
	else if(rbn_version == 3)
		h << nodes ;
	else if(rbn_version == 4)
		h << network_count << "_" << get_network_size(0) << "linkupdate" << link_update;

	string h2 = h.str();

	f_each_Kin = (n + "data_" + h2); //file names for different data
	f_hist_Kin = (n + "hist_data_" + h2);
	f_hist_Kout = (n + "hist_outdata_" + h2);
	f_hist_info = (n + "hist_info_" + h2);
	f_hist_last = (n + "hist_end_" + h2);
	f_each_period = (n + "period_" + h2);
	f_each_basin = (n + "basin_" + h2);
	f_pajek_structure = (n + "pajek_" + h2);
	f_hist_cc = (n + "hist_cc_" + h2);
}

int hparams::get_network_size(int n) const{
	if (n >= network_count)
		return 0;
	else return network_sizes[n];
}

void hparams::set_alpha(double alpha_){ //ustawia parametr alpha
	alpha.clear();
	alpha.push_back(alpha_);
	alpha_prop.clear();
	alpha_prop.push_back(1.0);
}

void hparams::set_alpha(doubles alpha_){ //ustawia parametry alpha
	alpha.clear();
	alpha_prop.clear();
	alpha = alpha_;
	for(unsigned int i = 0; i < alpha.size(); ++i)
		alpha_prop.push_back(1.0/alpha.size());
}
