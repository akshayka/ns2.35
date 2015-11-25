import argparse
import multiprocessing
import os
import subprocess
import sys

# TODO(akshayka): Seems like these are unused
import matplotlib
if os.uname()[0] == 'Darwin':
    matplotlib.use('macosx')
import matplotlib.pyplot as p
import numpy

PROTOCOLS = [
    "TCP/Newreno",
    "TCP/Linux/cubic",
    "TCP/Linux/compound",
    "TCP/Vegas",
    "TCP/Reno/XCP",
    "TCP/Rational-0.1",
    "TCP/Rational-1",
    "TCP/Rational-10",
    "TCP/MaxThroughput",
    "TCP/MinDelay",
]

# TODO(akshayka): What purpose does this serve?
AVG_BYTE_LIST = [16000, 96000, 192000]


def runonce(conffile, full_proto_name, proto,
            workload, ontype, gateway, nsrc, simtime,
            on, off, bneck, outfname, iteration, env=None):
    gw = gateway
    if proto.find("XCP") != -1:
        sink = 'TCPSink/XCPSink'
        gw = 'XCP'              # overwrite whatever was given
    elif proto.find("Linux") != -1:
        sink = 'TCPSink/Sack1'
    else:
        sink = 'TCPSink'

    if full_proto_name.find("CoDel") != -1:
        gw = "sfqCoDel"

    if ontype == "bytes":
        runstr = ('./remy2.tcl %s -tcp %s -sink %s -gw %s'
                 ' -ontype %s -onrand %s -avgbytes %d -offrand %s -offavg %s'
                 ' -nsrc %d -simtime %d' % (conffile, proto, sink, gw,
                 ontype, workload, on, workload, off,
                 nsrc, simtime))
    elif ontype == "time":
        runstr = ('./remy2.tcl %s -tcp %s -sink %s -gw %s '
                 '-ontype %s -onrand %s -onavg %d -offrand %s -offavg %s '
                 '-nsrc %d -simtime %d' % (conffile, proto, sink, gw,
                 ontype, workload, on, workload, off,
                 nsrc, simtime))
    else:
        runstr = ('./remy2.tcl %s -tcp %s -sink %s -gw %s '
                 '-ontype %s -offrand %s -offavg %s '
                 '-nsrc %d -simtime %d' % (conffile, proto, sink, gw,
                 ontype, workload, off,
                 nsrc, simtime))

    if bneck is not None:
        runstr += ' -bneck ' + bneck

    print ('[' + proto + ']' + 'iteration #' + str(iteration) +
            ': ' + runstr + ' ...')

    fnull = open(os.devnull, "w") 
    fout = open(outfname, "ab")
    output = subprocess.call(runstr, stdout=fout, stderr=fnull, shell=True,
        env=env)
    return


def experiment(conffile, whiskerdir, proto, numsrcs, bneck, worktypes,
               ontype, onofftimes, avgbytes, simtime, iterations, resdir):
    fullname = proto
    myenv = None
    if 'Rational' in fullname:
        proto, delta = fullname.split('-')
        whisker = whiskerdir + 'delta' + delta + '.dna'
        myenv = os.environ.copy()   
        myenv['WHISKERS'] = whisker
        
    # TODO(akshayka): Why the renaming of cubic?
    if proto == 'Cubic/sfqCoDel':
        proto = 'TCP/Linux/cubic'
    for wrk in worktypes:
        for onoff in onofftimes:
            for i in xrange(iterations):
                if ontype == 'bytes':
                    outfname = ('%s/%s.%s.nconn%d.%son%d.off%0.2f.simtime%d'
                               % (resdir,
                               fullname.replace('/','-'), wrk,
                               numsrcs, ontype, avgbytes,
                               onoff, simtime))
                    runonce(conffile=conffile,
                            full_proto_name=fullname, proto=proto,
                            workload=wrk, ontype=ontype, gateway='DropTail',
                            nsrc=numsrcs, simtime=simtime,
                            on=avgbytes, off=onoff,
                            bneck=bneck, outfname=outfname, iteration=i,
                            env=myenv)
                else:
                    outfname = ('%s/%s.%s.nconn%d.%son%d.off%0.2f.simtime%d'
                               % (resdir,
                               fullname.replace('/','-'), wrk, numsrcs,
                               ontype, onoff, onoff, simtime))
                    runonce(conffile=conffile,
                            full_proto_name=fullname, proto=proto,
                            workload=wrk, ontype=ontype, gateway='DropTail',
                            nsrc=numsrcs, simtime=simtime,
                            on=onoff, off=onoff,
                            bneck=bneck, outfname=outfname, iteration=i,
                            env=myenv)
                print outfname


def main(args=None):
    parser = argparse.ArgumentParser(
        description="experiment runner",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    # Experiment parameters
    parser.add_argument("-i", "--iterations", type=int, dest="iterations",
                        default=10,
                        help="number of experiments to run for each protocol")
    parser.add_argument("-s", "--simtime", type=float, dest="simtime",
                        default=100,
                        help="number of seconds to run each trial")
    parser.add_argument("-p", "--proto", nargs="+", dest="proto",
                        default=PROTOCOLS,
                        help="space-delimited list of protocols")

    # Remy-specific parameters
    parser.add_argument("-w", "--whiskerdir", type=str, dest="whiskerdir", 
                        default=('/home/vagrant/remy-reproduce-1.0/'
                                'ns-2.35/tcp/remy/rats/new/'),
                        help="directory containing remy whiskers")

    # Network topology
    parser.add_argument("-c", "--conffile", type=str, dest="remyconf", 
                        default="remyconf/dumbbell-buf1000-rtt150-bneck15.tcl",
                        help="path to Remy config file (Tcl)")
    parser.add_argument("-n", "--nsrc", type=int, dest="nsrc",
                        help="number of sources",
                        required=True)
    parser.add_argument("-t", "--type", type=str, dest="ontype",
                        default="bytes",
                        help="by bytes or by seconds")
    parser.add_argument("-b", "--bandwidth", type=str, dest="bandwidth",
                        default=None,
                        help="bandwidth of the bottleneck link, if any")

    # Output
    parser.add_argument("-d", "--dirresults", type=str, dest="resdir", 
                        help="directory for results",
                        required=True)

    args = parser.parse_args(args)

    if not os.path.exists(args.resdir):
        os.mkdir(args.resdir)

    conffile = args.remyconf
    simtime = args.simtime
    iterations = args.iterations

    protolist = args.proto
    for p in protolist:
        if p not in PROTOCOLS:
            print "Unsupported protocol %s " % p
            print "Supported protocols: " + str(PROTOCOLS)
            return

    onofftimes = [0.5]

    # from Allman's March 2012 data and 2013 CCR paper: 100 KB
    avgbytes = 100000
    worktypes = ['Exponential']
    bneck = args.bandwidth
    numsrcs = args.nsrc
    ontype = args.ontype
    whiskerdir = args.whiskerdir
    resdir = args.resdir

    # Need to parallelize this loop
    processes = []
    for proto in protolist:
        pr = multiprocessing.Process(target=experiment, args=(conffile,
                whiskerdir, proto, numsrcs, bneck,
                worktypes, ontype, onofftimes, avgbytes,
                simtime, iterations, resdir))
        processes.append(pr)
        print 'Launching processes for protocol ' + proto
        pr.start()

    # Join processes for the sake of hygiene
    for pr in processes:
        pr.join()


if __name__ == '__main__':
    main()
