# intel_perfcounter

Notice: pebs_example_with_script/ is just an example, normal/ is an application to use the perf_counter written by myself.

Need root privilege to catch the events, like ./sudo normal


./process profiling_results_1654599469.txt  | sort | uniq -c | sort -nr | awk '{print $2 "\t" $1}' | less 
