#Mybench.py

import m5
from m5.objects import *
m5.util.addToPath('../common')

binary_dir = '/home/xiang/workspace/gem5src/benchspec/CPU2006/'
data_dir = '/home/xiang/workspace/gem5src/benchspec/CPU2006/'

#====================
#400.perlbench
perlbench = LiveProcess()
perlbench.executable =  binary_dir+'400.perlbench/exe/perlbench_base.i386-m32-gcc42-nn'
data=data_dir+'400.perlbench/data/test/input/makerand.pl'
perlbench.cmd = [perlbench.executable] + [data]
perlbench.output = 'attrs.out'

#401.bzip2
bzip2 = LiveProcess()
bzip2.executable =  binary_dir+'401.bzip2/exe/bzip2_base.i386-m32-gcc42-nn'
data=data_dir+'401.bzip2/data/all/input/input.program'
bzip2.cmd = [bzip2.executable] + [data, '1']
bzip2.output = 'input.program.out'
#401.bzip2
bzip21 = LiveProcess()
bzip21.executable =  binary_dir+'401.bzip2/exe/bzip2_base.i386-m32-gcc42-nn'
data=data_dir+'401.bzip2/data/all/input/input.program'
bzip21.cmd = [bzip2.executable] + [data, '1']
bzip21.output = 'input.program.out'
#401.bzip2
bzip22 = LiveProcess()
bzip22.executable =  binary_dir+'401.bzip2/exe/bzip2_base.i386-m32-gcc42-nn'
data=data_dir+'401.bzip2/data/all/input/input.program'
bzip22.cmd = [bzip2.executable] + [data, '1']
bzip22.output = 'input.program.out'
#401.bzip2
bzip23 = LiveProcess()
bzip23.executable =  binary_dir+'401.bzip2/exe/bzip2_base.i386-m32-gcc42-nn'
data=data_dir+'401.bzip2/data/all/input/input.program'
bzip23.cmd = [bzip2.executable] + [data, '1']
bzip23.output = 'input.program.out'
#401.bzip2
bzip24 = LiveProcess()
bzip24.executable =  binary_dir+'401.bzip2/exe/bzip2_base.i386-m32-gcc42-nn'
data=data_dir+'401.bzip2/data/all/input/input.program'
bzip24.cmd = [bzip2.executable] + [data, '1']
bzip24.output = 'input.program.out'
#401.bzip2
bzip2 = LiveProcess()
bzip2.executable =  binary_dir+'401.bzip2/exe/bzip2_base.i386-m32-gcc42-nn'
data=data_dir+'401.bzip2/data/all/input/input.program'
bzip2.cmd = [bzip2.executable] + [data, '1']
bzip2.output = 'input.program.out'
#401.bzip2
bzip25 = LiveProcess()
bzip25.executable =  binary_dir+'401.bzip2/exe/bzip2_base.i386-m32-gcc42-nn'
data=data_dir+'401.bzip2/data/all/input/input.program'
bzip25.cmd = [bzip2.executable] + [data, '1']
bzip25.output = 'input.program.out'
#401.bzip2
bzip26 = LiveProcess()
bzip26.executable =  binary_dir+'401.bzip2/exe/bzip2_base.i386-m32-gcc42-nn'
data=data_dir+'401.bzip2/data/all/input/input.program'
bzip26.cmd = [bzip2.executable] + [data, '1']
bzip26.output = 'input.program.out'
#401.bzip2
bzip27 = LiveProcess()
bzip27.executable =  binary_dir+'401.bzip2/exe/bzip2_base.i386-m32-gcc42-nn'
data=data_dir+'401.bzip2/data/all/input/input.program'
bzip27.cmd = [bzip2.executable] + [data, '1']
bzip27.output = 'input.program.out'
#====================
#403.gcc
gcc = LiveProcess()
gcc.executable =  binary_dir+'403.gcc/exe/gcc_base.i386-m32-gcc42-nn'
data=data_dir+'403.gcc/data/test/input/cccp.i'
output='/home/xiang/workspace/gem5src/benchspec/CPU2006/403.gcc/data/test/output/cccp.s'
gcc.cmd = [gcc.executable] + [data]+['-o',output]
gcc.output = 'ccc.out'

#410.bwaves
bwaves = LiveProcess()
bwaves.executable =  binary_dir+'410.bwaves/exe/bwaves_base.i386-m32-gcc42-nn'
data=data_dir+'410.bwaves/data/test/input/bwaves.in'
bwaves.cmd = [bwaves.executable] + [data]

#====================
#416.gamess
gamess=LiveProcess()
gamess.executable =  binary_dir+'416.gamess/exe/gamess_base.i386-m32-gcc42-nn'
gamess.cmd = [gamess.executable]
gamess.input='/home/xiang/workspace/gem5src/benchspec/CPU2006/416.gamess/data/test/input/exam29.config'
gamess.output='exam29.output'

#429.mcf
mcf = LiveProcess()
mcf.executable =  binary_dir+'429.mcf/exe/mcf_base.i386-m32-gcc42-nn'
data=data_dir+'429.mcf/data/test/input/inp.in'
mcf.cmd = [mcf.executable] + [data]
mcf.output = 'inp.out'

mcf1 = LiveProcess()
mcf1.executable =  binary_dir+'429.mcf/exe/mcf_base.i386-m32-gcc42-nn'
data=data_dir+'429.mcf/data/test/input/inp.in'
mcf1.cmd = [mcf.executable] + [data]
mcf1.output = 'inp.out'

mcf2 = LiveProcess()
mcf2.executable =  binary_dir+'429.mcf/exe/mcf_base.i386-m32-gcc42-nn'
data=data_dir+'429.mcf/data/test/input/inp.in'
mcf2.cmd = [mcf.executable] + [data]
mcf2.output = 'inp.out'

mcf3 = LiveProcess()
mcf3.executable =  binary_dir+'429.mcf/exe/mcf_base.i386-m32-gcc42-nn'
data=data_dir+'429.mcf/data/test/input/inp.in'
mcf3.cmd = [mcf.executable] + [data]
mcf3.output = 'inp.out'

mcf4 = LiveProcess()
mcf4.executable =  binary_dir+'429.mcf/exe/mcf_base.i386-m32-gcc42-nn'
data=data_dir+'429.mcf/data/test/input/inp.in'
mcf4.cmd = [mcf.executable] + [data]
mcf4.output = 'inp.out'

mcf5 = LiveProcess()
mcf5.executable =  binary_dir+'429.mcf/exe/mcf_base.i386-m32-gcc42-nn'
data=data_dir+'429.mcf/data/test/input/inp.in'
mcf5.cmd = [mcf.executable] + [data]
mcf5.output = 'inp.out'

mcf6 = LiveProcess()
mcf6.executable =  binary_dir+'429.mcf/exe/mcf_base.i386-m32-gcc42-nn'
data=data_dir+'429.mcf/data/test/input/inp.in'
mcf6.cmd = [mcf.executable] + [data]
mcf6.output = 'inp.out'

mcf7 = LiveProcess()
mcf7.executable =  binary_dir+'429.mcf/exe/mcf_base.i386-m32-gcc42-nn'
data=data_dir+'429.mcf/data/test/input/inp.in'
mcf7.cmd = [mcf.executable] + [data]
mcf7.output = 'inp.out'



#====================
#433.milc
milc=LiveProcess()
milc.executable = binary_dir+'433.milc/exe/milc_base.i386-m32-gcc42-nn'
stdin=data_dir+'433.milc/data/test/input/su3imp.in'
milc.cmd = [milc.executable]
milc.input=stdin
milc.output='su3imp.out'

milc1=LiveProcess()
milc1.executable = binary_dir+'433.milc/exe/milc_base.i386-m32-gcc42-nn'
stdin=data_dir+'433.milc/data/test/input/su3imp.in'
milc1.cmd = [milc.executable]
milc1.input=stdin
milc1.output='su3imp.out'

milc2=LiveProcess()
milc2.executable = binary_dir+'433.milc/exe/milc_base.i386-m32-gcc42-nn'
stdin=data_dir+'433.milc/data/test/input/su3imp.in'
milc2.cmd = [milc.executable]
milc2.input=stdin
milc2.output='su3imp.out'

milc3=LiveProcess()
milc3.executable = binary_dir+'433.milc/exe/milc_base.i386-m32-gcc42-nn'
stdin=data_dir+'433.milc/data/test/input/su3imp.in'
milc3.cmd = [milc.executable]
milc3.input=stdin
milc3.output='su3imp.out'

#====================
#434.zeusmp
zeusmp=LiveProcess()
zeusmp.executable =  binary_dir+'434.zeusmp/exe/zeusmp_base.i386-m32-gcc42-nn'
zeusmp.cmd = [zeusmp.executable]
zeusmp.output = 'zeusmp.stdout'

#====================
#435.gromacs
gromacs = LiveProcess()
gromacs.executable =  binary_dir+'435.gromacs/exe/gromacs_base.i386-m32-gcc42-nn'
data=data_dir+'435.gromacs/data/test/input/gromacs.tpr'
gromacs.cmd = [gromacs.executable] + ['-silent','-deffnm',data,'-nice','0']

gromacs1 = LiveProcess()
gromacs1.executable =  binary_dir+'435.gromacs/exe/gromacs_base.i386-m32-gcc42-nn'
data=data_dir+'435.gromacs/data/test/input/gromacs.tpr'
gromacs1.cmd = [gromacs.executable] + ['-silent','-deffnm',data,'-nice','0']

gromacs2 = LiveProcess()
gromacs2.executable =  binary_dir+'435.gromacs/exe/gromacs_base.i386-m32-gcc42-nn'
data=data_dir+'435.gromacs/data/test/input/gromacs.tpr'
gromacs2.cmd = [gromacs.executable] + ['-silent','-deffnm',data,'-nice','0']

gromacs3 = LiveProcess()
gromacs3.executable =  binary_dir+'435.gromacs/exe/gromacs_base.i386-m32-gcc42-nn'
data=data_dir+'435.gromacs/data/test/input/gromacs.tpr'
gromacs3.cmd = [gromacs.executable] + ['-silent','-deffnm',data,'-nice','0']

#====================
#436.cactusADM
cactusADM = LiveProcess()
cactusADM.executable =  binary_dir+'436.cactusADM/exe/cactusADM_base.i386-m32-gcc42-nn'
data=data_dir+'436.cactusADM/data/test/input/benchADM.par'
cactusADM.cmd = [cactusADM.executable] + [data]
cactusADM.output = 'benchADM.out'

#437.leslie3d
leslie3d=LiveProcess()
leslie3d.executable =  binary_dir+'437.leslie3d/exe/leslie3d_base.i386-m32-gcc42-nn'
stdin=data_dir+'437.leslie3d/data/test/input/leslie3d.in'
leslie3d.cmd = [leslie3d.executable]
leslie3d.input=stdin
leslie3d.output='leslie3d.stdout'

leslie3d1=LiveProcess()
leslie3d1.executable =  binary_dir+'437.leslie3d/exe/leslie3d_base.i386-m32-gcc42-nn'
stdin=data_dir+'437.leslie3d/data/test/input/leslie3d.in'
leslie3d1.cmd = [leslie3d.executable]
leslie3d1.input=stdin
leslie3d1.output='leslie3d1.stdout'

leslie3d2=LiveProcess()
leslie3d2.executable =  binary_dir+'437.leslie3d/exe/leslie3d_base.i386-m32-gcc42-nn'
stdin=data_dir+'437.leslie3d/data/test/input/leslie3d.in'
leslie3d2.cmd = [leslie3d.executable]
leslie3d2.input=stdin
leslie3d2.output='leslie3d2.stdout'

leslie3d3=LiveProcess()
leslie3d3.executable =  binary_dir+'437.leslie3d/exe/leslie3d_base.i386-m32-gcc42-nn'
stdin=data_dir+'437.leslie3d/data/test/input/leslie3d.in'
leslie3d3.cmd = [leslie3d.executable]
leslie3d3.input=stdin
leslie3d3.output='leslie3d.stdout'

#444.namd
namd = LiveProcess()
namd.executable =  binary_dir+'444.namd/exe/namd_base.i386-m32-gcc42-nn'
input=data_dir+'444.namd/data/all/input/namd.input'
namd.cmd = [namd.executable] + ['--input',input,'--iterations','1','--output','namd.out']
namd.output='namd.stdout'

#444.namd1
namd1 = LiveProcess()
namd1.executable =  binary_dir+'444.namd/exe/namd_base.i386-m32-gcc42-nn'
input=data_dir+'444.namd/data/all/input/namd.input'
namd1.cmd = [namd.executable] + ['--input',input,'--iterations','1','--output','namd.out']
namd1.output='namd.stdout'

#445.gobmk
gobmk=LiveProcess()
gobmk.executable =  binary_dir+'445.gobmk/exe/gobmk_base.i386-m32-gcc42-nn'
stdin=data_dir+'445.gobmk/data/test/input/capture.tst'
gobmk.cmd = [gobmk.executable]+['--quiet','--mode','gtp']
gobmk.input=stdin
gobmk.output='capture.out'

#====================
#447.dealII
dealII=LiveProcess()
dealII.executable =  binary_dir+'447.dealII/exe/dealII_base.i386-m32-gcc42-nn'
dealII.cmd = [gobmk.executable]+['8']
dealII.output='log'


#450.soplex
soplex=LiveProcess()
soplex.executable =  binary_dir+'450.soplex/exe/soplex_base.i386-m32-gcc42-nn'
data=data_dir+'450.soplex/data/ref/input/pds-50.mps'
soplex.cmd = [soplex.executable]+['-m10000',data]
soplex.output = 'test.out'

#453.povray
povray=LiveProcess()
povray.executable =  binary_dir+'453.povray/exe/povray_base.i386-m32-gcc42-nn'
data=data_dir+'453.povray/data/test/input/SPEC-benchmark-test.ini'
#povray.cmd = [povray.executable]+['SPEC-benchmark-test.ini']
povray.cmd = [povray.executable]+[data]
povray.output = 'SPEC-benchmark-test.stdout'

#454.calculix
calculix=LiveProcess()
calculix.executable =  binary_dir+'454.calculix/exe/calculix_base.i386-m32-gcc42-nn'
data=data_dir+'454.calculix/data/test/input/beampic'
calculix.cmd = [calculix.executable]+['-i',data]
calculix.output = 'beampic.log'

#456.hmmer
hmmer=LiveProcess()
hmmer.executable =  binary_dir+'456.hmmer/exe/hmmer_base.i386-m32-gcc42-nn'
data=data_dir+'456.hmmer/data/test/input/bombesin.hmm'
hmmer.cmd = [hmmer.executable]+['--fixed', '0', '--mean', '325', '--num', '5000', '--sd', '200', '--seed', '0', data]
hmmer.output = 'bombesin.out'

#458.sjeng
sjeng=LiveProcess()
sjeng.executable =  binary_dir+'458.sjeng/exe/sjeng_base.i386-m32-gcc42-nn'
data=data_dir+'458.sjeng/data/test/input/test.txt'
sjeng.cmd = [sjeng.executable]+[data]
sjeng.output = 'test.out'

#459.GemsFDTD
GemsFDTD=LiveProcess()
GemsFDTD.executable =  binary_dir+'459.GemsFDTD/exe/GemsFDTD_base.i386-m32-gcc42-nn'
GemsFDTD.cmd = [GemsFDTD.executable]
GemsFDTD.output = 'test.log'

#462.libquantum
libquantum=LiveProcess()
libquantum.executable =  binary_dir+'462.libquantum/exe/libquantum_base.i386-m32-gcc42-nn'
libquantum.cmd = [libquantum.executable],'1397','8'
libquantum.output = 'test.out'

#464.h264ref
h264ref=LiveProcess()
h264ref.executable =  binary_dir+'464.h264ref/exe/h264ref_base.i386-m32-gcc42-nn'
data=data_dir+'464.h264ref/data/test/input/foreman_test_encoder_baseline.cfg'
h264ref.cmd = [h264ref.executable]+['-d',data]
h264ref.output = 'foreman_test_encoder_baseline.out'

#470.lbm
lbm=LiveProcess()
lbm.executable =  binary_dir+'470.lbm/exe/lbm_base.i386-m32-gcc42-nn'
data=data_dir+'470.lbm/data/test/input/100_100_130_cf_a.of'
lbm.cmd = [lbm.executable]+['20', 'reference.dat', '0', '1' ,data]
lbm.output = 'lbm.out'

#471.omnetpp
omnetpp=LiveProcess()
omnetpp.executable =  binary_dir+'471.omnetpp/exe/omnetpp_base.i386-m32-gcc42-nn'
data=data_dir+'471.omnetpp/data/test/input/omnetpp.ini'
omnetpp.cmd = [omnetpp.executable]+[data]
omnetpp.output = 'omnetpp.log'

#====================
#473.astar
astar=LiveProcess()
astar.executable =  binary_dir+'473.astar/exe/astar_base.i386-m32-gcc42-nn'
data=data_dir+'473.astar/data/test/input/lake.cfg'
astar.cmd = [astar.executable]+[data]
astar.output = 'revers.out'

#====================
#481.wrf
wrf=LiveProcess()
wrf.executable =  binary_dir+'481.wrf/exe/wrf_base.i386-m32-gcc42-nn'
wrf.cmd = [wrf.executable]+['/home/xiang/workspace/gem5src/benchspec/CPU2006/481.wrf/data/test/input/namelist.input']
wrf.output = 'rsl.out.0000'

#482.sphinx
sphinx3=LiveProcess()
sphinx3.executable =  binary_dir+'482.sphinx3/exe/sphinx_livepretend_base.i386-m32-gcc42-nn'
sphinx3.cmd = [sphinx3.executable]+['/home/xiang/workspace/gem5src/benchspec/CPU2006/482.sphinx3/data/test/input/ctlfile', '.', '/home/xiang/workspace/gem5src/benchspec/CPU2006/482.sphinx3/data/test/input/args.an4']
sphinx3.output = 'an4.out'

#483.xalancbmk
xalancbmk=LiveProcess()
xalancbmk.executable =  binary_dir+'483.xalancbmk/exe/Xalan_base.i386-m32-gcc42-nn'
xalancbmk.cmd = [xalancbmk.executable]+['-v','/home/xiang/workspace/gem5src/benchspec/CPU2006/483.xalancbmk/data/test/input/t5.xml','/home/xiang/workspace/gem5src/benchspec/CPU2006/483.xalancbmk/data/test/input/xalanc.xsl']
xalancbmk.output = 'test.out'

#998.specrand
specrand_i=LiveProcess()
specrand_i.executable = binary_dir+'998.specrand/exe/specrand_base.i386-m32-gcc42-nn'
specrand_i.cmd = [specrand_i.executable] + ['1255432124','234923']
specrand_i.output = 'rand.24239.out'

#999.specrand
specrand_f=LiveProcess()
specrand_f.executable = binary_dir+'999.specrand/exe/specrand_base.i386-m32-gcc42-nn'
specrand_f.cmd = [specrand_i.executable] + ['1255432124','234923']
specrand_f.output = 'rand.24239.out'

