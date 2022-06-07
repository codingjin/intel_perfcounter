import numpy as np
import argparse
from os import walk

def find_between( s, first, last ):
    try:
        start = s.index( first ) + len( first )
        end = s.index( last, start )
        return s[start:end]
    except ValueError:
        return ""

def main():

        #analysis file 
        parser = argparse.ArgumentParser(description='OPT.\n')
        parser.add_argument('path', type=str,  help='trace file path\n')
        args = parser.parse_args() 

        mypath = args.path
        filenames = next(walk(mypath), (None, None, []))[2]
        profile_file = mypath
        src_file = mypath

        for f in filenames:
                if "profiling" in f:
                        profile_file += f
                        print(profile_file)
                else:
                        src_file += f
                        print(src_file)
        
        #parse ground-truth
        gt_file = open(src_file, "r")
        gt_lines = gt_file.readlines()
        gt_tmp =  []
        all_set_tmp = []
        for line in gt_lines:
                if "hot set" in line:
                        result = find_between(line, "hot set: ", "-")
                        #print(result)
                        gt_tmp.append(result)
                        result += "-"
                        result = find_between(line, result, "\n")
                        #print(result)
                        gt_tmp.append(result)
                        result = find_between(line, "region: ", "-")
                        #print(result)
                        all_set_tmp.append(result)
                        result += "-"
                        result = find_between(line, result, "hot set")
                        #print(result)
                        all_set_tmp.append(result)
                
                elif "Elapsed time" in line:
                        print(line)

        
        profile = open(profile_file, "r")
        prof_tmp = profile.readlines()
        prof =  [ int(x, 16)  for x in prof_tmp]
        '''
        gt_file = open("gups/gt1.txt", "r")
        gt_tmp = gt_file.readlines()
        '''
        gt =  [ int(x, 16)  for x in gt_tmp]
        all_set =  [ int(x, 16)  for x in all_set_tmp]

        res = []
        neg = []
        count=0

        for i in prof:
                flag=0
                for ii in range(int(len(gt)/2)):
                        inx=ii*2
                        start = gt[inx]
                        end = gt[inx+1]
                        if i>= min(all_set[inx],start) and i<=all_set[inx+1]:
                                flag=1
                                if i>=start and i<=end:
                                        #print(i)
                                        res.append(i & int("2097152",16))
                                        #res.append(i & 4096)
                                else:
                                        neg.append(i & int("2097152",16))
                        else:
                                continue
                        
                if flag == 0:
                        #neg.append(i & 2097152)
                        count=count+1
                        

        result = np.unique(res)
        print(len(result))
        neg_result = np.unique(neg)
        print(len(neg_result))
        print(count)
        



if __name__ == "__main__":
    main()


'''
hread 0 region: 0x7f6828e73000 - 0x7f697e3c8550        hot set: 0x7f6828e72fff - 0x7f686d2b743f
starting thread [2]
Thread 1 region: 0x7f697e3c8550 - 0x7f6ad391daa0        hot set: 0x7f697e3c854f - 0x7f69c280c98f
starting thread [3]
Thread 2 region: 0x7f6ad391daa0 - 0x7f6c28e72ff0        hot set: 0x7f6ad391da9f - 0x7f6b17d61edf
starting thread [4]
Thread 3 region: 0x7f6c28e72ff0 - 0x7f6d7e3c8540        hot set: 0x7f6c28e72fef - 0x7f6c6d2b742f
starting thread [5]
Thread 4 region: 0x7f6d7e3c8540 - 0x7f6ed391da90        hot set: 0x7f6d7e3c853f - 0x7f6dc280c97f
starting thread [6]
Thread 5 region: 0x7f6ed391da90 - 0x7f7028e72fe0        hot set: 0x7f6ed391da8f - 0x7f6f17d61ecf
starting thread [7]
Thread 6 region: 0x7f7028e72fe0 - 0x7f717e3c8530        hot set: 0x7f7028e72fdf - 0x7f706d2b741f
starting thread [8]
Thread 7 region: 0x7f717e3c8530 - 0x7f72d391da80        hot set: 0x7f717e3c852f - 0x7f71c280c96f
starting thread [9]
Thread 8 region: 0x7f72d391da80 - 0x7f7428e72fd0        hot set: 0x7f72d391da7f - 0x7f7317d61ebf
starting thread [10]
Thread 9 region: 0x7f7428e72fd0 - 0x7f757e3c8520        hot set: 0x7f7428e72fcf - 0x7f746d2b740f
starting thread [11]
Thread 10 region: 0x7f757e3c8520 - 0x7f76d391da70       hot set: 0x7f757e3c851f - 0x7f75c280c95f
starting thread [12]
Thread 11 region: 0x7f76d391da70 - 0x7f7828e72fc0       hot set: 0x7f76d391da6f - 0x7f7717d61eaf
starting thread [13]
Thread 12 region: 0x7f7828e72fc0 - 0x7f797e3c8510       hot set: 0x7f7828e72fbf - 0x7f786d2b73ff
starting thread [14]
Thread 13 region: 0x7f797e3c8510 - 0x7f7ad391da60       hot set: 0x7f797e3c850f - 0x7f79c280c94f
starting thread [15]
Thread 14 region: 0x7f7ad391da60 - 0x7f7c28e72fb0       hot set: 0x7f7ad391da5f - 0x7f7b17d61e9f
starting thread [16]
Thread 15 region: 0x7f7c28e72fb0 - 0x7f7d7e3c8500       hot set: 0x7f7c28e72faf - 0x7f7c6d2b73ef
starting thread [17]
Thread 16 region: 0x7f7d7e3c8500 - 0x7f7ed391da50       hot set: 0x7f7d7e3c84ff - 0x7f7dc280c93f
starting thread [18]
Thread 17 region: 0x7f7ed391da50 - 0x7f8028e72fa0       hot set: 0x7f7ed391da4f - 0x7f7f17d61e8f
starting thread [19]
Thread 18 region: 0x7f8028e72fa0 - 0x7f817e3c84f0       hot set: 0x7f8028e72f9f - 0x7f806d2b73df
starting thread [20]
Thread 19 region: 0x7f817e3c84f0 - 0x7f82d391da40       hot set: 0x7f817e3c84ef - 0x7f81c280c92f
starting thread [21]
Thread 20 region: 0x7f82d391da40 - 0x7f8428e72f90       hot set: 0x7f82d391da3f - 0x7f8317d61e7f
starting thread [22]
Thread 21 region: 0x7f8428e72f90 - 0x7f857e3c84e0       hot set: 0x7f8428e72f8f - 0x7f846d2b73cf
starting thread [23]
Thread 22 region: 0x7f857e3c84e0 - 0x7f86d391da30       hot set: 0x7f857e3c84df - 0x7f85c280c91f
Thread 23 region: 0x7f86d391da30 - 0x7f8828e72f80       hot set: 0x7f86d391da2f - 0x7f8717d61e6f
'''
