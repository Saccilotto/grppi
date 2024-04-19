#!/usr/bin/python
# grppi dynamic: native omp tbb ff
from statistics import mean
import numpy as np
import os

app = ["bzip2", "ferret", "lane", "person"]
backend = ["thr", "omp", "tbb", "ff"] #"sequential"

folder = "experiments_for_PDP-2022"
for a in app:
    path_ext = folder + "/" + a  +  "/" + a + "_EXT/"
    dir = os.path.join(os.path.dirname(__file__), path_ext) 
    for b in backend:
        path_version = a + "_grppi_dynamic_" + b 
        path_read = dir + path_version + ".dat"
        th_path_write = dir + path_version  
        with open(os.path.join(os.path.dirname(__file__), path_read), 'r') as version:
            size = 3
            thread = ""
            exec_time = [] 
            latency = []
            it_sec_put = []  
            
            path1 = os.path.join(os.path.dirname(__file__), dir + path_version + "_exec.dat")
            exec = open(path1, 'w')
            path2 = os.path.join(os.path.dirname(__file__), dir + path_version + "_farm_lat.dat")
            lat = open(path2, 'w')
            path3= os.path.join(os.path.dirname(__file__), dir + path_version + "_farm_it_sec.dat")
            it_sec = open(path3, 'w')
            count = 1
            next(version)
            for line in version: 
                thread = line.split(' ')[0] 
                exec_time.append(line.split(' ')[1])
                latency.append(line.split(' ')[2])
                it_sec_put.append(line.split(' ')[3]) 
                
                count += 1
                if count % 3 == 0:
                    list_exec = []
                    #list_exec = list(map(float, exec_time))
                    list_lat = []
                    #list_lat = list(map(float, latency))
                    list_it = []
                    #list_it = list(map(float, it_sec_put))

                    for item in exec_time:
                        list_exec.append(float(item))

                    for item in latency:
                        list_lat.append(float(item))

                    for item in it_sec_put:
                        list_it.append(float(item))

                    #average statistics for 
                    avg_exec = mean(list_exec)
                    dev_exec = np.std(list_exec)

                    avg_lat = mean(list_lat)
                    dev_lat = np.std(list_lat)
                    
                    avg_it = mean(list_it)
                    dev_it = np.std(list_it)  

                    aux1 = thread + " " + str(avg_exec) + " " + str(dev_exec) + "\n"
                    aux2 = thread + " " + str(avg_lat) + " " + str(dev_lat) + "\n"
                    aux3 = thread + " " + str(avg_it) + " " + str(dev_it) + "\n" 

                    exec.write(aux1)
                    lat.write(aux2)
                    it_sec.write(aux3)

                    exec_time.clear()                  
                    latency.clear()
                    it_sec_put.clear()

                    list_exec.clear()                  
                    list_lat.clear()
                    list_it.clear()
            exec.close()
            lat.close()
            it_sec.close()
        version.close()
                
                
