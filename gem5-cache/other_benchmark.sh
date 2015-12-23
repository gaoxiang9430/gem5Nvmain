#!/bin/bash
mkdir ../test_result/20151220
mkdir ../test_result/20151220/smwl_brp
build/ARM/gem5.opt --outdir=../test_result/20151220/smwl_brp --debug-flags=HAP --debug-file=smwl_brp_2M.out --stats-file=wb_cache_smwl_brp.txt configs/example/spec2006.py --caches --l2cache --l2_size=256kB --l2_assoc=8 --l3cache --l3_size=2MB --cpu-type=detailed --nvmain-start=0GB --nvmain-end=1GB --dram-start=1GB --dram-end=1152MB --nvmain-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Pcm.config --dram-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Dram.config -b "soplex;mcf;wrf;lbm;" -n 4 > ../test_result/20151220/smwl_brp/smwl_brp_energy.txt

mkdir ../test_result/20151220/lsmc_brp
build/ARM/gem5.opt --outdir=../test_result/20151220/lsmc_brp --debug-flags=HAP --debug-file=lsmc_brp_2M.out --stats-file=wb_cache_lsmc_brp.txt configs/example/spec2006.py --caches --l2cache --l2_size=256kB --l2_assoc=8 --l3cache --l3_size=2MB --cpu-type=detailed --nvmain-start=0GB --nvmain-end=1GB --dram-start=1GB --dram-end=1152MB --nvmain-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Pcm.config --dram-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Dram.config -b "leslie3d;soplex;milc;cactusADM;" -n 4 > ../test_result/20151220/lsmc_brp/lsmc_brp_energy.txt

mkdir ../test_result/20151220/wsbm_brp
build/ARM/gem5.opt --outdir=../test_result/20151220/wsbm_brp --stats-file=wb_cache_wsbm_brp.txt configs/example/spec2006.py --caches --l2cache --l2_size=256kB --l2_assoc=8 --l3cache --l3_size=2MB --cpu-type=detailed --nvmain-start=0GB --nvmain-end=1GB --dram-start=1GB --dram-end=1152MB --nvmain-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Pcm.config --dram-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Dram.config -b "wrf;sjeng;bwaves;milc;" -n 4 > ../test_result/20151220/wsbm_brp/wsbm_brp_energy.txt

mkdir ../test_result/20151220/cgbl_brp
build/ARM/gem5.opt --outdir=../test_result/20151220/cgbl_brp --stats-file=wb_cache_cgbl_brp.txt configs/example/spec2006.py --caches --l2cache --l2_size=256kB --l2_assoc=8 --l3cache --l3_size=2MB --cpu-type=detailed --nvmain-start=0GB --nvmain-end=1GB --dram-start=1GB --dram-end=1152MB --nvmain-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Pcm.config --dram-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Dram.config -b "cactusADM;gromacs;bwaves;leslie3d;" -n 4 > ../test_result/20151220/cgbl_brp/cgbl_brp_energy.txt

mkdir ../test_result/20151220/gsol_brp
build/ARM/gem5.opt --outdir=../test_result/20151220/gsol_brp --debug-flags=HAP --debug-file=gsol_brp_2M.out --stats-file=wb_cache_gsol_brp.txt configs/example/spec2006.py --caches --l2cache --l2_size=256kB --l2_assoc=8 --l3cache --l3_size=2MB --cpu-type=detailed --nvmain-start=0GB --nvmain-end=1GB --dram-start=1GB --dram-end=1152MB --nvmain-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Pcm.config --dram-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Dram.config -b "gromacs;soplex;omnetpp;leslie3d;" -n 4 > ../test_result/20151220/gsol_brp/gsol_brp_energy.txt
