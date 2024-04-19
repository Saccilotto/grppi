#!/bin/bash

cpupower frequency-info

#setup
OUT_DIR="experiments_for_PDP-2022"

now=`date`

mkdir $OUT_DIR

#README info
printf "# About\n" > $OUT_DIR/README.md
printf " -Experiments on GrPPI using Fastflow, TBB, OMP and native backends for PDP-2022\n\n" >> $OUT_DIR/README.md

#Who
printf "## Who?\n" >> $OUT_DIR/README.md
printf " -André Sacilotto Santos\n\n" >> $OUT_DIR/README.md

#When
printf "## When?\n" >> $OUT_DIR/README.md
printf " - $now\n\n" >> $OUT_DIR/README.md

#What
printf "## What?\n" >> $OUT_DIR/README.md
printf " - Application: GRPPI Using TBB, OMP, C++THREADS, Fastflow backend" >> $OUT_DIR/README.md #application name
printf " - Version: farm implementations\n\n" >> $OUT_DIR/README.md
printf " - Modifications: frequency variations\n\n" >> $OUT_DIR/README.md
printf " - Problem size: large\n\n" >> $OUT_DIR/README.md
printf " - Evaluated metrics: Latency and throughput\n\n" >> $OUT_DIR/README.md
printf " - Source code(grppi branch): https://github.com/GMAP/SPBench\n\n" >> $OUT_DIR/README.md

#Where
printf "## Where?\n" >> $OUT_DIR/README.md
printf " - roadrunner\n\n" >> $OUT_DIR/README.md

#How
printf "## How?\n" >> $OUT_DIR/README.md
printf " - Compiler: GCC 9.3.0 C++: CXX_14 \n\n" >> $OUT_DIR/README.md
printf " - Repetitions: 5\n\n" >> $OUT_DIR/README.md
printf " - Number of threads: 0-40\n\n" >> $OUT_DIR/README.md
printf " - GCC optimizations:\n\n" >> $OUT_DIR/README.md

#experiments

PYTHON='python2.7'

INPUT='large'

SEQ="sequential"

APP="bzip2 ferret lane person"

PPI="grppi"

BACKCEND="thr omp tbb ff"

METRICS="-latency -throughput"

# 0-40 threads for each version:
for app in $APP
do
	for seq in $SEQ
	do			
		file_name_th=$app"_"$SEQ"_th-0"
		mkdir -p $OUT_DIR/$app/{$app"_RAW",$app"_EXT"}/$file_name_th
		version=$app"_"$SEQ  																							# build the name of a version for extracted data and to be run
		dir_ext=$OUT_DIR/$app/$app"_EXT"/$file_name_th/
		dir_raw=$OUT_DIR/$app/$app"_RAW"/$file_name_th/																    # build the name of a version for its raw data
		# prints labels into a new file for extracting after raw generation
		echo "num_threads exec_time latency item_per_sec" >> "${version}.dat"
		for i in {1..5} # repetitions
		do	
			generated_file=$file_name_th"-it-"$i																	# build the name of a version on current repetition
			now=`date`										
			echo "Loop $i | $generated_file | $now"
			
			$PYTHON spbench exec -version $version -input $INPUT $METRICS >> "${generated_file}_raw" 						# run command and print output
			# extracting execution time, end-to-end latency and items per second from raw data
			EXEC_TIME="$(grep "Execution time" ${generated_file}_raw | tr -s ' ' | cut -d' ' -f6)"
			LATENCY="$(grep "Total-latency" ${generated_file}_raw | tr -s ' ' | cut -d' ' -f5)"
			ITEM_PER_SEC="$(grep "Itens-per-second" ${generated_file}_raw | tr -s ' ' | cut -d' ' -f4)"	
			
			# printing the extraction into a new file with index as iterations
			echo "0 $EXEC_TIME $LATENCY $ITEM_PER_SEC" >> "${version}.dat"
			# saving the raw data into its folder
			mv "${generated_file}_raw" $dir_raw
			echo "-------------------------------------------------------------------------"
		done	
		# moving the extracted data into its folder
		cp "${version}.dat" $dir_ext
		echo "-------------------------------------------------------------------------"
	done
	mv "${version}.dat" "${version}_all.dat"
	mv "${version}_all.dat" $OUT_DIR/$app/$app"_EXT"/$app"_"$SEQ"_th-0"/ 
	echo "-------------------------------------------------------------------------"

	for ppi in $PPI
	do
		for backend in $BACKCEND
		do	
			name_version_run=$app"_"$ppi"_dynamic_generic"
			name_version=$app"_"$ppi"_dynamic"_$backend															# build the name of the version to be run
			echo "num_threads exec_time latency item_per_sec" >> "${name_version}.dat"
			# 1-40 threads for each version

			#t=2
			#max=17
			#while [ $t -lt $max ] 		# number of replicas
			for i in {1..40}
			do
				cd $OUT_DIR/$app/$app"_RAW"/
				mkdir $name_version"_th-"$t
				file_name_th=$name_version"_th-"$t
				dir_ext=$OUT_DIR/$app/$app"_EXT"/$name_version														# reference to the previously created dir of current version for raw data													
				dir_raw=$OUT_DIR/$app/$app"_RAW"/$name_version"_th-"$t												# reference to the previously created dir of current version for raw data 
				cd $PWD/../../..
				for i in {1..5} 	# repetitions
				do
					generated_file=$file_name_th"-it-"$i											    # build the name of the version on current repetition
					now=`date`
					echo "Loop $i | $generated_file | $now"
					
					$PYTHON spbench exec -version $name_version_run -input $INPUT -nthreads $t $METRICS -user_args $backend >> "${generated_file}_raw"	 # run command and print in new file
					# extracting execution time, end-to-end latency and items per second from raw data
					EXEC_TIME="$(grep "Execution time" ${generated_file}_raw | tr -s ' ' | cut -d' ' -f6)"
					LATENCY="$(grep "Total-latency" ${generated_file}_raw | tr -s ' ' | cut -d' ' -f5)"
					ITEM_PER_SEC="$(grep "Itens-per-second" ${generated_file}_raw | tr -s ' ' | cut -d' ' -f4)"
					
					# printing the number of threads(as index) + extraction into previously created file for extraction
					echo "$t $EXEC_TIME $LATENCY $ITEM_PER_SEC" >> "${name_version}.dat"
					# moving the raw data into its folder
					mv "${generated_file}_raw" $dir_raw
					echo "-------------------------------------------------------------------------"
				done
				# moving the extracted data into its folder
				cp "${name_version}.dat" $dir_ext
				echo "-------------------------------------------------------------------------"
				#cd $OUT_DIR/$app/$app"_RAW"/
				let "t+=2"
			done
			#cd $PWD/../../..
			mv "${name_version}.dat" "${name_version}_all.dat"
			mv "${name_version}_all.dat" $OUT_DIR/$app/$app"_EXT"/$name_version
			echo "-------------------------------------------------------------------------"
		done
	done
done

exit;
