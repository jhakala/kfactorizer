info = """
Macro to check a tree in the kfactory ntuples.
This macro has two required options:
-i is the input ktuple, and
-e is the energy cut value.
"""

example = """
  Example:
   python runkfactorizer.py -i ktuple.root -e 2.5
"""
# 
# for more info, please run: 
# python runkfactorizer.py --help
# 
# John Hakala 10/31/2016

from os import path, remove
from optparse import OptionParser
from datetime import datetime

timestamp = '{:%Y-%m-%d_%H:%M}'.format(datetime.now())

parser = OptionParser(description = info, epilog = example)
parser.add_option("-x", "--loadOrCompile", dest="loadOrCompile", default="compile",
                  type="string", help="either load or compile this macro [default: compile]" )
parser.add_option("-i", "--inputKtuple", dest="inputKtuple",
                  type="string", help="the input kTuple"                                     )
parser.add_option("-o", "--outputFile", dest="outputFile", default="tmp%s.root" % timestamp,
                  type="string", help="the output filename [default: tmp<timestamp>.root]"   )
parser.add_option("-q", "--quitAfter", dest="quitAfter",
                  type="int",    help="quit after processing this many events"               )
parser.add_option("-e", "--energyCutValue", dest="energyCutValue",
                  type="float",  help="the minimum rechit energy cut"                        )
parser.add_option("-b", action="store_true", dest="batch", default=False,
                                 help = "turn on batch mode"                                 )
(options, args) = parser.parse_args()

if options.inputKtuple is None or options.energyCutValue is None:
  parser.print_help()
  exit(1) 

from ROOT import *
if options.batch:
  gROOT.SetBatch()
 
ktreeName = "kfactorize/ktree"

# function to compile a C/C++ macro for loading into a pyroot session
if not options.loadOrCompile in ["load", "compile"]:
   print "please specify the -x option as either 'load' or 'compile'."   
   exit(1)
if not float(options.energyCutValue) >= 0:
   print "please specify the minimum rechit energy cut in the -e option."   
   exit(1)
if not path.exists(options.inputKtuple):
   print "input ktuple not found (%s)" % options.inputKtuple   
   exit(1)
else:
   print "\nInput file is %s\n" % options.inputKtuple
   print "\nAttempting to %s kfactorizer.\n" % options.loadOrCompile
   pastTense = "loaded" if options.loadOrCompile=="load" else "compiled"

def deleteLibs(macroName):
   # removes the previously compiled libraries
   if path.exists(macroName+"_C_ACLiC_dict_rdict.pcm"):
      remove(macroName+"_C_ACLiC_dict_rdict.pcm")
   if path.exists(macroName+"_C.d"):
      remove(macroName+"_C.d")
   if path.exists(macroName+"_C.so"):
      remove(macroName+"_C.so")

# either get the the kfactorizer from a library or compile from source, then run its Loop() method
if options.loadOrCompile == "compile":
   # get rid of old libraries
   deleteLibs("kfactorizer")
   # compile the macro fresh using g++
   exitCode = gSystem.CompileMacro("kfactorizer.C", "gOck")
   success=(exitCode==1)
elif options.loadOrCompile == "load":
   # load from old libraries
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
   inFile = TFile(options.inputKtuple)
   
   # get the ktree from the file specified by argument 1
   print "kfactorizer will now be used to analyze tree '%s' in file '%s'" % (ktreeName, inFile.GetName())
   ktree = inFile.Get(ktreeName)
   kinstance = kfactorizer(ktree)
if options.quitAfter is not None:
  kinstance.Loop(options.outputFile, options.energyCutValue, options.quitAfter)
else:
  kinstance.Loop(options.outputFile, options.energyCutValue)
