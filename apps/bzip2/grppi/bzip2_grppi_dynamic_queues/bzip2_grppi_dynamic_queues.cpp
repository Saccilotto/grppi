#include <iostream>
#include <type_traits>
#include <iostream>
#include <metrics.hpp>
#include <bzip2.hpp>
#include "grppi/grppi.h"
#include "dyn/dynamic_execution.h"

using namespace std;
using namespace experimental;

// COMPRESS
void compress_grppi_nat_omp(grppi::dynamic_execution ex) {
	grppi::pipeline(ex,
	    [&]() mutable -> optional<spb::Item> {
	      	if (spb::bytesLeft > 0) {
				spb::Item item;
				spb::source_comp_op(item);
				return item;			
			}else {
				return {};
			}
	    },
	    grppi::farm(spb::nthreads,
	    	[&](spb::Item item) {
      			spb::compress_op(item);
		    	return item;
    		}),
	    [&](spb::Item item) {spb::sink_comp_op(item); }
  	);
}

void compress_grppi_tbb_ff(grppi::dynamic_execution ex, grppi::mpmc_queue<spb::Item> & q1, grppi::mpmc_queue<spb::Item> & q2) {
	auto farm =	grppi::farm(1,
		[&](spb::Item item) {
			item = q1.pop();
			spb::compress_op(item);
			q1.push(item);
			return item;
		}
	);

	ex.pipeline(
	[&]() mutable -> optional<spb::Item> {
		if (spb::bytesLeft > 0) {
			spb::Item item;
			spb::source_comp_op(item);
			q1.push(item);
			return item;
		} else {
			return {};
		}
	},
	ex.pipeline<grppi::mpmc_queue<spb::Item>, decltype(farm), grppi::mpmc_queue<spb::Item>>(q1, farm, q2),	
	[&](spb::Item item) {
		item = q2.pop();
		spb::sink_comp_op(item); 
		}
	);
}

// DECOMPRESS
void decompress_grppi_nat_omp(grppi::dynamic_execution ex) {
	grppi::pipeline(ex,
	    [&]() mutable -> optional<spb::Item> {
	      	if (spb::items_counter < spb::bz2NumBlocks) {
				spb::Item item;
				spb::source_decomp_op(item);
				return item;
			} else {
				return {};
			}
		},
		grppi::farm(spb::nthreads,
			[&](spb::Item item) {
				spb::decompress_op(item);
				return item;
			}),
	    [&](spb::Item item) {spb::sink_decomp_op(item); }
  	);
}

void decompress_grppi_tbb_ff(grppi::dynamic_execution ex, grppi::mpmc_queue<spb::Item> & q1, grppi::mpmc_queue<spb::Item> & q2) {
	auto farm = grppi::farm(
		1,
		[&](spb::Item item) {
			item = q1.pop();
			spb::decompress_op(item);
			q1.push(item);
			return item;
		}
	);

	ex.pipeline(
	[&]() mutable -> optional<spb::Item>  {
		if (spb::items_counter < spb::bz2NumBlocks) {
			spb::Item item;
			spb::source_decomp_op(item);
			q1.push(item);
			return item;
		} else {
			return {};
		}
	},
	ex.pipeline<grppi::mpmc_queue<spb::Item>, decltype(farm), grppi::mpmc_queue<spb::Item>>(q1, farm, q1),	
	[&](spb::Item item) {
		item = q2.pop();
		spb::sink_decomp_op(item); 
		}
	);
}

bool run(bool compress) {
	string opt = spb::getArg(0);
	bool ordered = true;
	int queue_size = 1;
	auto queue_mode = grppi::queue_mode::blocking;
	int tokens = spb::nthreads * 10;

	if(compress) {
		if(opt == "thr") {
			auto exec = grppi::parallel_execution_native();
			if (ordered) {
				exec.enable_ordering();
			} else {
				exec.disable_ordering();
			}
			exec.set_queue_attributes(queue_size, queue_mode);

			compress_grppi_nat_omp(exec);	
			return true;
		} 
		if (opt == "omp") {
			auto exec = grppi::parallel_execution_omp();
			if (ordered) {
				exec.enable_ordering();
			} else {
				exec.disable_ordering();
			}
			exec.set_queue_attributes(queue_size, queue_mode);

			compress_grppi_nat_omp(exec);
			return true;
		}
		if(opt == "tbb") {
			auto exec = grppi::parallel_execution_tbb(spb::nthreads, ordered);
			exec.set_queue_attributes(queue_size, queue_mode, tokens);
			grppi::mpmc_queue<spb::Item> queue1 = exec.make_queue<spb::Item>();
			grppi::mpmc_queue<spb::Item> queue2 = exec.make_queue<spb::Item>();
			
			compress_grppi_tbb_ff(exec, queue1, queue2);
			return true;		
		}
		if(opt == "ff") {
			auto exec = grppi::parallel_execution_ff(spb::nthreads, ordered);
			grppi::mpmc_queue<spb::Item> queue1 = grppi::mpmc_queue<spb::Item>(1, grppi::queue_mode::blocking);
			grppi::mpmc_queue<spb::Item> queue2 = grppi::mpmc_queue<spb::Item>(1, grppi::queue_mode::blocking);
			compress_grppi_tbb_ff(exec, queue1, queue2);
			return true;
		}
		return false;
		
	} else {
		if(opt == "thr") {
			auto exec = grppi::parallel_execution_native();
			if (ordered) {
				exec.enable_ordering();
			} else {
				exec.disable_ordering();
			}
			exec.set_queue_attributes(queue_size, queue_mode);
			decompress_grppi_nat_omp(exec);	
			return true;
		} 
		if (opt == "omp") {
			auto exec = grppi::parallel_execution_omp();
			if (ordered) {
				exec.enable_ordering();
			} else {
				exec.disable_ordering();
			}
			exec.set_queue_attributes(queue_size, queue_mode);

			decompress_grppi_nat_omp(exec);
			return true;
		}
		if(opt == "tbb") {
			auto exec = grppi::parallel_execution_tbb(spb::nthreads, ordered);
			exec.set_queue_attributes(queue_size, queue_mode, tokens);
			grppi::mpmc_queue<spb::Item> queue1 = exec.make_queue<spb::Item>();
			grppi::mpmc_queue<spb::Item> queue2 = exec.make_queue<spb::Item>();
			
			decompress_grppi_tbb_ff(exec, queue1, queue2);
			return true;		
		}
		if(opt == "ff") {
			auto exec = grppi::parallel_execution_ff(spb::nthreads, ordered);
			grppi::mpmc_queue<spb::Item> queue1 = grppi::mpmc_queue<spb::Item>(1, grppi::queue_mode::blocking);
			grppi::mpmc_queue<spb::Item> queue2 = grppi::mpmc_queue<spb::Item>(1, grppi::queue_mode::blocking);

			decompress_grppi_tbb_ff(exec, queue1, queue2);
			return true;
		}
		return false;
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


