# Macro to check a tree in the kfactory ntuples
# This macro has two required options:
# -i is the input ktuple
# -e is the energy cut value
# Example: 
# python runkfactorizer.py -i ktuple.root -e 2.5
# 
# for more info, please run: 
# python runkfactorizer.py --help
# 
# John Hakala 10/31/2016

import os, subprocess
from optparse import OptionParser
from datetime import datetime

timestamp = '{:%Y-%m-%d_%H:%M}'.format(datetime.now())

parser = OptionParser()
parser.add_option("-x", "--loadOrCompile", dest="loadOrCompile", default="compile",
                  help="either load or compile this macro [default: compile]"                )
parser.add_option("-i", "--inputKtuple", dest="inputKtuple",
                  help="the input kTuple"                                                    )
parser.add_option("-o", "--outputFile", dest="outputFile", default="tmp%s.root" % timestamp,
                  help="the output filename [default: tmp<timestamp>.root]"                  )
parser.add_option("-q", "--quitAfter", dest="quitAfter",
                  help="quit after processing this many events"                              )
parser.add_option("-e", "--energyCutValue", dest="energyCutValue",
                  help="the minimum rechit energy cut"                                       )
parser.add_option("-b", action="store_true", dest="batch", default=False,
                  help = "turn on batch mode"                                                )
(options, args) = parser.parse_args()

from ROOT import *
if options.batch:
  gROOT.SetBatch()
 

# function to compile a C/C++ macro for loading into a pyroot session
if not options.loadOrCompile in ["load", "compile"]:
   print "please specify the -x option as either 'load' or 'compile'."   
   exit(1)
if not float(options.energyCutValue) >= 0:
   print "please specify the minimum rechit energy cut in the -e option."   
   exit(1)
if not os.path.exists(options.inputKtuple):
   print "input ktuple not found (%s)" % options.inputKtuple   
   exit(1)
else:
   print "\nInput file is %s\n" % options.inputKtuple
   print "\nAttempting to %s kfactorizer.\n" % options.loadOrCompile
   pastTense = "loaded" if options.loadOrCompile=="load" else "compiled"

def deleteLibs(macroName):
        # remove the previously compiled libraries
   if os.path.exists(macroName+"_C_ACLiC_dict_rdict.pcm"):
      os.remove(macroName+"_C_ACLiC_dict_rdict.pcm")
   if os.path.exists(macroName+"_C.d"):
      os.remove(macroName+"_C.d")
   if os.path.exists(macroName+"_C.so"):
      os.remove(macroName+"_C.so")
        # compile the macro using g++

# call the compiling function to compile the HbbGammaSelector, then run its Loop() method
if options.loadOrCompile == "compile":
   deleteLibs("kfactorizer")
   exitCode = gSystem.CompileMacro("kfactorizer.C", "gOck")
   success=(exitCode==1)
elif options.loadOrCompile == "load":
   exitCode = gSystem.Load('kfactorizer_C')
   success=(exitCode>=-1)
if not success:
   print "\nError... kfactorizer failed to %s. :-(" % options.loadOrCompile
   print "Make sure you're using an up-to-date version of ROOT by running cmsenv in a 8_0_X>22 CMSSW area."
   exit(1)
else:
   print "\nkfactorizer %s successfully."%pastTense
   if options.loadOrCompile=="compile":
      gSystem.Load('kfactorizer_C')
   file = TFile(options.inputKtuple)
   
   # get the ntuplizer/tree tree from the file specified by argument 1
   tree = file.Get("kfactorize/ktree")
   checker = kfactorizer(tree)
if options.quitAfter is not None:
  checker.Loop(options.outputFile, float(options.energyCutValue), int(options.quitAfter))
else:
  checker.Loop(options.outputFile, float(options.energyCutValue))
