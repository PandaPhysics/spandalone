from base import Definition
from branch import Branch
from physics import PhysicsObject

class RefBranch(Branch):
    """
    Reference definition. Definition file syntax:
    <name>/<type>Ref
    References are written as unsigned integers to the trees.
    """

    def __init__(self, line):
        Definition.__init__(self, line, '([a-zA-Z_][a-zA-Z0-9_]*)(|\\[.+\\])/([^ ]+)Ref(?:|/([!m]+))$')
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

        # create a branch for the index with name {name}_
        Branch.__init__(self, '{name}_{arrdef}/S{mod} = -1'.format(name = self.refname, arrdef = arrdef, mod = mod))

    def write_decl(self, out, context):
        if context == 'datastore':
            out.writeline('ContainerBase const* {name}Container_{{0}};'.format(name = self.refname))
            Branch.write_decl(self, out, context)
        else:
            if context == 'Singlet':
                out.indent -= 1
                out.writeline('protected:')
                out.indent += 1
                out.writeline('ContainerBase const* {name}Container_{{0}};'.format(name = self.refname))
                Branch.write_decl(self, out, context)
                out.indent -= 1
                out.writeline('public:')
                out.indent += 1

            if self.is_array():
                out.writeline('Ref<{type}> {name}{arrdef}{{}};'.format(type = self.objname, name = self.refname, arrdef = self.arrdef_text()))
            else:
                out.writeline('Ref<{type}> {name};'.format(type = self.objname, name = self.refname))

    def write_set_address(self, out, context):
        if '!' in self.modifier:
            return

        if context == 'Element':
            out.writeline('utils::setAddress(_tree, _name, "{name}", gStore.getData(this).{name}, _branches, true);'.format(name = self.name))
        else:
            Branch.write_set_address(self, out, context)

    def write_book(self, out, context):
        if '!' in self.modifier:
            return

        if context == 'Element':
            out.writeline('utils::book(_tree, _name, "{name}", "{arrdef}", \'{type}\', gStore.getData(this).{name}, _branches);'.format(name = self.name, arrdef = self.arrdef_text(), type = self.type, refname = self.refname))
        else:
            Branch.write_book(self, out, context)

    def init_default(self, lines, context):
        if self.is_array():
            return

        if context == 'Singlet':
            lines.append('{name}({name}Container_, {name}_)'.format(name = self.refname))
        elif context == 'Element':
            lines.append('{name}(gStore.getData(this).{name}Container_, gStore.getData(this).{name}_[0])'.format(name = self.refname))

    def init_standard(self, lines, context):
        if self.is_array():
            return

        if context == 'Element':
            lines.append('{name}(_data.{name}Container_, _data.{name}_[_idx])'.format(name = self.refname))

    def init_copy(self, lines, context):
        if self.is_array(): 
            return

        if context == 'Singlet' or context == 'TreeEntry':
            lines.append('{name}Container_(_src.{name}Container_)')
            Branch.init_copy(lines, context)
            lines.append('{name}({name}Container_, {name}_)'.format(name = self.refname))
        elif context == 'Element':
            lines.append('{name}(gStore.getData(this).{name}Container_, gStore.getData(this).{name}_[0])'.format(name = self.refname))

    def write_default_ctor(self, out, context):
        if not self.is_array():
            return

        if context == 'Singlet':
            out.writeline('auto& {name}Indices({name}_);'.format(name = self.refname))
        else:
            out.writeline('auto& {name}Indices(gStore.getData(this).{name}_[0]);'.format(name = self.refname))

        self._write_ref_set(out)

    def write_standard_ctor(self, out, context):
        if not self.is_array():
            return

        out.writeline('auto& {name}Indices(_data.{name}_[_idx]);'.format(name = self.refname))

        self._write_ref_set(out)

    def write_copy_ctor(self, out, context):
        if self.is_array():
            if context == 'Singlet':
                out.writeline('auto& {name}Indices({name}_);'.format(name = self.refname))
            else:
                out.writeline('auto& {refname}Indices(gStore.getData(this).{name}_[0]);'.format(name = self.refname))
    
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
                out.writeline('{name}{subscript}.setIndex({array}[i{depth}]);'.format(name = self.refname, subscript = subscript, array = array, depth = depth))
            else:
                out.writeline('auto& {newarray}({array}[i{depth}]);'.format(newarray = newarray, array = array, depth = depth))

            array = newarray

        for depth in range(len(self.arrdef)):
            out.indent -= 1
            out.writeline('}')

    def write_assign(self, out, context):
        subscript = ''
        for depth in range(len(self.arrdef)):
            out.writeline('for (UInt_t i{depth}(0); i{depth} != {n}; ++i{depth}) {{'.format(depth = depth, n = self.arrdef[depth]))
            out.indent += 1
            subscript += '[i{depth}]'.format(depth = depth)

        out.writeline('{name}{subscript} = _src.{name}{subscript};'.format(name = self.refname, subscript = subscript))

        for depth in range(len(self.arrdef)):
            out.indent -= 1
            out.writeline('}')

    def write_init(self, out, context):
        subscript = ''
        for depth in range(len(self.arrdef)):
            out.writeline('for (UInt_t i{depth}(0); i{depth} != {n}; ++i{depth}) {{'.format(depth = depth, n = self.arrdef[depth]))
            out.indent += 1
            subscript += '[i{depth}]'.format(depth = depth)

        out.writeline('{name}{subscript}.init();'.format(name = self.refname, subscript = subscript))

        for depth in range(len(self.arrdef)):
            out.indent -= 1
            out.writeline('}')

    def write_dump(self, out):
        out.writeline('_out << "{name} = " << {name} << std::endl;'.format(name = self.refname))
