#!/usr/bin/env python

import sys
import os
import shutil
from argparse import ArgumentParser
thisdir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
sys.path.append(thisdir + '/lib')
from panda.generator import Generator

argParser = ArgumentParser(description = 'Generate C++ code for a flat tree')
argParser.add_argument('configs', metavar = 'CONFIG', nargs = '*', help = 'Tree definition files.')
argParser.add_argument('--out', '-o', dest = 'outdir', default = '.', help = 'Output directory.')
argParser.add_argument('--clear-custom', '-C', dest = 'clear_custom', action = 'store_true', help = 'Clear custom code.')

args = argParser.parse_args()
del sys.argv[1:]

if len(args.configs) == 0:
    def_files = []
    for fname in os.listdir(thisdir + '/defs'):
        if fname.endswith('.def'):
            args.configs.append(thisdir + '/defs/' + fname)

generator = Generator()

# sort args.configs by requirement
for path in args.configs:
    generator.add_config(path)

generator.outdir = args.outdir
generator.preserve_custom = (not args.clear_custom)

generator.initialize()

generator.parse()

generator.write_cpp()

if os.path.exists(args.outdir + '/../../.SCRAM/Environment'):
    # running in a CMSSW environment?
    with open(args.outdir + '/../../.SCRAM/Environment') as environment:
        if 'CMSSW' in environment.readline():
            # if so, we need to write the build file
            with open('{outdir}/{namespace}/BuildFile.xml'.format(outdir = args.outdir, namespace = generator.namespace), 'w') as buildFile:
                buildFile.write('<use name="root"/>\n')
                buildFile.write('<use name="{PACKAGE}/Framework"/>\n'.format(PACKAGE = os.path.basename(args.outdir)))
                buildFile.write('<export>\n')
                buildFile.write('  <lib name="1"/>\n')
                buildFile.write('</export>\n')
            
            generator.write_linkdef()
else:
    # running in a standalone environment
    try:
        shutil.rmtree(args.outdir + '/Framework')
    except OSError:
        pass

    shutil.copytree(os.path.dirname(__file__) + '/../Framework', args.outdir + '/Framework')
    with open(os.path.dirname(__file__) + '/../Makefile') as source:
        with open(args.outdir + '/' + generator.namespace + '/Makefile', 'w') as out:
            out.write(source.read().replace('@NAMESPACE@', generator.namespace))

#relval.write_header(trees, common.PACKDIR + '/RelVal/interface/EnumerateBranches.h')
