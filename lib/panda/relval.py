from branch import Branch
from physics import PhysicsObject
from output import FileOutput

# Beginning of EnumerateBranches.h
header = """#ifndef IWILLSEEYOULATER
#define IWILLSEEYOULATER 1

#include <vector>
#include <functional>
#include <algorithm>
#include <cassert>
#include "PandaTree/Objects/interface/Event.h"

namespace testpanda {

  template <int P>
  struct plotter {
    constexpr static const char* name = nullptr;
  };"""

# Each defined structure must follow this format
template = """
  template <>
  struct plotter <%i> {
    constexpr static const char* name = "%s/%s";
    std::vector<float> operator () (panda::Event& event) {
      %s
      return output;
    }
  };"""

# Ending of EnumerateBranches.h
footer = """
};

#endif"""


def parents(objdef):
    """
    Get all parents and self recursively
    """

    output = [objdef]
    if objdef.parent not in ['Element', 'Singlet', 'TreeEntry', 'EventBase']:
        output.extend(parents(PhysicsObject.get(objdef.parent)))
    return output


def plot_set(objdef):
    """
    Get the set of members of an object to plot
    """

    to_plot = []

    acceptable = ['bool', 'char', 'int', 'float', 'double',
                  'Bool_t', 'Char_t', 'UChar_t',
                  'UShort_t', 'Short_t', 'UInt_t', 'Int_t',
                  'Long64_t', 'ULong64_t', 'Float_t', 'Double_t']

    # Regular branches to plot
    to_plot += [b.name for b in objdef.branches \
                    if 'packed' not in b.name and \
                    not b.name.endswith('_') and \
                    not b.is_array() and \
                    Branch.TYPE_MAP.get(b.type, b.type) in acceptable]

    # Functions to plot
    to_plot += [f.signature.split()[0] for f in objdef.functions \
                    if '() const' in f.signature and \
                    f.type in acceptable]

    # Get inherited members to plot
    if objdef.parent not in ['Element', 'Singlet', 'TreeEntry', 'EventBase']:
        to_plot += plot_set(PhysicsObject.get(objdef.parent))

    return set(to_plot)


def write_header(trees, file_name):

    # Track the number of plots and which objects have been plotted thus far
    num_plots = 0
    parsed_objs = set()

    # Open the writing tool
    writer = FileOutput(file_name)
    writer.writeline(header)

    for tree in trees:
        # We only want to write stuff in 'events' TTree with this tool
        if tree.name in ['Event', 'EventBase']:

            # Plot first branches of single variables in panda::Event (like npv, eventNum, etc.)
            for plot in plot_set(tree):
                if plot in parsed_objs:
                    continue

                parsed_objs.add(plot)

                writer.writeline(
                    template % (num_plots,            # Which plot we're defining
                                'common',             # The output directory of the plot
                                plot.rstrip('()'),    # The name of the plot file (without extension)
                                'std::vector<float> output {float(event.%s)};' % plot))  # The vector of values that fills the histogram
                num_plots += 1

            for obj in tree.objbranches:
                if obj.name in parsed_objs:
                    continue

                parsed_objs.add(obj.name)

                objdef = PhysicsObject.get(obj.objname)
                set_to_plot = plot_set(objdef)

                # Two ways to fill the histograms whether or not this is a collection
                action_string = 'std::vector<float> output {{float(event.' + obj.name + '.{branch})}};'
                if obj.conttype == 'Collection':

                    # Throw in size of the collection
                    plot = 'size()'
                    writer.writeline(template % (num_plots, obj.name, plot.rstrip('()'), action_string.format(branch=plot)))
                    num_plots += 1

                    action_string = """std::vector<float> output;
      for (auto& i : event.{objname})
        output.push_back(i.{{branch}});""".format(objname=obj.name)

                # Print sizes of references
                for refbranch in [b for o in parents(objdef) for b in o.branches]:
                    # Check if actually a refbranch
                    if hasattr(refbranch, 'refname'):
                        name = refbranch.name.rstrip('_')
                        member = 'size' if 'std::vector' in refbranch.type else 'isValid'
                        writer.writeline(template % (num_plots, obj.name, '%s_%s' % (name, member), action_string.format(init='', branch='%s.%s()' % (name, member))))
                        num_plots += 1

                for plot in set_to_plot:
                    assertion = ''
                    sortedby = obj.modifiers.get('sortedby', '').split(',')
                    if sortedby[0] == plot.rstrip('()'):
                        compare = sortedby[1] if len(sortedby) > 1 else 'greater'
                        assertion = '\n      assert(std::is_sorted(output.begin(), output.end(), std::{compare}<float>()));'.format(compare=compare)

                    writer.writeline(template % (num_plots, obj.name, plot.rstrip('()'),
                                                 action_string.format(branch=plot) + assertion))
                    num_plots += 1

    writer.writeline('\n#define NUM_PLOTS %i' % num_plots)
    writer.writeline(footer)
