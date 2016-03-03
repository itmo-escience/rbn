#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <vector>

#include "handler.hpp"

typedef vector<int> ints;
typedef vector<double> doubles;

using namespace std;

//command arg: 40 2 1 0.5 0.5 0.6 1 1000 addon 1
//
int main(int argc, char* argv[]){
	ints levels;
	doubles p_ch;
	int repetitions, iterations;
	string addon;
	bool pajek_structure = false;
	if(argc == 1){
		levels.push_back(20); //20 nodes
		levels.push_back(2); //2 subnetworks
		levels.push_back(1);
	
		p_ch.push_back(0.5);
		p_ch.push_back(0.5);
		p_ch.push_back(0.6);
		repetitions = 1;
		iterations = 1000;
		addon = "3_2_15";

		pajek_structure = true;
	}
	else if(argc == 2){
			//cout << "fdfd " <<
		handler h;
		h = handler(argv[1]);
		h.work();
		return 0;
	}
	else{
		string s;

		double val = 0;
		int i = 1;
		while (true){
			s = string(argv[i]);
			istringstream is(s);
			is >> val;
			++i;
			if (int(val) == val)
				levels.push_back((int)val);
			else break;
		}
		i-=1;

		p_ch.push_back(val);
		for(unsigned int j = 1; j < levels.size(); ++j){
			s = string(argv[i + j]);
			istringstream is(s);
			is >> val;

			p_ch.push_back(val);
		}
		i+=levels.size();
		{
			s = string(argv[i]);
			istringstream is(s);
			is >> repetitions;
			++i;
		}
		{
			s = string(argv[i]);
			istringstream is(s);
			is >> iterations;
			++i;
		}

		if(argc >= i){
			addon = string(argv[i]);
			++i;
			if(argc >= i){
				s = string(argv[i]);
				istringstream is(s);
				is >> pajek_structure;
			}
		}
	}

	handler h;
	h = handler(levels, p_ch, repetitions, iterations);
	
	h.save(handler::EACH_KIN);
	h.save(handler::EACH_PERIOD);
	h.save(handler::HIST_INFO);
	h.save(handler::HIST_KIN);
	if(pajek_structure)
		h.save(handler::PAJEK_STRUCTURE);
	h.save(handler::HIST_KOUT);

	h.add_n(addon);
	h.set_save_hist_from(0);
	h.set_inform_after(999);
	h.work();
	//*/
	return 0;
}

