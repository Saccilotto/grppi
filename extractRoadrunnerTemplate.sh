#!/bin/bash

DIR="experiments_for_PDP-2022"
EXT="EXT"
RAW="RAW"
SEQ="sequential"
APP="bzip2 lane ferret person"
PPI="grppi"
BACKCEND="thr omp tbb ff"
PATTERN="farm"

cd $DIR
for app in $APP
do
    #mkdir extraction_std_dev/
    cd $app    
    file_seq=$app"_"$SEQ"_th-0"
    for back in $BACKCEND
    do
        versionfile=$app"_"$PPI"_dynamic_"$back
        echo "threads exec_time latency item_per_sec" >> $app"_"$EXT/$versionfile".dat"
        th=0
        for i in {1..5}
        do
            seq_file=$app"_"$RAW/$file_seq/$file_seq"-it-"$i"_raw"
            EXEC_TIME="$(grep "Execution time" ${seq_file} | tr -s ' ' | cut -d' ' -f5)"
            LATENCY="$(grep "Total-latency" ${seq_file} | tr -s ' ' | cut -d' ' -f4)"
            ITEM_PER_SEC="$(grep "Itens-per-second" ${seq_file} | tr -s ' ' | cut -d' ' -f3)"
            echo "$th $EXEC_TIME $LATENCY $ITEM_PER_SEC" >> $app"_"$EXT/${versionfile}".dat"
        done   

        #t=2
        #max=17
        #while [ $t -lt $max ] 		# number of replicas
        for i in {1..40}
        do
            for i in {1..5}
            do
                generatedfile=$app"_"$RAW/$versionfile"_th-"$t/$versionfile"_th-"$t"-it-"$i"_raw"
                EXEC_TIME="$(grep "Execution time" ${generatedfile} | tr -s ' ' | cut -d' ' -f5)"
                LATENCY="$(grep "Total-latency" ${generatedfile} | tr -s ' ' | cut -d' ' -f4)"
                ITEM_PER_SEC="$(grep "Itens-per-second" ${generatedfile} | tr -s ' ' | cut -d' ' -f3)"
                echo "$t $EXEC_TIME $LATENCY $ITEM_PER_SEC" >> $app"_"$EXT/${versionfile}".dat"
            done  
            let "t+=2" 
        done
    done
    cd ..
done
        
exit;

