from base import Definition
from branch import Branch

class GenericBranch(Branch):
    """
    Branch with a generic object. Must be implemented as a vector of the object.
    Syntax
    <name>([size])/<type>( = <init>)
    """

    def __init__(self, line):
        Definition.__init__(self, line, '([a-zA-Z_][a-zA-Z0-9_]*)(|\\[.+\\])/([^ ]+)(?:|/([!m]+))(?:| += +(.*))$')

        self.name = self.matches.group(1)
        # is this an array branch?
        arrdef = self.matches.group(2)
        if arrdef:
            self.arrdef = arrdef.strip('[]').split('][')
        else:
            self.arrdef = []

        self.type = self.matches.group(3)
        for dim in reversed(self.arrdef):
            self.type = 'std::array<{type}, {dim}>'.format(type = self.type, dim = dim)

        self.modifier = self.matches.group(4)
        if self.modifier is None:
            self.modifier = ''

        self.init = self.matches.group(5) # used in decl
        if self.init is None:
            self.init = ''

        # initializer: used in init()
        if self.init:
            init = self.init
        else:
            init = '{type}()'.format(type = self.matches.group(3))

        if self.is_array():
            self.initializer = ''
            arr = '*{name}'.format(name = self.name)
            for iarr in range(len(self.arrdef)):
                self.initializer += 'for (auto& p{iarr} : {arr}) '.format(iarr = iarr, arr = arr)
                arr = 'p{iarr}'.format(iarr = iarr)
            self.initializer += 'p{iarr} = {init};'.format(iarr = iarr, init = init)
        else:
            self.initializer = '*{name} = {init};'.format(name = self.name, init = init)

    def typename(self):
        return self.type

    def write_decl(self, out, context):
        if context == 'datastore':
            template = 'std::vector<{type}>* {name}{{0}};'
        elif context == 'Singlet' or context == 'TreeEntry':
            template = '{type}* {name}{{0}};'
        elif context == 'Element':
            template = '{type}* {name};'.format(type = self.type, name = self.name)

        line = template.format(type = self.type, name = self.name, arrdef = self.arrdef_text())

        if '!' in self.modifier:
            line += ' // transient'

        out.writeline(line)

    def write_allocate(self, out, context):
        # context must be datastore
        out.writeline('{name} = new std::vector<{type}>(nmax_);'.format(name = self.name, type = self.type))

    def write_deallocate(self, out, context):
        # context must be datastore
        out.writeline('delete {name};'.format(name = self.name))
        out.writeline('{name} = 0;'.format(name = self.name))

    def write_set_address(self, out, context):
        if '!' in self.modifier:
            return

        if context == 'datastore':
            namevar = '_name'
        elif context == 'Singlet':
            namevar = 'name_'
        elif context == 'Element':
            namevar = 'name'
        elif context == 'TreeEntry':
            namevar = '""'

        out.writeline('utils::setAddress(_tree, {namevar}, "{name}", &{name}, _branches, _setStatus);'.format(namevar = namevar, name = self.name))

    def write_book(self, out, context):
        if '!' in self.modifier:
            return

        if context == 'datastore':
            namevar = '_name'
        elif context == 'Singlet':
            namevar = 'name_'
        elif context == 'Element':
            namevar = '_name'
        elif context == 'TreeEntry':
            namevar = '""'

        if context == 'datastore':
            type_name = 'std::vector<{type}>'.format(type = self.type)
        else:
            type_name = self.type

        out.writeline('utils::book(_tree, {namevar}, "{name}", "{type}", &{name}, _branches);'.format(namevar = namevar, name = self.name, type = type_name))

    def write_resize_vectors(self, out, context):
        out.writeline('{name}->resize(_size);'.format(name = self.name))

    def init_default(self, lines, context):
        if context == 'Element':
            lines.append('{name}(&(*gStore.getData(this).{name})[0])'.format(name = self.name))
        else:
            lines.append('{name}(new {type}({init}))'.format(name = self.name, type = self.type, init = self.init))

    def init_standard(self, lines, context):
        if context == 'Element':
            lines.append('{name}(&(*_data.{name})[_idx])'.format(name = self.name))

    def init_copy(self, lines, context):
        if context == 'Singlet' or context == 'TreeEntry':
            lines.append('{name}(new {type}{{*_src.{name}}})'.format(name = self.name, type = self.type))
        elif context == 'Element':
            lines.append('{name}(&(*gStore.getData(this).{name})[0])'.format(name = self.name))

    def write_assign(self, out, context):
        out.writeline('*{name} = *_src.{name};'.format(name = self.name))

    def write_init(self, out, context):
        out.writeline(self.initializer)

    def write_dump(self, out):
        out.writeline('_out << "{name} = ({type} object)" << std::endl;'.format(name = self.name, type = self.type))
