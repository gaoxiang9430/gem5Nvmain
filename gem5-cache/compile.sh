#!/bin/bash
sudo scons EXTRAS=../nvmain-test build/ARM/gem5.opt
sudo build/ARM/gem5.opt --outdir=m5out --debug-flags=CacheRepl --debug-file=cacherepl.out configs/example/se.py --caches --l2cache --l3cache --cpu-type=detailed --nvmain-start=0GB --nvmain-end=1GB --dram-start=1GB --dram-end=1152MB --nvmain-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Pcm.config --dram-config=/home/xiang/workspace/gem5src/nvmain-test/Config/My_Dram.config -c tests/test-progs/hello/bin/arm/linux/hello
