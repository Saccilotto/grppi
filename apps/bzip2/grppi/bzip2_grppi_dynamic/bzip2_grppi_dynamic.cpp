/*
	This code is related to the grppi project from GMAP labs.
	@Author: André Sacilotto Santos

	This version is from the app_grppi_dynamic's (see more about it at repo wiki) 
	set of implementations of which strategy prioritize the standard grppi 
	functionalities by using the "util.h" inside grppi's samples folder and dynamic execution 
	for targeting the specific backends chosen by the user at execution time.
	The context of these implementations is bounded with SPBench benchmark that 
	supply an extent of functionalities that serve as an outline for communicating 
	with the benchmark's programs (e.g bzip2, lane_detection) and as a way of measuring 
	the implementation's performance via execution made from inside this platform regarding
	various metrics provided by SPBench.
*/

#include <typeinfo>
#include <iostream>
#include <metrics.hpp>
#include <bzip2.hpp>
#include "grppi/grppi.h"
#include "util.h"

using namespace std;
using namespace experimental;

struct Definition {
	string ex;
} def;		

grppi::dynamic_execution getDef() {
	return execution_mode(def.ex); // returns execution type from util.h sample file inside grppi based on the type of execution passed as parameter.
}

void compress_grppi() {
	def.ex = spb::getArg(0);
	auto exC = grppi::dynamic_execution(getDef());

	grppi::pipeline(exC,
	    []() mutable -> optional<spb::Item> {
	      	if (spb::bytesLeft > 0) {
				spb::Item item;
				spb::source_comp_op(item);
				return item;
			} else {
				return {};
			}
	    },
	    grppi::farm(spb::nthreads,
	    	[](spb::Item item) {
      			spb::compress_op(item);
		    	return item;
    		}),
	    [](spb::Item item) {spb::sink_comp_op(item); }
  	);
}

void compress() {	
	spb::data_metrics metrics = spb::init_metrics();
  	compress_grppi();
	spb::stop_metrics(metrics);
}

void decompress_grppi() {
	def.ex = spb::getArg(0);
	auto exD = grppi::dynamic_execution(getDef());

	grppi::pipeline(exD,
	    []() mutable -> optional<spb::Item> {
	      	if (spb::items_counter < spb::bz2NumBlocks) {
				spb::Item item;
				spb::source_decomp_op(item);
				return item;
			} else {
				return {};
			}
	    },
	    grppi::farm(spb::nthreads,
	    	[](spb::Item item) {
				spb::decompress_op(item);
		    	return item;
    		}),
	    [](spb::Item item) {spb::sink_decomp_op(item); }
  	);
}

void decompress() {
	spb::data_metrics metrics = spb::init_metrics();
  	decompress_grppi();
	spb::stop_metrics(metrics);
}

int main (int argc, char* argv[]) {
	spb::bzip2_main(argc, argv);
	
	return 0;
}