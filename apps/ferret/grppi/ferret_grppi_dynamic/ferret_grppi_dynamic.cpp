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

#include <metrics.hpp>
#include <ferret.hpp>

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

void ferret() {
    def.ex = spb::getArg(0);
	auto ex = grppi::dynamic_execution(getDef());

    grppi::pipeline(ex,
	    []() mutable -> optional<spb::Item> {
            spb::Item item;
            if(!spb::source_op(item)) {
                return {};
            }else {
                return item;
            }
	    },
	    grppi::farm(spb::nthreads,
	    	[](spb::Item item) {
			    spb::segmentation_op(item);
                spb::extract_op(item);
                spb::vectorization_op(item);
                spb::rank_op(item);
		    	return item;
    		}),
	    [](spb::Item item) {sink_op(item); }
  	);
}
 
int main(int argc, char *argv[]) {
    spb::init_bench(argc, argv);
    spb::data_metrics metrics = spb::init_metrics();
    ferret();
    spb::stop_metrics(metrics);
    spb::end_bench();
    return 0;
}
