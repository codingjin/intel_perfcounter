# intel_perfcounter

Need root privilege to catch the events, like ./sudo normal


./process profiling_results_1654599469.txt  | sort | uniq -c | sort -nr | awk '{print $2 "\t" $1}' | less 
