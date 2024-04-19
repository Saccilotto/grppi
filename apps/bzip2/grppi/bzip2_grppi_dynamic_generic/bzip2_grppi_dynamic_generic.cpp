/*
	This code is related to the grppi project from GMAP labs.
	@Author: André Sacilotto Santos

	This version is from the app_grppi_dynamic_generic's (see more about it at repo wiki) 
	set of implementations. These implementations try to make all the possible adjustments
	regarding 0.4.0 grppi release in order to achieve better performance results than the 
	previous set of implementations (app_grppi_dynamic). 
	In this code there a execution_mode method that makes a similar distinction between all 
	available execution policies as in the "util.h" from grppi samples, but adds specific
	configurations needed in each one of the execution policy chosen by the user when executing
	the program via SPBench.

	OBS.
	The separation on the method that receives the chosen execution between (native,omp) and (tbb, ff)
	may no longer be needed since "run" and "execution_mode" methods are doing all the specific 
	configuration. Both methods may be only one in future versions.
*/ 

#include <typeinfo>
#include <iostream>
#include <metrics.hpp>
#include <bzip2.hpp>
#include "grppi/grppi.h"
#include "dyn/dynamic_execution.h"

using namespace std;
using namespace experimental;

// COMPRESS
void compress_grppi_nat_omp(grppi::dynamic_execution & ex) {
	grppi::pipeline(ex,
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

void compress_grppi_tbb_ff(grppi::dynamic_execution & ex) {
	grppi::pipeline(ex,
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

// DECOMPRESS
void decompress_grppi_nat_omp(grppi::dynamic_execution & ex) {
	grppi::pipeline(ex,
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

void decompress_grppi_tbb_ff(grppi::dynamic_execution & ex) {
	grppi::pipeline(ex,
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

// Execution setup
grppi::dynamic_execution execution_mode(grppi::queue_mode queue_mode, bool set_ordering, int queue_size, int tokens) {
  string opt = spb::getArg(0);
  if ("seq" == opt) return grppi::sequential_execution{};

  if ("thr" == opt) {
    if(set_ordering) {
        auto aux = grppi::parallel_execution_native();
		aux.enable_ordering();
        aux.set_queue_attributes(queue_size, queue_mode); 
        return aux;
    }
    if(!set_ordering) {
        auto aux = grppi::parallel_execution_native();
		aux.disable_ordering();
        aux.set_queue_attributes(queue_size, queue_mode); 
        return aux;
    }
  };
  if ("omp" == opt)  {
    if(set_ordering) {
        auto aux = grppi::parallel_execution_omp();
		aux.enable_ordering();
        aux.set_queue_attributes(queue_size, queue_mode);
        return aux;
    }
    if(!set_ordering) {
        auto aux = grppi::parallel_execution_omp();
		aux.disable_ordering();
        aux.set_queue_attributes(queue_size, queue_mode); 
        return aux;
    }
  };
  if ("tbb" == opt) {
    if(set_ordering) {
        auto aux = grppi::parallel_execution_tbb(1, true);
        aux.set_queue_attributes(queue_size, queue_mode, tokens);
        return aux;
    }
    if(!set_ordering) {
        auto aux = grppi::parallel_execution_tbb(1, false);
        aux.set_queue_attributes(queue_size, queue_mode, tokens); 
        return aux;
    }
  };
  if ("ff" == opt) {
    if(set_ordering) {
        auto aux = grppi::parallel_execution_ff(spb::nthreads, true);
        return aux;
    }
    if(!set_ordering) {
        auto aux = grppi::parallel_execution_ff(spb::nthreads, false);
        return aux;
    }
  };
  return {};
}

void run(bool compress) {
	auto exec = execution_mode(grppi::queue_mode::blocking, true, 1, spb::nthreads);
	if(compress) {
		if(exec.has_execution()) {
			if(spb::getArg(0) == "thr" || spb::getArg(0) == "omp") {
				compress_grppi_nat_omp(exec);
			} 
			if(spb::getArg(0) == "tbb" || spb::getArg(0) == "ff") {
				compress_grppi_tbb_ff(exec);
			}
		}
	} else {
		if(exec.has_execution()) {
			if(spb::getArg(0) == "thr" || spb::getArg(0) == "omp") {
				decompress_grppi_nat_omp(exec);
			} 
			if(spb::getArg(0) == "tbb" || spb::getArg(0) == "ff") {
				decompress_grppi_tbb_ff(exec);
			}
		}
	}
    
}

// comp and decomp callers
void compress() {	
	spb::data_metrics metrics = spb::init_metrics();
	run(true);
	spb::stop_metrics(metrics);
}

void decompress() {
	spb::data_metrics metrics = spb::init_metrics();
	run(false);
	spb::stop_metrics(metrics);
}

int main (int argc, char* argv[]) {
	spb::bzip2_main(argc, argv);
	
	return 0;
}


