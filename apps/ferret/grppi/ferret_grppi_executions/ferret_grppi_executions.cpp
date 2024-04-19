#include <metrics.hpp>
#include <ferret.hpp>

#include "grppi/grppi.h"
#include "util.h"

using namespace std;
using namespace experimental;

/* struct Definition {

} def; */	

void nat() {
    auto exec = grppi::parallel_execution_native();
    exec.disable_ordering();
    exec.set_queue_attributes(1 , grppi::queue_mode::blocking);

	grppi::pipeline(exec,
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

void omp() {
    auto exec = grppi::parallel_execution_omp();
    exec.disable_ordering();
    exec.set_queue_attributes(1 , grppi::queue_mode::blocking);

	grppi::pipeline(exec,
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

void tbb_exec() {
    auto exec = grppi::parallel_execution_tbb();
    exec.disable_ordering();
    exec.set_queue_attributes(1 , grppi::queue_mode::blocking, spb::nthreads * 10);
    grppi::mpmc_queue<spb::Item> tbb1(1, grppi::queue_mode::blocking);
    grppi::mpmc_queue<spb::Item> tbb2(1, grppi::queue_mode::blocking);

	exec.pipeline(tbb1,
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
	    [](spb::Item item) {sink_op(item); },
        tbb2
  	);
}

void ff_exec() {
    auto exec = grppi::parallel_execution_ff();
    exec.disable_ordering();
    //grppi::detail_ff::pipeline_impl ffPipe(spb::nthreads, false, exec);
    //ffPipe.set_queue_attributes(1, grppi::queue_mode::blocking);
    grppi::mpmc_queue<spb::Item> ff1(1, grppi::queue_mode::blocking);
    grppi::mpmc_queue<spb::Item> ff2(1, grppi::queue_mode::blocking);

	exec.pipeline(ff1,
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
	    [](spb::Item item) {sink_op(item); },
        ff2
  	);
}

void ferret() {
    string ex = spb::getArg(0);
    if(ex == "thr") {
        nat();
    }
    if(ex == "omp") { 
        omp();
    }
    if(ex == "tbb") {
        tbb_exec();
    }
    if(ex == "ff") {
        ff_exec();
    }
}

int main(int argc, char *argv[]) {
    spb::init_bench(argc, argv);
    spb::data_metrics metrics = spb::init_metrics();
    ferret();
    spb::stop_metrics(metrics);
    spb::end_bench();
    return 0;
}

