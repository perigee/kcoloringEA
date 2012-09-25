import os
import glob

if __name__ == "__main__":
    print("in progress")
    path = "angerData"
    outputfile = "analysis/clique.csv"
    for infile in glob.glob(os.path.join(path, '*.col') ):
        cmd = "./eagraph " + infile + " 2 2" + " >> " + outputfile
        os.system(cmd)
        print('current file is: ',infile)

