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

#include <metrics.hpp>
#include <person_recognition.hpp>

#include "grppi/grppi.h"
#include "dyn/dynamic_execution.h"
#include "util.h"

using namespace std;
using namespace experimental;

void grppi_nat_omp (grppi::dynamic_execution & ex) {
	grppi::pipeline(ex,
	    []() mutable -> optional<spb::Item> {
            spb::Item item;
            if(!spb::source_op(item)) {
                return {};
            } else {
                return item;
            }
	    },
	    grppi::farm(spb::nthreads,
	    	[](spb::Item item) {
				spb::detect_op(item); //detect faces in the image:
				spb::recognize_op(item); //analyze each detected face:
		    	return item;
    		}),
	    [](spb::Item item) {sink_op(item); }
  	);
}

void grppi_tbb_ff (grppi::dynamic_execution & ex) {
	grppi::pipeline(ex,
	    []() mutable -> optional<spb::Item> {
            spb::Item item;
            if(!spb::source_op(item)) {
                return {};
            } else {
                return item;
            }
	    },
	    grppi::farm(1,
	    	[](spb::Item item) {
				spb::detect_op(item); //detect faces in the image:
				spb::recognize_op(item); //analyze each detected face:
		    	return item;
    		}),
	    [](spb::Item item) {sink_op(item); }
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
        auto aux = grppi::parallel_execution_tbb(spb::nthreads, true);
        //aux.enable_ordering();
        aux.set_queue_attributes(queue_size, queue_mode, tokens);
        return aux;
    }
    if(!set_ordering) {
        auto aux = grppi::parallel_execution_tbb(spb::nthreads, false);
        aux.set_queue_attributes(queue_size, queue_mode, tokens); 
        //aux.disable_ordering();
        return aux;
    }
  };
  if ("ff" == opt) {
    if(set_ordering) {
        auto aux = grppi::parallel_execution_ff(spb::nthreads, true);
        //aux.enable_ordering();
        return aux;
    }
    if(!set_ordering) {
        auto aux = grppi::parallel_execution_ff(spb::nthreads, false);
        //aux.disable_ordering();
        return aux;
    }
  };
  return {};
}

void run() {
    auto exec = execution_mode(grppi::queue_mode::blocking, true, 1, spb::nthreads);
    if (exec.has_execution()) {
        if(spb::getArg(0) == "thr" || spb::getArg(0) == "omp") {
            grppi_nat_omp(exec);
            // return true;
        }
        
        if(spb::getArg(0) == "ff"|| spb::getArg(0) == "tbb") {
            grppi_tbb_ff(exec);
            // return true;
        }
    }   
    // return false;
}

int main (int argc, char* argv[]){
	// Disabling internal OpenCV's support for multithreading 
	setNumThreads(0);
	spb::init_bench(argc, argv);
	spb::data_metrics metrics = spb::init_metrics();
	run();
	spb::stop_metrics(metrics);
	spb::end_bench();
	return 0;
}
