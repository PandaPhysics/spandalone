from base import Definition
from refbranch import RefBranch
from generic import GenericBranch
from physics import PhysicsObject

class RefVectorBranch(RefBranch, GenericBranch):
    """
    RefVector definition. Definition file syntax:
    <name>/<type>RefVector
    Vector of references are written as vector of unsigned integers to the trees.
    """

    def __init__(self, line):
        Definition.__init__(self, line, '([a-zA-Z_][a-zA-Z0-9_]*)(|\\[.+\\])/([^ ]+)RefVector(?:|/([!m]+))$')
        self.refname = self.matches.group(1)
        arrdef = self.matches.group(2)
        self.objname = self.matches.group(3)
        try:
            objdef = PhysicsObject.get(self.objname)
        except KeyError:
            raise Definition.NoMatch()

        if objdef.is_singlet():
            raise RuntimeError('Cannot create reference to single object ' + objdef.name)

        modifier = self.matches.group(4)
        if modifier is None:
            mod = ''
        else:
            mod = '/' + modifier

        # create a branch for the indices with name {name}_
        GenericBranch.__init__(self, '{name}_{arrdef}/std::vector<Short_t>{mod}'.format(name = self.refname, arrdef = arrdef, mod = mod))

    def write_decl(self, out, context):
        if context == 'datastore':
            out.writeline('ContainerBase const* {name}Container_{{0}};'.format(name = self.refname))
            GenericBranch.write_decl(self, out, context)
        else:
            if context == 'Singlet':
                out.indent -= 1
                out.writeline('protected:')
                out.indent += 1
                out.write('ContainerBase const* {name}Container_{{0}};'.format(name = self.refname))
                GenericBranch.write_decl(self, out, context)
                out.indent -= 1
                out.writeline('public:')
                out.indent += 1

            if self.is_array():
                out.writeline('RefVector<{type}> {name}{arrdef}{{}};'.format(type = self.objname, name = self.refname, arrdef = self.arrdef_text()))
            else:
                out.writeline('RefVector<{type}> {name};'.format(type = self.objname, name = self.refname))

    def write_set_address(self, out, context):
        if context == 'Element':
            # address of the indices vector of the first refvector (if this is an array of refvectors) is the address of the array<vector<unsigned>, N>
            # very ugly but should work
            subscript = ''.join('[0]' for a in self.arrdef)
            out.writeline('utils::setAddress(_tree, _name, "{name}", &{refname}{subscript}.indices(), _branches, true);'.format(name = self.name, refname = self.refname, subscript = subscript))
        else:
            GenericBranch.write_set_address(self, out, context)

    def write_book(self, out, context):
        if context == 'Element':
            # address of the indices vector of the first refvector (if this is an array of refvectors) is the address of the array<vector<unsigned>, N>
            # very ugly but should work
            subscript = ''.join('[0]' for a in self.arrdef)
            out.writeline('utils::book(_tree, _name, "{name}", "{type}", &{refname}{subscript}.indices(), _branches);'.format(name = self.name, type = self.type, refname = self.refname, subscript = subscript))
        else:
            GenericBranch.write_book(self, out, context)

    def init_default(self, lines, context):
        if self.is_array():
            return

        if context == 'Singlet':
            lines.append('{name}({name}Container_, *{name}_)'.format(name = self.refname))
        elif context == 'Element':
            lines.append('{name}(gStore.getData(this).{name}Container_, (*gStore.getData(this).{name}_)[0])'.format(name = self.refname))

    def init_standard(self, lines, context):
        if self.is_array():
            return

        if context == 'Element':
            lines.append('{name}(_data.{name}Container_, (*_data.{name}_)[_idx])'.format(name = self.refname))

    def init_copy(self, lines, context):
        if self.is_array(): 
            return

        if context == 'Singlet' or context == 'TreeEntry':
            lines.append('{name}Container_(_src.{name}Container_)')
            Branch.init_copy(lines, context)
            lines.append('{name}(*{name}_)'.format(name = self.refname))
        elif context == 'Element':
            lines.append('{name}(gStore.getData(this).{name}Container_, (*gStore.getData(this).{name}_)[0])'.format(name = self.refname))

    def write_default_ctor(self, out, context):
        if not self.is_array():
            return

        if context == 'Singlet':
            out.writeline('auto& {name}Indices(*{name}_);'.format(name = self.refname))
        else:
            out.writeline('auto& {name}Indices((*gStore.getData(this).{name}_)[0]);'.format(name = self.refname))

        self._write_ref_set(out)

    def write_standard_ctor(self, out, context):
        if not self.is_array():
            return

        out.writeline('auto& {name}Indices((*_data.{name}_)[_idx]);'.format(name = self.refname))

        self._write_ref_set(out)

    def write_copy_ctor(self, out, context):
        if self.is_array():
            if context == 'Singlet':
                out.writeline('auto& {name}Indices(*{name}_);'.format(name = self.refname))
            else:
                out.writeline('auto& {refname}Indices((*gStore.getData(this).{name}_)[0]);'.format(name = self.refname))
    
            self._write_ref_set(out)

        self.write_assign(out, context)

    def _write_ref_set(self, out, copy = False):
        subscript = ''
        array = self.refname + 'Indices'            
        for depth in range(len(self.arrdef)):
            out.writeline('for (UInt_t i{depth}(0); i{depth} != {size}; ++i{depth}) {{'.format(depth = depth, size = self.arrdef[depth]))
            subscript += '[i{depth}]'.format(depth = depth)
            newarray = 'arr{depth}'.format(depth = depth)

            out.indent += 1

            if depth == len(self.arrdef) - 1:
                out.writeline('{name}{subscript}.setIndices({array}[i{depth}]);'.format(name = self.refname, subscript = subscript, array = array, depth = depth))
            else:
                out.writeline('auto& {newarray}({array}[i{depth}]);'.format(newarray = newarray, array = array, depth = depth))

            array = newarray

        for depth in range(len(self.arrdef)):
            out.indent -= 1
            out.writeline('}')
