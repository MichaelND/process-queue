import os
import sys

SHORT = False
MIXED = False
BURST = False
MCORES = False

def usage(status=0):
    print '''Usage: {} [workload]
    -s 		workloads/io-mixed-duration-short.sh
    -m 		workloads/io-mixed-duration-mixed.sh
    -b 		workloads/io-mixed-duration-mixed-burst.sh
    -mc		workloads/io-mixed-duration-medium.sh'''.format(
        os.path.basename(sys.argv[0])
    )
    sys.exit(status)

args = sys.argv[1:]
while len(args) and args[0].startswith('-') and len(args[0]) > 1:
    arg = args.pop(0)
    if arg == '-s':
    	SHORT = True
    elif arg == '-m':
    	MIXED = True
    elif arg == '-b':
    	BURST = True
    elif arg == '-mc':
    	MCORES = True
    elif arg == '-h':
        usage(0)
    else:
        usage(1)

if SHORT == True:
	os.system('workloads/io-mixed-duration-short.sh')
elif MIXED == True: 
	os.system('workloads/io-mixed-duration-mixed.sh')
elif BURST == True:
	os.system('workloads/io-mixed-duration-mixed-burst.sh')
elif MCORES == True:
	os.system('workloads/io-mixed-duration-medium.sh')
else: 
	usage(0)
